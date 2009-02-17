/**********************************************************doxygen*//** @file
 * @brief   BTSP parameters structure and methods
 *
 * Bottleneck traveling salesman problem parameters structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
void
arrow_btsp_params_init(arrow_btsp_params *params)
{
    params->confirm_sol = ARROW_FALSE;
    params->supress_ebst = ARROW_FALSE;
    params->find_short_tour = ARROW_FALSE;
    params->lower_bound = 0;
    params->upper_bound = INT_MAX;
    params->num_steps = 0;
}

void 
arrow_btsp_params_destruct(arrow_btsp_params *params)
{
    if(params->num_steps > 0)
    {
        int i;
        for(i = 0; i < params->num_steps; i++)
        {
            arrow_btsp_solve_plan_destruct(&(params->steps[i]));
        }
    }
}

