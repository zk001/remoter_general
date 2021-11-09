/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#include "types.h"

#define __compiletime_assert(condition, msg, prefix, suffix) do { } while (0)

#define _compiletime_assert(condition, msg, prefix, suffix) \
  __compiletime_assert(condition, msg, prefix, suffix)

#define compiletime_assert(condition, msg) \
  _compiletime_assert(condition, msg, __compiletime_assert_, __COUNTER__)

#define __native_word(t) \
  (sizeof(t) == sizeof(char) || sizeof(t) == sizeof(short) || \
   sizeof(t) == sizeof(int) || sizeof(t) == sizeof(long))

#define compiletime_assert_rwonce_type(t)					\
  compiletime_assert(__native_word(t) || sizeof(t) == sizeof(long long),	\
      "Unsupported access size for {READ,WRITE}_ONCE().")

/*
 * Circular doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
  struct g_list_head name = LIST_HEAD_INIT(name)

/**
 * INIT_LIST_HEAD - Initialize a list_head structure
 * @list: list_head structure to be initialized.
 *
 * Initializes the list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static inline void INIT_LIST_HEAD(struct g_list_head *list)
{
  list->next = list;
  list->prev = list;
}

static inline bool __list_add_valid(struct g_list_head *new,
    struct g_list_head *prev,
    struct g_list_head *next)
{
  return true;
}
static inline bool __list_del_entry_valid(struct g_list_head *entry)
{
  return true;
}


/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct g_list_head *new,
    struct g_list_head *prev,
    struct g_list_head *next)
{
  if (!__list_add_valid(new, prev, next))
    return;

  next->prev = new;
  new->next  = next;
  new->prev  = prev;
  prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void g_list_add(struct g_list_head *new, struct g_list_head *head)
{
  __list_add(new, head, head->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void g_list_add_tail(struct g_list_head *new, struct g_list_head *head)
{
  __list_add(new, head->prev, head);
}

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
    const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
    (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
  container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
  list_entry((ptr)->next, type, member)

/**
 * list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_next_entry(pos, member) \
  list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_entry_is_head - test if the entry points to the head of the list
 * @pos:	the type * to cursor
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry_is_head(pos, head, member)				\
  (&pos->member == (head))

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
  for (pos = list_first_entry(head, typeof(*pos), member);	\
      !list_entry_is_head(pos, head, member);			\
      pos = list_next_entry(pos, member))

#endif
