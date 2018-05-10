#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include "DTree.h"
#include "Queue.h"
#include "LDisk.h"

DTNode makeDTNode(char *name, struct tm *timestamp, DTFileAttrs attrs) {

  DTNode node = (DTNode) malloc(sizeof(struct d_t_node));

  node->attrs = attrs;
  node->parent = NULL;
  if (attrs == NULL)
    //if it's a dir
    node->children = makeLinkedList();
  else
    // if its a file
    node->children = NULL;

  node->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(node->name, name);

  node->absPath = NULL;

  // Copy the timestamp or make a new one with the current time
  node->timestamp = (struct tm *) malloc(sizeof(struct tm));
  if (timestamp == NULL) {
    // If null, make it the current time!
    dtreeUpdateTimestamp(node);
  } else {
    node->timestamp->tm_sec = timestamp->tm_sec;
    node->timestamp->tm_min = timestamp->tm_min;
    node->timestamp->tm_hour = timestamp->tm_hour;
    node->timestamp->tm_mday = timestamp->tm_mday;
    node->timestamp->tm_mon = timestamp->tm_mon;
    node->timestamp->tm_year = timestamp->tm_year;
    node->timestamp->tm_wday = timestamp->tm_wday;
    node->timestamp->tm_yday = timestamp->tm_yday;
    node->timestamp->tm_isdst = timestamp->tm_isdst;
  }

  return node;
}

void freeDTNode(DTNode node) {
  // If it's a file
  if (dtreeIsFile(node)) {
    // Free file attributes
    freeDTFileAttrs(node->attrs);
    node->attrs = NULL;
  } else {
    freeLinkedList(node->children);
    node->children = NULL;
  }

  node->parent = NULL;
  // Name is allocated
  free(node->name);
  node->name = NULL;
  free(node->absPath);
  node->absPath = NULL;

  free(node->timestamp);
  node->timestamp = NULL;

  free(node);
}

DTFileAttrs makeDTFileAttrs(size_t size, Lfile file) {
  DTFileAttrs attrs = (DTFileAttrs) malloc(sizeof(struct d_t_file_attrs));
  attrs->size = size;

  if (file == NULL) {
    attrs->file = makeLfile();
  } else {
    attrs->file = file;
  }
  return attrs;
}

void freeDTFileAttrs(DTFileAttrs attrs) {
  attrs->size = 0;
  freeLfile(attrs->file);
  attrs->file = NULL;
  free(attrs);
}

void dtreeInitRoot(DTNode root) {
  char *absPath = malloc(sizeof(char) * (strlen(root->name) + (PATH_SEPARATOR_LEN * 2) + 1));
  strcpy(absPath, PATH_SEPARATOR_PTR);
  strcat(absPath, root->name);
  root->absPath = absPath;
  // Set the global
  ABS_DIR_ROOT = root;
}

void dtreeAdd(DTNode parent, DTNode child) {
  child->parent = parent;

  // Store the absolute path
  //    parentAbs/childName
  size_t parentAbsLen = strlen(parent->absPath);
  char *absPath = malloc(sizeof(char) * (parentAbsLen + PATH_SEPARATOR_LEN + strlen(child->name) + 1));
  strcpy(absPath, parent->absPath);
  strcat(absPath, PATH_SEPARATOR_PTR);
  strcat(absPath, child->name);

  child->absPath = absPath;

  llAddElem(parent->children, child);
}

/**
 * Removes node and everything underneath it
 */
void dtreeRecursiveRemove(DTNode node) {
  // Remove from parent's children list
  if (node->parent != NULL) {
    llRemoveElem(node->parent->children, node, dtreeCompareNodes);
  }

  if (!dtreeIsFile(node)) {
    llMap(node->children, dtreeRecursiveRemove);
  }

  // Free Node
  freeDTNode(node);
}

void dtreeTraverseApplyDF(DTNode root, void func(DTNode node)) {
  // Null is base case
  if (root != NULL) {
    func(root);

    // Recursively apply to all children and traverse
    LLNode curNode = root->children->head;
    while (curNode != NULL) {
      dtreeTraverseApplyDF(curNode->data, func);
      curNode = curNode->next;
    }
  }
}

