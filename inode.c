// inode implementation

#include <stdio.h>
#include <stdlib.h>

#include "inode.h"
#include "pages.h"
#include "bitmap.h"

/* Definition of the inode structure:

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[2]; // direct pointers
    int iptr; // single indirect pointer
} inode; */

void 
print_inode(inode* node) {
    printf("inode located at %p:\n", node);
    printf("Reference count: %d\n", node->refs);
    printf("Node Permission + Type: %d\n", node->mode);
    printf("Node size in bytes: %d\n", node->size);
    printf("Node direct pointers: %d, %d\n", node->ptrs[0], node->ptrs[1]);
    printf("Node indirect pointer: %d\n", node->iptr);
}

inode* 
get_inode(int inum) {
    inode* inodes = get_inode_bitmap() + 32;
    return &inodes[inum];
}

// take a look at the malloc implementation
int 
alloc_inode() {
    int nodenum;
    for (int ii = 0; ii < 256; ++ii) {
        if (!bitmap_get(get_inode_bitmap(), ii)) {
            bitmap_put(get_inode_bitmap(), ii, 1);
            nodenum = ii;
            break;
        }
    }
    inode* new_node = get_inode(nodenum);
    new_node->refs = 1;
    new_node->size = 0;
    new_node->mode = 0;
    new_node->ptrs[0] = alloc_page();
    return nodenum;
}

// marks the inode as free in the bitmap and then clears the pointer locations
void 
free_inode() {
    // take a look at malloc for the bitmap
    

}

// grows the inode, if size gets too big, it allocates a new page if possible
int grow_inode(inode* node, int size) {
    if (size > 4096) {
        // we will want to allocate pages for the challenge, but for now, error
        return -1;
    }
   node->size = size;
   return node->ptrs[0];
}

// shrinks an inode size and deallocates pages if we've freed them up
int shrink_inode(inode* node, int size);

// gets the page number for the inode
int inode_get_pnum(inode* node, int fpn);

