#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "LDisk.h"
#include "Lfile.h"

/*new LDisk*/
LDisk makeLDisk(size_t blockSize, size_t maxSize) {

  LDisk newDisk = malloc(sizeof(struct ldisk));

  newDisk->blocks = makeLinkedList();
  newDisk->blockSize = blockSize;
  newDisk->maxSize = maxSize;
  newDisk->maxBlocks = maxSize / blockSize;

  // Init one node will all free
  // as disk is allocated node is split free / used
  LDnode initNode = makeLDnode(0, newDisk->maxBlocks, BLOCK_FREE);
  llAddElem(newDisk->blocks, initNode);

  return newDisk;

}

void freeLDisk(LDisk disk) {

  llMap(disk->blocks, freeLDnode);
  freeLinkedList(disk->blocks);
  disk->blockSize = 0;
  disk->maxBlocks = 0;
  disk->maxSize = 0;
  free(disk);

}

//new node constructor
LDnode makeLDnode(unsigned long blockIDStart, unsigned long numIDs, int status) {

  LDnode newNode = (LDnode) malloc(sizeof(struct ldnode));
  newNode->blockIDs = (unsigned long *) malloc(sizeof(unsigned long) * (numIDs + 1));

  for (unsigned long i = 0; i < numIDs; i++) 
    newNode->blockIDs[i] = blockIDStart + i;
  
  newNode->numBlockIDs = numIDs;
  newNode->status = status;
  newNode->blockIDStart = blockIDStart;
  newNode->fragmentation = 0;

  return newNode;

}

void freeLDnode(LDnode node) {

  free(node->blockIDs);
  node->status = 0;
  node->numBlockIDs = 0;
  node->blockIDStart = 0;
  node->fragmentation = 0;
  free(node);

}

void *ldiskFragSizeHelper(LDnode node, size_t *acc) {

  *acc += node->fragmentation;
  return acc;

}

size_t ldiskFragSize(LDisk disk) {

  size_t size = 0;
  llFold(disk->blocks, &size, ldiskFragSizeHelper);
  return size;

}

size_t ldnodeNumFreeBlocks(LDnode node) {

  if (node->status == BLOCK_FREE) 
    return node->numBlockIDs;
  else 
    return 0;
  
}

void *ldiskNumFreeBlocksHelper(LDnode node, int *acc) {

  if (node->status == BLOCK_FREE) 
    *acc += node->numBlockIDs;
  
  return acc;

}

int ldiskNumFreeBlocks(LDisk disk) {
  int numFree = 0;
  /*count num free */
  llFold(disk->blocks, &numFree, ldiskNumFreeBlocksHelper);
  return numFree;
}

//size of free blocks is x * block_size (block_size is parameter -b) 
size_t ldiskSizeOfFreeBlocks(LDisk disk) {
  return ldiskNumFreeBlocks(disk) * disk->blockSize;
}

LDnode ldiskGetBlockNode(LDisk disk, size_t physicalAddr) {
  // Block id
  size_t blockId = physicalAddr / disk->blockSize;

  return llElemSearch(disk->blocks, &blockId, ldnodeCompareNodesToBlockId);

}

long ldiskRealloc(LDisk disk, size_t curAddr, size_t bytes) {

  // Try to fill up the fragmentation first
  // If not, more space

  LDnode curNode = ldiskGetBlockNode(disk, curAddr);

  curNode->fragmentation = 0;

  if (curNode->fragmentation > bytes)
    return NO_ALLOC_NEEDED;

  else {
    // Fill up rest and allocate more
    size_t remaining = bytes - curNode->fragmentation;
    return ldiskAlloc(disk, remaining);
  }

}

/**
 *  returns INSUFFICIENT_SPACE for error
 *  or physical addr
 * @param disk
 * @param bytes
 * @return
 */
long ldiskAlloc(LDisk disk, size_t bytes) {
  // Traverse nodes, looking for enough blocks
  long toReturn = INSUFFICIENT_SPACE;
  LLNode curNode = disk->blocks->head;
  size_t numBlocksNeeded = (size_t) ceil((double) bytes / (double) disk->blockSize);
  size_t fragmentation = (numBlocksNeeded * disk->blockSize) - bytes;
  int index = 0;
  while (curNode != NULL) {
    if (ldnodeNumFreeBlocks(curNode->data) >= numBlocksNeeded) {
      // Split node into two nodes
      LDnode toSplit = curNode->data;
      size_t usedIdStart = toSplit->blockIDStart;
      size_t stillFreeIdStart = toSplit->blockIDs[numBlocksNeeded] + 1;
      LDnode usedNode = makeLDnode(usedIdStart, numBlocksNeeded, BLOCK_USED);
      // Store the amount of fragmentation
      usedNode->fragmentation = fragmentation;

      size_t numStillFree = toSplit->numBlockIDs - numBlocksNeeded;
      if (numStillFree > 0) {
        // Don't allow overflow
        if (stillFreeIdStart + numStillFree >= disk->maxBlocks) {
          numStillFree = disk->maxBlocks - stillFreeIdStart - 1;
        }
        LDnode freeNode = makeLDnode(stillFreeIdStart, numStillFree, BLOCK_FREE);
        // Now replace those in the block list
        curNode->data = freeNode;

        // Add the used node right before
        llAddElemAt(disk->blocks, usedNode, index);
      } else {
        // Otherwise no more free, just replace
        curNode->data = usedNode;
      }

      // Free the old one
      freeLDnode(toSplit);

      // return the physical address
      toReturn = usedIdStart * disk->blockSize;
      break;
    }

    curNode = curNode->next;
    index++;
  }

  return toReturn;
}

