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

void FileRemoveFilter(pFileEntry file, void *data){
	if(file->FileName[0]=='$') file->us.v.system=1;
	if(IsDir(file)){
		if( (file->us.v.FileNameLength==4 && file->FileName[0]=='.' && file->FileName[1]=='s' && file->FileName[2]=='v'  && file->FileName[3]=='n')
           || (file->us.v.FileNameLength==4 && file->FileName[0]=='.' && file->FileName[1]=='g' && file->FileName[2]=='i'  && file->FileName[3]=='t')
           || (file->us.v.FileNameLength==3 && file->FileName[0]=='C' && file->FileName[1]=='V' && file->FileName[2]=='S')
           ){
            file->children=NULL;
		}
		if((file->us.v.FileNameLength==4 && strncmp(file->FileName,"Temp",4)==0)
           || (file->us.v.FileNameLength==24 && strncmp(file->FileName,"Temporary Internet Files",24)==0)
           ){
			if((file->up.parent->us.v.FileNameLength==14 && strncmp(file->up.parent->FileName,"Local Settings",14)==0)
               ){
                file->children=NULL;
			}
		}
		if(file->us.v.FileNameLength==5 && strncmp(file->FileName,"cache",5)==0){
			if(file->up.parent->us.v.FileNameLength==3 && strncmp(file->up.parent->FileName,"var",3)==0){
                file->children=NULL;
			}
		}
	}
}


BOOL CloseVolumeHandle(int i){
	if(g_hVols[i]==NULL) return 0;
	return CloseHandle(g_hVols[i]);
}

