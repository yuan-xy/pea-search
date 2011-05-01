#define MAX_RQRS_LEN 0x1000

typedef struct {
	DWORD32 RqLen;	/* Total length of request, not including this field */
	CHAR Command;
	BYTE Record [MAX_RQRS_LEN];
} REQUEST;

typedef struct {
	DWORD32 RsLen;	/* Total length of response, not including this field */
	CHAR Status;
	BYTE Record [MAX_RQRS_LEN];
} RESPONSE;

#define RQ_SIZE sizeof (REQUEST)
#define RQ_HEADER_LEN RQ_SIZE-MAX_RQRS_LEN
#define RS_SIZE sizeof (RESPONSE)
#define RS_HEADER_LEN RS_SIZE-MAX_RQRS_LEN

#define CS_TIMEOUT 5000

#define MAX_CLIENTS  5 /* Maximum number of clients for serverNP */

#define SERVER_PIPE L"\\\\.\\PIPE\\SERVER"


/* Commands for the statistics maintenance function. */

#define CS_INIT			1
#define CS_RQSTART		2
#define CS_RQCOMPLETE	3
#define CS_REPORT		4
#define CS_TERMTHD		5
