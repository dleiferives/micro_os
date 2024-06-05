#ifndef FAT_HELPER_LIB_dleiferives
#define FAT_HELPER_LIB_dleiferives
#define MAX_OEM_NAME_LENGTH 8
#include <stdlib.h>
#include <stdio.h>
extern int GLOBAL_FAT_DEVICE_SECTORS_IN_RAM;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct {
	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media. This field is the FAT32 32-bit count of
	    // sectors occupied by ONE FAT. BPB_FATSz16 must be 0.
	    unsigned int BPB_FATSz32;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media.
	    // Bits 0-3 -- Zero-based number of active FAT. Only valid if mirroring
	    // is disabled.
	    // Bits 4-6 -- Reserved.
	    // Bit 7 -- 0 means the FAT is mirrored at runtime into all FATs.
	    //          1 means only one FAT is active; it is the one referenced
	    //          in bits 0-3.
	    // Bits 8-15 -- Reserved.
	    unsigned short BPB_ExtFlags;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media.
	    // High byte is major revision number.
	    // Low byte is minor revision number. This is the version number of
	    // the FAT32 volume. This supports the ability to extend the FAT32
	    // media type in the future without worrying about old FAT32 drivers
	    // mounting the volume. This document defines the version to 0:0. If
	    // this field is non-zero, back-level Windows versions will not mount
	    // the volume.
	    // NOTE: Disk utilities should respect this field and not operate on
	    // volumes with a higher major or minor version number than that for
	    // which they were designed. FAT32 file system drivers must check
	    // this field and not mount the volume if it does not contain a version
	    // number that was defined at the time the driver was written.
	    unsigned short BPB_FSVer;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media. This is set to the cluster number of the
	    // first cluster of the root directory, usually 2 but not required to be 2.
	    // NOTE: Disk utilities that change the location of the root directory
	    // should make every effort to place the first cluster of the root
	    // directory in the first non-bad cluster on the drive (i.e., in cluster 2,
	    // unless it’s marked bad). This is specified so that disk repair utilities
	    // can easily find the root directory if this field accidentally gets
	    // zeroed.
	    unsigned int BPB_RootClus;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media. Sector number of FSINFO structure in the
	    // reserved area of the FAT32 volume. Usually 1.
	    // NOTE: There will be a copy of the FSINFO structure in BackupBoot,
	    // but only the copy pointed to by this field will be kept up to date (i.e.,
	    // both the primary and backup boot record will point to the same
	    // FSINFO sector).
	    unsigned short BPB_FSInfo;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media. If non-zero, indicates the sector number
	    // in the reserved area of the volume of a copy of the boot record.
	    // Usually 6. No value other than 6 is recommended.
	    unsigned short BPB_BkBootSec;

	    // This field is only defined for FAT32 media and does not exist on
	    // FAT12 and FAT16 media. Reserved for future expansion. Code
	    // that formats FAT32 volumes should always set all of the bytes of
	    // this field to 0.
	    unsigned char BPB_Reserved[12];

	    // The following fields have the same definitions as they do for FAT12
	    // and FAT16 media, but their offsets are different in the boot sector
	    // for FAT32 media.
	    unsigned char BS_DrvNum;
	    unsigned char BS_Reserved1;
	    unsigned char BS_BootSig;
	    unsigned int BS_VolID;
	    char BS_VolLab[11];
	    char BS_FilSysType[8];
        } FAT_32_Device;

	typedef struct 
	{
	    // Int 0x13 drive number (e.g. 0x80). This field supports MS-DOS
	    // bootstrap and is set to the INT 0x13 drive number of the media
	    // (0x00 for floppy disks, 0x80 for hard disks).
	    // NOTE: This field is actually operating system specific.
	    unsigned char BS_DrvNum;

	    // Reserved (used by Windows NT). Code that formats FAT volumes
	    // should always set this byte to 0.
	    unsigned char BS_Reserved1;

	    // Extended boot signature (0x29). This is a signature byte that
	    // indicates that the following three fields in the boot sector are
	    // present.
	    unsigned char BS_BootSig;

	    // Volume serial number. This field, together with BS_VolLab,
	    // supports volume tracking on removable media. These values allow
	    // FAT file system drivers to detect that the wrong disk is inserted in a
	    // removable drive. This ID is usually generated by simply combining
	    // the current date and time into a 32-bit value.
	    unsigned int BS_VolID;

	    // Volume label. This field matches the 11-byte volume label
	    // recorded in the root directory.
	    // NOTE: FAT file system drivers should make sure that they update
	    // this field when the volume label file in the root directory has its
	    // name changed or created. The setting for this field when there is no
	    // volume label is the string “NO NAME ”.
	    char BS_VolLab[11];

	    // One of the strings “FAT12 ”, “FAT16 ”, or “FAT ”.
	    // NOTE: Many people think that the string in this field has
	    // something to do with the determination of what type of FAT—
	    // FAT12, FAT16, or FAT32—that the volume has. This is not true.
	    // You will note from its name that this field is not actually part of the
	    // BPB. This string is informational only and is not used by Microsoft
	    // file system drivers to determine FAT type because it is frequently
	    // not set correctly or is not present. See the FAT Type Determination
	    // section of this document. This string should be set based on the
	    // FAT type though, because some non-Microsoft FAT file system
	    // drivers do look at it.
	    char BS_FilSysType[8];
        } FAT_16_or_12_Device;
