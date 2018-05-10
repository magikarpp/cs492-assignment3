
#include <stdlib.h>
#include "LinkedList.h"


LinkedList makeLinkedList() {

  LinkedList list = (LinkedList) malloc(sizeof(struct linked_list));
  list->head = NULL;
  list->length = 0;
  return list;

}

LLNode makeLLNode(void *data) {

  LLNode node = (LLNode) malloc(sizeof(struct ll_node));
  node->data = data;
  node->next = NULL;
  return node;

}

void freeLLNode(LLNode node) {

  node->data = NULL;
  node->next = NULL;
  free(node);

}

void freeLinkedList(LinkedList list) {
  // Not freeing elements
  LLNode currNode = list->head;

  while (currNode != NULL) {

    LLNode toFree = currNode;
    currNode = currNode->next;
    free(toFree);

  }

  list->head = NULL;
  list->length = 0;
  free(list);

}

LLNode llNodeAt(LinkedList list, int index) {

  LLNode currNode = list->head;

  for (int i = 0; i < index; i++, currNode = currNode->next) {}

  return currNode;

}

void* llElemAt(LinkedList list, int index) {
  return llNodeAt(list, index)->data;
}

void* llElemSearch(LinkedList list, void* query, Comparitor compare) {

  LLNode currNode = list->head;

  for (int i = 0; i < list->length; i++, currNode = currNode->next) {

    if (compare(currNode->data, query)) 
      return currNode->data;

  }

  // Not in list
  return NULL;

}


/**
 * Add element at head
 * @param list
 * @param elem
 */
void llAddElem(LinkedList list, void *elem) {

  LLNode newNode = makeLLNode(elem);

  if (list->head == NULL) 
    list->head = newNode;

  else {

    newNode->next = list->head;
    list->head = newNode;

  }

  list->length++;

}

void llAppend(LinkedList list, void* elem) {

  if (list->length == 0) 
    llAddElem(list, elem);

  else {

    LLNode newNode = makeLLNode(elem);
    LLNode currNode = list->head;

    for (int i = 0; i < list->length - 1; i++, currNode = currNode->next) {}

    currNode->next = newNode;
    list->length++;

  }

}

void* llReplaceAt(LinkedList list, void* elem, int index) {

  LLNode node = llNodeAt(list, index);
  void* oldData = node->data;
  node->data = elem;

  return oldData;

}

void llAddElemAt(LinkedList list, void *elem, int index) {

    if (index >= list->length) 
      llAppend(list, elem);

    else if (index == 0)
      llAddElem(list, elem);

    else {
      LLNode newNode = makeLLNode(elem);
      LLNode toAddAfter = llNodeAt(list, index - 1);
      newNode->next = toAddAfter->next;
      toAddAfter->next = newNode;
      list->length++;
    }

}

int llIndexOf(LinkedList list, void *elem, Comparitor compare) {

  LLNode currNode = list->head;

  for (int i = 0; i < list->length; i++, currNode = currNode->next) {

    if (compare(currNode->data, elem))
      return i;

  }
 
  return -1;

}

void* llRemoveNode(LinkedList list, LLNode prev, LLNode node) {

  LLNode toRemove = node;

  // If removing the head
  if (toRemove == list->head)
    list->head = toRemove->next;

  else
    prev->next = toRemove->next;

  void* toReturn = toRemove->data;
  toRemove->next = NULL;

  freeLLNode(toRemove);
  list->length--;
  return toReturn;

}

void* llRemoveElem(LinkedList list, void *elem, Comparitor compare) {

  LLNode prevNode = list->head;
  LLNode currNode = list->head;

  for (int i = 0; i <= list->length; i++, currNode = currNode->next) {

    if (compare(currNode->data, elem)) 
      return llRemoveNode(list, prevNode, currNode);

    prevNode = currNode;

  }

  // Not in list
  return NULL;

}

void* llRemoveNodeAt(LinkedList list, int index) {

  LLNode prevNode = list->head;
  LLNode currNode = list->head;
  for (int i = 0; i <= index; i++, prevNode = currNode, currNode = currNode->next) {}

  return llRemoveNode(list, prevNode, currNode);

}

/**
 * map a function to every element of the list
 * @param list
 * @param func
 */
void llMap(LinkedList list, void func(void*)) {

  LLNode currNode = list->head;

  while (currNode != NULL) {

    LLNode toApply = currNode;
    currNode = currNode->next;
    func(toApply->data);

  }

}

void* llFold(LinkedList list, void* initialVal, void* func(void*, void*)) {

  LLNode currNode = list->head;

  while (currNode != NULL) {

    initialVal = func(currNode->data, initialVal);
    currNode = currNode->next;

  }

  return initialVal;
}

void* llToArray(LinkedList list, size_t elemSize) {

  void** array = malloc(elemSize * list->length);
  LLNode currNode = list->head;

  for (int i = 0; i < list->length; i++) {

    array[i] = currNode->data;
    currNode = currNode->next;

  }

  return array;

}

// Basic comparitors
int llPointerCompare(void* pointer, void* other) {
  return pointer == other;
}