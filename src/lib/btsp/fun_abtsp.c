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
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  delta [in] delta parameter
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
atsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int delta, int i, int j);

/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
atsp_basic_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
atsp_basic_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  delta [in] delta parameter
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
atsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem,
                    int delta, double tour_length, int *tour);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_btsp_fun_basic_atsp(int shallow, arrow_btsp_fun *fun)
{    
    fun->data = NULL;
    fun->shallow = shallow;
    fun->get_cost = atsp_basic_get_cost;
    fun->initialize = atsp_basic_initialize;
    fun->destruct = atsp_basic_destruct;
    fun->feasible = atsp_basic_feasible;
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
atsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int delta, int i, int j)
{
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if(cost < 0)
        return cost;
    else if(cost <= delta)
        return 0;
    else
        return cost;
}

int
atsp_basic_initialize(arrow_btsp_fun *fun)
{ 
    return ARROW_SUCCESS;
}

void
atsp_basic_destruct(arrow_btsp_fun *fun)
{ }

int
atsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
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