typedef union {
	FAT_32_Device FAT_32;
	FAT_16_or_12_Device FAT_16_or_12;	
} FAT_Type;


typedef struct{
	unsigned int id;
    unsigned char ptr_valid;
	//unsigned int offset;
	//unsigned int entry_offset;
	unsigned char *data;
}FAT_Sector;

typedef struct {
    // Jump instruction to boot code. This field has two allowed forms:
    // jmpBoot[0] = 0xEB, jmpBoot[1] = 0x??, jmpBoot[2] = 0x90
    // and
    // jmpBoot[0] = 0xE9, jmpBoot[1] = 0x??, jmpBoot[2] = 0x??
    // 0x?? indicates that any 8-bit value is allowed in that byte. What this
    // forms is a three-byte Intel x86 unconditional branch (jump)
    // instruction that jumps to the start of the operating system bootstrap
    // code. This code typically occupies the rest of sector 0 of the volume
    // following the BPB and possibly other sectors. Either of these forms
    // is acceptable. JmpBoot[0] = 0xEB is the more frequently used
    // format.
    unsigned char BS_jmpBoot[3];

    // There are many misconceptions about this field. It is
    // only a name string. Microsoft operating systems don’t pay any
    // attention to this field. Some FAT drivers do. This is the reason that
    // the indicated string, “MSWIN4.1”, is the recommended setting,
    // because it is the setting least likely to cause compatibility problems.
    // If you want to put something else in here, that is your option, but
    // the result may be that some FAT drivers might not recognize the
    // volume. Typically this is some indication of what system formatted
    // the volume.
    char BS_OEMName[MAX_OEM_NAME_LENGTH];

    // Count of bytes per sector. This value may take on only the
    // following values: 512, 1024, 2048 or 4096. If maximum
    // compatibility with old implementations is desired, only the value
    // 512 should be used. There is a lot of FAT code in the world that is
    // basically “hard wired” to 512 bytes per sector and doesn’t bother to
    // check this field to make sure it is 512. Microsoft operating systems
    // will properly support 1024, 2048, and 4096.
    // Note: Do not misinterpret these statements about maximum
    // compatibility. If the media being recorded has a physical sector size
    // N, you must use N and this must still be less than or equal to 4096.
    // Maximum compatibility is achieved by only using media with
    // specific sector sizes.
    unsigned short BPB_BytsPerSec;

    // Number of sectors per allocation unit. This value must be a power
    // of 2 that is greater than 0. The legal values are 1, 2, 4, 8, 16, 32, 64,
    // and 128. Note however, that a value should never be used that
    // results in a “bytes per cluster” value (BPB_BytsPerSec *
    // BPB_SecPerClus) greater than 32K (32 * 1024). There is a
    // misconception that values greater than this are OK. Values that
    // cause a cluster size greater than 32K bytes do not work properly; do
    // not try to define one. Some versions of some systems allow 64K
    // bytes per cluster value. Many application setup programs will not
    // work correctly on such a FAT volume.
    unsigned char BPB_SecPerClus;

    // Number of reserved sectors in the Reserved region of the volume
    // starting at the first sector of the volume. This field must not be 0.
    // For FAT12 and FAT16 volumes, this value should never be
    // anything other than 1. For FAT32 volumes, this value is typically
    // 32. There is a lot of FAT code in the world “hard wired” to 1
    // reserved sector for FAT12 and FAT16 volumes and that doesn’t
    // bother to check this field to make sure it is 1. Microsoft operating
    // systems will properly support any non-zero value in this field.
    unsigned short BPB_RsvdSecCnt;

    // The count of FAT data structures on the volume. This field should
    // always contain the value 2 for any FAT volume of any type.
    // Although any value greater than or equal to 1 is perfectly valid,
    // many software programs and a few operating systems’ FAT file
    // system drivers may not function properly if the value is something
    // other than 2. All Microsoft file system drivers will support a value
    // other than 2, but it is still highly recommended that no value other
    // than 2 be used in this field.
    // The reason the standard value for this field is 2 is to provide redun-
    // dancy for the FAT data structure so that if a sector goes bad in one
    // of the FATs, that data is not lost because it is duplicated in the other
    // FAT. On non-disk-based media, such as FLASH memory cards,
    // where such redundancy is a useless feature, a value of 1 may be
    // used to save the space that a second copy of the FAT uses, but
    // some FAT file system drivers might not recognize such a volume
    // properly.
    unsigned char BPB_NumFATs;

    // Root directory entry count
    unsigned short BPB_RootEntCnt;

    // Old 16-bit total count of sectors on the volume.
    unsigned short BPB_TotSec16;

    // Media descriptor
    unsigned char BPB_Media;

    // FAT size (16-bit)
    unsigned short BPB_FATSz16;

    // Sectors per track for interrupt 0x13. This field is only relevant for
    // media that have a geometry (volume is broken down into tracks by
    // multiple heads and cylinders) and are visible on interrupt 0x13.
    // This field contains the “sectors per track” geometry value.
    unsigned short BPB_SecPerTrk;

    // Number of heads for interrupt 0x13. This field is relevant as
    // discussed earlier for BPB_SecPerTrk. This field contains the one
    // based “count of heads”. For example, on a 1.44 MB 3.5-inch floppy
    // drive this value is 2.
    unsigned short BPB_NumHeads;

    // Count of hidden sectors preceding the partition that contains this
    // FAT volume. This field is generally only relevant for media visible
    // on interrupt 0x13. This field should always be zero on media that
    // are not partitioned. Exactly what value is appropriate is operating
    // system specific.
    unsigned int BPB_HiddSec;

    // New 32-bit total count of sectors on the volume.
    // This count includes the count of all sectors in all four regions of the
    // volume. This field can be 0; if it is 0, then BPB_TotSec16 must be
    // non-zero. For FAT32 volumes, this field must be non-zero. For
    // FAT12/FAT16 volumes, this field contains the sector count if
    // BPB_TotSec16 is 0 (count is greater than or equal to 0x10000).
    unsigned int BPB_TotSec32;

    // Union of FAT32-specific entries and FAT16/FAT12-specific entries
    FAT_Type FAT_Type_Specifics;
    long FAT_Type_Val;
    long FirstDataSector;
    long FATSz;
    long RootDirSectors;


    FAT_Sector * sectors;
} FAT_Device;

