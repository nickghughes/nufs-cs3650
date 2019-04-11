// Implementation of directory.h

#include "slist.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"
#include <string.h>
#include <stdio.h>

/*
typedef struct dirent {
    char name[DIR_NAME]; // The name of the directory
    int  inum; // the number of the inode in the inode table?
    char _reserved[12]; // What the fuck is this??
} dirent; */


void directory_init() {
    // we already have our page for the inodes allocated
    // the root inode will be node 0
    inode* rootnode = get_inode(alloc_inode());
    rootnode->mode = 040755;
}

int 
directory_lookup(inode* dd, const char* name) {
    printf("looking for \"%s\" in dirlookup\n", name);
    if (!strcmp(name, "")) {
        printf("this is root, returning 0\n");
        return 0; 
    }
    else if (dd->mode == 040755) { // dd is a directory, we can go
                                   // to the pointer and find a dirent*
        dirent* subdirs = pages_get_page(dd->ptrs[0]);
        for (int ii = 0; ii < 64; ++ii) {
            dirent csubdir = subdirs[ii];
            if (strcmp(name, csubdir.name) == 0) {
                printf("found a match! returning %d\n", csubdir.inum);
                return csubdir.inum;
            }
        }
        // if we can't find something that matches the name, return -1
        return -1;
    }
    else { // dd is a file, we return negative 1
        printf("not a directory, returning -1\n");
        return -1;
    }
}

// not sure if that's any different but imma use this to parse
int 
tree_lookup(const char* path) {
    int ii = 0;
    int jj = 0;
    int curnode = 0;
    
    // creates an slist with all the dir names and the file name
    slist* pathlist = s_split(path, '/');
    slist* currdir = pathlist;
    while (currdir != NULL) {
        // we look for the name of the next dir in the current one
        curnode = directory_lookup(get_inode(curnode), currdir->data);
        if (curnode == -1) {
            s_free(pathlist);
            return -1;
        }
        currdir = currdir->next;
    }
    s_free(pathlist);
    printf("tree lookup: %s is at node %d\n", path, curnode);
    return curnode;
}
    
// puts a new directory entry into the dir at dd that points to inode inum
int directory_put(inode* dd, const char* name, int inum) {
    int numentries = dd->size / sizeof(dirent);
    dirent* entries = pages_get_page(dd->ptrs[0]);
    dirent new;
    strncpy(new.name, name, DIR_NAME); 
    new.inum = inum;
    entries[numentries] = new;
    dd->size = dd->size + sizeof(dirent);
    printf("running dir_put, putting %s, inum %d, on page %d\n", name, inum, dd->ptrs[0]);
    return 0;
}

// should this free the inode? I think so...
int directory_delete(inode* dd, const char* name);

// list of directories where? at the path? wait... this is for ls
slist* directory_list(const char* path) {
    int working_dir = tree_lookup(path);
    inode* w_inode = get_inode(working_dir);
    int numdirs = w_inode->size / sizeof(dirent);
    dirent* dirs = pages_get_page(w_inode->ptrs[0]);
    slist* dirnames = NULL; 
    for (int ii = 0; ii < numdirs; ++ii) {
        dirnames = s_cons(dirs[ii].name, dirnames);
    }
    return dirnames;
}


void print_directory(inode* dd) {
    int numdirs = dd->size / sizeof(dirent);
    dirent* dirs = pages_get_page(dd->ptrs[0]);
    for (int ii = 0; ii < numdirs; ++ii) {
        printf("%s\n", dirs[ii].name);
    }
}

