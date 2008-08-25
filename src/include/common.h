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
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); fprintf(stderr, args); \
    fprintf(stderr, "\n")


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
 *  heap.c
 ****************************************************************************/
/**
 *  @brief  Binary heap
 */
typedef struct arrow_heap
{
    int *keys;
    int *values;
    int *pos;
    int size;
    int max_size;
} arrow_heap;

/**
 *  @brief  Initializes a new binary heap.
 *  @param  heap [out] heap structure
 *  @param  max_size [in] maximum size of heap
 */
int
arrow_heap_init(arrow_heap *heap, int max_size);

/**
 *  @brief  Destructs a binary heap.
 *  @param  heap [out] heap structure
 */
void
arrow_heap_destruct(arrow_heap *heap);

/**
 *  @brief  Empties the heap of items.
 *  @param  heap [out] heap structure
 */
void
arrow_heap_empty(arrow_heap *heap);

/**
 *  @brief  Inserts a new (key,value) pair into the heap.
 *  @param  heap [out] heap structure
 *  @param  key [in] key to insert
 *  @param  value [in] value to insert -- must be between 0 and [n-1] and
 *              not a duplicate value.
 */
int
arrow_heap_insert(arrow_heap *heap, int key, int value);

/**
 *  @brief  Determines if the given value is present in the heap.
 *  @param  heap [in] heap structure
 *  @param  value [in] value to check
 *  @return True or false, if the value is present in the heap
 */
int
arrow_heap_in(arrow_heap *heap, int value);

/**
 *  @brief  Changes the key for the given value in the heap.
 *  @param  heap [out] heap structure
 *  @param  key [in] the new key
 *  @param  value [in] node value to change
 */
void
arrow_heap_change_key(arrow_heap *heap, int key, int value);

/**
 *  @brief  Returns the value with the smallest key in the heap.
 *  @brief  heap [in] heap structure
 *  @return The value with the smallest key in the heap.
 */
inline int
arrow_heap_get_min(arrow_heap *heap);

/**
 *  @brief  Removes the value with the smallest key from the heap.
 *  @brief  heap [out] heap structure
 */
void
arrow_heap_delete_min(arrow_heap *heap);

/**
 *  @brief  Prints heap structure
 *  @brief  heap [in] heap structure
 */
void
arrow_heap_print(arrow_heap *heap);


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
 *  @brief  Swaps the two given lists.
 *  @param  a [out] the first list
 *  @param  b [out] the second list
 */
void
arrow_llist_swap(arrow_llist *a, arrow_llist *b);

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

/**
 *  @brief  Copys data from linked list into array.  Assume array is big
 *          enough to handle linked list!
 *  @param  list [in] linked list structure
 *  @param  array [out] array to copy items into
 */
void
arrow_llist_to_array(arrow_llist *list, int *array);


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
 *  @param  pretty [in] if ARROW_TRUE, formats output to 8 nodes/row
 */
void
arrow_problem_print(arrow_problem *problem, int pretty);

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


/****************************************************************************
 *  util.c
 ****************************************************************************/
/**
 *  @brief  Creates an integer array.
 *  @param  size [in] size of array
 *  @param  array [out] pointer to array that will be created
 */
inline int
arrow_util_create_int_array(int size, int **array);

/**
 *  @brief  Creates a full integer matrix.
 *  @param  rows [in] number of rows
 *  @param  cols [in] number of columns
 *  @param  matrix [out] pointer to matrix that will be created
 *  @param  space [out] pointer to matrix space that will be created
 */
inline int
arrow_util_create_int_matrix(int rows, int cols, int ***matrix, int **space);

/**
 *  @brief  Prints an error message to stderr with consistent formatting.
 *  @param  file_name [in] file error occured in
 *  @param  line_num [in] line number error occured at
 *  @param  message [in] error message to write
 */
inline void
arrow_util_print_error(const char *file_name, int line_num,
                       const char *message);

/**
 *  @brief  Used to measure timings.
 *  @return a value representing the CPU time in seconds
 */             
inline double
arrow_util_zeit();

/**
 *  @brief  Redirects STDOUT stream to a file (can be used
 *      to completely surpress output by directing to /dev/null).
 *  @param  filename [in] name of file to direct STDOUT to
 *  @param  old_stream [out] existing file handle for STDOUT stream 
 *      (necessary for restoring stream afterwards)
 */
void
arrow_util_redirect_stdout_to_file(const char *filename, int *old_stream);

/**
 *  @brief  Restores STDOUT stream that's been redirected.
 *  @param  old_stream [in] existing file handle for STDOUT stream
 */
