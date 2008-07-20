/**********************************************************doxygen*//** @file
 * @brief   Common functions and data structures.
 *
 * Common functions and data structures exposed by the callable library.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
/* Start conditional header inclusion */
#ifndef ARROW_COMMON_H
#define ARROW_COMMON_H

/* Start C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif

/****************************************************************************
 *  Standard header files
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <float.h>
#include <regex.h>
#include <getopt.h>
#include "concorde.h"

/****************************************************************************
 *  Debugging info macro.  Rename ARROW_DEBUG to disable debug info
 ****************************************************************************/
#define ARROW_DEBUG
#ifdef  ARROW_DEBUG
    #define arrow_debug printf
#else
    #define arrow_debug
#endif


/****************************************************************************
 *  Error reporting macro
 ****************************************************************************/
#define arrow_print_error(args...) \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); fprintf(stderr, args)


/****************************************************************************
 *  Global Constants
 ****************************************************************************/
#define ARROW_VERSION "1.0"
#define ARROW_DEV_NULL "/dev/null"
#define ARROW_SUCCESS 1
#define ARROW_FAILURE 0
#define ARROW_TRUE 1
#define ARROW_FALSE 0

#define ARROW_OPTION_INT 1
#define ARROW_OPTION_DOUBLE 2
#define ARROW_OPTION_STRING 3

#define CONCORDE_SUCCESS 0
#define CONCORDE_FAILURE 1

/****************************************************************************
 *  bintree.c
 ****************************************************************************/
/**
 *  @brief  Binary tree data structure.
 */
typedef struct arrow_bintree
{
    struct arrow_bintree_node *root_node;   /**< root node of tree */
    int size;                               /**< size of tree */
} arrow_bintree;

/**
 *  @brief  Binary tree node.
 */
typedef struct arrow_bintree_node
{
    int data;                               /**< data contained in node */
    int has_left_node;                      /**< true if left node exists */
    int has_right_node;                     /**< true if right node exists */
    struct arrow_bintree_node *left_node;   /**< left node */
    struct arrow_bintree_node *right_node;  /**< right node */
} arrow_bintree_node;

/**
 *  @brief  Initializes the binary tree data structure.
 *  @param  tree [out] binary tree structure
 */
void
arrow_bintree_init(arrow_bintree *tree);

/**
 *  @brief  Destructs a binary tree data structure.
 *  @param  tree [out] binary tree structure
 */
void
arrow_bintree_destruct(arrow_bintree *tree);

/**
 *  @brief  Inserts a value into the binary tree.
 *  @param  tree [out] binary tree structure
 *  @param  value [in] value to insert into tree
 */
int
arrow_bintree_insert(arrow_bintree *tree, int value);

/**
 *  @brief  Initializes the binary tree data structure.
 *  @param  tree [out] binary tree structure
 *  @param  array [out] array to be created and filled
 */
int
arrow_bintree_to_array(arrow_bintree *tree, int **array);

/**
 *  @brief  Prints out the values of the binary tree.
 *  @param  tree [in] binary tree structure
 */ 
void
arrow_bintree_print(arrow_bintree *tree);


/****************************************************************************
 *  llist.c
 ****************************************************************************/
/**
 *  @brief  Linked list item
 */
typedef struct arrow_llist_item
{
    int data;                       /**< list item data */
    struct arrow_llist_item *next;  /**< next item in list */
} arrow_llist_item;

/**
 *  @brief  Linked list data structure
 */
typedef struct arrow_llist
{
    struct arrow_llist_item *head;  /**< head item in list */
    struct arrow_llist_item *tail;  /**< tail item in list */
    int size;                       /**< size of list */
} arrow_llist;

/**
 *  @brief  Initializes a new linked list
 *  @param  list [out] linked list structure
 */
void
arrow_llist_init(arrow_llist *list);

/**
 *  @brief  Destructs contents of a linked list
 *  @param  list [out] list structure
 */
void
arrow_llist_destruct(arrow_llist *list);

/**
 *  @brief  Inserts a new item at the head of the list
 *  @param  list [out] linked list structure
 *  @param  value [in] value to insert
 */
int
arrow_llist_insert_head(arrow_llist *list, int value); 

/**
 *  @brief  Inserts a new item at the tail of the list
 *  @param  list [out] linked list structure
 *  @param  value [in] value to insert
 */
int
arrow_llist_insert_tail(arrow_llist *list, int value);

/**
 *  @brief  Inserts a new item after the given item in the list. NOTE:
 *          procedure makes no attempt to verify given item is in the list!
 *  @param  list [out] linked list structure
 *  @param  item [in] linked list item
 *  @param  value [in] value to insert
 */
int
arrow_llist_insert_after(arrow_llist *list, arrow_llist_item *item, 
                         int value);

/**
 *  @brief  Removes the head item in the list. 
 *  @param  list [out] linked list structure
 *  @param  value [out] value of removed item
 */
void
arrow_llist_remove_head(arrow_llist *list, int *value);

/**
 *  @brief  Removes the tail item in the list.  NOTE: O(n) procedure!
 *  @param  list [out] linked list structure
 *  @param  value [out] value of removed item
 */
