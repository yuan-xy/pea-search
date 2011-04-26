#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILE_SEARCH_DRIVE_H_
#define FILE_SEARCH_DRIVE_H_

#include "env.h"
#include <windows.h>

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
	UINT type;  // Determines whether a disk drive is a removable, fixed, CD-ROM, RAM disk, or network drive.
	DWORD serialNumber;  //the volume serial number.
	WCHAR volumeName[32];  //A pointer to a buffer that receives the name of a specified volume. The maximum buffer size is MAX_PATH+1.
	WCHAR fsName[8];  //the name of the file system, for example, the FAT file system or the NTFS file system
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
 * 打印给定的驱动器的详细信息
 * @param i  驱动器编号
 */
extern void PrintDriveDetails(int);

#endif  // FILE_SEARCH_DRIVE_H_

#ifdef __cplusplus
}
#endif
