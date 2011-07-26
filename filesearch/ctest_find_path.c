#include "env.h"
#include <stdio.h>
#include "search.h"
#include "main.h"

#define EXPECT(cond) if(!(cond)) { ret=1; goto error;}

int main(int argc, char *argv[]){
	int ret = 0;
	pFileEntry dir;
	gigaso_init();
	dir = find_file(L"c:",2);
	EXPECT(dir!=NULL);
error:
	//bug fix1: only save db in  gigaso_destory when scan thread finish.
	//bug fix2: correct set root file entry when load db
	//gigaso_destory();
	return ret;
}
