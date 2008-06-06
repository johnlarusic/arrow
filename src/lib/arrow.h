/**********************************************************doxygen*//** @file
 * @brief   Header file for the Arrow callable library.
 *
 * Function prototypes and structures exposed by the callable library.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#ifndef ARROW_H
#define ARROW_H

#ifdef __cplusplus
extern "C" {
#endif

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
 *  To remove debugging info, simply remove (or rename) the ARROW_DEBUG macro
 ****************************************************************************/
#define ARROW_DEBUG
#ifdef  ARROW_DEBUG
    #define arrow_debug printf
#else
    #define arrow_debug
#endif

/****************************************************************************
 *  Global Constants
 ****************************************************************************/
#define ARROW_VERSION "1.0"
#define ARROW_DEV_NULL "/dev/null"

#define ARROW_SUCCESS 1
#define ARROW_FAILURE 0

#define ARROW_ERROR_INPUT 0
#define ARROW_ERROR_FATAL -1
#define ARROW_ERROR_NON_FATAL -2
#define ARROW_TRUE 1
#define ARROW_FALSE 0

#define ARROW_BTSP_SOLVE_PLAN_BASIC 1
#define ARROW_BTSP_SOLVE_PLAN_CONSTRAINED 2

#define ARROW_DEFAULT_BASIC_ATTEMPTS 3

#define ARROW_OPTION_INT 1
#define ARROW_OPTION_DOUBLE 2
#define ARROW_OPTION_STRING 3

#define CONCORDE_SUCCESS 0
#define CONCORDE_FAILURE 1


/****************************************************************************
 *  Macros
 ****************************************************************************/
#define arrow_print_error(message) \
    arrow_util_print_error(__FILE__, __LINE__, message)

/****************************************************************************
 *  Data Structures
 ****************************************************************************/
/**
 *  @brief  BBSSP solver result
 */
typedef struct arrow_bbssp_result
{
    int obj_value;          /**< objective value */
    double total_time;      /**< total time */
} arrow_bbssp_result;

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
 *  @brief  Problem data structure.
 */
