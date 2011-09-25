#include <windows.h>
#include "drive.h"
#include "ntfs.h"
#include "global.h"

UTF8 rootNames[26][3] = {"A:", "B:", "C:", "D:", "E:", "F:", "G:", "H:", "I:", "J:", "K:", "L:", "M:", "N:", "O:", "P:", "Q:", "R:", "S:", "T:", "U:", "V:", "W:", "X:", "Y:", "Z:"};

BOOL		g_bVols[26]={0};
BOOL		g_loaded[DIRVE_COUNT_OFFLINE]={0};
BOOL		g_expires[26]={0};
DriveInfo	g_VolsInfo[DIRVE_COUNT_OFFLINE]={0};
HANDLE      g_hVols[26]={0};
pFileEntry g_rootVols[DIRVE_COUNT_OFFLINE]={0};

USN         g_curFirstUSN[26]={0};
USN         g_curNextUSN[26]={0};
DWORDLONG   g_curJournalID[26]={0};
HANDLE      g_hThread[26]={0};
DWORD       g_BytesPerCluster[26];
DWORD       g_FileRecSize[26];
PBYTE       g_pOutBuffer[26]={0};

unsigned int ALL_FILE_COUNT;
