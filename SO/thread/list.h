#ifndef LIST_H
#define LIST_H

#include "game.h"

typedef struct ListNode {
    Message data;
    struct ListNode* next;
} ListNode;

typedef struct {
    ListNode* head;
    ListNode* tail;
    int count;
    pthread_mutex_t mutex;
} List;

// Initialize a new list
void list_init(List* list);

// Add a message to the end of the list
void list_push(List* list, Message msg);

// Remove and return the first message from the list
Message list_pop(List* list);

// Get the count of messages in the list
int list_count(List* list);

// Clear all messages from the list
void list_clear(List* list);

// Free the list and all its nodes
void list_free(List* list);

#endif 