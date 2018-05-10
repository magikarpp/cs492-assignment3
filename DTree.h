//Hierarchical Directory Tree G

#ifndef FILESYSTEM_DTREE_H
#define FILESYSTEM_DTREE_H

#include <sys/types.h>
#include <time.h>
#include "Lfile.h"
#include "LinkedList.h"

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_PTR "\\"
#define PATH_SEPARATOR_LEN 2
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_PTR "/"
#define PATH_SEPARATOR_LEN 1
#endif

// GLOBALS
DTNode ABS_DIR_ROOT;
DTNode PWD;

// FUNCTIONS

/**
 * Make a Directory Tree Node
 * @param name
 * @param attrs - nullable
 * @param left
 * @param right
 * @return
 */
DTNode makeDTNode(char* name, struct tm *timestamp, DTFileAttrs attrs);

void freeDTNode(DTNode node);

/**
 * File attributes are for file nodes only
 * @param size
 * @param timestamp
 * @param file
 * @return
 */
DTFileAttrs makeDTFileAttrs(size_t size, Lfile file);

void freeDTFileAttrs(DTFileAttrs attrs);

/**
 * Initializes the global root of the file system
 * @param root
 */
void dtreeInitRoot(DTNode root);

/**
 * Adds the child node to a parent
 * @param upper
 * @param lower
 */
void dtreeAdd(DTNode parent, DTNode child);

/**
 * Removes node and all childrenz
 */
void dtreeRecursiveRemove(DTNode node);

/**
 * Depth First
 * Traverse over the tree, applying func to each node
 * @param root
 * @param func
 */
void dtreeTraverseApplyDF(DTNode root, void func(DTNode node));
/**
 * Breadth First
 * Traverse over the tree, applying func to each node
 * @param root
 * @param func
 */
void dtreeTraverseApplyBF(DTNode root, void func(DTNode node));

/**
 * The total number of nodes in the tree
 * @param root
 * @return
 */
int dtreeSize(DTNode root);

int dtreeIsEmpty(DTNode root);

int dtreeIsFile(DTNode node);

long dtreeAppendToFile(DTNode fileNode, LDisk disk, size_t bytes);

void dtreeRemoveFromFile(DTNode fileNode, LDisk disk, size_t bytes);

void dtreeUpdateTimestamp(DTNode node);

/**
 * Search for a child of the root by the child's name
 * @param root
 * @param name
 * @return
 */
DTNode dtreeGetChildByName(DTNode root, char*name);

/**
 * Parses the path and traverses the tree
 * @param root
 * @param path
 * @return
 */
DTNode dtreeGetParentByAbsPath(DTNode root, char *path);

void dtreePrintNode(DTNode node);

void dtreePrintDF(DTNode root);

void dtreePrintBF(DTNode root);

void dtreePrintFile(DTNode node);

int dtreeCompareNodes(DTNode node, DTNode otherNode);

int dtreeCompareNodeToName(DTNode node, char* name);

#endif //FILESYSTEM_DTREE_H
