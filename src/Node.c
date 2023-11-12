#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

/**
 * @brief Prints the children of a given node.
 *
 * @param node Pointer to the node whose children are to be printed.
 */
void printChildren(struct Node *node, int rank)
{
    if (node->childrenCount > 0)
    {
        struct Node **children = (node->children);
        for (int i = 0; i < node->childrenCount; i++)
        {
            struct Node *child = children[i];
            printChild(child, i);
        }
        //   printf("Rank: %d, %d\n", children->rank);
    }
}

void printChild(struct Node *node, int i)
{
    printf("\t %d. Child: %d\n", i, node->rank);
}

char *NodetoString(const struct Node *node)
{
    return convertToJson(node);
}

cJSON *nodeToJson(const struct Node *node);

char *convertToJson(const struct Node *node)
{
    cJSON *jsonNode = nodeToJson(node);
    char *jsonString = cJSON_Print(jsonNode);
    cJSON_Delete(jsonNode);
    return jsonString;
}

cJSON *nodeToJson(const struct Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    cJSON *jsonNode = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonNode, "rank", node->rank);
    cJSON_AddNumberToObject(jsonNode, "depth", node->depth);
    cJSON_AddNumberToObject(jsonNode, "parent", node->parent);
    cJSON_AddNumberToObject(jsonNode, "childrenCount", node->childrenCount);

    if (node->childrenCount > 0 && node->children != NULL)
    {
        cJSON *childrenArray = cJSON_CreateArray();
        for (int i = 0; i < node->childrenCount; ++i)
        {
            // printf("Nodetojson: Adding child %d\n", i);
            cJSON_AddItemToArray(childrenArray, nodeToJson(node->children[i]));
            // printf("Nodetojson: After Adding child %d\n", i);
        }
        cJSON_AddItemToObject(jsonNode, "children", childrenArray);
    }

    return jsonNode;
}

struct Node *jsonToNode(const cJSON *jsonNode);

struct Node *convertFromJson(const char *jsonString)
{
    cJSON *jsonNode = cJSON_Parse(jsonString);
    struct Node *node = jsonToNode(jsonNode);
    cJSON_Delete(jsonNode);
    return node;
}

struct Node *jsonToNode(const cJSON *jsonNode)
{
    if (jsonNode == NULL)
        return NULL;

    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL)
        return NULL;

    node->rank = cJSON_GetObjectItemCaseSensitive(jsonNode, "rank")->valueint;
    node->depth = cJSON_GetObjectItemCaseSensitive(jsonNode, "depth")->valueint;
    node->parent = cJSON_GetObjectItemCaseSensitive(jsonNode, "parent")->valueint;
    node->childrenCount = cJSON_GetObjectItemCaseSensitive(jsonNode, "childrenCount")->valueint;

    if (node->childrenCount > 0)
    {
        node->children = malloc(sizeof(struct Node *) * node->childrenCount);
        if (node->children == NULL)
        {
            free(node);
            return NULL;
        }

        cJSON *childrenArray = cJSON_GetObjectItemCaseSensitive(jsonNode, "children");
        cJSON *child;
        int i = 0;
        cJSON_ArrayForEach(child, childrenArray)
        {
            node->children[i++] = jsonToNode(child);
        }
    }
    else
    {
        node->children = NULL;
    }

    return node;
}

/**
 * Prints the full graph recursivley starting from the given node.
 *
 * @param node The starting node of the graph to be printed.
 */
void printFullGraph(struct Node *node)
{
    if (node == NULL)
        return;

    // printing part
    for (int i = 0; i < node->depth; i++)
    {
        printf("\t");
    }
    printf("%d\n", node->rank);

    // end printing part
    if (node->childrenCount > 0)
    {
        struct Node **children = (node->children);
        for (int i = 0; i < node->childrenCount; i++)
        {
            struct Node *child = children[i];
            printFullGraph(child);
        }
    }
}
