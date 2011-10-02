extern "C" {
#include "env.h"
#include <windows.h>
#include <dbt.h>
#include <stdio.h>
#include "global.h"
#include "util.h"
#include "search.h"
#include "ntfs.h"
#include "write.h"

__forceinline ULONG RunLength(PUCHAR run)
{
    return(*run & 0x0f) + ((*run >> 4) & 0x0f) + 1;
}


//返回LCN-d-gap  注意d-gap可为负
__forceinline LONGLONG RunLCN(PUCHAR run)
{
    UCHAR n1 = *run & 0x0f;
    UCHAR n2 = (*run >> 4) & 0x0f;
    if(0==n2) return 0;
    LONGLONG lcn=(CHAR)(run[n1 + n2]);//带符号转换
    LONG i = 0;
    for (i = n1 +n2 - 1; i > n1; --i)
        lcn = (lcn << 8) + run[i];
    return lcn;
}

__forceinline ULONGLONG RunCount(PUCHAR run)
{
    UCHAR n =  *run & 0xf;
    ULONGLONG count = 0;
    ULONG i = 0;
    for (i = n; i > 0; i--)
        count = (count << 8) + run[i];
    return count;
}

BOOL init_size_time(pFileEntry file, void *data){
	__try{
		if(IsRoot(file->FileReferenceNumber)) return 1;
		//int i = getDrive(file);
		int i = *(int *)data;
		DWORD FileNameLength = file->us.v.FileNameLength;
		HANDLE hVol=g_hVols[i];
		DWORD dwBytesPerCluster=g_BytesPerCluster[i];
		DWORD dwFileRecSize=g_FileRecSize[i];

		ULONGLONG outSize,*pOutSize=&outSize;

		PNTFS_FILE_RECORD_OUTPUT_BUFFER pMftRecord=(PNTFS_FILE_RECORD_OUTPUT_BUFFER)g_pOutBuffer[i];
		PFILE_RECORD_HEADER pfileRecordheader=(PFILE_RECORD_HEADER)pMftRecord->FileRecordBuffer;
		PATTRIBUTE pAttribute=NULL;
		PFILENAME_ATTRIBUTE pFileNameAttr;

		NTFS_FILE_RECORD_INPUT_BUFFER mftRecordInput;
		mftRecordInput.FileReferenceNumber.QuadPart=file->FileReferenceNumber;

		DWORD dwRet;

		BOOL flag = DeviceIoControl(hVol,FSCTL_GET_NTFS_FILE_RECORD
			,&mftRecordInput,sizeof(mftRecordInput)
			,pMftRecord,dwFileRecSize,&dwRet,NULL);

		if(!flag){
			WIN_ERROR
			return 0;
		}

		for(pAttribute = (PATTRIBUTE)((PBYTE)pfileRecordheader +pfileRecordheader->AttributeOffset)
			;pAttribute->AttributeType!=AttributeFileName
			;pAttribute=(PATTRIBUTE)((PBYTE)pAttribute +pAttribute->Length)
			){
				if(pAttribute==NULL || pAttribute->Length==0 ||pAttribute->AttributeType==AttributeEnd ) return 0;
		}
		pFileNameAttr=PFILENAME_ATTRIBUTE(
			(PBYTE)pAttribute+PRESIDENT_ATTRIBUTE(pAttribute)->ValueOffset
			);

		set_time(file,(PFILETIME)&pFileNameAttr->ChangeTime);
		if(IsDir(file)){
			SET_SIZE(file,0);
			return 1;
		}

		//以下获取文件大小信息
		if(pFileNameAttr->DataSize!=0){
			*pOutSize=pFileNameAttr->DataSize;
			goto done;
		}
	
		//30属性大小为0时可能不正确
		//读80属性获得文件大小
		for(pAttribute=(PATTRIBUTE)((PBYTE)pAttribute +pAttribute->Length)
			;pAttribute->AttributeType!=AttributeEnd && pAttribute->AttributeType<AttributeData
			;pAttribute=(PATTRIBUTE)((PBYTE)pAttribute +pAttribute->Length)
			);
		if(pAttribute->AttributeType==AttributeEnd || pAttribute->AttributeType>AttributeData){
			for(pAttribute = (PATTRIBUTE)((PBYTE)pfileRecordheader +pfileRecordheader->AttributeOffset)
				;pAttribute->AttributeType<AttributeAttributeList
				;pAttribute=(PATTRIBUTE)((PBYTE)pAttribute +pAttribute->Length)
				);
			if(pAttribute->AttributeType>AttributeAttributeList){
				return 0; //my_assert(0&&"没有找到80属性也没有20属性", 0);
			}else{
				PATTRIBUTE_LIST pAttriList;
				if(pAttribute->Nonresident){
					PNONRESIDENT_ATTRIBUTE pNonResident=PNONRESIDENT_ATTRIBUTE(pAttribute);
					PBYTE pRun=(PBYTE)pAttribute+pNonResident->RunArrayOffset;
					ULONGLONG Lcn = RunLCN( pRun );//只获取第1簇
					ULONGLONG nCount = RunCount( pRun );
					my_assert(nCount<=pNonResident->HighVcn-pNonResident->LowVcn+1, 0);
					LARGE_INTEGER file_offset;
					file_offset.QuadPart=Lcn*dwBytesPerCluster;
					SetFilePointerEx(hVol,file_offset, NULL, FILE_BEGIN );
					PBYTE   pBuffferRead = (PBYTE)malloc_safe(dwBytesPerCluster);///第一簇
					DWORD   dwRead = 0;
					ReadFile(hVol,pBuffferRead,dwBytesPerCluster, &dwRead, NULL );
					PBYTE   pBufferEnd=pBuffferRead+dwRead;
					for(pAttriList=PATTRIBUTE_LIST(pBuffferRead);
						pAttriList->AttributeType!=AttributeData;
						pAttriList=PATTRIBUTE_LIST(PBYTE(pAttriList)+pAttriList->Length)
						);
					my_assert(pAttriList->AttributeType==AttributeData && "难道不等?", 0);
					if(pAttriList->AttributeType==AttributeData){
						mftRecordInput.FileReferenceNumber.QuadPart=0xffffffffffff&pAttriList->FileReferenceNumber;
						DeviceIoControl(hVol,FSCTL_GET_NTFS_FILE_RECORD
							,&mftRecordInput,sizeof(mftRecordInput)
							,pMftRecord,dwFileRecSize,&dwRet,NULL);
						pAttribute = (PATTRIBUTE)((PBYTE)pfileRecordheader +pfileRecordheader->AttributeOffset);
						my_assert(AttributeData==pAttribute->AttributeType && "一定是80属性", 0);
						if(pAttribute->Nonresident){
							*pOutSize=PNONRESIDENT_ATTRIBUTE(pAttribute)->DataSize;
						}else{
							*pOutSize=PRESIDENT_ATTRIBUTE(pAttribute)->ValueLength;
						}
					}
					free_safe(pBuffferRead);
				}else{
					for(pAttriList=PATTRIBUTE_LIST((PBYTE)pAttribute+PRESIDENT_ATTRIBUTE(pAttribute)->ValueOffset);
						pAttriList->AttributeType!=AttributeEnd && pAttriList->AttributeType<AttributeData;
						pAttriList=PATTRIBUTE_LIST(PBYTE(pAttriList)+pAttriList->Length)
						);
					if(pAttriList->AttributeType==AttributeData){
						mftRecordInput.FileReferenceNumber.QuadPart=0xffffffffffff&pAttriList->FileReferenceNumber;
						DeviceIoControl(hVol,FSCTL_GET_NTFS_FILE_RECORD
							,&mftRecordInput,sizeof(mftRecordInput)
							,pMftRecord,dwFileRecSize,&dwRet,NULL);
						pAttribute = (PATTRIBUTE)((PBYTE)pfileRecordheader +pfileRecordheader->AttributeOffset);
						my_assert(AttributeData==pAttribute->AttributeType && "一定是80属性", 0);
						if(pAttribute->Nonresident){
							*pOutSize=PNONRESIDENT_ATTRIBUTE(pAttribute)->DataSize;
						}else{
							*pOutSize=PRESIDENT_ATTRIBUTE(pAttribute)->ValueLength;
						}
					}else{
						my_assert(0&&"属性列表中没有80属性", 0);
					}
				}
			}
		}else{
			if(pAttribute->Nonresident){
				*pOutSize=PNONRESIDENT_ATTRIBUTE(pAttribute)->DataSize;
			}else{
				*pOutSize=PRESIDENT_ATTRIBUTE(pAttribute)->ValueLength;
			}
		}
	done:
		SET_SIZE(file,file_size_shorten(outSize));
		my_assert( FileNameLength == file->us.v.FileNameLength, 0);
		return 1;
	} __finally{
		;
	}
	return 0;
}

DWORD WINAPI init_size_time_all(pFileEntry root){
	//pFileEntry root = (pFileEntry)param;
	int i = getDrive(root);
	my_assert(IsNtfs(i),1);
	FilesIterate(root,(void (*)(pFileEntry , void *))init_size_time,&i);
	printf("init size/time finish for drive %c .\n", i+'A');
	save_db(i);
	return 0;
}

DWORD WINAPI MonitorProc(PVOID pParam)
{
    const DWORD SEARCH_TITLE_REASON_FLAG=
        //支持文件名更改
        USN_REASON_FILE_CREATE
        |USN_REASON_FILE_DELETE
        |USN_REASON_RENAME_OLD_NAME
        |USN_REASON_RENAME_NEW_NAME

        //支持文件属性更改 //无最近访问时间支持
//         |USN_REASON_DATA_EXTEND
//         |USN_REASON_DATA_OVERWRITE
//         |USN_REASON_DATA_TRUNCATION
//         |USN_REASON_BASIC_INFO_CHANGE   //文件属性改变
//         |USN_REASON_OBJECT_ID_CHANGE    //LAST ACCESS
        ;
	pFileEntry root = (pFileEntry)pParam;
    DWORD dwDri=getDrive(root);
    HANDLE hVol=g_hVols[dwDri];
    READ_USN_JOURNAL_DATA rujd;
    rujd.BytesToWaitFor=0;//此处由1改成0 0时等待记录卷访问阻塞
    rujd.ReasonMask=SEARCH_TITLE_REASON_FLAG;
    rujd.ReturnOnlyOnClose=0;
    rujd.StartUsn=g_curNextUSN[dwDri];
    rujd.Timeout=0;
    rujd.UsnJournalID=g_curJournalID[dwDri];

    DWORD dwBytes;
    DWORD dwRetBytes;
    BYTE Buffer[USN_PAGE_SIZE];
    PUSN_RECORD pRecord;

    for(;;)
    {
        Sleep(100);
        if(!DeviceIoControl(hVol,
            FSCTL_READ_USN_JOURNAL,
            &rujd,
            sizeof(rujd),
            Buffer,
            USN_PAGE_SIZE,
            &dwBytes,
            NULL) )
        {
            DWORD dwError=GetLastError();
            switch(dwError)
            {
            case ERROR_INVALID_FUNCTION:
                break;
            case ERROR_INVALID_PARAMETER:
                break;
            case ERROR_JOURNAL_DELETE_IN_PROGRESS:
                DWORD cb;
                DELETE_USN_JOURNAL_DATA del_ujd;
                del_ujd.UsnJournalID = rujd.UsnJournalID;
                del_ujd.DeleteFlags = USN_DELETE_FLAG_NOTIFY;
                if(!DeviceIoControl(hVol, FSCTL_DELETE_USN_JOURNAL,
                    &del_ujd, sizeof(DELETE_USN_JOURNAL_DATA),
                    NULL, 0, &cb, NULL
                    )) {
                }
                CreateUsnJournal(hVol);
                USN_JOURNAL_DATA ujd;
                QueryUsnJournal(hVol,&ujd);
                g_curJournalID[dwDri]=ujd.UsnJournalID;
                g_curNextUSN[dwDri]=ujd.NextUsn;
                break;
            case ERROR_JOURNAL_NOT_ACTIVE:
                break;
            case ERROR_JOURNAL_ENTRY_DELETED:
                break;
            default:
                break;
            }
            //DebugBreak();
        }
        if(dwBytes<=sizeof(USN)) {continue;}//结束!

        dwRetBytes = dwBytes - sizeof(USN);//跳过了1个USN，此USN是下一论查询起点
        //即本轮没有收录
        //改进 ：将所有的监视记录先整理下
        pRecord = PUSN_RECORD((PBYTE)Buffer+sizeof(USN));
        while(dwRetBytes > 0 )//若跳过1个USN后，还有多余字节，则至少有1个记录
        {
            updateFileEntry(pRecord, dwDri);
            dwRetBytes -= pRecord->RecordLength;
            //找下一个记录
            pRecord = (PUSN_RECORD)(((PBYTE)pRecord) + pRecord->RecordLength);
        }
        //更新起始USN
        rujd.StartUsn = g_curNextUSN[dwDri]=*(USN*)Buffer;
    }
}

BOOL StartMonitorThreadNTFS(int i){
	if(g_hThread[i]!=NULL) return 0;
	g_hThread[i]=CreateThread(NULL,0,MonitorProc,g_rootVols[i],0,0);
	if(g_hThread[i]==NULL) return 0;
	return 1;
}

}// extern "C"
