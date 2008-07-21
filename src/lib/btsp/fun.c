/**********************************************************doxygen*//** @file
 * @brief   Cost matrix transformation functions.
 *
 * Cost matrix transformation functions for the bottleneck traveling salesman
 * problem (BTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_apply(arrow_btsp_fun *fun, arrow_problem *old_problem, 
                     int delta, arrow_problem *new_problem)
{
    CCutil_init_datagroup(&(new_problem->data));
    new_problem->size = old_problem->size;
    sprintf(new_problem->name, "%s", old_problem->name);
    new_problem->shallow = fun->shallow;
    new_problem->get_cost = old_problem->get_cost;
    
    if(!fun->shallow)
    {
        /* Create structure to hold new cost matrix */
        int ret;
        ret = arrow_util_CCdatagroup_init_matrix(old_problem->size, 
                                                 &(new_problem->data));
        if(ret != ARROW_SUCCESS)
            return ARROW_FAILURE;
    }
    
    /* Apply the function to the cost matrix */
    if(!fun->apply(fun, old_problem, delta, new_problem))
    {
        arrow_print_error("Could not apply cost matrix function.");
        return ARROW_FAILURE;
    }
    
    return ARROW_SUCCESS;
}

void
arrow_btsp_fun_destruct(arrow_btsp_fun *fun)
{
    fun->destruct(fun);
}
