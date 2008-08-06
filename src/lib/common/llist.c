/**********************************************************doxygen*//** @file
 * @brief   Linked list implementation.
 *
 * Methods for working with a linked list structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/**
 *  @brief  Constructs a new item structure with the given value.
 *  @param  item [out] pointer to item structure
 *  @param  value [in] value to assign to new item
 */
int
construct_item(arrow_llist_item **item, int value);

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
void
arrow_llist_init(arrow_llist *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void
arrow_llist_destruct(arrow_llist *list)
{
    int value;
    while(list->size > 0)
        arrow_llist_remove_head(list, &value);
}

void
arrow_llist_swap(arrow_llist *a, arrow_llist *b)
{
    arrow_llist_item *head = a->head;
    arrow_llist_item *tail = a->tail;
    int size = a->size;
    
    a->head = b->head;
    a->tail = b->tail;
    a->size = b->size;
    
    b->head = head;
    b->tail = tail;
    b->size = size;
}

int
arrow_llist_insert_head(arrow_llist *list, int value)
{
    arrow_llist_item *item;
    if(!construct_item(&item, value))
        return ARROW_FAILURE;
    
    if(list->size == 0)
    {
        /* If list is empty, item becomes both head and tail */
        list->head = item;
        list->tail = item;
        item->next = NULL;
    }
    else
    {
        item->next = list->head;
        list->head = item;
    }
    list->size++;
    return ARROW_SUCCESS;
}

int
arrow_llist_insert_tail(arrow_llist *list, int value)
{
    arrow_llist_item *item;
    if(!construct_item(&item, value))
        return ARROW_FAILURE;    
    item->next = NULL;
    
    if(list->size == 0)
    {
        /* If list is empty, item becomes both head and tail */
        list->head = item;
        list->tail = item;
    }
    else
    {
        list->tail->next = item;
        list->tail = item;
    }
    list->size++;
    return ARROW_SUCCESS;
}

int
arrow_llist_insert_after(arrow_llist *list, arrow_llist_item *item, 
                         int value)
{
    if(item == NULL)
    {
        arrow_print_error("Can't insert after a NULL item.");
        return ARROW_FAILURE;
    }
    else if(item == list->tail)
    {
        return arrow_llist_insert_tail(list, value);
    }
    else
    {
        arrow_llist_item *new_item;  
        if(!construct_item(&new_item, value))
            return ARROW_FAILURE;
        
        new_item->next = item->next;
        item->next = new_item;
        
        list->size++;
    }
}

void
arrow_llist_remove_head(arrow_llist *list, int *value)
{
    if(list->size == 0)
    {
        *value = INT_MIN;
    }
    else if(list->size == 1)
    {
        *value = list->head->data;
        free(list->head);
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;
    }
    else
    {
        *value = list->head->data;
        arrow_llist_item *next = list->head->next;
        free(list->head);
        list->head = next;
        list->size--;
    }
}

void
arrow_llist_remove_tail(arrow_llist *list, int *value)
{
    if(list->size == 0)
    {
        *value = INT_MIN;
    }
    else if(list->size == 1)
    {
        arrow_llist_remove_head(list, value);
    }
    else
    {
        *value = list->tail->data;
        
        arrow_llist_item *cur = list->head;
        while(cur->next != list->tail)
            cur = cur->next;
        
        cur->next = NULL;
        free(list->tail);
        list->tail = cur;
        list->size--;
    }
}

void
arrow_llist_remove_after(arrow_llist *list, arrow_llist_item *item, 
                         int *value)
{
    if((list->size == 0) || (item == list->tail))
    {
        *value = INT_MIN;
    }
    else if(item == list->head)
    {
        arrow_llist_remove_head(list, value);
    }
    else
    {
        *value = list->tail->data;
        arrow_llist_item *next = item->next;   
        item->next = item->next->next;
        free(next);
        list->size--;
    }    
}

void
arrow_llist_print(arrow_llist *list)
{
    printf("list (%d): ", list->size);
    arrow_llist_item *cur = list->head;
    while(cur != NULL)
    {
        printf("%d, ", cur->data);
        cur = cur->next;
    }
    printf("EOL\n");
}

void
arrow_llist_to_array(arrow_llist *list, int *array)
{
    int i = 0;
    arrow_llist_item *item = list->head;
    while(item != NULL)
    {
        array[i] = item->data;
        i++;
        item = item->next;
    }
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
construct_item(arrow_llist_item **item, int value)
{
    if((*item = malloc(sizeof(arrow_llist_item))) == NULL)
    {
        arrow_print_error("Error allocating memory for arrow_llist_item.");
        *item = NULL;
        return ARROW_FAILURE;
    }
    
    (*item)->data = value;
    (*item)->next = NULL;
    
    return ARROW_SUCCESS;
}
