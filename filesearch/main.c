#include "env.h"
#include <locale.h>
#include <windows.h>
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
//TODO: 做成服务与进程间通信、log库、win64移植、linux移植、UI部分、首页常用文档、langchy快捷启动程序、Explorer整合、全文检索、文件缩略图、邮件搜索
//TODO: 数据库过期时异步更新、文件系统监视代码排错、
//TODO: 支持压缩文件、光盘镜像中的文件查询
//TODO: 所有windows系统调用查看返回值
//TODO: 文件的资源管理器类树状浏览


void search0(void *name){
	pFileEntry *result=NULL;
	int count=0,i;
		NEW(SearchEnv, sEnv);
		sEnv->case_sensitive=0;
		sEnv->order=0;
		sEnv->file_type=0;
		sEnv->path_name = L"E:\\backup";
		sEnv->path_len=0;
	count = search((WCHAR *)name,sEnv,&result);
	for(i=0;i<count;i++){
		pFileEntry file = *(result+i);
		FSIZE size;
		size = GET_SIZE(file);
		print_time(file);
		print_full_path(file);
		//PrintFilenameMB(file);
		printf("\n");
		if(i>30) break;
	}
	free_safe(sEnv);
	free_safe(result);
}

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
	StartMonitorThread(i);
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
		//TODO: 发送就绪事件通知
	}
}

void scan(int i){
	if(g_loaded[i]) return;
	resetMap(i);
	if(IsNtfs(i)){
		ScanMFT(i);
		add2Map(genRootFileEntry(i), i);
		build_dir(i);
		CreateThread(NULL,0,init_size_time_all,g_rootVols[i],0,0);
		after_build(i);
	}else{
		scanRoot(genRootFileEntry(i),i);
		after_build(i);
	}
	g_loaded[i]=1;
	//TODO: 发送就绪事件通知
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
	}
}

SHARELIB_API BOOL gigaso_init(){
	printf("load hz : %d milli-seconds.\n",time_passed(init_chinese));
	parse_pinyin("fang'an");
	parse_pinyin("chanzao");
	parse_pinyin("fangan");
	setlocale (LC_ALL, "");
	my_assert(24 == sizeof(FileEntry), 0);
	my_assert(WORD_SIZE == sizeof(void *), 0);
	InitDrives();
	//DrivesIterator(PrintDriveDetails);
	ValidDrivesIterator(load_online_db);
	NtfsDrivesIterator(scan);
	FatDrivesIterator(scan);
	StartDriveChangeMonitorThread(DriveChangeListener);
	if(load_offline) load_offline_dbs();
	return 1;
}

SHARELIB_API BOOL gigaso_destory(){
	save_db_all();
	ValidDrivesIterator(StopMonitorThread);
	StopDriveChangeMonitorThread();
	ValidDrivesIterator(CloseVolumeHandle);
	return 1;
}

int main(){
	WCHAR ss[64];
	gigaso_init();
	do{
		//wscanf_s(L"%[^\n]",ss,31);
		fgetws(ss,63,stdin);
		if(ss[0]==L'q') break;
		printf("search : %d milli-seconds\n",time_passed_p1(search0,ss));
	}while(1);
	gigaso_destory();
	return 0;
}
