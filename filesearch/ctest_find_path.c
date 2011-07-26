#include <stdio.h>
#include "search.h"

#define EXPECT(cond) if(!(cond)) return 1;

int main(int argc, char *argv[]){
	pFileEntry dir = find_file(L"c:",2*sizeof(wchar_t));
	EXPECT(dir!=NULL);
	return 0;
}
