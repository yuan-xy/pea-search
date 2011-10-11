#include "env.h"
#include <winioctl.h>
#include <assert.h>
#include <stdio.h>
#include "ntfs.h"
#include "util.h"
#include "global.h"
#include "suffix.h"

BOOL OpenNtfsHandle(int i){
	WCHAR szVolumePath[8];
	HANDLE hVolume;
	swprintf(szVolumePath,8,L"\\\\.\\%c:",i+'A');
	hVolume=CreateFileW(szVolumePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE!=hVolume){
		g_hVols[i]=hVolume;
		return 1;
	}else{
		WIN_ERROR;
		return 0;
	}

}

BOOL CloseNtfsHandle(int i){
	return CloseHandle(g_hVols[i]);
}


void InitVolumeData(int i){
	DWORD dwWritten;
	NTFS_VOLUME_DATA_BUFFER ntfsVolData;
	DeviceIoControl(g_hVols[i],FSCTL_GET_NTFS_VOLUME_DATA,
		NULL, 0, &ntfsVolData, sizeof(ntfsVolData), &dwWritten, NULL);
	//printf("%d:%d,%d.%s\n",dwWritten,ntfsVolData.BytesPerCluster,ntfsVolData.BytesPerFileRecordSegment,g_VolsInfo[i].volumeName);
	g_BytesPerCluster[i]=ntfsVolData.BytesPerCluster;
	g_FileRecSize[i]=sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER)-1+ntfsVolData.BytesPerFileRecordSegment;//???
	g_pOutBuffer[i]=(PBYTE)malloc_safe(g_FileRecSize[i]);
}

void initUSN(int i){
	USN_JOURNAL_DATA ujd;
	HANDLE hVolume = g_hVols[i];
	if(!QueryUsnJournal(hVolume,&ujd)){
		switch(GetLastError())
		{
		case ERROR_JOURNAL_NOT_ACTIVE:
			{
				CreateUsnJournal(hVolume);
				QueryUsnJournal(hVolume,&ujd);
			}
			break;
		case ERROR_JOURNAL_DELETE_IN_PROGRESS:
			{
				DWORD cb;
				DELETE_USN_JOURNAL_DATA del_ujd;
				del_ujd.UsnJournalID = ujd.UsnJournalID;
				del_ujd.DeleteFlags = USN_DELETE_FLAG_NOTIFY;
				if(!DeviceIoControl(hVolume, FSCTL_DELETE_USN_JOURNAL,
					&del_ujd, sizeof(DELETE_USN_JOURNAL_DATA),
					NULL, 0, &cb, NULL
					)) {
				}
				CreateUsnJournal(hVolume);
				QueryUsnJournal(hVolume,&ujd);
			}
			break;
		default:
			break;
		}
	}
	g_curJournalID[i]=ujd.UsnJournalID;
	g_curFirstUSN[i]=ujd.FirstUsn;
	g_curNextUSN[i]=ujd.NextUsn;
}

INLINE pFileEntry initFileEntry(PUSN_RECORD r,int i){
	int str_len = (int) r->FileNameLength/sizeof(WCHAR);
	int len = wchar_to_utf8_len(r->FileName,str_len);
	NEW0_FILE(ret,len);
	ret->FileReferenceNumber = (KEY)r->FileReferenceNumber;
	ret->up.ParentFileReferenceNumber = (KEY)r->ParentFileReferenceNumber;
	ret->us.v.FileNameLength = len;
	ret->us.v.StrLen = str_len;
	wchar_to_utf8_nocheck(r->FileName,r->FileNameLength/sizeof(WCHAR),ret->FileName,len);
	if(is_dir(r)){
		ret->ut.v.suffixType = SF_DIR;
		ret->us.v.dir = 1;
	}
	if(is_readonly(r)) ret->us.v.readonly = 1;
	if(is_hidden(r)) ret->us.v.hidden = 1;
	if(is_system(r)) ret->us.v.system = 1;
	add2Map(ret,i);
	ALL_FILE_COUNT +=1;
	return ret;
}


