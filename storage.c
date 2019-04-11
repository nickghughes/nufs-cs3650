// implementation of storage.h

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "slist.h"
#include "pages.h"
#include "directory.h"
#include "inode.h"
#include "bitmap.h"

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
storage_write(const char* path, const char* buf, size_t size, off_t offset)
{
    // get the start point with the path
    char* start = pages_get_page(tree_lookup(path));
    if (size + offset > 4096) {
        return -1;
    }
    start = start + offset;
    memcpy(start, buf, size);
    return size;
    
}

int
storage_read(const char* path, const char* buf, size_t size, off_t offset)
{
    inode* node = get_inode(tree_lookup(path));
    
    char* data = pages_get_page(node->ptrs[0]);
    data = data + offset;
    if (size + offset > 4096) {
        size = 4096 - offset;
    }
    memcpy(buf, data, size);
    return size;
}

int 
storage_truncate(const char *path, off_t size) {
    // TODO: This is a pretty vital function
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

// Don't worry about these next four for now
int
storage_unlink(const char* path) {
}

int    storage_link(const char *from, const char *to);
int    storage_rename(const char *from, const char *to);
int    storage_set_time(const char* path, const struct timespec ts[2]);

// TODO: ask what this does
slist* storage_list(const char* path) {
    return directory_list(path);
}

