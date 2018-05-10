

#ifndef FILESYSTEM_LINKEDLIST_H
#define FILESYSTEM_LINKEDLIST_H

typedef struct linked_list *LinkedList;
typedef struct ll_node *LLNode;
typedef int (*Comparitor)(void (*other), void (*elem));

struct linked_list {

  LLNode head;
  int length;

};

struct ll_node {

  void *data;
  LLNode next;

};

/**
 * Make a linked list by passing a
 * @param compare - function to compare two elements in the list
 */
LinkedList makeLinkedList();

/**
 * Frees memory
 * @param list
 */
void freeLinkedList(LinkedList list);

/**
 * Get elem at index
 * @param list
 * @param n
 * @return
 */
void *llElemAt(LinkedList list, int index);

void *llElemSearch(LinkedList list, void *query, Comparitor compare);

/**
 * Get index of elem
 * @param list
 * @param elem
 * @return
 */
int llIndexOf(LinkedList list, void *elem, Comparitor compare);

/**
 * Add an element to the start
 * @param list
 * @param elem
 */
void llAddElem(LinkedList list, void *elem);

/**
 * Adds an element at index
 * @param list
 * @param elem
 * @param index
 */
void llAddElemAt(LinkedList list, void *elem, int index);

/**
 * Adds an element to the end of list
 * @param list
 * @param elem
 */
void llAppend(LinkedList list, void *elem);

void *llReplaceAt(LinkedList list, void *elem, int index);

/**
 * Remove an element at unknown index
 * @param list
 * @param elem
 * @return
 */
void *llRemoveElem(LinkedList list, void *elem, Comparitor compare);

/**
 * Removes node at the given index
 * @param list
 * @param index
 * @return
 */
void *llRemoveNodeAt(LinkedList list, int index);

/**
 * Removes a Linked List Node
 * @param list
 * @param prev
 * @param node
 * @return
 */
void* llRemoveNode(LinkedList list, LLNode prev, LLNode node);

/**
 * map
 * @param list
 * @param func
 */
void llMap(LinkedList list, void func(void *elem));

/**
 * Fold into one value
 * @param list
 * @param initialVal
 * @param func
 * @return
 */
void *llFold(LinkedList list, void *initialVal, void *func(void *elem, void *acc));

/**
 * Shallow copy of list to allocated arr
 * @param list
 * @param elemSize
 * @return
 */
void *llToArray(LinkedList list, size_t elemSize);

int llPointerCompare(void* pointer, void* other);

#endif //FILESYSTEM_LINKEDLIST_H