typedef enum {
	ATTR_READ_ONLY = 0x01,
	ATTR_HIDDEN = 0x02,
	ATTR_SYSTEM = 0x04,
	ATTR_VOLUME_ID = 0x08,
	ATTR_DIRECTORY = 0x10,
	ATTR_ARCHIVE = 0x20,
	ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID,
}FAT_DIR_ENTRY_ATTR;

typedef struct {
	unsigned char DIR_Name[11];
	FAT_DIR_ENTRY_ATTR DIR_Attr;
	unsigned char DIR_NTRes; // must be set to 0
	unsigned char DIR_CrtTimeTenth;
	u16 DIR_CrtTime;
	u16 DIR_CrtDate;
	u16 DIR_LstAccDate;
	u16 DIR_FstClusHI;
	u16 DIR_WrtTime;
	u16 DIR_WrtDate;
	u16 DIR_FstClusLO;
	u32 DIR_FileSize;
}FAT_Dir_Entry;

unsigned int read_bytes_to_int(FILE *fp, int *cursor_loc, int offset_start,
                               int offset, int size);

void read_bytes_to_char_arr(FILE *fp, unsigned char *source, int *cursor,
                            int offset_start, int offset, int size);

int FAT_Device_identify_fat_type_and_init(FAT_Device *d, int *c, FILE *fp);

FAT_Device FAT_Device_init(FILE *fp);

int FAT_Device_first_sector_of_cluster(FAT_Device *d, int n);

int FAT_Device_get_cluster_sector_number(FAT_Device *d, int cluster_number);
int FAT_Device_get_cluster_entry_offset(FAT_Device *d, int cluster_number);

// TODO this could be so much cleaner.
// way too much nesting
void FAT_Device_load_sector(FAT_Device *d, FILE *fp, int sector_num);
// TODO :does not actually read two values. make it do so
unsigned short FAT_Device_12_get_cluster_entry_val(FAT_Device *d,FILE *fp, int cluster_number);

// TODO :does not actually read four values. make it do so
unsigned int FAT_Device_32_get_cluster_entry_val(FAT_Device *d,FILE *fp, int cluster_number);

unsigned short FAT_Device_16_get_cluster_entry_val(FAT_Device *d, FILE *fp,int cluster_number);
void FAT_Device_init_fat_type_specifics(FAT_Device *device, int *cursor, FILE *fp);
void FAT_Device_init_fat12_fat16_bpb(FAT_Device *device, FILE *fp);
void FAT_Device_init_fat32_bpb(FAT_Device *device, FILE *fp);

unsigned int FAT_Device_get_cluster_entry_val(FAT_Device *d, FILE *fp, int cluster_number);

void FAT_Device_Sector_read_to_char_arr(FAT_Device *d, FILE *fp, int sector_number, unsigned char *source, unsigned int start_offset, unsigned int offset, unsigned int size);

unsigned int FAT_Device_Sector_read_to_int(FAT_Device *d, FILE *fp, int sector_number,unsigned int start_offset, unsigned int offset, unsigned int size);

FAT_Dir_Entry FAT_Device_get_dir(FAT_Device *d, FILE *fp, int sector_number, int entry_number);
#endif
