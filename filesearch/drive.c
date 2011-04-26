#include <stdio.h>
#include "drive.h"
#include "global.h"

BOOL is_fix_drive(int i){
	return g_VolsInfo[i].type==DRIVE_FIXED;
}

BOOL is_cdrom_drive(int i){
	return g_VolsInfo[i].type==DRIVE_CDROM;
}

BOOL is_removable_drive(int i){
	return g_VolsInfo[i].type==DRIVE_REMOVABLE;
}

void InitDrive(int i){
	WCHAR name[4];
	g_bVols[i]=1;
	swprintf(name,4,L"%c:\\",i+'A');
	g_VolsInfo[i].type = GetDriveType(name);
	GetVolumeInformation(name,g_VolsInfo[i].volumeName,32,&g_VolsInfo[i].serialNumber,NULL,NULL,g_VolsInfo[i].fsName,8);
}

void InitDrives(){
	DWORD drives=GetLogicalDrives();
	int i=0,mask=1;
	for(;i<26;i++){
		if(drives&mask) InitDrive(i);
		mask<<=1;
	}
}

void DrivesIterator(pDriveVisitor f){
	int i=0;
	for(;i<26;i++){
		if(g_bVols[i]){
			(*f)(i);
		}
	}
}

void ValidDrivesIterator(pDriveVisitor f){
	int i=0;
	for(;i<26;i++){
		if(g_bVols[i] && g_VolsInfo[i].serialNumber){
			(*f)(i);
		}
	}
}

BOOL IsNtfs(int i){
	return wcsncmp(g_VolsInfo[i].fsName,L"NTFS",4)==0;
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

