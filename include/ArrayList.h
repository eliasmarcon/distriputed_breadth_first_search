#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct ArrayList
{
    int size;
    int *list;
};

struct ArrayList *mergeLists(struct ArrayList *list1, struct ArrayList *list2);

char *toString(struct ArrayList *list);

int inList(int value, struct ArrayList *list);