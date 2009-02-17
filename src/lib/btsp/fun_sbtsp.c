/**********************************************************doxygen*//** @file
 * @brief   Cost matrix functions for the symmetric BTSP.
 *
 * Cost matrix transformation functions for the symmetric bottleneck traveling
 * salesman problem (SBTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Private structures
 ****************************************************************************/


/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  problem [in] problem structure
 *  @param  delta [in] delta parameter
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
               int delta, int i, int j);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
basic_destruct(arrow_btsp_fun *fun);

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
basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem,
               int delta, double tour_length, int *tour);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_btsp_fun_basic(int shallow, arrow_btsp_fun *fun)
{    
    fun->data = NULL;
    fun->shallow = shallow;
    fun->get_cost = basic_get_cost;
    fun->destruct = basic_destruct;
    fun->feasible = basic_feasible;
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
               int delta, int i, int j)
{
    int cost = base_problem->get_cost(base_problem, i, j);
    return (cost <= delta ? 0 : cost);
}

void
basic_destruct(arrow_btsp_fun *fun)
{ }

int
basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
               int delta, double tour_length, int *tour)
{
    return (tour_length <= 0 ? ARROW_TRUE : ARROW_FALSE);
}