DWORD ScanMFT(int i){
	BYTE RecvBuffer[sizeof(DWORDLONG) + 0x80000];
	HANDLE hVolume=g_hVols[i];
	DWORD cbRet,count=0;
	PUSN_RECORD pRecord,pEnd;
	MFT_ENUM_DATA med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = 0;
	med.HighUsn = g_curNextUSN[i];
	while (DeviceIoControl(hVolume, FSCTL_ENUM_USN_DATA,
		&med, sizeof(med),
		RecvBuffer, sizeof(RecvBuffer), &cbRet,
		NULL)
		)
	{
		for(pRecord = ((PUSN_RECORD) (&RecvBuffer[sizeof(USN)])),pEnd=((PUSN_RECORD) (RecvBuffer + cbRet));
			pRecord<pEnd;
			pRecord = ((PUSN_RECORD) ((PBYTE)pRecord + pRecord->RecordLength))
			){
			initFileEntry(pRecord, i);
			count++;
		}
		med.StartFileReferenceNumber=*(DWORDLONG*)RecvBuffer;
	}
	printf("indexed %d\n",count);
	return count;
}

INLINE BOOL CreateUsnJournal(HANDLE hVolume)
{
    DWORD cb;
    CREATE_USN_JOURNAL_DATA cujd;
    cujd.MaximumSize = 32768*1024; //32M
    cujd.AllocationDelta = 4096*1024; //4M
    return DeviceIoControl(hVolume, FSCTL_CREATE_USN_JOURNAL,
        &cujd, sizeof(cujd), NULL, 0, &cb, NULL);
}

INLINE BOOL QueryUsnJournal(HANDLE hVolume,PUSN_JOURNAL_DATA pUsnJournalData)
{
    DWORD cb;
    return DeviceIoControl(hVolume
        ,FSCTL_QUERY_USN_JOURNAL
        ,NULL,0,
        pUsnJournalData,sizeof(USN_JOURNAL_DATA), &cb
        ,NULL);
}

static void genFileEntryOne(PUSN_RECORD r,int i){
    	pFileEntry file = initFileEntry(r,i);
    	if(attachParent(file,i)){
    		SuffixProcess(file,NULL);
    		init_size_time(file,&i);
    	}
}

#define FIND_FILE_0(r,i)	findFile((KEY)(r)->FileReferenceNumber,(KEY)(r)->ParentFileReferenceNumber,i);

static DWORDLONG old_frn,old_pfrn;
int old_i;

void updateFileEntry(PUSN_RECORD r,int i){
	DWORD dwReason = r->Reason;
	//printf("%x: %ls\n",r->Reason,r->FileName);
    if((USN_REASON_FILE_CREATE&dwReason)&&(USN_REASON_CLOSE&dwReason)){//增
		genFileEntryOne(r,i);
	}else if(USN_REASON_RENAME_OLD_NAME&dwReason){
		old_frn = r->FileReferenceNumber;		
		old_pfrn = r->ParentFileReferenceNumber;
		old_i = i;
	}else if((USN_REASON_RENAME_NEW_NAME&dwReason) && (dwReason&USN_REASON_CLOSE)){//重命名
		if(old_pfrn==r->ParentFileReferenceNumber && i==old_i){
			pFileEntry pmodify = findFile(old_frn,old_pfrn,i);
			renameFile(pmodify ,r->FileName,r->FileNameLength);
		}else{
			pFileEntry pnew = findDir(r->ParentFileReferenceNumber,i);
			pFileEntry file = findFile(old_frn,old_pfrn,old_i);
			#ifdef MY_DEBUG
				assert(file->FileReferenceNumber==(KEY)r->FileReferenceNumber);
			#endif
			moveFile(file,pnew);
		}
    }else if((dwReason&USN_REASON_FILE_DELETE)&&(USN_REASON_CLOSE&dwReason)){//删
        	pFileEntry file = FIND_FILE_0(r,i);
			if(file!=NULL){
				deleteFile(file);
			}else{
				FERROR(file);
			}
    }
}




