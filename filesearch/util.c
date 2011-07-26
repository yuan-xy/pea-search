#include "tcmalloc.h"
//#pragma comment(linker, "/include:__tcmalloc")

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "util.h"


void assert_debug(int exp){
	#ifdef MY_DEBUG
		assert(exp);
	#endif
}

int time_passed(void (*f)()){
#ifdef WIN32
	int end,start = GetTickCount();
	(*f)();
	end = GetTickCount();
	return end-start;
#else
	time_t start,end;
	start = time(NULL);
	if(start == (time_t)-1) return -1;
	(*f)();
	end = time(NULL);
	return (int)(end-start)*1000;
#endif
}

int time_passed_p1(void (*f)(void *),void *p1){
#ifdef WIN32
	int end,start = GetTickCount();
	(*f)(p1);
	end = GetTickCount();
	return end-start;
#else
	time_t start,end;
	start = time(NULL);
	if(start == (time_t)-1) return -1;
	(*f)(p1);
	end = time(NULL);
	return (int)(end-start)*1000;
#endif
}

int time_passed_p2(void (*f)(void *,void *),void *p1, void *p2){
#ifdef WIN32
	int end,start = GetTickCount();
	(*f)(p1,p2);
	end = GetTickCount();
	return end-start;
#else
	time_t start,end;
	start = time(NULL);
	if(start == (time_t)-1) return -1;
	(*f)(p1,p2);
	end = time(NULL);
	return (int)(end-start)*1000;
#endif
}

int time_passed_ret(int (*f)(),int *ret_data){
#ifdef WIN32
	int end,start = GetTickCount();
	*ret_data = (*f)();
	end = GetTickCount();
	return end-start;
#else
	time_t start,end;
	start = time(NULL);
	if(start == (time_t)-1) return -1;
	(*f)();
	end = time(NULL);
	return (int)(end-start)*1000;
#endif
}


void * malloc_safe(size_t len){
	void *ret = tc_malloc(len);
	if(ret==NULL){
		MEM_ERROR;
		exit(EXIT_FAILURE);
	}
	return ret;
}

void * realloc_safe(void *ptr, size_t len){
	void *ret = tc_realloc(ptr,len);
	if(ret==NULL){
		MEM_ERROR;
		exit(EXIT_FAILURE);
	}
	return ret;
}

void free_safe(void *ptr){
#ifdef MY_DEBUG
	assert(ptr);
#endif
	if (ptr) tc_free(ptr);
	ptr = NULL;
}

FSIZE file_size_shorten(ULONGLONG size){
	unsigned short ret ;
	ret = (unsigned short ) (size / 1000000000);
	if(ret>0) return 0xc00 | ret;
	ret = (unsigned short ) (size / 1000000);
	if(ret>0) return 0x800 | ret;
	ret = (unsigned short ) (size / 1000);
	if(ret>0) return 0x400 | ret;
	return (FSIZE)size;
}

int file_size_unit(FSIZE size){
	return size >> 10;
}

int file_size_amount(FSIZE size){
	return 0x3FF & size;
}

pUTF8 wchar_to_utf8(const WCHAR *in, int insize_c, int *out_size_b){
    int buffer_len = WideCharToMultiByte(CP_UTF8, 0, in, insize_c, NULL, 0, NULL, NULL);
    if (buffer_len <= 0){
    	return 0;
    }else{
    	pUTF8 utf8 = (pUTF8)malloc_safe(buffer_len);
    	WideCharToMultiByte(CP_UTF8, 0, in, insize_c, utf8, buffer_len, NULL, NULL);
    	if(out_size_b!=NULL) *out_size_b = buffer_len;
    	return utf8;
    }
}

WCHAR* utf8_to_wchar(const pUTF8 in, int insize_b, int *out_size_c){
    int buffer_len = MultiByteToWideChar(CP_UTF8, 0, in, insize_b, NULL, 0);
    if (buffer_len <= 0){
    	return 0;
    }else{
    	WCHAR *wstr = (WCHAR *)malloc_safe(buffer_len*sizeof(WCHAR));
    	MultiByteToWideChar(CP_UTF8, 0, in, insize_b, wstr, buffer_len);
    	if(out_size_c!=NULL) *out_size_c = buffer_len;
    	return wstr;
    }
}