void dtreeTraverseApplyBF(DTNode root, void func(DTNode node)) {
  Queue queue = makeQueue();
  enqueue(queue, root);

  while (nonempty(queue)) {
    DTNode node = dequeue(queue);
    func(node);
    // Enqueue all children
    if (node->children != NULL) {
      LLNode traverseNode = node->children->head;
      while (traverseNode != NULL) {
        enqueue(queue, traverseNode->data);
        traverseNode = traverseNode->next;
      }
    }
  }

  freeQueue(queue);
}

void *dtreeSizeHelper(DTNode node, int *acc) {

  //fold kinda
  if (node == NULL)
    return acc;

  *acc += 1;
  if (!dtreeIsFile(node))
    llFold(node->children, acc, dtreeSizeHelper);

  return acc;
}

int dtreeSize(DTNode root) {
  int baseSize = 0;
  dtreeSizeHelper(root, &baseSize);
  return baseSize;
}

int dtreeIsEmpty(DTNode root) {
  return root->children->length == 0;
}

void dtreeUpdateTimestamp(DTNode node) {
  time_t currenttime = time(NULL);
  localtime_r(&currenttime, node->timestamp);
}


int dtreeIsFile(DTNode node) {
  return node->children == NULL;
}

long dtreeAppendToFile(DTNode fileNode, LDisk disk, size_t bytes) {

  long ret = lfileAddBytes(fileNode->attrs->file, disk, bytes);

  if (ret != INSUFFICIENT_SPACE) {
    //update size
    fileNode->attrs->size += bytes;
    dtreeUpdateTimestamp(fileNode);

  }

  return ret;

}

void dtreeRemoveFromFile(DTNode fileNode, LDisk disk, size_t bytes) {
  lfileRemoveBytes(fileNode->attrs->file, disk, bytes);
  if (bytes > fileNode->attrs->size) {
    fileNode->attrs->size = 0;
  } else {
    fileNode->attrs->size -= bytes;
  }
  dtreeUpdateTimestamp(fileNode);
}

/**
 * Does not support modifiers like: { . .. ... } etc
 * @param root
 * @param path
 * @return
 */
DTNode dtreeGetParentByAbsPath(DTNode root, char *path) {
  char *toFree = (char *) malloc(sizeof(char) * (strlen(path) + 1));
  char *nextSubPath = toFree;
  strcpy(nextSubPath, path);

  // Chop off the root path
  char *nextSep = strchr(nextSubPath, PATH_SEPARATOR);
  nextSubPath = nextSep + sizeof(char);

  DTNode searchNode = root;
  DTNode toReturn = NULL;
  while (searchNode != NULL) {

    // if the next name is the final name, found it
    if (strchr(nextSubPath, PATH_SEPARATOR) == NULL) {
      toReturn = searchNode;
      break;
    }

    nextSep = strchr(nextSubPath, PATH_SEPARATOR);
    size_t baseLen = nextSep - nextSubPath;
    char baseName[baseLen + 1];
    strncpy(baseName, nextSubPath, baseLen);
    baseName[baseLen] = '\0';

    // otherwise keep going down the tree
    nextSubPath = nextSep + sizeof(char); // skip the separator
    searchNode = dtreeGetChildByName(searchNode, baseName);
  }

  free(toFree);
  return toReturn;
}

DTNode dtreeGetChildByName(DTNode root, char *name) {
  return llElemSearch(root->children, name, dtreeCompareNodeToName);
}

// Printing Functions
void dtreePrintNode(DTNode node) {
  printf("%s\n", node->absPath);
  fflush(stdout);
}

void dtreePrintDF(DTNode root) {
  dtreeTraverseApplyDF(root, dtreePrintNode);
}

void dtreePrintBF(DTNode root) {
  dtreeTraverseApplyBF(root, dtreePrintNode);
}

void dtreePrintFile(DTNode node) {
  char timeStr[80] = {0};
  strftime(timeStr, 80, "%b %d %H:%M", node->timestamp);
  printf("%li %s ", node->attrs->size, timeStr);
  printf("\n");
  lfilePrint(node->attrs->file);
}

// COMPARITORS

/**
 *
 * @param node
 * @param otherNode
 * @return
 */
int dtreeCompareNodes(DTNode node, DTNode otherNode) {
  return strcmp(node->absPath, otherNode->absPath) == 0;
}

int dtreeCompareNodeToName(DTNode node, char *name) {
  return strcmp(node->name, name) == 0;
}
