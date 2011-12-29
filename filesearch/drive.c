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

void ValidFixDrivesIterator(pDriveVisitor f){
	int i=0;
	for(;i<26;i++){
		if(g_bVols[i] && g_VolsInfo[i].serialNumber && g_VolsInfo[i].type == DRIVE_FIXED ){
			(*f)(i);
		}
	}
}


