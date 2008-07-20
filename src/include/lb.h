/**********************************************************doxygen*//** @file
 * @brief   Lower bound functions for the bottleneck TSP.
 *
 * Lower bound functions for the bottleneck TSP.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
/* Start conditional header inclusion */
#ifndef ARROW_LB_H
#define ARROW_LB_H

/* Start C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif

/**
 *  @brief  A lower bound result
 */
typedef struct arrow_bound_result
{
    int obj_value;          /**< objective value */
    double total_time;      /**< total time */
} arrow_bound_result;


/****************************************************************************
 *  2mb.c
 ****************************************************************************/ 
/**
 *  @brief  Solves the 2-max bound (2MB) on the given problem.
 *  @param  problem [in] problem data
 *  @param  result [out] 2MB solution
 */
int
arrow_2mb_solve(arrow_problem *problem, arrow_bound_result *result);


/****************************************************************************
 *  bbssp.c
 ****************************************************************************/ 
/**
 *  @brief  Solves the bottleneck biconnected spanning subgraph problem 
 *          (BBSSP) on the given problem.
 *  @param  problem [in] problem data
 *  @param  info [in] problem info
 *  @param  result [out] BBSSP solution
 */
int
arrow_bbssp_solve(arrow_problem *problem, arrow_problem_info *info, 
                  arrow_bound_result *result);

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
 *  bscssp.c
 ****************************************************************************/
/**
 *  @brief  Solves the bottleneck strongly connected spanning subgraph problem
 *          (BSCSSP) on the given graph.
 *  @param  problem [in] problem data
 *  @param  info [in] problem info
 *  @param  result [out] BSCSSP solution
 */
int
arrow_bscssp_solve(arrow_problem *problem, arrow_problem_info *info, 
                   arrow_bound_result *result);


/****************************************************************************
 *  dcbpb.c
 ****************************************************************************/ 
/**
 *  @brief  Solves the degree constrained bottleneck paths bound (DCBPB).
 *  @param  problem [in] problem data
 *  @param  result [out] BPB solution
 */
int
arrow_dcbpb_solve(arrow_problem *problem, arrow_bound_result *result);


/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
