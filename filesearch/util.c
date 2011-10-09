#include "GIGASOConfig.h"

#ifdef HAVE_64_BIT
	#define TC_MALLOC malloc
	#define TC_REALLOC realloc
	#define TC_FREE free
#else
	#include "tcmalloc.h"
	//#pragma comment(linker, "/include:__tcmalloc")
	#define TC_MALLOC tc_malloc
	#define TC_REALLOC tc_realloc
	#define TC_FREE tc_free
#endif


#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "util.h"
#include "md5.h"


void assert_debug(int exp){
	#ifdef MY_DEBUG
		//assert(exp);
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
	void *ret = TC_MALLOC(len);
	if(ret==NULL){
		MEM_ERROR;
		exit(EXIT_FAILURE);
	}
	return ret;
}

void * realloc_safe(void *ptr, size_t len){
	void *ret = TC_REALLOC(ptr,len);
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
	if (ptr) TC_FREE(ptr);
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

#ifdef WIN32
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

int wchar_to_utf8_2(const WCHAR *in, int insize_c, pUTF8 out, int *out_buffer_size){
    int buffer_len = WideCharToMultiByte(CP_UTF8, 0, in, insize_c, NULL, 0, NULL, NULL);
    if (buffer_len <= 0){
    	return 0;
    }else if(out_buffer_size<buffer_len){
		return -1;
	}else{
    	WideCharToMultiByte(CP_UTF8, 0, in, insize_c, out, buffer_len, NULL, NULL);
		return buffer_len;
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

int utf8_to_wchar_2(const pUTF8 in, int insize_b, wchar_t *out, int *out_buffer_size){
    int buffer_len = MultiByteToWideChar(CP_UTF8, 0, in, insize_b, NULL, 0);
    if (buffer_len <= 0){
    	return 0;
    }else if(out_buffer_size<buffer_len){
		return -1;
	}else{
    	MultiByteToWideChar(CP_UTF8, 0, in, insize_b, out, buffer_len);
    	return buffer_len;
    }
}

void wchar_to_char(const wchar_t *ws, char *buffer, int buffer_size){
	WideCharToMultiByte(CP_ACP, 0, ws, wcslen(ws), buffer, buffer_size, NULL, NULL);
}
#else
pUTF8 wchar_to_utf8(const WCHAR *in, int insize_c, int *out_size_b){
	//TODO iconv
}

int wchar_to_utf8_2(const WCHAR *in, int insize_c, pUTF8 out, int *out_buffer_size){
	//TODO iconv
}

WCHAR* utf8_to_wchar(const pUTF8 in, int insize_b, int *out_size_c){
	//TODO iconv
}

int utf8_to_wchar_2(const pUTF8 in, int insize_b, wchar_t *out, int *out_buffer_size){
	//TODO iconv
}

void wchar_to_char(const wchar_t *ws, char *buffer, int buffer_size){
	//TODO iconv
}
#endif

wchar_t *wcsrchr_me(const wchar_t *name, int len, const wchar_t C){
	int index=len-1;
		for(;index>0;index--){
			if( *(name+index) == C) return name+index;
		}
	return NULL;
}

static void MD5Print(unsigned char *digest, char *digest_str){
  int i;
  for (i = 0; i < MD5_LEN; i++) sprintf (digest_str+i*2, "%02x", *(digest+i));
}

void MD5Str(char *string, char *md5){
  MD5_CTX context;
  unsigned char digest[MD5_LEN];
  unsigned int len = strlen (string);
  MD5Init (&context);
  MD5Update (&context, string, len);
  MD5Final (digest, &context);
  MD5Print(digest,md5);
}

void MD5File(char *filename, char *md5){
  FILE *file;
  MD5_CTX context;
  int len;
  unsigned char buffer[1024], digest[MD5_LEN];
  if ((file = fopen (filename, "rb")) == NULL) return;
  MD5Init (&context);
  while (len = fread (buffer, 1, 1024, file))
    MD5Update (&context, buffer, len);
  MD5Final (digest, &context);
  fclose (file);
  MD5Print(digest,md5);
}

