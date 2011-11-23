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
#include "unixfs.h"

BOOL read_build_check(int i){
	BOOL flag = load_db(i);
	if(!flag) return 0;
	build_dir(i);
	FilesIterate(g_rootVols[i],(pFileVisitor)check_file_entry,&flag);
	return 1;
}

void after_build(int i){
}

void load_online_db(int i){
	if(read_build_check(i)){
		g_loaded[i]=1;
		printf("load drive %c finished.\n",i+'A');
	}
}

static void scan(){
    int i = MAC_DRIVE_INDEX;
	if(g_loaded[i]) return;
	resetMap(i);
	scanUnix(genRootFileEntry(i),i);
	g_loaded[i]=1;
	save_db(i);
	printf("Scan drive %c finished.\n",i+'A');
}

void rescan(int i){
	if(i<0 || i>=DIRVE_COUNT || g_loaded[i] == 0) return;
	g_loaded[i] = 0;
	deleteFile(g_rootVols[i]);
	scan();
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
    //TODO: 根据mount情况得到onlinedb，然后load。
    printf("scan passed %d ms.\n",time_passed(scan));
	StartMonitorThread(MAC_DRIVE_INDEX);
	return 1;
}

BOOL gigaso_destory(){
	save_db(MAC_DRIVE_INDEX);
	StopMonitorThread(MAC_DRIVE_INDEX);
	return 1;
}

