#include "env.h"
#include <stdio.h>
#include "search.h"
#include "main.h"
#include "global.h"

#define EXPECT(cond) if(!(cond)) { fprintf(stderr,"ASSERT ERROR: line %d in '%s'\n",__LINE__, __FILE__);ret=1; goto error;}

#define EQUAL_STR(str,file) strnicmp(str,file->FileName,strlen(str))==0

static pFileEntry my_find_file(WCHAR *name){
	return find_file(name,wcslen(name));
}

void wait_c_ready(){
	if(g_bVols[2]){
		while(!g_loaded[2]) Sleep(100);
	}
}

int main(int argc, char *argv[]){
	int ret = 0;
	pFileEntry dir;
	gigaso_init();
	wait_c_ready();
	dir = find_file(L"c:",2);
	EXPECT(dir!=NULL);
	dir = find_file(L"C:/",3);
	EXPECT(dir!=NULL);
	dir = find_file(L"C:\\",3);
	EXPECT(dir!=NULL);
	dir = my_find_file(L"C:/program files");
	EXPECT(dir!=NULL);
	EXPECT(EQUAL_STR("program files",dir));
	dir = my_find_file(L"C:/windows");
	EXPECT(dir!=NULL);
	dir = my_find_file(L"C:\\WINDOWS\\system32\\drivers\\etc\\hosts");
	EXPECT(dir!=NULL);
	EXPECT(EQUAL_STR("hosts",dir));
	dir = my_find_file(L"system32\\drivers\\etc\\hosts");
	EXPECT(dir==NULL);
	dir = my_find_file(L"C:\\vvvvvvvvvvvvv\\wwwwwwwww");
	EXPECT(dir==NULL);
	dir = my_find_file(L"n:\\vvvvvvvvv");
	EXPECT(dir==NULL);
	deleteDir(g_rootVols[2]);
	g_loaded[2]=0;
	gigaso_destory();
error:
	return ret;
}
