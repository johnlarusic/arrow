/**********************************************************doxygen*//** @file
 * @brief   Cost matrix functions for the asymmetric BTSP.
 *
 * Cost matrix transformation functions for the asymmetric bottleneck
 * traveling salesman problem (SBTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Applies basic ABTSP function to the cost matrix of the old problem
 *          to create the new problem (deep copy).
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_atsp_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                      int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Destructs a basic ABTSP function structure
 *  @param  fun [out] the function structure to destruct
 */
void
basic_atsp_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not for the basic
 *          ATSP transformation.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  delta [in] delta parameter
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
basic_atsp_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
                    int delta, double tour_length, int *tour);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_basic_atsp(int shallow, arrow_btsp_fun *fun)
{
    if(shallow)
    {
        arrow_print_error("Not supported.\n");
        return ARROW_FAILURE;
    }
    else
    {
        fun->shallow = ARROW_FALSE;
        fun->apply = basic_atsp_deep_apply;
    }

    fun->feasible_length = 0;
    fun->destruct = basic_atsp_destruct;
    fun->feasible = basic_atsp_feasible;

    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
basic_atsp_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                      int delta, arrow_problem *new_problem)
{
    int i, j, cost;
    for(i = 0; i < new_problem->size; i++)
    {
        for(j = 0; j <= i; j++)
        {
            cost = old_problem->get_cost(old_problem, i, j);
            
            if((cost >= 0) && (cost <= delta))
                new_problem->data.adj[i][j] = 0;
            else
                new_problem->data.adj[i][j] = cost;
        }
    }
    return ARROW_SUCCESS;
}

void
basic_atsp_destruct(arrow_btsp_fun *fun)
{ }

int
basic_atsp_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
                    int delta, double tour_length, int *tour)
{
    int i, u, v, cost;
    int trans_edges = 0;
    
    for(i = 0; i < problem->size; i++)
    {
        u = tour[i];
        v = tour[(i + 1) % problem->size];
        cost = problem->get_cost(problem, u, v);
        
        if(cost > delta)
            return ARROW_FALSE;
            
        if(cost < 0)
            trans_edges++;
    }
    
    /* If an edge's cost is strictly less than 0, then it was introduced into
       the transformation from asymmetric to symmetric.  Any tour we find must
       use each of these edges in order for the tour to be valid. */
    if(trans_edges == (problem->size / 2))
        return ARROW_TRUE;
    else
        return ARROW_FALSE;
}


