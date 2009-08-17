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
arrow_balanced_tsp_dt(arrow_problem *problem, 
                      arrow_problem_info *info,
                      arrow_btsp_params *params, 
                      int lb_only,
                      arrow_bound_result *lb_result,
                      arrow_btsp_result *tour_result);

/****************************************************************************
 *  fun_baltsp.c
 ****************************************************************************/
/**
 *  @brief  Basic BalTSP to TSP function.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_baltsp_fun_basic(int shallow, arrow_btsp_fun *fun);

/**
 *  @brief  BalTSP to TSP function that favours values closer to the max_cost.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_baltsp_fun_ut(int shallow, arrow_btsp_fun *fun);

/**
 *  @brief  BalTSP Controled Shake.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  infinity [in] value to use as "infinity"
 *  @param  random_min [in] minimum random value to generate
 *  @param  random_max [in] maximum random value to generate
 *  @param  info [in] information about the original problem
 *  @param  fun [out] function structure
 */
int
arrow_baltsp_fun_shake(int shallow, int infinity, 
                       int random_min, int random_max,
                       arrow_problem_info *info, arrow_btsp_fun *fun);
 
 
/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
