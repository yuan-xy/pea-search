#ifdef __cplusplus
extern "C" {
#endif

#ifndef  FILE_SEARCH_MAC_PORT_H_
#define  FILE_SEARCH_MAC_PORT_H_

void _beginthread( 
   void( *start_address )( void * ),
   unsigned stack_size,
   void *arglist 
);

int GetLastError();

long GetTickCount();

char* strrev(char* szT);

char* _itoa(int value, char*  str, int radix);
    
void GetUserNameA(char *fbuffer, int *size);

#endif  //  FILE_SEARCH_MAC_PORT_H_

#ifdef __cplusplus
}
#endif
