/**********************************************************doxygen*//** @file
 * @brief   BTSP solve plan structure and methods
 *
 * Bottleneck traveling salesman problem solve plan structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
void
arrow_btsp_solve_plan_init(arrow_btsp_solve_plan *plan)
{
    plan->attempts = 0;
}

void 
arrow_btsp_solve_plan_destruct(arrow_btsp_solve_plan *plan)
{
    arrow_btsp_fun_destruct(&(plan->fun));
    switch(plan->tsp_solver)
    {
        case ARROW_TSP_CC_EXACT:
            if((int *)plan->tsp_params != NULL)
                free(plan->tsp_params);
            return;
            break;
        case ARROW_TSP_CC_LK:
            arrow_tsp_cc_lk_params_destruct(
                (arrow_tsp_cc_lk_params *)plan->tsp_params);
            return;
            break;
    }
    
}
