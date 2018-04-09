#ifndef _ORANGES_FS_H_
#define _ORANGES_FS_H_

/*
 * @struct dev_drv_map fs.h "include/sys/fs.h"
 * @brief the Device_nr.\ - Driver_nr.\ MAP
 *
 */ 
typedef struct dev_drv_map
{
    int driver_nr;  /*the proc nr of the device driver*/ 
}DEV_DRV_MAP;


/*
 * @def MAGIC_V1
 * @brief Magic number of FS v1.0
 */ 
#define MAGIC_V1 0x111

/*
 * @struct super_block fs.h  "include/fs.h"
 * @brief  the 2nd sector of the FS 
 *
 * Remember to change SUPER_BLOCK_SIZE if the members are changed.
 *
 * @attention Remember to change boot/include/load,inc::SB_*  if the members are changed 
 */ 
struct super_block 
{
    u32 magic; //magic number 
    u32 nr_inodes; //how many inodes 
    u32 nr_sects; //how many sectors (including bit maps)
    u32 nr_imap_sects; //how many inode-map sectors
    u32 nr_smap_sects; //how many sector-map sectors 
    u32 n_1st_sect;  //Number of the 1st data sector 

    u32 nr_inode_sects; //how many inode sectors 
    u32 root_inode;  //inode nr of root directory
    u32 inode_size; //inode size 
    u32 inode_isize_off; // offset of 'struct inode::i_size'
    u32 inode_start_off; //offset of 'struct inode::i_start_sect'
    u32 dir_ent_size; //DIR_ENTRY_SIZE 
    u32 dir_ent_inode_off; //Offset of "struct dir_entry::inode_nr
    u32 dir_ent_fname_off; // offset of 'struct dir_entry::name'

    //the following items are only present in memory
    int sb_dev; //the super block's home device 

};

/*
 * @def SUPER_BLK_MAGIC_V1 
 * @brief The size of super block 
 *
 * Note that this is the size of the struct in the device, not in memory.
 * The size in memory is larger because of some more members.
 */ 
#define SUPER_BLOCK_SIZE  56


/*
 * @struct inode 
 * @brief i-node 
 *
 * The file, currently, have tree valid attributes only:
 *  - size 
 *  - start_sect
 *  - nr_sects 
 *
 *  The start_sect and nr_sects locate the file in the device,
 *  and the size show how many bytes is used 
 *  If size (nr_sects * SECTOR_SIZE), the rest bytes are wasted and 
 *  reserved for later writing.
 *
 *  NOTE: Remember to change INODE_SIZE if the members are changed. 
 */ 
struct inode 
{
    u32 i_node; //Access mode. unused currently 
    u32 i_size;  //File size 
    u32 i_start_sect; //The first sector of the data 
    u32 i_nr_sects; //how many sector the file occupies 
    u8 _unused[16];  //Stuff for alignment 

    //the following items are only present in memory
    int i_dev;
    int i_cnt; //How many procs share this node 
    int i_num; //inode nr.  
};

/*
 * @def INODE_SIZE
 * @brief The size of i-node stored in the device 
 *
 * Note that this is the size of the struct in the device. Not in memory.
 * The size in memory is larger because of some moe members.
 */ 
#define INODE_SIZE   32 

/*
 * @struct file_desc
 * @brief File descriptor 
 */ 
struct file_desc
{
    int fd_mode; //R or W 
    int fd_pos; //Current position for R/W 
    struct inode* fd_inode;  // Ptr to the i-node 
};

/*
 * Since all invocations of 'rw_sector()' in FS look similar (most of the 
 * params are the same), we use this macro to make code more readable.
 *
 * Before I write this macro. I found almost every rw_sector invocation 
 * line matchs this emacs-style regex:
 */ 
#define RD_SECT(dev, sect_nr) rw_sector(DEV_READ, \
        dev, \
        (sect_nr) * SECTOR_SIZE, \
        SECTOR_SIZE, /*read one sector*/ \
        TASK_FS, \
        fsbuf);

#define WR_SECT(dev, sect_nr) rw_sector(DEV_WRITE, \
        dev,  \
        (sect_nr) * SECTOR_SIZE,  \
        SECTOR_SIZE, \
        TASK_FS, \
        fsbuf);

#endif
