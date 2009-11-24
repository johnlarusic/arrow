/**********************************************************doxygen*//** @file
 * @brief   BTSP parameters structure and methods
 *
 * Bottleneck traveling salesman problem parameters structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "baltsp.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
void
arrow_baltsp_params_init(arrow_baltsp_params *params)
{
    params->with_improvements = ARROW_FALSE;
    params->lower_bound = 0;
    params->btsp_min_cost = 0;
    params->btsp_max_cost = INT_MAX;
    params->mstsp_min_cost = INT_MAX;
    params->num_steps = 0;
}
