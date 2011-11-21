#include "env.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "util.h"
#include "search.h"
#include "write.h"
#include "suffix.h"
#include "fs_common.h"
#include "chinese.h"
#include "exception_dump.h"
#include "macfs.h"

BOOL read_build_check(int i){
	BOOL flag = load_db(i);
	if(!flag) return 0;
	build_dir(i);
	FilesIterate(g_rootVols[i],(pFileVisitor)check_file_entry,&flag);
	return 1;
}

void after_build(int i){
	FilesIterate(g_rootVols[i],FileRemoveFilter,NULL);
}

void load_online_db(int i){
	if(read_build_check(i)){
		after_build(i);
		g_loaded[i]=1;
		printf("load drive %c finished.\n",i+'A');
	}
}

void scan(int i){
	if(g_loaded[i]) return;
	resetMap(i);
	scanMac(genRootFileEntry(i),i);
	after_build(i);
	g_loaded[i]=1;
	save_db(i);
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

BOOL gigaso_init(){
	if(4 == sizeof(void *)){
		my_assert(24 == sizeof(FileEntry), 0);
	}else{
		my_assert(40 == sizeof(FileEntry), 0);
	}
	breakpad_init();
	//request_dump();
	init_chinese();
	setlocale(LC_ALL, "");
	load_online_db(MAC_DRIVE_INDEX);
	scan(MAC_DRIVE_INDEX);
	StartMonitorThread(MAC_DRIVE_INDEX);
	return 1;
}

BOOL gigaso_destory(){
	save_db(MAC_DRIVE_INDEX);
	StopMonitorThread(MAC_DRIVE_INDEX);
	return 1;
}

