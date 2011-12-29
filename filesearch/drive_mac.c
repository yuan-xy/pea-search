#include <stdio.h>
#include "drive.h"
#include "global.h"


void InitDrives(){
    struct statfs *mntbufp = NULL;
    int flags=0;
    int ret = getmntinfo(&mntbufp,flags);
    int i,j;
    struct statfs *p = mntbufp;
    for(i=0,j=0;i<ret;j++){
        while(p->f_blocks<1 || strncmp(p->f_fstypename,"devfs",5)==0 ){
            i++;
            if(i>=ret) break;
            p++;
        }
        printf("%d,%d:%llu\n",i,j,p->f_blocks);
        g_VolsInfo[j].type = p->f_type;
        g_VolsInfo[j].serialNumber = p->f_fsid.val[0];
        //g_VolsInfo[j].volumeName = "";
        strncpy(g_VolsInfo[j].fsName, p->f_fstypename,8);
        g_VolsInfo[j].totalMB = p->f_bsize * p->f_blocks  >> 20;
        g_VolsInfo[j].totalFreeMB = p->f_bsize * p->f_bfree  >> 20;
        strcpy(g_VolsInfo[j].mntfromname , p->f_mntfromname);
        strcpy(g_VolsInfo[j].mntonname, p->f_mntonname);
        i++;p++;
    }
}

