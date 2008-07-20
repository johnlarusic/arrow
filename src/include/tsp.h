/**********************************************************doxygen*//** @file
 * @brief   Solvers and heuristics for the traveling salesman problem.
 *
 * Solvers and heuristics for the traveling salesman problem.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
/* Start conditional header inclusion */
#ifndef ARROW_TSP_H
#define ARROW_TSP_H

/* Start C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif


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
 *  @param  params [out] LK parameters structure
 */
void 
arrow_tsp_lk_params_destruct(arrow_tsp_lk_params *params);

/**
 *  @brief  Solves TSP with Concorde's exact solver.
 *  @param  problem [in] problem to solve
 *  @param  initial_tour [in] an initial tour (can be NULL)
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



/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
