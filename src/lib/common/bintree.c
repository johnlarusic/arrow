/**********************************************************doxygen*//** @file
 * @brief   Binary tree implementation.
 *
 * Methods for working with Arrow's binary tree data structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Constructs a new node structure with the given value.
 *  @param  node [out] pointer to node structure
 *  @param  value [in] value to assign to new node
 */
int
construct_node(arrow_bintree_node **node, int value);

/**
 *  @brief  Frees the memory of the given node and its child nodes.
 *  @param  node [out] node structure
 */
void
destruct_node(arrow_bintree_node *node);

/**
 *  @brief  Inserts a given value into the tree at the given node, or one of
 *          its child nodes.
 *  @param  tree [in] pointer to tree structure
 *  @param  node [out] pointer to node structure
 *  @param  value [in] value to assign to new node
 */
int
insert_at(arrow_bintree *tree, arrow_bintree_node *node, int value);

/**
 *  @brief  Recursive helper function to fill an array in nondecreasing order.
 *  @param  node [out] pointer to an node structure
 *  @param  array [out] array to fill
 *  @param  pos [out] current position in array
 */
void
fill_array(arrow_bintree_node *node, int *array, int *pos);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
void
arrow_bintree_init(arrow_bintree *tree)
{
    tree->size = 0;
    tree->root_node = NULL;
}

void
arrow_bintree_destruct(arrow_bintree *tree)
{
    if(tree->size > 0)
        destruct_node(tree->root_node);
    free(tree->root_node);
    tree->root_node = NULL;
    tree->size = 0;
}

int
arrow_bintree_insert(arrow_bintree *tree, int value)
{ 
    int ret;

    if(tree->size == 0)
    {
        arrow_bintree_node *new_node;
        ret = construct_node(&new_node, value);
        tree->root_node = new_node;
        tree->size++;
    }
    else
    {
        ret = insert_at(tree, tree->root_node, value);
    }

    return ret;
}

void
arrow_bintree_to_array(arrow_bintree *tree, int *array)
{
    int pos = 0;
    fill_array(tree->root_node, array, &pos);
}

int
arrow_bintree_to_new_array(arrow_bintree *tree, int **array)
{
    if(!arrow_util_create_int_array(tree->size, array))
        return ARROW_FAILURE;
    arrow_bintree_to_array(tree, *array);
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
construct_node(arrow_bintree_node **node, int value)
{
    if((*node = malloc(sizeof(arrow_bintree_node))) == NULL)
    {
        arrow_print_error("Error allocating memory for arrow_bintree_node.");
        node = NULL;
        return ARROW_FAILURE;
    }
    
    (*node)->data = value;
    (*node)->has_left_node = ARROW_FALSE;
    (*node)->has_right_node = ARROW_FALSE;
    (*node)->left_node = NULL;
    (*node)->right_node = NULL;
    
    return ARROW_SUCCESS;
}

void
destruct_node(arrow_bintree_node *node)
{
    if(node->has_left_node == ARROW_TRUE)
    {
        destruct_node(node->left_node);
        free(node->left_node);
        node->left_node = NULL;
    }
    if(node->has_right_node == ARROW_TRUE)
    {
        destruct_node(node->right_node);
        free(node->right_node);
        node->right_node = NULL;
    }
}

int
insert_at(arrow_bintree *tree, arrow_bintree_node *node, int value)
{
    int ret;
    arrow_bintree_node *new_node;
    
    /* Check for duplicate value */
    if(node->data == value)
    {
        return ARROW_SUCCESS;
    }
    /* Insert to the left of this node */
    else if(node->data > value)
    {
        if(node->has_left_node == ARROW_FALSE)
        {
            ret = construct_node(&new_node, value);
            if(ret == ARROW_FAILURE)
                return ret;
            node->left_node = new_node;
            node->has_left_node = ARROW_TRUE;
            tree->size++;
            return ARROW_SUCCESS;
        }
        else
        {
            return insert_at(tree, node->left_node, value);
        }
    }
    /* Insert to the right of this node */
    else
    {
        if(node->has_right_node == ARROW_FALSE)
        {
            ret = construct_node(&new_node, value);
            if(ret == ARROW_FAILURE)
                return ret;
            node->right_node = new_node;
            node->has_right_node = ARROW_TRUE;
            tree->size++;
            return ARROW_SUCCESS;
        }
        else
        {
            return insert_at(tree, node->right_node, value);
        }
    }
}

void
fill_array(arrow_bintree_node *node, int *array, int *pos)
{
    if(node->has_left_node)
        fill_array(node->left_node, array, pos);
    array[*pos] = node->data;
    (*pos)++;
    if(node->has_right_node)
        fill_array(node->right_node, array, pos);
}
