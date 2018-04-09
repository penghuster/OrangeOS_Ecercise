#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include "hd.h"
#include "fs.h"

/*
 * strip_path
 *
 * Get the basename from the fullpath 
 *
 * In Orange'S FS v1.0, all files are stored in the root directory. there is no 
 * sub-folder thing.
 *
 * This routine should be called at the very beginning of file operations,
 * such as open(), read() and write(). it accepts the full path and return two
 * things: the basename and a ptr of the root dir's inode.
 *
 * Filename may contain any character except '/' and '\\0'
 *
 * @param[out] filename the string for result
 * @param[in]  pathname The full pathname
 * @param[out] ppinode The ptr of the dir's inode will be stored here.
 *
 * @return Zero if success, otherwise the pathname is invalid.
 */ 
PUBLIC int strip_path(char* filename, const char *pathname, struct inode** ppinode)
{
    const char *s = pathname;
    char *t = filename;

    if(0 == s)
        return -1;
    if(*s == '/')
        s++;

    //check each character
    while(*s)
    {
        if(*s == '/')
            return -1;
        *t++ = *s++;
        //if filename is too long, just truncate it 
        if(t - filename >= MAX_FILENAME_LEN)
            break;
    }
    *t = 0;

    *ppinode = root_inode;
    return 0;
}


/*
 * search_file
 *
 * Search the file and return the inod_nr 
 *
 * @param[in] path the full path of the file to search 
 * 
 * @return Ptr to the i-node of the file if success, otherwise zero
 *
 * @see open()
 * @see do_open()
 */ 
PUBLIC int search_file(char *path)
{
    int i, j;
    
    char filename[MAX_PATH];
    memset(filename, 0, MAX_FILENAME_LEN);
    struct inode *dir_inode;
    if(strip_path(filename, path, &dir_inode) != 0)
        return 0;

    if(filename[0] == 0) //path: "/"
        return dir_inode->i_num;

    //search the dir for the file 
    int dir_blk0_nr = dir_inode->i_start_sect;
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE  - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    
    int m = 0;
    struct dir_entry *pde;
    for(i = 0; i < nr_dir_blks; i++)
    {
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
        pde = (struct dir_entry *)fsbuf;
        
        //此处需要确保 SECTOR_SIZE 是 DIR_ENTRY_SIZE 的整数倍
        for(j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++)
        {
            if(memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
                return pde->inode_nr;

            if(++m > nr_dir_entries)
                break;
        }

        if(m > nr_dir_entries)
            break;
    }

    return 0;
}


