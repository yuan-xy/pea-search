#include <stdio.h>
#include "drive.h"
#include "global.h"

static BOOL IsUsbDriver(int i){
	HANDLE hDevice;
    WCHAR tcsDrvName[] = L"\\\\.\\X:";
    tcsDrvName[4] = i+L'A';
    hDevice = CreateFile( tcsDrvName,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL
                                );
    if ( hDevice == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }else{
		DWORD dwOutLen;
		BOOL res;
	    BYTE buff[1024] = {0};
	    PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
		STORAGE_PROPERTY_QUERY StoragePropertyQuery;
	    StoragePropertyQuery.PropertyId = StorageDeviceProperty;
	    StoragePropertyQuery.QueryType = PropertyStandardQuery;

	    pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)buff;
	    pDevDesc->Size = sizeof(buff);
	    
	    res = DeviceIoControl( hDevice,
	                                IOCTL_STORAGE_QUERY_PROPERTY,
	                                &StoragePropertyQuery,
	                                sizeof(STORAGE_PROPERTY_QUERY),
	                                pDevDesc,
	                                pDevDesc->Size,
	                                &dwOutLen,
	                                NULL
	                               );
	    CloseHandle(hDevice);

	    if (res) {
	        return pDevDesc->BusType == BusTypeUsb;
	    } else {
	        return FALSE;
	    }
	}


    
}


void get_drive_space(int i){
	ULARGE_INTEGER TotalNumberOfBytes, TotalNumberOfFreeBytes;
	WCHAR name[4];
	swprintf(name,4,L"%c:\\",i+'A');
	if(g_VolsInfo[i].serialNumber==0) return;
	GetDiskFreeSpaceEx(name,NULL,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
	g_VolsInfo[i].totalMB = (TotalNumberOfBytes.HighPart)*4000 + ((TotalNumberOfBytes.LowPart)>>20);
	g_VolsInfo[i].totalFreeMB = (TotalNumberOfFreeBytes.HighPart)*4000 + ((TotalNumberOfFreeBytes.LowPart)>>20);
}

void InitDrive(int i){
	WCHAR name[4];
	g_bVols[i]=1;
	swprintf(name,4,L"%c:\\",i+'A');
	g_VolsInfo[i].type = GetDriveType(name);
	if(g_VolsInfo[i].type==DRIVE_FIXED){
		if(IsUsbDriver(i)) g_VolsInfo[i].type=DRIVE_REMOVABLE;
	}
	{
		WCHAR volumeName[32];
		WCHAR fsName[8];
		GetVolumeInformation(name,volumeName,32,&g_VolsInfo[i].serialNumber,NULL,NULL,fsName,8);
		wchar_to_utf8_nocheck(volumeName, 32, g_VolsInfo[i].volumeName, 56);
		wchar_to_utf8_nocheck(fsName, 8, g_VolsInfo[i].fsName, 8);
	}
	get_drive_space(i);
}

void InitDrives(){
	DWORD drives=GetLogicalDrives();
	int i=0,mask=1;
	for(;i<26;i++){
		if(drives&mask) InitDrive(i);
		mask<<=1;
	}
}

BOOL IsNtfs(int i){
	return strncmp(g_VolsInfo[i].fsName,"NTFS",4)==0;
}

void NtfsDrivesIterator(pDriveVisitor f){
	int i=0;
	for(;i<26;i++){
		if(g_bVols[i] && g_VolsInfo[i].serialNumber){
			if(IsNtfs(i)) (*f)(i);
		}
	}
}

void FatDrivesIterator(pDriveVisitor f){
	int i=0;
	for(;i<26;i++){
		if(g_bVols[i] && g_VolsInfo[i].serialNumber){
			if(!IsNtfs(i)) (*f)(i);
		}
	}
}

void PrintDriveDetails(int i){
	printf("driver %c's type is %x\n",i+'A',g_VolsInfo[i].type);
	printf(":%u,%ls,%ls.\n",g_VolsInfo[i].serialNumber,g_VolsInfo[i].volumeName,g_VolsInfo[i].fsName);
}
