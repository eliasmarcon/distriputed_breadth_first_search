#ifndef node_h
#define node_h
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct Node
{
    int rank;
    int depth;
    int parent;
    int childrenCount;
    struct Node **children;
};
char *NodetoString(const struct Node *node);
char *convertToJson(const struct Node *node);
struct Node *convertFromJson(const char *jsonString);
#endif /* node_h */