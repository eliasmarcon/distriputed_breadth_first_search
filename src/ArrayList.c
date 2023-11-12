#include "../include/ArrayList.h"

/**
 * Merges two ArrayLists into a single ArrayList.
 *
 * @param list1 The first ArrayList to merge.
 * @param list2 The second ArrayList to merge.
 * @return A new ArrayList containing all elements from list1 and list2.
 */
struct ArrayList *mergeLists(struct ArrayList *list1, struct ArrayList *list2)
{
    int size1 = list1->size;
    int size2 = list2->size;
    int newSize = size1 + size2;
    int *mergedList = malloc(newSize * sizeof(int));
    memcpy(mergedList, list1->list, size1 * sizeof(int));
    memcpy(mergedList + size1, list2->list, size2 * sizeof(int));
    free(list2);
    free(list1);
    struct ArrayList *new = malloc(sizeof(struct ArrayList));
    new->list = mergedList;
    new->size = newSize;
    return new;
}

/**
 * Converts the given ArrayList to a string representation.
 *
 * @param list The ArrayList to convert.
 * @return A string representation of the ArrayList.
 */
char *toString(struct ArrayList *list)
{
    // Initial estimate for string size
    int estimatedSize = 1; // Start with 1 for the null terminator
    for (int i = 0; i < list->size; i++)
    {
        estimatedSize += snprintf(NULL, 0, "%d,", list->list[i]);
    }

    char *str = malloc(sizeof(char) * estimatedSize);
    if (!str)
    {
        return NULL; // Return NULL if memory allocation fails
    }

    char *current = str;
    for (int i = 0; i < list->size; i++)
    {
        current += sprintf(current, "%d,", list->list[i]);
    }

    if (list->size > 0)
    {
        current[-1] = '\0'; // Replace the last comma with a null terminator
    }
    else
    {
        *str = '\0';
    }

    return str;
}

/**
 * Check if a value is in the given ArrayList.
 *
 * @param value The value to search for.
 * @param list The ArrayList to search in.
 * @return 1 if the value is in the list, 0 otherwise.
 */
int inList(int value, struct ArrayList *list)
{
    for (int i = 0; i < list->size; i++)
        if (list->list[i] == value)
            return 1;
    return 0;
}