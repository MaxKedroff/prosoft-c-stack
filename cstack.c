#include "cstack.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 10



struct node{
    const struct node* prev;
    unsigned int size;
    char data[];
};

typedef struct node* stack_t;

typedef struct stack_entry{
    int reserved;
    stack_t stack;
    unsigned int count;
} stack_entry_t;

struct stack_entries_table{
    unsigned int size;
    stack_entry_t* entries;
};

static stack_entries_table g_table = {0u, NULL};


static int initialize_table(void)
    {
        if (g_table.entries != NULL){
            return 0;
        }

        g_table.entries = (stack_entry_t*)calloc(CAPACITY, sizeof(stack_entry_t));
        if (!g_table.entries){
            return -1;
        }

        for (unsigned int i = 0; i < CAPACITY; i++){
            g_table.entries[i].reserved = 0;
            g_table.entries[i].stack = NULL;
            g_table.entries[i].count = 0;
        }
        g_table.size = CAPACITY;
        return 0;
    }
static int expand_table(void){
    unsigned int new_capacity = (g_table.size == 0) ? CAPACITY : g_table.size * 2;
    stack_entry_t* new_entries = (stack_entry_t*)realloc(g_table.entries, new_capacity * sizeof(stack_entry_t));
    if (!new_entries){
        return -1;
    }
    for (unsigned int i = g_table.size; i < new_capacity; i++) {
        new_entries[i].reserved = 0;
        new_entries[i].stack = NULL;
        new_entries[i].count = 0;
    }

    g_table.entries = new_entries;
    g_table.size = new_capacity;
    return 0;
}


hstack_t stack_new(void)
{
    if (g_table.entries == NULL){
        if (initialize_table() != 0){
            return -1;
        }
    }
    for (unsigned int i = 0; i < g_table.size; i++){
        if (g_table.entries[i].reserved == 0){
            g_table.entries[i].stack = NULL;
            g_table.entries[i].count = 0;
            g_table.entries[i].reserved = 1;
            return (hstack_t)i;
        }
    }
    if (expand_table() != 0){
        return -1;
    }
    for (unsigned int i = 0; i < g_table.size; i++){
        if (g_table.entries[i].reserved == 0) {
            g_table.entries[i].stack = NULL;
            g_table.entries[i].count = 0;
            g_table.entries[i].reserved = 1;
            return (hstack_t)i;
        }
    }

    return -1;
}

static void free_stack_nodes(stack_t node) {
    while (node) {
        stack_t tmp = node;
        node = tmp->prev;
        free(tmp);
    }
}


void stack_free(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) != 0){
        return;
    }
    stack_entry_t* entry = &(g_table.entries[hstack]);

    if (entry->stack != NULL){
        free_stack_nodes(entry->hstack);
        entry->stack = NULL;
    }

    entry->count = 0;
    entry->reserved = 0;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0) return 1;
    if ((unsigned int)hstack >= g_table.size) return 1;
    if (g_table.entries == NULL) return 1;
    if (g_table.entries[hstack].reserved != 1) return 1;
    return 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) != 0){
        return 0;
    }
    return g_table.entries[hstack].count;
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (stack_valid_handler(hstack) != 0){
        return;
    }
    if (data_in == NULL || size == 0){
        return;
    }
    stack_entry_t* entry = &(g_table.entries[hstack]);

    stack_t new_node = (stack_t)malloc(sizeof(struct node) + size);
    if (!new_node) return;

    new_node->prev = entry->stack;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    entry->stack = new_node;
    entry->count++;
}



unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (stack_valid_handler(hstack) != 0){
        return 0;
    }
    stack_entry_t* entry = &(g_table.entries[hstack]);
    if (entry->count == 0 || entry->stack == NULL){
        return 0;
    }

    stack_t top_node = entry->stack;

    unsigned int to_copy = (size < top_node->size) ? size : top_node->size;
    if (data_out && size > 0){
        memcpy(data_out, top_node->data, to_copy);
    }
    entry->stack = top_node->prev;
    entry->count--;
    free(top_node);

    return to_copy;
}

