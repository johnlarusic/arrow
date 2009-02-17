/**********************************************************doxygen*//** @file
 * @brief   Cost matrix functions for the constrained BTSP.
 *
 * Cost matrix transformation functions for the constrained bottleneck
 * traveling salesman problem (SBTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Private data structures
 ****************************************************************************/
/**
 *  @brief  Concorde userdat structure for constrained cost matrix function.
 */
typedef struct cbtsp_basic_data
{
    int infinity;           /**< value to use for "infinity" */
    double feasible_length; /**< feasible length of a tour */
} cbtsp_basic_data;


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
basic_cbtsp_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
basic_cbtsp_destruct(arrow_btsp_fun *fun);

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
basic_cbtsp_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, double tour_length, int *tour);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_btsp_fun_cbtsp_basic(int shallow, double feasible_length, int infinity,
                           arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if((fun->data = malloc(sizeof(cbtsp_basic_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for cbtsp_basic_data");
        return ARROW_FAILURE;
    }
    
    fun->data->infinity = infinity;
    fun->data->feasible_length = feasible_length;
    
    fun->shallow = shallow;
    fun->get_cost = basic_cbtsp_get_cost;
    fun->destruct = basic_cbtsp_destruct;
    fun->feasible = basic_cbtsp_feasible;
    
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
basic_cbtsp_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j)
{
    cbtsp_basic_data *data = (cbtsp_basic_data *)fun->data;
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if(cost <= delta)
        return cost;
    else
        return data->infinity;
}

void
basic_cbtsp_destruct(arrow_btsp_fun *fun)
{
    free(fun->data);
}

int
basic_cbtsp_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                    int delta, double tour_length, int *tour)
{
    cbtsp_basic_data *data = (cbtsp_basic_data *)fun->data;
    return (tour_length <= data->feasible_length ? ARROW_TRUE : ARROW_FALSE);
}