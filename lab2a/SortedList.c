//NAME: Harrison Cassar
//EMAIL: Harrison.Cassar@gmail.com
//ID: 505114980

#include "SortedList.h"
#include <string.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	//check for invalid list (non-existant head)
	if (element == NULL || list == NULL || list->key != NULL || (list->next != list && list->prev == list) || (list->next == list && list->prev != list))
		return;

	//check for empty list
	if (list->next == list)
	{
		list->next = element;
		list->prev = element;
		element->prev = list;
		element->next = list;
		return;
	}

	SortedListElement_t* curr = list->next;
	SortedListElement_t* past = list;

	for (;;)
	{
		if (curr == NULL || curr->next == NULL)
			return;

		//check if fits right before there
		if (strcmp(curr->key, element->key) > 0)
		{
			past->next = element;
			curr->prev = element;
			element->prev = past;
			element->next = curr;
			break;
		}

		//hit end of list, append item to end
		if (curr->next == list)
		{
			curr->next = element;
			element->prev = curr;
			element->next = list;
			list->prev = element;
			break;
		}

		past = curr;
		curr = curr->next;
	}
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element)
{
	//check for invalid param
	if (element == NULL)
		return 1;

	//corrupted prev/next ptrs
	if (element->next == NULL || element->next->prev != element)
		return 1;

	if (element->prev == NULL || element->prev->next != element)
		return 1;

	element->prev->next = element->next;
	element->next->prev = element->prev;

	return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	//check for invalid inputted list
	if (list == NULL || list->key != NULL || (list->next != list && list->prev == list) || (list->next == list && list->prev != list))
		return NULL;

	//check for if requested element is the header
	if (key == NULL)
		return list;

	SortedListElement_t* curr = list->next;

	for (;;)
	{
		//check for invalid element in list
		if (curr == NULL)
			return NULL;

		//check for hitting end of list, thus meaning no matching element is found
		if (curr->key == NULL)
			return NULL;

		//check for equal keys
		if (strcmp(curr->key,key) == 0)
			return curr;

		curr = curr->next;
	}
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list)
{
	//check for invalid inputted list
	if (list == NULL || list->key != NULL || (list->next != list && list->prev == list) || (list->next == list && list->prev != list))
		return -1;

	int count = 0;

	//check for empty list
	if (list->next == list)
		return count;

	SortedListElement_t* curr = list->next;

	for (;;)
	{
		//check for corruption
		if (curr == NULL)
			return -1;

		//corrupted prev/next ptrs
		if (curr->next == NULL || curr->next->prev != curr)
			return -1;
		if (curr->prev == NULL || curr->prev->next != curr)
			return -1;

		//check if hit end of list
		if (curr == list)
			return count;

		count++;
		curr = curr->next;
	}
}