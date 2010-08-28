 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef PARTITION_HANDLE_H
#define PARTITION_HANDLE_H

#include <gccore.h>
#include <vector>
#include <string>

#define MAX_PARTITIONS          32 /* Maximum number of partitions that can be found */
#define MAX_MOUNTS              10 /* Maximum number of mounts available at one time */
#define MAX_SYMLINK_DEPTH       10 /* Maximum search depth when resolving symbolic links */

#define MBR_SIGNATURE           0x55AA
#define EBR_SIGNATURE           MBR_SIGNATURE

#define PARTITION_BOOTABLE      0x80 /* Bootable (active) */
#define PARTITION_NONBOOTABLE   0x00 /* Non-bootable */

#define BYTES_PER_SECTOR        512 /* Default in libogc */

typedef struct _PARTITION_RECORD {
    u8 status;                              /* Partition status; see above */
    u8 chs_start[3];                        /* Cylinder-head-sector address to first block of partition */
    u8 type;                                /* Partition type; see above */
    u8 chs_end[3];                          /* Cylinder-head-sector address to last block of partition */
    u32 lba_start;                          /* Local block address to first sector of partition */
    u32 block_count;                        /* Number of blocks in partition */
} __attribute__((__packed__)) PARTITION_RECORD;


typedef struct _MASTER_BOOT_RECORD {
    u8 code_area[446];                      /* Code area; normally empty */
    PARTITION_RECORD partitions[4];         /* 4 primary partitions */
    u16 signature;                          /* MBR signature; 0xAA55 */
} __attribute__((__packed__)) MASTER_BOOT_RECORD;

typedef struct _EXTENDED_BOOT_RECORD {
    u8 code_area[446];                      /* Code area; normally empty */
    PARTITION_RECORD partition;             /* Primary partition */
    PARTITION_RECORD next_ebr;              /* Next extended boot record in the chain */
    u8 reserved[32];                        /* Normally empty */
    u16 signature;                          /* EBR signature; 0xAA55 */
} __attribute__((__packed__)) EXTENDED_BOOT_RECORD;

typedef struct _PartitionFS
{
    const char * FSName;
    u32 LBA_Start;
    u32 SecCount;
    bool Bootable;
    u8 PartitionType;
    u8 PartitionNum;
    u32 EBR_Sector;
} PartitionFS;


class PartitionHandle
{
    public:
        //! Constructor reads the MBR and all EBRs and lists up the Partitions
        PartitionHandle(const DISC_INTERFACE *discio);
        //! Destructor unmounts drives
        ~PartitionHandle();
        //! Is Drive inserted
        bool IsInserted() { if(!interface) return false; else return interface->isInserted(); };
        //! Is the partition Mounted
        bool IsMounted(int pos);
        //! Mount a specific Partition
        bool Mount(int pos, const char * name);
        //! UnMount a specific Partition
        void UnMount(int pos);
        //! UnMount all Partition
        void UnMountAll() { for(u32 i = 0; i < PartitionList.size(); ++i) UnMount(i); };
        //! Get the Mountname
        const char * MountName(int pos) { if(pos < 0 || pos >= (int) MountNameList.size() || !MountNameList[pos].size()) return NULL; else return MountNameList[pos].c_str(); };
        //! Get the Name of the FileSystem e.g. "FAT32"
        const char * GetFSName(int pos) { if(valid(pos)) return PartitionList[pos].FSName; else return NULL; };
        //! Get the LBA where the partition is located
        u32 GetLBAStart(int pos) { if(valid(pos)) return PartitionList[pos].LBA_Start; else return 0; };
        //! Get the partition size in sectors of this partition
        u32 GetSecCount(int pos) { if(valid(pos)) return PartitionList[pos].SecCount; else return 0; };
        //! Check if the partition is Active or NonBootable
        bool IsActive(int pos) { if(valid(pos)) return PartitionList[pos].Bootable; else return false; };
        //! Get the partition type
        int GetPartitionType(int pos) { if(valid(pos)) return PartitionList[pos].PartitionType; else return -1; };
        //! Get the entrie number in MBR of this partition
        int GetPartitionNum(int pos) { if(valid(pos)) return PartitionList[pos].PartitionNum; else return -1; };
        //! Get the EBR sector where this partition is described
        int GetEBRSector(int pos) { if(valid(pos)) return PartitionList[pos].EBR_Sector; else return 0; };
        //! Get the count of found partitions
        int GetPartitionCount() { return PartitionList.size(); };
        //! Get the partition size in bytes
        u64 GetSize(int pos) { if(valid(pos)) return (u64) PartitionList[pos].SecCount*BYTES_PER_SECTOR; else return 0; };
        //! Get the whole partition record struct
        PartitionFS * GetPartitionRecord(int pos) { if(valid(pos)) return &PartitionList[pos]; else return NULL; };
        //! Get the disc interface of this handle
        const DISC_INTERFACE * GetDiscInterface() { return interface; };
    protected:
        bool valid(int pos) { return (pos >= 0 && pos < (int) PartitionList.size()); }
        int FindPartitions();
        void CheckEBR(u8 PartNum, sec_t ebr_lba);

        const DISC_INTERFACE *interface;
        std::vector<PartitionFS> PartitionList;
        std::vector<std::string> MountNameList;
};

#endif
