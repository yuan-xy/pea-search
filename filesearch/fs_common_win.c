#include "env.h"
#include <stdio.h>
#include <time.h>
#include "global.h"
#include "fs_common.h"
#include "suffix.h"
#include "ntfs.h"
#include "fat.h"

int getDrive(pFileEntry file){
	pFileEntry parent = file;
	while(parent->up.parent!=NULL) parent = parent->up.parent;
	return *(parent->FileName) - L'A';
}

BOOL CloseVolumeHandle(int i){
	if(g_hVols[i]==NULL) return 0;
	return CloseHandle(g_hVols[i]);
}

