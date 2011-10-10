#include "env.h"

long GetTickCount()
{
	struct timeval current;
	gettimeofday(&current, NULL);
	return current.tv_sec * 1000 + current.tv_usec/1000;
}

char* strrev(char* szT)
{
    if ( !szT ){
		return "";
	}else{
	    int i = strlen(szT);
	    int t = !(i%2)? 1 : 0;      // 检查串长度.
		int j,k;
	    for(j = i-1 , k = 0 ; j > (i/2 -t) ; j-- ){
	        char ch  = szT[j];
	        szT[j]   = szT[k];
	        szT[k++] = ch;
	    }
	    return szT;	
	}
}

char* _itoa(int value, char*  str, int radix)
{
    int  rem = 0;
    int  pos = 0;
    char ch  = '!' ;
    do
    {
        rem    = value % radix ;
        value /= radix;
        if ( 16 == radix )
        {
            if( rem >= 10 && rem <= 15 )
            {
                switch( rem )
                {
                    case 10:
                        ch = 'a' ;
                        break;
                    case 11:
                        ch ='b' ;
                        break;
                    case 12:
                        ch = 'c' ;
                        break;
                    case 13:
                        ch ='d' ;
                        break;
                    case 14:
                        ch = 'e' ;
                        break;
                    case 15:
                        ch ='f' ;
                        break;
                }
            }
        }
        if( '!' == ch )
        {
            str[pos++] = (char) ( rem + 0x30 );
        }
        else
        {
            str[pos++] = ch ;
        }
    }while( value != 0 );
    str[pos] = '\0' ;
    return strrev(str);
}