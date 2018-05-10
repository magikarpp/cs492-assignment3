#include <stdlib.h>
#include <stdio.h>

#include "Lfile.h"
#include "LDisk.h"
#include "LinkedList.h"

/*constructor for a new Lfile*/
Lfile makeLfile() {
  Lfile newFile = malloc(sizeof(struct lfile));
  newFile->blocks = makeLinkedList();
  return newFile;
}

void freeLfile(Lfile file) {

  llMap(file->blocks, freeLFnode);
  freeLinkedList(file->blocks);
  file->blocks = NULL;
  free(file);

}

LFnode makeLFnode(size_t start) {

  LFnode newNode = malloc(sizeof(struct ldnode));
  newNode->startPhysicalAddr = start;

  return newNode;

}

void freeLFnode(LFnode node) {

  node->startPhysicalAddr = 0;
  free(node);

}

size_t lfileLastBlockAddr(Lfile file) {

  LFnode node = llElemAt(file->blocks, 0);
  return node->startPhysicalAddr;

}

void lfileAddBlockHelper(Lfile file, size_t physicalAddress) {

  LFnode blockNode = makeLFnode(physicalAddress);
  llAddElem(file->blocks, blockNode);

}

void lfileAddBlock(Lfile file, LDisk disk, size_t physicalAddress) {

  if (file->blocks->length == 0)
    lfileAddBlockHelper(file, physicalAddress);

  else {

    size_t lastBlockPhysAddr = lfileLastBlockAddr(file);

    LDnode lastNode = ldiskGetBlockNode(disk, lastBlockPhysAddr);
    LDnode toAddNode = ldiskGetBlockNode(disk, physicalAddress);

    if (ldnodeAreContiguous(disk, lastNode, toAddNode))
      ldiskJoinContiguousNodes(disk, lastNode, toAddNode, BLOCK_USED);

    else
      lfileAddBlockHelper(file, physicalAddress);

  }
}

int lfileAddBytes(Lfile file, LDisk disk, size_t bytes) {

  long addr;
  if (file->blocks->length == 0)
    addr =  ldiskAlloc(disk, bytes);

  else {

    size_t curAddr = lfileLastBlockAddr(file);
    addr = ldiskRealloc(disk, curAddr, bytes);

  }

  if (addr == INSUFFICIENT_SPACE) {
    return INSUFFICIENT_SPACE;
  }
  else{
    lfileAddBlock(file, disk, (size_t) addr);
  }


  return 0;

}

void lfileRemoveBytes(Lfile file, LDisk disk, size_t bytes) {

  LLNode llNode = file->blocks->head;

  while (llNode != NULL && bytes > 0) {

    LFnode lFnode = (LFnode) llNode->data;
    bytes = ldiskDealloc(disk, file, lFnode, bytes);
    llNode = llNode->next;

  }

  // If there are still bytes to remove, whatever
}

void lfNodePrint(LFnode node) {
  printf("Block address: %li\n", node->startPhysicalAddr);
}

// Printing
void lfilePrint(Lfile file) {
  llMap(file->blocks, lfNodePrint);
}