void ldiskJoinContiguousNodes(LDisk disk, LDnode prev, LDnode next, int status) {

  LDnode joinNode = makeLDnode(prev->blockIDStart, prev->numBlockIDs + next->numBlockIDs, status);

  // Need to maintain fragmentation if BLOCK USED
  size_t frag = 0;
  if (status == BLOCK_USED) {
    // This works because when called, previous should be 100% full
    // And fragmentation will only occur from the next block
    frag = next->fragmentation;
  }
  joinNode->fragmentation = frag;

  // Remove both the prev from disk block's list
  int index = llIndexOf(disk->blocks, prev, ldnodeCompareNodes);
  llRemoveNodeAt(disk->blocks, index);
  // Replace at the index
  llReplaceAt(disk->blocks, joinNode, index);

  // Free both
  freeLDnode(prev);
  freeLDnode(next);
}

int ldnodeAreContiguous(LDisk disk, LDnode node, LDnode other) {

  return llIndexOf(disk->blocks, node, ldnodeCompareNodes) == llIndexOf(disk->blocks, other, ldnodeCompareNodes) - 1;

}


void ldiskJoinFreeBlocksHelper(LDisk disk, LDnode prev, LDnode next) {
  // If both are free, join em
  if (prev->status == BLOCK_FREE && next->status == BLOCK_FREE) 
    ldiskJoinContiguousNodes(disk, prev, next, BLOCK_FREE);
  
}

void ldiskJoinFreeBlocks(LDisk disk) {
  // Join all the free contiguous blocks
  LLNode llNode = disk->blocks->head;

  while (llNode != NULL && llNode->next != NULL) {

    ldiskJoinFreeBlocksHelper(disk, llNode->data, llNode->next->data);
    llNode = llNode->next;

  }

}

size_t ldiskDealloc(LDisk disk, Lfile file, LFnode lFnode, size_t bytes) {

  // Free max amount bytes
  LDnode nodeToSplit = ldiskGetBlockNode(disk, lFnode->startPhysicalAddr);
  int toSplitIndex = llIndexOf(disk->blocks, nodeToSplit, ldnodeCompareNodes);

  size_t totalBytes = nodeToSplit->numBlockIDs * disk->blockSize;
  size_t totalBytesUsed = totalBytes - nodeToSplit->fragmentation;
  size_t removeRemainder = 0;

  size_t freeStartID;
  size_t numFreeBlocks;
  LDnode freeNode;
  // Split the block into used / free, or just free
  // Then remove node to split and add the used -> free
  if (bytes < totalBytesUsed) {
    // Some of the current node will still be used, need to split
    removeRemainder = 0;
    size_t nodeRemainder = totalBytesUsed - bytes;
    size_t numBlocksNeeded = (size_t) ceil((double) nodeRemainder / (double) disk->blockSize);
    LDnode usedNode = makeLDnode(nodeToSplit->blockIDStart, numBlocksNeeded, BLOCK_USED);
    usedNode->fragmentation = (numBlocksNeeded * disk->blockSize) - nodeRemainder;
  
    freeStartID = nodeToSplit->blockIDStart + numBlocksNeeded + 1;
    numFreeBlocks = nodeToSplit->numBlockIDs - numBlocksNeeded;
    freeNode = makeLDnode(freeStartID, numFreeBlocks, BLOCK_FREE);

    llReplaceAt(disk->blocks, usedNode, toSplitIndex);
    llAddElemAt(disk->blocks, freeNode, toSplitIndex + 1);
  
    freeLDnode(nodeToSplit);

    // lFnode will have the same starting address
  } else {
    
    removeRemainder = totalBytesUsed - bytes;
   
    nodeToSplit->status = BLOCK_FREE;
    nodeToSplit->fragmentation = 0;
  
    llRemoveElem(file->blocks, lFnode, llPointerCompare);
    freeLFnode(lFnode);

  }

  // Join all the free blocks if there are any. Could do this more simply here
  ldiskJoinFreeBlocks(disk);

  return removeRemainder;
}

// Printing

void ldiskPrintFootprintHelper(LDnode node) {
  if (node->status == BLOCK_USED) {
    printf("In use: ");
  } else {
    printf("Free: ");
  }
  printf("%li-%li\n", node->blockIDStart, node->blockIDStart + node->numBlockIDs);
}

void ldiskPrintFootprint(LDisk disk) {
  printf("Num blocks: %li\n", disk->maxBlocks);
  printf("Num free blocks: %i\n", ldiskNumFreeBlocks(disk));
  llMap(disk->blocks, ldiskPrintFootprintHelper);
  printf("Fragmentation: %li\n", ldiskFragSize(disk));
}

int ldnodeCompareNodes(LDnode node, LDnode other) {

  return node == other;
  
}

/**
 * Check if node contains id
 * @param node
 * @param id
 * @return
 */
int ldnodeCompareNodesToBlockId(LDnode node, size_t *id) {

  size_t maxId = node->blockIDStart + node->numBlockIDs;
  return *id >= node->blockIDStart && *id <= maxId;

}