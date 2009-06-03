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

/****************************************************************************
 *  baltsp.c
 ****************************************************************************/
int 
arrow_balanced_tsp_solve(arrow_problem *problem, 
                         arrow_problem_info *info,
                         arrow_btsp_params *params, 
                         arrow_bound_result *lb_result,
                         tarrow_btsp_result *tour_result);

/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif