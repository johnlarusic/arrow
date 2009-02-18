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
 *  @brief  Info for basic constrained cost matrix function.
 */
typedef struct cbtsp_basic_data
{
    int infinity;           /**< value to use for "infinity" */
    double feasible_length; /**< feasible length of a tour */
} cbtsp_basic_data;

/**
 *  @brief  Info for shake constrained cost matrix function.
 */
typedef struct cbtsp_shake_data
{
    int infinity;               /**< value to use for "infinity" */
    double feasible_length;     /**< feasible length of a tour */
    arrow_problem_info *info;   /**< problem info */
    int random_min;             /**< minimum random number generated */
    int random_max;             /**< maximum random number generated */
    int *random_list;           /**< random list of integers */
    int random_list_length;     /**< random list length */
} cbtsp_shake_data;


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
cbtsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j);

/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
cbtsp_basic_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
cbtsp_basic_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] the problem to check against
 *  @param  delta [in] delta parameter
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
cbtsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, double tour_length, int *tour);
                     
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
cbtsp_shake_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j);

/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
cbtsp_shake_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
cbtsp_shake_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] the problem to check against
 *  @param  delta [in] delta parameter
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
cbtsp_shake_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
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
    
    cbtsp_basic_data *cbtsp_data = (cbtsp_basic_data *)fun->data;
    cbtsp_data->infinity = infinity;
    cbtsp_data->feasible_length = feasible_length;
    
    fun->shallow = shallow;
    fun->get_cost = cbtsp_basic_get_cost;
    fun->initialize = cbtsp_basic_initialize;
    fun->destruct = cbtsp_basic_destruct;
    fun->feasible = cbtsp_basic_feasible;
    
    return ARROW_SUCCESS;
}

int
arrow_btsp_fun_cbtsp_shake(int shallow, double feasible_length, int infinity,
                           int random_min, int random_max, 
                           arrow_problem_info *info, arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if((fun->data = malloc(sizeof(cbtsp_shake_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for cbtsp_shake_data");
        return ARROW_FAILURE;
    }
    
    cbtsp_shake_data *cbtsp_data = (cbtsp_shake_data *)fun->data;
    cbtsp_data->infinity = infinity;
    cbtsp_data->feasible_length = feasible_length;
    cbtsp_data->info = info;
    
    cbtsp_data->random_min = random_min;
    cbtsp_data->random_max = random_max;
    cbtsp_data->random_list_length = info->cost_list_length;
    
    if(!arrow_util_create_int_array(cbtsp_data->random_list_length,
                                    &(cbtsp_data->random_list)))
    {
        arrow_print_error("Could not allocate memory for random list");
        return ARROW_FAILURE;
    }
    
    fun->shallow = shallow;
    fun->get_cost = cbtsp_shake_get_cost;
    fun->initialize = cbtsp_shake_initialize;
    fun->destruct = cbtsp_shake_destruct;
    fun->feasible = cbtsp_shake_feasible;
    
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
cbtsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j)
{
    cbtsp_basic_data *data = (cbtsp_basic_data *)fun->data;
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if(cost <= delta)
        return cost;
    else
        return data->infinity;
}

int
cbtsp_basic_initialize(arrow_btsp_fun *fun)
{ 
    return ARROW_SUCCESS;
}

void
cbtsp_basic_destruct(arrow_btsp_fun *fun)
{
    if(fun->data != NULL)
    {
        free(fun->data);
        fun->data = NULL;
    }
}

int
cbtsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                    int delta, double tour_length, int *tour)
{
    cbtsp_basic_data *data = (cbtsp_basic_data *)fun->data;
    return (tour_length <= data->feasible_length ? ARROW_TRUE : ARROW_FALSE);
}

int
cbtsp_shake_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                     int delta, int i, int j)
{
    cbtsp_shake_data *data = (cbtsp_shake_data *)fun->data;
    int cost = base_problem->get_cost(base_problem, i, j);
    
    int pos = -1;
    if(!arrow_problem_info_cost_index(data->info, cost, &pos))
    {
        arrow_print_error("Could not find cost in ordered cost list!");
        return data->infinity;
    }
    
    if(cost <= delta)
        return cost + data->random_list[pos];
    else
        return data->infinity;
}

int
cbtsp_shake_initialize(arrow_btsp_fun *fun)
{ 
    int val;
    cbtsp_shake_data *data = (cbtsp_shake_data *)fun->data;
    
    /* Need to generate a new list of random numbers */
    arrow_bintree tree;
    arrow_bintree_init(&tree);
    while(tree.size < data->random_list_length)
    {
        val = arrow_util_random_between(data->random_min, data->random_max);
        arrow_bintree_insert(&tree, val);
    }
    arrow_bintree_to_array(&tree, data->random_list);
    
    return ARROW_SUCCESS;
}

void
cbtsp_shake_destruct(arrow_btsp_fun *fun)
{
    if(fun->data != NULL)
    {
        cbtsp_shake_data *data = (cbtsp_shake_data *)fun->data;
        free(data->random_list);
        
        free(fun->data);
        fun->data = NULL;
    }
}

int
cbtsp_shake_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                    int delta, double tour_length, int *tour)
{
    int i, u, v, cost;
    int n = base_problem->size;
    double actual_length = 0.0;
    
    cbtsp_shake_data *data = (cbtsp_shake_data *)fun->data;
    
    for(i = 0; i < n; i++)
    {
        u = tour[i];
        v = tour[(i + 1) % n];
        cost = base_problem->get_cost(base_problem, u, v);
        
        if(cost > delta)
            return ARROW_FALSE;
        
        actual_length += cost;
        if(actual_length > data->feasible_length)
            return ARROW_FALSE;
    }
    
    return ARROW_TRUE;
}