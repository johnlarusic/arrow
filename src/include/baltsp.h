/**********************************************************doxygen*//** @file
 * @brief   Functions for solving the bottleneck TSP.
 *
 * Functions for solving the bottleneck TSP.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
/* Start conditional header inclusion */
#ifndef ARROW_BALTSP_H
#define ARROW_BALTSP_H

/* Start C++ wrapper */
#ifdef __cplusplus
    extern "C" {
#endif

#include "lb.h"
#include "btsp.h"

/****************************************************************************
 *  baltsp.c
 ****************************************************************************/
int 
arrow_balanced_tsp_solve(arrow_problem *problem, 
                         arrow_problem_info *info,
                         arrow_btsp_params *params, 
                         int lb_only,
                         arrow_bound_result *lb_result,
                         arrow_btsp_result *tour_result);

/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif