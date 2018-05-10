
#ifndef FILESYSTEM_DATATYPES_H
#define FILESYSTEM_DATATYPES_H

#include <sys/types.h>
#include "LinkedList.h"

// LDISK

typedef struct ldnode *LDnode;
struct ldnode {
  unsigned long *blockIDs; //not physical address
  unsigned long numBlockIDs; // Numof blockIDs
  int status; //0-free 1-used
  unsigned long blockIDStart;
  size_t fragmentation;
};

typedef struct ldisk *LDisk;
struct ldisk {
  LinkedList blocks;
  size_t blockSize;
  size_t maxSize;
  size_t maxBlocks;
};

// LFILE

typedef struct lfnode *LFnode;
struct lfnode {
  size_t startPhysicalAddr; //not the block ID
};

typedef struct lfile *Lfile;
struct lfile {
  LinkedList blocks;
};

// DTREE

typedef struct d_t_node *DTNode;
typedef struct d_t_file_attrs *DTFileAttrs;

struct d_t_node {

  char *name;
  char *absPath;
  struct tm *timestamp;

  // For files, otherwise null
  DTFileAttrs attrs;
  DTNode parent;
  LinkedList children;
};

struct d_t_file_attrs {
  size_t size;
  Lfile file;
};

#endif //FILESYSTEM_DATATYPES_H
