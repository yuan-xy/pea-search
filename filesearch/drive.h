#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_DRIVE_H_
#define FILE_SEARCH_DRIVE_H_

#include "env.h"

#ifdef APPLE
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

#define DRIVE_UNKNOWN 0 //The drive type cannot be determined.
#define DRIVE_NO_ROOT_DIR 1 //The root path is invalid; for example, there is no volume mounted at the specified path.
#define DRIVE_REMOVABLE 2 //The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.
#define DRIVE_FIXED 3 //The drive has fixed media; for example, a hard disk drive or flash drive.
#define DRIVE_REMOTE 4 //The drive is a remote (network) drive.
#define DRIVE_CDROM 5 //The drive is a CD-ROM drive.
#define DRIVE_RAMDISK 6 //The drive is a RAM disk.
#endif


/**
 * 是否是固定的驱动器
 * @param i 驱动器编号
 */
extern BOOL is_fix_drive(int i);
/**
 * 是否是光盘的驱动器
 * @param i 驱动器编号
 */
extern BOOL is_cdrom_drive(int i);
/**
 * 是否是可移动的驱动器
 * @param i 驱动器编号
 */
extern BOOL is_removable_drive(int i);

struct driveInfo{
#ifdef APPLE
	uint32_t	bsize;	/* fundamental file system block size */ 
	uint64_t	blocks;	/* total data blocks in file system */ 
	uint64_t	bfree;	/* free blocks in fs */ 
	fsid_t		fsid;		/* file system id */ 
	uid_t		owner;	/* user that mounted the filesystem */ 
	uint32_t	flags;	/* copy of mount exported flags */ 
	uint32_t	fssubtype;	/* fs sub-type (flavor) */ 
	char		fstypename[MFSTYPENAMELEN];	/* fs type name */ 
	char		mntonname[MAXPATHLEN];	/* directory on which mounted */ 
	char		mntfromname[MAXPATHLEN];	/* mounted filesystem */
#endif
	UINT type;  // Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive.
	DWORD serialNumber;  //the volume serial number.
	UTF8 volumeName[56];
	UTF8 fsName[8];  //the name of the file system, for example, the FAT file system or the NTFS file system
	int totalMB; //最大4PB
	int totalFreeMB;
};


typedef struct driveInfo DriveInfo, *pDriveInfo;

/**
 * 初始化所有有效的驱动器，包括驱动器是否有效、驱动器类型和序列号等信息。
 */
extern void InitDrives();

/**
 * 初始化给定的驱动器
 * @param i 驱动器编号
 */
extern void InitDrive(int i);

/**
 * 访问给定的驱动器
 * @param i  驱动器编号
 */
typedef void (*pDriveVisitor)(int i);

/**
 * 遍历所有存在的驱动器。没有插入光盘的光驱也是存在的驱动器。
 */
extern void DrivesIterator(pDriveVisitor);

/**
 * 遍历所有有效的驱动器。没有插入光盘的光驱不是有效的驱动器。
 */
extern void ValidDrivesIterator(pDriveVisitor);

extern void ValidFixDrivesIterator(pDriveVisitor);


#ifdef WIN32
/**
 * 给定的驱动器是否是NTFS类型的
 * @param i  驱动器编号
 */
extern BOOL IsNtfs(int i);

/**
 * 遍历所有有效的NTFS类型的驱动器。
 */
extern void NtfsDrivesIterator(pDriveVisitor);
extern void FatDrivesIterator(pDriveVisitor);

/**
 * 获得驱动器的磁盘空间与可用空间
 * @param i 驱动器编号
 */
extern void get_drive_space(int i);

#endif


#endif  // FILE_SEARCH_DRIVE_H_

#ifdef __cplusplus
}
#endif