void
arrow_llist_remove_tail(arrow_llist *list, int *value);

/**
 *  @brief  Removes the item after the given one in the list. NOTE:
 *          procedure makes no attempt to verify given item is in the list!
 *  @param  list [out] linked list structure
 *  @param  item [in] linked list item
 *  @param  value [out] value of removed item
 */
void
arrow_llist_remove_after(arrow_llist *list, arrow_llist_item *item, 
                         int *value);
                         
/**
 *  @brief  Prints a list of items in the list. 
 *  @param  list [in] linked list structure
 */
void
arrow_llist_print(arrow_llist *list);


/****************************************************************************
 *  options.c
 ****************************************************************************/
/**
 *  @brief  Program options structure
 */
typedef struct arrow_option
{
    char short_option;              /**< short option (flag) */
    const char *long_option;        /**< long option */
    const char *help_message;       /**< help message to display for option */
    int data_type;                  /**< one of ARROW_OPTION_INT, 
                                         ARROW_OPTION_DOUBLE, 
                                        ARROW_OPTION_STRING */
    void *data_ptr;          /**< pointer to variable to hold parameter */
    int opt_required;       /**< if true ensures option is present */
    int arg_required;       /**< if true, ensures argument for parameter
                                 passed, otherwise puts 1 into data_ptr if
                                 parameter is present */
} arrow_option;

/**
 *  @brief  Parses program options
 *  @param  num_opts [in] the number of program options
 *  @param  options [out] array of program options
 *  @param  description [in] description of program
 *  @param  usage [in] usage message
 *  @param  argc [in] number of program arguments
 *  @param  argv [out] array of program arguments (might be rearranged)
 *  @param  opt_in [out] index first program option appears at
 */
int
arrow_options_parse(int num_opts, arrow_option options[],  char *description, 
                    char *usage, int argc, char *argv[], int *opt_ind);


/****************************************************************************
 *  problem.c
 ****************************************************************************/
/**
 *  @brief  Problem data structure.
 */
typedef struct arrow_problem
{
    int size;           /**< problem size */
    int symmetric;      /**< indicates if cost matrix is symmetric */
    CCdatagroup data;   /**< Concorde data structure for problem. */
    int shallow;        /**< indicates use of shallow copy of data */
    char name[64];      /**< name of problem (can be NULL) */
    
    /**
     *  @brief  Returns the cost between node i and node j.
     *  @param  this [in] problem data
     *  @param  i [in] node i
     *  @param  j [in] node j
     *  @return cost between node i and j.
     */
    int 
    (*get_cost)(struct arrow_problem *this, int i, int j);
} arrow_problem;

/**
 *  @brief  Problem information data structure.
 */
typedef struct arrow_problem_info
{
    int *cost_list;         /**< sorted list of unique costs from problem. */
    int cost_list_length;   /**< length of cost list. */
    int min_cost;           /**< smallest cost in problem. */
    int max_cost;           /**< largest cost in problem. */
} arrow_problem_info;

/**
 *  @brief  Reads a problem from a TSPLIB file.
 *  @param  file_name [in] path to TSPLIB file
 *  @param  problem [out] problem data structure
 */
int
arrow_problem_read(char *file_name, arrow_problem *problem);

/**
 *  @brief  Deallocates problem data structure.
 *  @param  problem [out] problem data structure
 */
void
arrow_problem_destruct(arrow_problem *problem);

/**
 *  @brief  Builds ordered cost list and finds min/max cost in a problem.
 *  @param  problem [in] problem data structure
 *  @param  info [out] problem info data structure
 */
int
arrow_problem_info_get(arrow_problem *problem, arrow_problem_info *info);

/**
 *  @brief  Deallocates problem info data structure.
 *  @param  info [in] problem info data structure
 */
void
arrow_problem_info_destruct(arrow_problem_info *info);

/**
 *  @brief  Prints out information about a problem.
 *  @param  problem [in] problem data structure
 */
void
arrow_problem_print(arrow_problem *problem);

/**
 *  @brief  Retrieves cost between nodes i and j
 *  @param  problem [in] pointer to arrow_problem structure
 *  @param  i [in] id of starting node
 *  @param  j [in] id of ending node
 *  @return cost between node i and node j
 */
inline int 
arrow_problem_get_cost(arrow_problem *problem, int i, int j);

/**
 *  @brief  Reads a TSPLIB tour file
 *  @param  file_name [in] the TSPLIB tour file to read
 *  @param  size [in] the number of cities in the tour
 *  @param  tour [out] an array to hold the tour in node-node format
 */
int
arrow_problem_read_tour(char *file_name, int size, int *tour);

/**
 *	@brief	Transforms an asymmetric BTSP problem of n nodes into a symmetric
 *			BTSP problem with 2n nodes.
 *  @param  old_problem [in] the asymmetric problem
 *	@param	infinity [in] value to use as "infinity"
 *	@param	new_problem [out] the new symmetric problem
 */
int
arrow_problem_abtsp_to_sbtsp(arrow_problem *old_problem, int infinity, 
                             arrow_problem *new_problem);


/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
