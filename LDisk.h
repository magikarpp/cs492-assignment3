#ifndef FILESYSTEM_LDISK_H
#define FILESYSTEM_LDISK_H

#include "LinkedList.h"

#define BLOCK_FREE 0
#define BLOCK_USED 1
#define INSUFFICIENT_SPACE (-2)
#define NO_ALLOC_NEEDED (-1)

#include "datatypes.h"

// GLOBALS
LDisk DISK;

// FUNCTIONS


LDisk makeLDisk(size_t blockSize, size_t maxSize);

void freeLDisk(LDisk disk);

/*constructor for a new node*/
LDnode makeLDnode(unsigned long blockIDStart, unsigned long numIDs, int status);

void freeLDnode(LDnode node);

/*size of free blocks calculated as x * block_size,
where block_size is specified by the parameter -b */
size_t ldiskSizeOfFreeBlocks(LDisk disk);

LDnode ldiskGetBlockNode(LDisk disk, size_t physicalAddr);

void ldiskJoinContiguousNodes(LDisk disk, LDnode node, LDnode other, int status);

int ldnodeAreContiguous(LDisk disk, LDnode node, LDnode other);

/**
 * Size of free blocks on one node
 * @param node
 * @return
 */
size_t ldnodeNumFreeBlocks(LDnode node);

/**
 * Allocate space the disk
 * @param bytes
 */
long ldiskAlloc(LDisk disk, size_t bytes);

long ldiskRealloc(LDisk disk, size_t curAddr, size_t bytes);

/**
 * Deallocates max num bytes from phys addr
 * @param disk
 * @param physicalAddr
 * @param bytes
 * @returns The remainder if bytes > total size in address
 */
size_t ldiskDealloc(LDisk disk, Lfile file, LFnode lFnode, size_t bytes);

void ldiskPrintFootprint(LDisk disk);

int ldnodeCompareNodes(LDnode node, LDnode other);

int ldnodeCompareNodesToBlockId(LDnode node, size_t *id);

#endif // FILESYSTEM_LDISK_H