void
arrow_util_restore_stdout(int old_stream);

/**
 *  @brief  Makes a shallow copy of the Concorde CCdatagroup structure.
 *  @param  from [in] the CCdatagroup structure to copy from
 *  @param  to [out] the CCdatagroup structure to copy to
 */
void
arrow_util_CCdatagroup_shallow_copy(CCdatagroup *from, CCdatagroup *to);

/**
 *  @brief  Initializes an upper-diagonal matrix norm structure for
 *          Concorde that is ready to be filled in with values.
 *  @param  size [in] the number of cities/vertices
 *  @param  dat [out] the CCdatagroup structure to create
 */
int
arrow_util_CCdatagroup_init_matrix(int size, CCdatagroup *dat);

/**
 *  @brief  Performs a binary search to find the wanted element in a
 *          sorted integer array.
 *  @param  array [in] the array to search (note: must be sorted in
 *              non-increasing order)
 *  @param  size [in] size of the array
 *  @param  element [in] the element to find in the array
 *  @param  pos [out] the index where the element can be found in the array
 */
int
arrow_util_binary_search(int *array, int size, int element, int *pos);

/**
 *	@brief	Determines if the given string turns up a match for the given
 *			regular expression pattern.
 *	@param	string [in] the string to match against
 *	@param	pattern [in] the regular expression pattern to match
 *	@return	ARROW_TRUE if a match is found, ARROW_FALSE if not.
 */
int
arrow_util_regex_match(char *string, char *pattern);

/**
 *  @brief  Prints out the given program arguments to the specified file.
 *  @param  argc [in] the number of arguments
 *  @param  argv [in] the program arugment array
 *  @param  out [in] the file handle to print out to
 */
void
arrow_util_print_program_args(int argc, char *argv[], FILE *out);

/**
 *  @brief  Seeds the random number generator.  Pass a value of 0 to seed
 *          with the current time.
 *  @param  seed [in] the random number seed.
 */
void
arrow_util_random_seed(int seed);

/**
 *  @brief  Returns a random number between 0 and RAND_MAX (normally,
 *          RAND_MAX = INT_MAX).
 *  @return a random integer.
 */
inline int
arrow_util_random();

/**
 *  @brief  Returns a random number between min and max.
 *  @param  min [in] the minimum random number to return
 *  @param  max [in] the maximum random number to return
 *  @return a random integer in the range [min, max]
 */
inline int
arrow_util_random_between(int min, int max);

/**
 *  @brief  Randomly permutes the elements of the given array.
 *  @param  size [in] the size of the array.
 *  @param  array [out] the array to permute.
 */
void
arrow_util_permute_array(int size, int *array);

void
arrow_util_write_tour(arrow_problem *problem, char *comment, int *tour, 
                      FILE *out);

void
arrow_util_sbtsp_to_abstp_tour(arrow_problem *problem, int *old_tour,
                               int *new_tour);


/****************************************************************************
 *  xml.c
 ****************************************************************************/
/**
 *  @brief  Writes an XML tag out for an integer value
 *  @param  name [in] the tag name
 *  @param  value [in] the value to write
 *  @param  out [out] the stream to write to
 */
inline void
arrow_xml_element_int(char *name, int value, FILE *out);

/**
 *  @brief  Writes an XML tag out for a double value
 *  @param  name [in] the tag name
 *  @param  value [in] the value to write
 *  @param  out [out] the stream to write to
 */
inline void
arrow_xml_element_double(char *name, double value, FILE *out);

/**
 *  @brief  Writes an XML tag out for a string value
 *  @param  name [in] the tag name
 *  @param  value [in] the value to write
 *  @param  out [out] the stream to write to
 */
inline void
arrow_xml_element_string(char *name, char *value, FILE *out);

/**
 *  @brief  Writes an XML tag out for a boolean value
 *  @param  name [in] the tag name
 *  @param  value [in] the value to write
 *  @param  out [out] the stream to write to
 */
inline void
arrow_xml_element_bool(char *name, int value, FILE *out);

inline void
arrow_xml_attribute_int(char *name, int value, FILE *out);

inline void
arrow_xml_attribute_string(char *name, char *value, FILE *out);

inline void
arrow_xml_element_start(char *name, FILE *out);

inline void
arrow_xml_element_end(FILE *out);

inline void
arrow_xml_element_open(char *name, FILE *out);

inline void
arrow_xml_element_close(char *name, FILE *out);

inline void
arrow_xml_attribute_start(char *name, FILE *out);

inline void
arrow_xml_attribute_end(FILE *out);


/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
