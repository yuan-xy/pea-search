#include "env.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "util.h"
#include "ntfs.h"
#include "fat.h"
#include "drive.h"
#include "search.h"
#include "write.h"
#include "suffix.h"
#include "fs_common.h"
#include "drive_change.h"
#include "chinese.h"
#include "server.h"
#include "exception_dump.h"
#include "desktop.h"
#include "common.h"

BOOL read_build_check(int i){
	BOOL flag = load_db(i);
	if(!flag) return 0;
	build_dir(i);
	FilesIterate(g_rootVols[i],check_file_entry,&flag);
	return 1;
}

void after_build(int i){
	FilesIterate(g_rootVols[i],FileRemoveFilter,NULL);
	FilesIterate(g_rootVols[i],SuffixProcess,NULL);
}

void load_online_db(int i){
	//if(i!=2) return;
	if(IsNtfs(i)){
		OpenNtfsHandle(i);
		InitVolumeData(i);
		initUSN(i);
	}
	if(read_build_check(i)){
		after_build(i);
		g_loaded[i]=1;
	}
}

void scan(int i){
	if(g_loaded[i]) return;
	resetMap(i);
	if(IsNtfs(i)){
		ScanMFT(i);
		add2Map(genRootFileEntry(i), i);
		build_dir(i);
		{
			HANDLE h = CreateThread(NULL,0,init_size_time_all,g_rootVols[i],0,0);
			SetThreadPriority(h,THREAD_PRIORITY_BELOW_NORMAL);
			ResumeThread(h);
			CloseHandle(h);
		}
		after_build(i);
		g_loaded[i]=1;
	}else{
		scanRoot(genRootFileEntry(i),i);
		after_build(i);
		g_loaded[i]=1;
		save_db(i);
	}
	printf("Scan drive %c finished.\n",i+'A');
}

void rescan(int i){
	if(i<0 || i>=DIRVE_COUNT || g_loaded[i] == 0) return;
	g_loaded[i] = 0;
	deleteFile(g_rootVols[i]);
	scan(i);
}

static int get_next_offline_slot(){
	int i;
	for(i=DIRVE_COUNT;i<DIRVE_COUNT_OFFLINE;i++){
		if(!g_loaded[i]) return i;
	}
	return -1;
}

void DriveChangeListener(int i, BOOL add){
	if(add){
		printf(" new drive %c \n",i+'A');
		InitDrive(i);
		load_online_db(i);
		scan(i);
	}else{
		printf(" remove drive %c \n",i+'A');
		StopMonitorThread(i);
		CloseVolumeHandle(i);
		g_bVols[i]=0;
		g_loaded[i]=0;
		{
			int off_id = get_next_offline_slot();
			g_loaded[off_id]=1;
			g_VolsInfo[off_id] = g_VolsInfo[i];
			g_rootVols[off_id] = g_rootVols[i];
		}
	}
}

DWORD WINAPI  ScanAll(PVOID pParam){
	NtfsDrivesIterator(scan);
	FatDrivesIterator(scan);
	StartDriveChangeMonitorThread(DriveChangeListener);
	ValidFixDrivesIterator(StartMonitorThread);
	return 0;
}

BOOL gigaso_init(){
	if(4 == sizeof(void *)){
		my_assert(24 == sizeof(FileEntry), 0);
	}else{
		printf("file entry size:%d\n",sizeof(FileEntry));
	}
	if(!WindowsVersionOK()) return 0;
	breakpad_init();
	//request_dump();
	init_chinese();
	setlocale(LC_ALL, "");
	InitDrives();
	//DrivesIterator(PrintDriveDetails);
	ValidDrivesIterator(load_online_db);
	CreateThread(NULL,0,ScanAll,NULL,0,0);
	return 1;
}

BOOL gigaso_destory(){
	ValidDrivesIterator(save_db);
	ValidFixDrivesIterator(StopMonitorThread);
	StopDriveChangeMonitorThread();
	ValidDrivesIterator(CloseVolumeHandle);
	return 1;
}

static void print_all_stat(WCHAR *str){
	char buffer[1024];
	print_stat(statistic(str,NULL),buffer);
	printf("%s\n",buffer);
}

