// implementation of storage.h

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "slist.h"
#include "pages.h"
#include "directory.h"
#include "inode.h"
#include "bitmap.h"
#include "util.h"

void
storage_init(const char* path) {
    // initialize the pages
    pages_init(path);
    // allocate a page for the inode list
    if (!bitmap_get(get_pages_bitmap(), 1)) { 
        int newpage = alloc_page();
        printf("second inode page allocated at page %d\n", newpage);
    }
    // the remaining pages will be alloced when we put data in them

    // then we initialize the root directory if it isn't allocated
    if (!bitmap_get(get_pages_bitmap(), 2)) {
        printf("initializing root directory");
        directory_init();
    }
}

// check to see if the file is available, if not returns -ENOENT
int
storage_access(const char* path) {

    int rv = tree_lookup(path);
    if (rv >= 0)
        return 0;
    else
        return -ENOENT;
}

// mutates the stat with the inode features at the path
int 
storage_stat(const char* path, struct stat* st) {
    int working_inum = tree_lookup(path);
    if (working_inum > 0) {
        inode* node = get_inode(working_inum);
        st->st_mode = node->mode;
        st->st_size = node->size;
        return 0;
    }
    return -1;
}

int 
storage_truncate(const char *path, off_t size) {
    int inum = tree_lookup(path);
    inode* node = get_inode(inum);
    if (node->size < size) {
	grow_inode(node, size);
    } else {
	shrink_inode(node, size);
    }
    return 0;
}

int 
storage_write(const char* path, const char* buf, size_t size, off_t offset)
{
    // get the start point with the path
    inode* write_node = get_inode(tree_lookup(path));
    if (write_node->size < size + offset) {
        storage_truncate(path, size + offset);
    }
    int bindex = 0;
    int nindex = offset;
    int rem = size;
    while (rem > 0) {
        char* dest = pages_get_page(inode_get_pnum(write_node, nindex));
        dest += nindex % 4096;
        int cpyamnt = min(rem, 4096 - (nindex % 4096));
        memcpy(dest, buf + bindex, cpyamnt);
        bindex += cpyamnt;
        nindex += cpyamnt;
        rem -= cpyamnt;
    }
    return size;    
}

int
storage_read(const char* path, char* buf, size_t size, off_t offset)
{
    printf("storage_read called, buffer is\n%s\n", buf);
    inode* node = get_inode(tree_lookup(path));
    int bindex = 0;
    int nindex = offset;
    int rem = size;
    while (rem > 0) {
        char* src = pages_get_page(inode_get_pnum(node, nindex));
        src += nindex % 4096;
        int cpyamnt = min(rem, 4096 - (nindex % 4096));
        memcpy(buf + bindex, src, cpyamnt);
        bindex += cpyamnt;
        nindex += cpyamnt;
        rem -= cpyamnt;
    }
    return size;
}

int
storage_mknod(const char* path, int mode) {
    // should add a direntry of the correct mode to the
    // directory at the path
    int new_inode = alloc_inode();
    inode* node = get_inode(new_inode);
    node->mode = mode;
    node->size = 0;
    node->refs = 1;
    inode* parent_dir = get_inode(0);
    char name[strlen(path) - 1];
    for (int ii = 1; ii < strlen(path); ++ii) {
        name[ii] = path[ii];
    }
    directory_put(parent_dir, path+1, new_inode);
    print_directory(node);
    if (new_inode)
        return 0;
    return -1;
}

// this is used for the removal of a link. If refs are 0, then we also
// delete the inode associated with the dirent
int
storage_unlink(const char* path) {
    int    link_inode = tree_lookup(path);
    slist* pathlist = s_split(path, '/');
    char*  parentpath = s_reconstruct(s_droplast(pathlist), '/');
    inode* parent = get_inode(tree_lookup(parentpath));
    char*  nodename = s_getlast(pathlist);

  
    int rv = directory_delete(parent, nodename);

    s_free(pathlist);
    free(parentpath);
    free(nodename);

    return rv;
}

int    
storage_link(const char *from, const char *to) {
    int tnum = tree_lookup(to);
    if (tnum < 0) {
        return tnum;
    }
    //some real nonsense to figure out the parent dir of the from path
    slist* flist = s_split(from, '/');
    slist* fdir = flist;
    char* fpath = malloc(strlen(from));
    fpath[0] = 0;
    while (fdir->next != NULL) {
        strncat(fpath, "/", 1);
        strncat(fpath, fdir->data, 48);
        fdir = fdir->next;
    }
    //look up the directory
    int dir = tree_lookup(fpath);
    if (dir < 0) {
        s_free(flist);
        free(fpath);
        return dir;
    }
    directory_put(get_inode(dir), fdir->data, tnum);
    s_free(flist);
    free(fpath);
    return 0;
}

int
storage_symlink(const char* to, const char* from) {
    int rv = storage_mknod(from, 0120000);
    if (rv < 0 ) {
        return rv;
    }
    storage_write(from, to, strlen(to), 0);
    return 0;
}

int
storage_readlink(const char* path, char* buf, size_t size) {
    return storage_read(path, buf, size, 0);
}

int    storage_rename(const char *from, const char *to);
int    storage_set_time(const char* path, const struct timespec ts[2]);

slist* storage_list(const char* path) {
    return directory_list(path);
}

