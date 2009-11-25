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

/**
 *  @brief  BTSP algorithm parameters
 */
typedef struct arrow_baltsp_params
{
    int with_improvements;          /**< use running-time improvments */
    int lower_bound;                /**< balanced TSP lower bound */
    int btsp_min_cost;              /**< smallest cost in a BTSP tour */
    int btsp_max_cost;              /**< largest cost in a BTSP tour */
    int mstsp_min_cost;             /**< smallest cost in a MSTSP tour */
    int num_steps;                  /**< the number of solve plan steps */
    arrow_btsp_solve_plan *steps;   /**< solve plan steps */
    int infinity;                   /**< value to use for "infinity" */
    int deep_copy;
} arrow_baltsp_params;

/****************************************************************************
 *  baltsp-dt.c
 ****************************************************************************/
int 
arrow_balanced_tsp_dt(arrow_problem *problem, 
                      arrow_problem_info *info,
                      arrow_btsp_params *params, 
                      int lb_only, int with_improvements,
                      arrow_bound_result *lb_result,
                      arrow_btsp_result *tour_result);

/****************************************************************************
 *  baltsp-dt2.c
 ****************************************************************************/
int 
arrow_balanced_tsp_dt2(arrow_problem *problem, 
                       arrow_problem_info *info,
                       arrow_baltsp_params *params, 
                       double *lb_time,
                       arrow_btsp_result *tour_result);

/****************************************************************************
 *  baltsp-ib.c
 ****************************************************************************/
int 
arrow_balanced_tsp_ib(arrow_problem *problem, 
                      arrow_problem_info *info,
                      arrow_btsp_params *params, 
                      int lb_only, int with_improvements,
                      arrow_bound_result *lb_result,
                      arrow_btsp_result *tour_result);

/****************************************************************************
 *  baltsp-ib2.c
 ****************************************************************************/
int 
arrow_balanced_tsp_ib2(arrow_problem *problem, 
                       arrow_problem_info *info,
                       arrow_baltsp_params *params,
                       arrow_btsp_params *btsp_params,
                       double *lb_time,
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

/**
 *  @brief  BalTSP Iterative Bottleneck transformation.
 *  @param  shallow [in] ARROW_TRUE for shallow copy, ARROW_FALSE for deep
 *  @param  fun [out] function structure
 */
int
arrow_baltsp_fun_ib(int shallow, arrow_btsp_fun *fun);

int
arrow_baltsp_fun_dt2(int shallow, int random_min, int random_max,
                     arrow_problem_info *info, arrow_btsp_fun *fun);


/****************************************************************************
 *  fun_baltsp.c
 ****************************************************************************/
void
arrow_baltsp_params_init(arrow_baltsp_params *params);
 
/* End C++ wrapper */
#ifdef __cplusplus
    }
#endif

/* End conditional header inclusion */
#endif
