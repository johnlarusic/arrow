/**********************************************************doxygen*//** @file
 * @brief   Functions for solving the bottleneck TSP.
 *
 * Functions for solving the bottleneck TSP.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
/* Start conditional header inclusion */
#ifndef ARROW_BTSP_H
#define ARROW_BTSP_H

/* Start C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif

#include "tsp.h"

#define ARROW_BTSP_SOLVE_PLAN_BASIC 1
#define ARROW_BTSP_SOLVE_PLAN_CONSTRAINED 2
#define ARROW_BTSP_SOLVE_PLAN_CONSTRAINED_SHAKE 3

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
    int solver_attempts[ARROW_TSP_SOLVER_COUNT]; /**< calls to solver */
    double solver_time[ARROW_TSP_SOLVER_COUNT];  /**< total time for solver */
    double total_time;      /**< total time */
} arrow_btsp_result;

/**
 *  @brief  BTSP Cost matrix function definition
 */
typedef struct arrow_btsp_fun
{
    void *data;             /**< data required by function */
    int shallow;            /**< indicates use of shallow copy of data */

    /**
     *  @brief  Retrieves cost between nodes i and j from the function.
     *  @param  fun [in] function structure
     *  @param  problem [in] problem structure
     *  @param  delta [in] delta parameter
     *  @param  i [in] id of start node
     *  @param  j [in] id of end node
     *  @return cost between node i and node j
     */
    int 
    (*get_cost)(struct arrow_btsp_fun *fun, arrow_problem *base_problem,
                int delta, int i, int j);
                
    /**
     *  @brief  Initializes the function structure for a new problem
     *  @param  fun [out] function structure
     */
    int 
    (*initialize)(struct arrow_btsp_fun *fun);
    
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
     *  @param  delta [in] the delta parameter
     *  @param  tour_length [in] the length of the given tour
     *  @param  tour [in] the tour in node-node format
     *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
     */
    int
    (*feasible)(struct arrow_btsp_fun *fun, arrow_problem *base_problem,
                int delta, double tour_length, int *tour);
} arrow_btsp_fun;

/**
 *  @brief  BTSP feasibility solve step plan
 */
typedef struct arrow_btsp_solve_plan
{
    int tsp_solver;                 /**< TSP solver to use */
    void *tsp_params;               /**< TSP params to use */
    arrow_btsp_fun fun;             /**< the cost matrix function to apply */
    int attempts;                   /**< number of attempts to perform */
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
    arrow_btsp_solve_plan confirm_plan;   /**< confirm plan */
} arrow_btsp_params;


/****************************************************************************
 *  btsp.c
 ****************************************************************************/
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


/****************************************************************************
 *  fun.c
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


/****************************************************************************
 *  fun_atsp.c
 ****************************************************************************/
/**
 *  @brief  Basic BTSP to TSP function for asymmetric problem instances.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_basic_atsp(int shallow, arrow_btsp_fun *fun);


/****************************************************************************
 *  fun_sbtsp.c
 ****************************************************************************/
/**
 *  @brief  Basic BTSP to TSP function.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_basic(int shallow, arrow_btsp_fun *fun);

/**
 *  @brief  Controlled Type I Shake.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  random_min [in] minimum random number to generate
 *  @param  random_max [in] maximum random number to generate
 *  @param  info [in] information about the original problem
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_basic_shake_i(int shallow, int random_min, int random_max, 
                             arrow_problem_info *info, arrow_btsp_fun *fun);

/****************************************************************************
 *  fun_cbtsp.c
 ****************************************************************************/
/**
 *  @brief  Constrained BTSP to TSP function
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  feasible_length [in] length of feasible tour
 *  @param  infinity [in] value to use as "infinity"
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_cbtsp_basic(int shallow, double feasible_length, int infinity,
                           arrow_btsp_fun *fun);

/**
 *  @brief  Constrained "Shake" BTSP to TSP function
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  feasible_length [in] length of feasible tour
 *  @param  infinity [in] value to use as "infinity"
 *  @param  random_min [in] minimum random value to generate
 *  @param  random_max [in] maximum random value to generate
 *  @param  info [in] information about the original problem
 *  @param  fun [out] function structure
 */
int
arrow_btsp_fun_cbtsp_shake(int shallow, double feasible_length, int infinity,
                           int random_min, int random_max, 
                           arrow_problem_info *info, arrow_btsp_fun *fun);
                                 

/****************************************************************************
 *  params.c
 ****************************************************************************/               
/**
 *  @brief  Inititalizes BTSP parameter structure.
 *  @param  params [out] BTSP parameters structure
 */
void
arrow_btsp_params_init(arrow_btsp_params *params);


/****************************************************************************
 *  result.c
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
 *  @brief  Prints results out in XML format
 *  @param  result [in] result structure
 *  @param  out [out] file to wrtie to
 */
void
arrow_btsp_result_print_xml(arrow_btsp_result *result, FILE *out);

/**
 *  @brief  Prints results out
 *  @param  result [in] result structure
 *  @param  out [out] file to wrtie to
 */
void
arrow_btsp_result_print_pretty(arrow_btsp_result *result, FILE *out);


/****************************************************************************
 *  solve_plan.c
 ****************************************************************************/
/**
 *  @brief  Inititalizes BTSP solve plan structure.
 *  @param  plan [out] BTSP solve plan structure
 */
void
arrow_btsp_solve_plan_init(arrow_btsp_solve_plan *plan);

/**
 *  @brief  Destructs a BTSP solve plan structure.
 *  @param  plan [out] BTSP solve plan structure
 */
void 
arrow_btsp_solve_plan_destruct(arrow_btsp_solve_plan *plan);


/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
