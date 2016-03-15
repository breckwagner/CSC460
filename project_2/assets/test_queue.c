#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct listNode {
	struct listNode *prev;
	struct listNode *next;
	void *value;
} listNode;

typedef struct listIter {
	listNode *next;
	int direction;
} listIter;

typedef struct list {
	listNode *head;
	listNode *tail;
	void *(*dup)(void *ptr);
	void (*free)(void *ptr);
	int (*match)(void *ptr, void *key);
	unsigned int len;
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
list *listCreate(void);
void listRelease(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listIter *listGetIterator(list *list, int direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, int index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */













/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define zmalloc malloc
#define zfree   free

//#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
list *listCreate(void)
{
	struct list *list;

	if ((list = zmalloc(sizeof(*list))) == NULL)
		return NULL;
	list->head = list->tail = NULL;
	list->len = 0;
	list->dup = NULL;
	list->free = NULL;
	list->match = NULL;
	return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
void listRelease(list *list)
{
	unsigned int len;
	listNode *current, *next;

	current = list->head;
	len = list->len;
	while(len--) {
		next = current->next;
		if (list->free) list->free(current->value);
		zfree(current);
		current = next;
	}
	zfree(list);
}

/* Add a new node to the list, to head, contaning the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeHead(list *list, void *value)
{
	listNode *node;

	if ((node = zmalloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = NULL;
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}
	list->len++;
	return list;
}

/* Add a new node to the list, to tail, contaning the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeTail(list *list, void *value)
{
	listNode *node;

	if ((node = zmalloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next = node;
		list->tail = node;
	}
	list->len++;
	return list;
}

list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
	listNode *node;

	if ((node = zmalloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (after) {
		node->prev = old_node;
		node->next = old_node->next;
		if (list->tail == old_node) {
			list->tail = node;
		}
	} else {
		node->next = old_node;
		node->prev = old_node->prev;
		if (list->head == old_node) {
			list->head = node;
		}
	}
	if (node->prev != NULL) {
		node->prev->next = node;
	}
	if (node->next != NULL) {
		node->next->prev = node;
	}
	list->len++;
	return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void listDelNode(list *list, listNode *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
	if (list->free) list->free(node->value);
	zfree(node);
	list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
listIter *listGetIterator(list *list, int direction)
{
	listIter *iter;

	if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
	if (direction == AL_START_HEAD)
		iter->next = list->head;
	else
		iter->next = list->tail;
	iter->direction = direction;
	return iter;
}

/* Release the iterator memory */
void listReleaseIterator(listIter *iter) {
	zfree(iter);
}

/* Create an iterator in the list private iterator structure */
void listRewind(list *list, listIter *li) {
	li->next = list->head;
	li->direction = AL_START_HEAD;
}

void listRewindTail(list *list, listIter *li) {
	li->next = list->tail;
	li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
listNode *listNext(listIter *iter)
{
	listNode *current = iter->next;

	if (current != NULL) {
		if (iter->direction == AL_START_HEAD)
			iter->next = current->next;
		else
			iter->next = current->prev;
	}
	return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list *listDup(list *orig)
{
	list *copy;
	listIter *iter;
	listNode *node;

	if ((copy = listCreate()) == NULL)
		return NULL;
	copy->dup = orig->dup;
	copy->free = orig->free;
	copy->match = orig->match;
	iter = listGetIterator(orig, AL_START_HEAD);
	while((node = listNext(iter)) != NULL) {
		void *value;

		if (copy->dup) {
			value = copy->dup(node->value);
			if (value == NULL) {
				listRelease(copy);
				listReleaseIterator(iter);
				return NULL;
			}
		} else
			value = node->value;
		if (listAddNodeTail(copy, value) == NULL) {
			listRelease(copy);
			listReleaseIterator(iter);
			return NULL;
		}
	}
	listReleaseIterator(iter);
	return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
listNode *listSearchKey(list *list, void *key)
{
	listIter *iter;
	listNode *node;

	iter = listGetIterator(list, AL_START_HEAD);
	while((node = listNext(iter)) != NULL) {
		if (list->match) {
			if (list->match(node->value, key)) {
				listReleaseIterator(iter);
				return node;
			}
		} else {
			if (key == node->value) {
				listReleaseIterator(iter);
				return node;
			}
		}
	}
	listReleaseIterator(iter);
	return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimante
 * and so on. If the index is out of range NULL is returned. */
listNode *listIndex(list *list, int index) {
	listNode *n;

	if (index < 0) {
		index = (-index)-1;
		n = list->tail;
		while(index-- && n) n = n->prev;
	} else {
		n = list->head;
		while(index-- && n) n = n->next;
	}
	return n;
}

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

#define MAXTHREAD     16
#define WORKSPACE     256   // in bytes, per THREAD
#define MAXMUTEX      8
#define MAXEVENT      8
#define MSECPERTICK   10   // resolution of a system tick in milliseconds
#define MINPRIORITY   10   // 0 is the highest priority, 10 the lowest


#ifndef NULL
#define NULL          0   /* undefined */
#endif

typedef unsigned int PID;        // always non-zero if it is valid
typedef unsigned int MUTEX;      // always non-zero if it is valid
typedef unsigned char PRIORITY;
typedef unsigned int EVENT;      // always non-zero if it is valid
typedef unsigned int TICK;

typedef enum process_states {
	DEAD = 0,
	READY,
	RUNNING,
	SUSPENDED
} PROCESS_STATES;

typedef enum kernel_request_type {
	NONE = 0,
	CREATE,
	NEXT,
	TERMINATE
} KERNEL_REQUEST_TYPE;

typedef struct process_descriptor {
	unsigned char *stack_pointer;
	unsigned char workSpace[WORKSPACE];
	PROCESS_STATES state;
	voidfuncptr code;
	KERNEL_REQUEST_TYPE request;
	PRIORITY priority;
	PID id;
	int argument;
	uint32_t expires;
	//LIST_ENTRY(process_descriptor) pointers;
} ProcessDescriptor;



list *running_queue;

// A list of process_descriptor(s)
list *sleep_queue;

static ProcessDescriptor Process[MAXTHREAD];



static void _schedule_task(ProcessDescriptor *pd, list * queue) {
	listNode *node;
	listIter *it = listGetIterator(queue, AL_START_HEAD);
	for (;;) {
		node = listNext(it);
		if (node == NULL) {
			// Add priority level to queue
			list *l = listCreate();
			listAddNodeTail(queue, l);
			listAddNodeTail(l, pd);
			printf("listCreate - Adding[%d]\n", pd->id);
			break;
		} else {
			//
			listNode * sub_queue_node = listFirst((list *)listNodeValue(node));
			if (sub_queue_node == NULL) {
				// shouldn't happen but if it does remove it
				printf("Empty Subqueue\n");
				break;
			}
			ProcessDescriptor * sub_queue_head_element = ((ProcessDescriptor *)listNodeValue(sub_queue_node));
			list * sub_queue = listNodeValue(node);
			if(sub_queue_head_element->priority == pd->priority) {
				listAddNodeTail(sub_queue, pd);
				printf("Adding[%d], ", pd->id);
				break;
			} else if (sub_queue_head_element->priority > pd->priority) {
				list *l = listCreate();
				listInsertNode(queue, node, l, false);
				listAddNodeTail(l, pd);
				printf("listCreate priority[%d]", pd->priority);
				//listInsertNode(sub_queue, sub_queue_node, pd, false);
				break;
			}
		}
	}
	listReleaseIterator(it);
}

static listNode * get_sublist_head(list * l) {
	listNode * sub_queue_node = listFirst((list *)listNodeValue(listFirst(l)));
	return listNodeValue(sub_queue_node);
}

static list * get_sub_list (list * l, uint16_t index) {
	if (listLength(l) <= index) return NULL;
	else return (list *) listNodeValue(listIndex(l, index));
}

static listNode * get_next_node(list * queue, listNode * sub_queue_node) {
	if (listNextNode(sub_queue_node) != NULL) {
		sub_queue_node = listNextNode(sub_queue_node);
	} else if (get_sublist_head(queue) != NULL) {
		sub_queue_node = get_sublist_head(queue);
	} else {
		//listRelease()
		//listDelNode(queue, )
		//queue
	}
	return listNodeValue(sub_queue_node);
}

static listNode * round_robin (list * l, listNode * n) {
  if (listNextNode(n) != NULL) {
    return listNextNode(n);
  } else {
    return listFirst(l);
  }
}

int main(int argc, char *argv[]) {
	running_queue = listCreate();
	for(int i = 0; i < MAXTHREAD; i++) {
		memset(&(Process[i]), 0, sizeof(ProcessDescriptor));
		Process[i].id = 1+i;
		Process[i].priority=i%2;
		_schedule_task(&(Process[i]), running_queue);
	}
	/*
	printf("\nlength[%d]\n", listLength(running_queue));

	{
	listIter *it = listGetIterator(running_queue, AL_START_HEAD);
	listNode *node;
	while ((node = listNext(it)) != NULL) {
		//for (listNode *node_2 = listNext(it); node != NULL; node = listNext(it)) {
			listNode * sub_queue_node = listFirst((list *)listNodeValue(listFirst(running_queue)));
			ProcessDescriptor * sub_queue_head_element = ((ProcessDescriptor *)listNodeValue(sub_queue_node));

			printf("id[%d]\n", sub_queue_head_element->id );
		//}
	}
	}*/

	list * x = get_sub_list(running_queue, 0);
	listNode * current_process_node = listFirst(x);
	for (int d = 0; d < 10; d++) {
		current_process_node = round_robin(x, current_process_node);
		ProcessDescriptor * f = listNodeValue(current_process_node);
		printf("id---[%d]\n", f->id);
	}

	if (x != NULL) {
		ProcessDescriptor * y = listNodeValue(listFirst(x));
		printf("id[%d]\n", y->id);
	}



	//printf("id[%d]\n", ((ProcessDescriptor *)get_sublist_head(running_queue))->id );



	list * l = listNodeValue(listFirst(running_queue));
	listIter *it = listGetIterator(l, AL_START_HEAD);
	listNode *node;
	while ((node = listNext(it)) != NULL) {
		//for (listNode *node_2 = listNext(it); node != NULL; node = listNext(it)) {
			//printf("id[%d]\n", ((ProcessDescriptor *)listNodeValue(node))->id );
		//}
	}

	//printf("%d", r->id);


}