typedef struct arrow_problem
{
    int size;           /**< problem size */
    int symmetric;      /**< indicates if cost matrix is symmetric */
    CCdatagroup data;   /**< Concorde data structure for problem. */
    int shallow;        /**< indicates use of shallow copy of data */
    char *name;         /**< name of problem (can be NULL) */
    
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
 *  @brief  LK algorithm parameters
 */
typedef struct arrow_tsp_lk_params
{
    int random_restarts;    /**< the number of random restarts to perform */
    int stall_count;        /**< the max number of 4-swap kicks to perform
                                 without making progress */
    int kicks;              /**< the number of 4-swap kicks to perform */
    int kick_type;          /**< the type of kick: one of CC_LK_RANDOM_KICK, 
                                 CC_LK_GEOMETRIC_KICK, or CC_LK_CLOSE_KICK;
                                 see Concorde documentation for more info */
    double time_bound;      /**< stop after this running time reached; set to 
                                 0 to have no time bound, must be positive */
    double length_bound;    /**< stop after finding tour of this length; must
                                 be non-negative */
    int *initial_tour;      /**< initial tour (may be NULL) */
} arrow_tsp_lk_params;

/**
 *  @brief  TSP result (including result from LK heuristic)
 */
typedef struct arrow_tsp_result
{
    int found_tour;         /**< true if a tour was found, false otherwise */
    double obj_value;       /**< objective value (tour length) */
    int *tour;              /**< tour that was found in node-node format */
    double total_time;      /**< total time */
} arrow_tsp_result;

/**
 *  @brief  BTSP result
 */
typedef struct arrow_btsp_result
{
    int found_tour;         /**< true if a tour was found, false otherwise */
    int obj_value;          /**< objective value (largest cost in tour) */
    double tour_length;     /**< length of the tour found */
    int *tour;              /**< tour that was found in node-node format */
    int optimal;            /**< indicates if the solution is optimal */
    int bin_search_steps;   /**< number of steps in binary search */
    int linkern_attempts;   /**< number of calls to LK heuristic */
    double linkern_time;    /**< total time calling LK heuristic */
    int exact_attempts;     /**< number of calls to exact TSP solver */
    double exact_time;      /**< total time calling exact TSP solver */
    double total_time;      /**< total time */
} arrow_btsp_result;

/**
 *  @brief  BTSP Cost matrix function definition
 */
typedef struct arrow_btsp_fun
{
    void *data;             /**< data required by function */
    int shallow;            /**< indicates use of shallow copy of data */
    int feasible_length;    /**< the length of a feasible tour (normally 0) */
    
    /**
     *  @brief  Applies the function to the given problem
     *  @param  fun [in] function structure
     */
    int 
    (*apply)(struct arrow_btsp_fun *fun, arrow_problem *old_problem,
             int delta, arrow_problem *new_problem);
    
    /**
     *  @brief  Destructs the function structure
     *  @param  fun [out] function structure
     */
    void 
    (*destruct)(struct arrow_btsp_fun *fun);
    
    /**
     *  @brief  Determines if the given tour is feasible or not.
     *  @param  fun [in] function structure
     *  @param  problem [in] the problem to check against
     *  @param  tour_length [in] the length of the given tour
     *  @param  tour [in] the tour in node-node format
     *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
     */
    int
    (*feasible)(struct arrow_btsp_fun *fun, arrow_problem *problem,
                double tour_length, int *tour);
} arrow_btsp_fun;

/**
 *  @brief  BTSP feasibility solve step plan
 */
typedef struct arrow_btsp_solve_plan
{
    int plan_type;                 /**< the type of plan (see macros) */
    int use_exact_solver;          /**< use exact TSP solver? */
    arrow_btsp_fun fun;            /**< the cost matrix function to apply */
    arrow_tsp_lk_params lk_params; /**< LK params to use */
    int upper_bound_update;        /**< Check for better upper bound?*/
    int attempts;                  /**< number of attempts to perform */
} arrow_btsp_solve_plan;

/**
 *  @brief  BTSP algorithm parameters
 */
typedef struct arrow_btsp_params
{
    int confirm_sol;                /**< confirm sol. with exact solver? */
    int supress_ebst;               /**< supress EBST-heuristic? */
    int find_short_tour;            /**< find short BSTP tour? */
    int lower_bound;                /**< initial lower bound */
    int upper_bound;                /**< initial upper bound */
    int num_steps;                  /**< the number of solve plan steps */
    arrow_btsp_solve_plan *steps;   /**< solve plan steps */
} arrow_btsp_params;

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

/****************************************************************************
 *  bbssp.c
 ****************************************************************************/ 
/**
 *  @brief  Solves the bottleneck biconnected spanning subgraph problem 
 *          (BBSSP) on the given problem.
 *  @param  problem [in] problem data
 *  @param  result [out] BBSSP solution
 */
int
arrow_bbssp_solve(arrow_problem *problem, arrow_problem_info *info, 
                  arrow_bbssp_result *result);

/**
 *  @brief  Determines if the graph is biconnected using only edges with costs
            less than or equal to the given value.
 *  @param  problem [in] problem data
 *  @param  max_cost [in] value to check biconnectivity question against
 *  @param  result [out] ARROW_TRUE if biconnected, ARROW_FALSE otherwise.
 */
int
arrow_bbssp_biconnected(arrow_problem *problem, int max_cost, int *result);


/****************************************************************************
 *  bintree.c
 ****************************************************************************/
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
 *  btsp.c
 ****************************************************************************/
/**
 *  @brief  Initializes the BTSP result structure.
 *  @param  problem [in] problem to solve
 *  @param  result [out] BTSP result structure
 */
int
arrow_btsp_result_init(arrow_problem *problem, arrow_btsp_result *result);

/**
 *  @brief  Destructs a BTSP result structure.
 *  @param  result [out] BTSP result structure
 */
void
arrow_btsp_result_destruct(arrow_btsp_result *result);

/**
 *  @brief  Solves TSP with Concorde's exact solver.
 *  @param  problem [in] problem to solve
 *  @param  info [in] extra problem info
 *  @param  params [in] parameters for solver (can be NULL)
 *  @param  result [out] BTSP solution
 */
int
arrow_btsp_solve(arrow_problem *problem, arrow_problem_info *info,
                 arrow_btsp_params *params, arrow_btsp_result *result);
                 
/**
 *  @brief  Inititalizes BTSP parameter structure.
 *  @param  params [out] BTSP parameters structure
 */
void
arrow_btsp_params_init(arrow_btsp_params *params);

/**
 *  @brief  Destructs a BTSP parameters structure.
 *  @param  result [out] BTSP parameters structure
 */
void 
arrow_btsp_params_destruct(arrow_btsp_params *params);

/**
 *  @brief  Inititalizes BTSP solve plan structure.
 *  @param  params [out] BTSP solve plan structure
 */
void
arrow_btsp_solve_plan_init(arrow_btsp_solve_plan *plan);

/**
 *  @brief  Destructs a BTSP solve plan structure.
 *  @param  result [out] BTSP solve plan structure
 */
void 
arrow_btsp_solve_plan_destruct(arrow_btsp_solve_plan *plan);


/****************************************************************************
 *  btsp_fun.c
 ****************************************************************************/
/**
 *  @brief  Applies the given function to the given problem to create a new
 *          problem.
 *  @param  fun [in] function structure
 *  @param  old_problem [in] existing problem
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] new problem to create
 */
int
arrow_btsp_fun_apply(arrow_btsp_fun *fun, arrow_problem *old_problem, 
                     int delta, arrow_problem *new_problem);
 
/**
 *  @brief  Destructs a function structure
 *  @param  fun [out] function structure
 */
void
arrow_btsp_fun_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Basic BTSP to TSP function.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_basic(int shallow, arrow_btsp_fun *fun);

/**
 *  @brief  Constrained BTSP to TSP function
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  feasible_length [in] length of feasible tour
 *  @param  infinity [in] value to use as "infinity"
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_constrained(int shallow, int feasible_length, int infinity, 
                           arrow_btsp_fun *fun);


/****************************************************************************
 *  options.c
 ****************************************************************************/
int
arrow_options_parse(int num_opts, arrow_option options[],  char *description, 
                    char *usage, int argc, char *argv[], int *opt_ind);


/****************************************************************************
 *  problem.c
 ****************************************************************************/
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


/****************************************************************************
 *  tsp.c
 ****************************************************************************/
/**
 *  @brief  Initializes the TSP result structure.
 *  @param  problem [in] problem to solve
 *  @param  result [out] TSP result structure
 */
int
arrow_tsp_result_init(arrow_problem *problem, arrow_tsp_result *result);

/**
 *  @brief  Destructs a TSP result structure.
 *  @param  result [out] TSP result structure
 */
void
arrow_tsp_result_destruct(arrow_tsp_result *result);

/**
 *  @brief  Sets default parameters for Lin-Kernighan heuristic:
 *              - random_restarts = 0
 *              - stall_count = problem->size
 *              - kicks = (problem->size / 2), at least 500
 *              - kick_type = CC_LK_GEOMETRIC_KICK
 *              - time_bound = 0.0
 *              - length_bound = 0.0
 *              - initial_tour = NULL
 *  @param  problem [in] problem to solve
 *  @param  params [out] LK parameters structure
 */
void
arrow_tsp_lk_params_init(arrow_problem *problem, arrow_tsp_lk_params *params);

/**
 *  @brief  Destructs a LK parameters structure.
 *  @param  result [out] LK parameters structure
 */
void 
arrow_tsp_lk_params_destruct(arrow_tsp_lk_params *params);

/**
 *  @brief  Solves TSP with Concorde's exact solver.
 *  @param  problem [in] problem to solve
 *  @param  in_tour [in] an initial tour (can be NULL)
 *  @param  result [out] TSP solution
 */
int 
arrow_tsp_exact_solve(arrow_problem *problem, int *initial_tour, 
                      arrow_tsp_result *result);

/**
 *  @brief  Solves TSP with Concorde's Lin-Kernighan heuristic.
 *  @param  problem [in] problem to solve
 *  @param  params [in] Lin-Kernighan params (can be NULL)
 *  @param  result [out] TSP solution
 */                  
int
arrow_tsp_lk_solve(arrow_problem *problem, arrow_tsp_lk_params *params,
                   arrow_tsp_result *result);


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
 *  @oaram  to [out] the CCdatagroup structure to copy to
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


#ifdef __cplusplus
}
#endif

#endif /* not arrow */
