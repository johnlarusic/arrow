/**********************************************************doxygen*//** @file
 * @brief   TSP result structure and methods
 *
 * Traveling salesman problem results structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_tsp_result_init(arrow_problem *problem, arrow_tsp_result *result)
{
    if(!arrow_util_create_int_array(problem->size, &(result->tour)))
    {
        result = NULL;
        return ARROW_FAILURE;
    }
    result->found_tour = ARROW_FALSE;
    result->obj_value = -1.0;
    result->total_time = 0.0;
    return ARROW_SUCCESS;
}

void 
arrow_tsp_result_destruct(arrow_tsp_result *result)
{
    if(result->tour != NULL)
    {
        free(result->tour);
        result->tour = NULL;
    }
}
