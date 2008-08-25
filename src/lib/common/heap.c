/**********************************************************doxygen*//** @file
 * @brief   Binary heap implementation.
 *
 * Methods for working with Arrow's binary heap data structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Sifts nodes up as necessary to maintain heap property.
 *  @param  heap [out] heap structure
 *  @param  i [in] node to sift up from
 */
void
siftup(arrow_heap *heap, int i);

/**
 *  @brief  Sifts nodes down as necessary to maintain heap property.
 *  @param  heap [out] heap structure
 *  @param  i [in] node to sift down from
 */
void
siftdown(arrow_heap *heap, int i);

/**
 *  @brief  Returns the position of the parent to the given node.
 *  @param  heap [out] heap structure
 *  @param  i [in] node position
 *  @return position of parent node, or -1 if the node is the parent
 */
inline int
parent(arrow_heap *heap, int i);

/**
 *  @brief  Determines if the given node is a leaf node in the heap tree.
 *  @param  heap [out] heap structure
 *  @param  i [in] node position
 *  @return ARROW_TRUE is node is a leaf, ARROW_FALSE otherwise
 */
inline int
is_leaf(arrow_heap *heap, int i);

/**
 *  @brief  Returns the position of the left child to the given node.
 *  @param  heap [out] heap structure
 *  @param  i [in] node position
 *  @return position of left child node, or -1 if no child exists
 */
inline int
left_child(arrow_heap *heap, int i);

/**
 *  @brief  Returns the position of the right child to the given node.
 *  @param  heap [out] heap structure
 *  @param  i [in] node position
 *  @return position of right child node, or -1 if no child exists
 */
inline int
right_child(arrow_heap *heap, int i);

/**
 *  @brief  Returns the smaller of the two child nodes to the given node.
 *  @param  heap [out] heap structure
 *  @param  i [in] node position
 *  @return The smaller of the left and right nodes, or -1 if node is leaf
 */
int
min_child(arrow_heap *heap, int i);

/**
 *  @brief  Swaps the two given nodes in the heap tree
 *  @param  heap [out] heap structure
 *  @param  i [in] first node position
 *  @param  j [in] second node position
 */
void
swap_nodes(arrow_heap *heap, int i, int j);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_heap_init(arrow_heap *heap, int max_size)
{
    int i;
    
    if(!arrow_util_create_int_array(max_size, &(heap->keys)))
        goto CLEANUP;
    if(!arrow_util_create_int_array(max_size, &(heap->values)))
        goto CLEANUP;
    if(!arrow_util_create_int_array(max_size, &(heap->pos)))
        goto CLEANUP;
        
    for(i = 0; i < max_size; i++)
    {
        heap->keys[i] = -1;
        heap->values[i] = -1;
        heap->pos[i] = -1;
    }
        
    heap->size = 0;
    heap->max_size = max_size;
    return ARROW_SUCCESS;

CLEANUP:
    arrow_heap_destruct(heap);
    return ARROW_FAILURE;
}

void
arrow_heap_destruct(arrow_heap *heap)
{
    if(heap->keys != NULL)
        free(heap->keys);
    if(heap->values != NULL)
        free(heap->values);
    if(heap->pos != NULL)
        free(heap->pos);
}

void
arrow_heap_empty(arrow_heap *heap)
{
    int i;
    for(i = 0; i < heap->max_size; i++)
    {
        heap->keys[i] = -1;
        heap->values[i] = -1;
        heap->pos[i] = -1;
    }    
    heap->size = 0;
}

int
arrow_heap_insert(arrow_heap *heap, int key, int value)
{
    if(heap->size == heap->max_size)
    {
        arrow_print_error("Heap is full.");
        return ARROW_FAILURE;
    }
    if(heap->pos[value] >= 0)
    {
        arrow_print_error("Value already in heap.");
        return ARROW_FAILURE;
    }
    if(value > heap->max_size)
    {
        arrow_print_error("Value larger than heap size.");
        return ARROW_FAILURE;
    }
 
    int spot = heap->size;
    heap->keys[spot] = key;
    heap->values[spot] = value;
    heap->pos[value] = spot;
    
    heap->size += 1;
    siftup(heap, spot);
    
    return ARROW_SUCCESS;
}

void
arrow_heap_change_key(arrow_heap *heap, int key, int value)
{
    int i = heap->pos[value];
    int old_key = heap->keys[i];
    heap->keys[i] = key;
    
    if(old_key < key)
        siftdown(heap, i);
    else
        siftup(heap, i);
}

inline int
arrow_heap_get_min(arrow_heap *heap)
{
    return heap->values[0];
}

void
arrow_heap_delete_min(arrow_heap *heap)
{
    int value = heap->values[0];
    swap_nodes(heap, 0, heap->size - 1);
    heap->size -= 1;
    siftdown(heap, 0);
    heap->pos[value] = -1;
}

void
arrow_heap_print(arrow_heap *heap)
{
    int i;
    
    printf("i\tval\tkey\n");
    for(i = 0; i < heap->size; i++)
        printf("%d\t%d\t%d\n", i, heap->values[i], heap->keys[i]);
    printf("\n");
    
    printf("i\tpos\n");
    for(i = 0; i < heap->max_size; i++)
        printf("%d\t%d\n", i, heap->pos[i]);
    printf("\n");
}

/****************************************************************************
 * Private function implementations
 ****************************************************************************/
void
siftup(arrow_heap *heap, int i)
{
    int pred;
    while(i != 0)
    {
        pred = parent(heap, i);
        if(heap->keys[i] < heap->keys[pred])
        {
            swap_nodes(heap, i, pred);
            i = pred;
        }
        else
            return;
    }
}

void
siftdown(arrow_heap *heap, int i)
{
    int min;
    while(!is_leaf(heap, i))
    {
        min = min_child(heap, i);
        if(heap->keys[i] > heap->keys[min])
        {
            swap_nodes(heap, i, min);
            i = min;
        }
        else
            return;        
    }
}

inline int
parent(arrow_heap *heap, int i)
{
    if(i == 0)
        return -1;
    else
        return (i - 1) / 2;
}

inline int
is_leaf(arrow_heap *heap, int i)
{
    return (i * 2 + 1 >= heap->size ? ARROW_TRUE : ARROW_FALSE);
}

inline int
left_child(arrow_heap *heap, int i)
{
    int pos = i * 2 + 1;
    return (pos >= heap->size ? -1 : pos);
}

inline int
right_child(arrow_heap *heap, int i)
{
    int pos = i * 2 + 2;
    return (pos >= heap->size ? -1 : pos);
}

int
min_child(arrow_heap *heap, int i)
{
    if(is_leaf(heap, i))
        return -1;
    int left = left_child(heap, i);
    int right = right_child(heap, i);
        
    if(right == -1)
        return left;
    else
        return (heap->keys[left] < heap->keys[right] ? left : right);
}

void
swap_nodes(arrow_heap *heap, int i, int j)
{
    int temp;

    temp = heap->pos[heap->values[i]];
    heap->pos[heap->values[i]] = heap->pos[heap->values[j]];
    heap->pos[heap->values[j]] = temp;

    temp = heap->keys[i];
    heap->keys[i] = heap->keys[j];
    heap->keys[j] = temp;
    
    temp = heap->values[i];
    heap->values[i] = heap->values[j];
    heap->values[j] = temp;
}
