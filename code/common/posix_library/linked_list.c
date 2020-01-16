#include "posix_library.h"

void add_node(struct node *n, struct node **p)
{
    n->next = *p;
    *p = n;
}
struct node *get_node(struct node *p, int pos)
{
    struct node *start = p;
    for (int i = 0; i < pos; i++) {
        if (start->next != NULL)
            start = start->next;
        else
            return NULL;
    }
    return start;
}
struct node *get_last_node(struct node *p)
{
    if (p == NULL)
        return p;
    struct node *start = p;
    for(;;) {
        if (start->next != NULL)
            start = start->next;
        else
            break;
    }
    return start;
}
struct node *remove_node(struct node **p, int pos)
{
    struct node *node;
    struct node *previous;
    if (pos < 0)
        return NULL;
    node = get_node(*p, pos);
    if (pos > 0) {
        previous = get_node(*p, pos - 1);
        previous->next = node->next;
    }
    else {
        *p = node->next;
    }
    return node;
}
void free_linked_list(struct node **p)
{
    while (*p != NULL)
        remove_node(p, 0);
}
