/**********************************************************doxygen*//** @file
 * @brief   Cost matrix functions for the Balanced TSP.
 *
 * Cost matrix transformation functions for the balanced traveling
 * salesman problem (BalTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/ 
#include "common.h"
#include "baltsp.h"

/****************************************************************************
 * Private structures
 ****************************************************************************/
/**
 *  @brief  Info for shake type I cost matrix function.
 */
typedef struct baltsp_shake_data
{
    int infinity;               /**< value to use for "infinity" */
    arrow_problem_info *info;   /**< problem info */
    int random_min;             /**< minimum random number generated */
    int random_max;             /**< maximum random number generated */
    int *random_list;           /**< random list of integers */
    int random_list_length;     /**< random list length */
} baltsp_shake_data;


/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
baltsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                      int min_cost, int max_cost, int i, int j);
               
/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
baltsp_basic_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
baltsp_basic_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
baltsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                      int min_cost, int max_cost, double tour_length, int *tour);

/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
baltsp_ut_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                   int min_cost, int max_cost, int i, int j);
               
/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
baltsp_ut_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                   int min_cost, int max_cost, double tour_length, int *tour);
                     
/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
baltsp_shake_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                      int min_cost, int max_cost, int i, int j);

/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
baltsp_shake_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
baltsp_shake_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
baltsp_ib_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                   int min_cost, int max_cost, int i, int j);
               
/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
baltsp_ib_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                   int min_cost, int max_cost, double tour_length, int *tour);

/**
 *  @brief  Retrieves cost between nodes i and j from the function.
 *  @param  fun [in] function structure
 *  @param  base_problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  i [in] id of start node
 *  @param  j [in] id of end node
 *  @return cost between node i and node j
 */
int
baltsp_dt2_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, int i, int j);
               
/**
 *  @brief  Initializes the function data.
 *  @param  fun [out] the function structure to initialize
 */
int
baltsp_dt2_initialize(arrow_btsp_fun *fun);
              
/**
 *  @brief  Destructs the function data.
 *  @param  fun [out] the function structure to destruct
 */
void
baltsp_dt2_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
baltsp_dt2_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, double tour_length, int *tour);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_baltsp_fun_basic(int shallow, arrow_btsp_fun *fun)
{    
    fun->data = NULL;
    fun->shallow = shallow;
    fun->get_cost = baltsp_basic_get_cost;
    fun->initialize = baltsp_basic_initialize;
    fun->destruct = baltsp_basic_destruct;
    fun->feasible = baltsp_basic_feasible;
    return ARROW_SUCCESS;
}

int
arrow_baltsp_fun_ut(int shallow, arrow_btsp_fun *fun)
{    
    fun->data = NULL;
    fun->shallow = shallow;
    fun->get_cost = baltsp_ut_get_cost;
    fun->initialize = baltsp_basic_initialize;
    fun->destruct = baltsp_basic_destruct;
    fun->feasible = baltsp_ut_feasible;
    return ARROW_SUCCESS;
}

int
arrow_baltsp_fun_shake(int shallow, int infinity, 
                       int random_min, int random_max,
                       arrow_problem_info *info, arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if((fun->data = malloc(sizeof(baltsp_shake_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for baltsp_shake_data");
        return ARROW_FAILURE;
    }
    
    baltsp_shake_data *shake_data = (baltsp_shake_data *)fun->data;
    shake_data->infinity = infinity;
    shake_data->info = info;
    
    shake_data->random_min = random_min;
    shake_data->random_max = random_max;
    shake_data->random_list_length = info->cost_list_length;
    
    if(!arrow_util_create_int_array(shake_data->random_list_length,
                                    &(shake_data->random_list)))
    {
        arrow_print_error("Could not allocate memory for random list");
        return ARROW_FAILURE;
    }
    
    fun->shallow = shallow;
    fun->get_cost = baltsp_shake_get_cost;
    fun->initialize = baltsp_shake_initialize;
    fun->destruct = baltsp_shake_destruct;
    fun->feasible = baltsp_basic_feasible;
    
    return ARROW_SUCCESS;
}

int
arrow_baltsp_fun_ib(int shallow, arrow_btsp_fun *fun)
{    
    fun->data = NULL;
    fun->shallow = shallow;
    fun->get_cost = baltsp_ib_get_cost;
    fun->initialize = baltsp_basic_initialize;
    fun->destruct = baltsp_basic_destruct;
    fun->feasible = baltsp_ib_feasible;
    return ARROW_SUCCESS;
}

int
arrow_baltsp_fun_dt2(int shallow, int random_min, int random_max,
                     arrow_problem_info *info, arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if((fun->data = malloc(sizeof(baltsp_shake_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for baltsp_shake_data");
        return ARROW_FAILURE;
    }
    
    baltsp_shake_data *shake_data = (baltsp_shake_data *)fun->data;
    shake_data->infinity = INT_MAX;
    shake_data->info = info;
    
    shake_data->random_min = random_min;
    shake_data->random_max = random_max;
    shake_data->random_list_length = info->cost_list_length;
    
    if(!arrow_util_create_int_array(shake_data->random_list_length,
                                    &(shake_data->random_list)))
    {
        arrow_print_error("Could not allocate memory for random list");
        return ARROW_FAILURE;
    }
    
    fun->shallow = shallow;
    fun->get_cost = baltsp_dt2_get_cost;
    fun->initialize = baltsp_dt2_initialize;
    fun->destruct = baltsp_shake_destruct;
    fun->feasible = baltsp_dt2_feasible;
    
    return ARROW_SUCCESS;
}

/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
baltsp_basic_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, int i, int j)
{
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if((cost >= min_cost) && (cost <= max_cost))
        return 0;
    else
        return cost + 1;
}

int
baltsp_basic_initialize(arrow_btsp_fun *fun)
{ 
    return ARROW_SUCCESS;
}

void
baltsp_basic_destruct(arrow_btsp_fun *fun)
{ }

int
baltsp_basic_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                   int min_cost, int max_cost, double tour_length, int *tour)
{
    return (tour_length == 0 ? ARROW_TRUE : ARROW_FALSE);
}

int
baltsp_ut_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, int i, int j)
{
    int cost = base_problem->get_cost(base_problem, i, j);
    int infinity = base_problem->size * (max_cost - min_cost + 1);
    
    if((cost >= min_cost) && (cost <= max_cost))
        return max_cost - cost;
    else
        return infinity;
}

int
baltsp_ut_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                   int min_cost, int max_cost, double tour_length, int *tour)
{
    int infinity = base_problem->size * (max_cost - min_cost + 1);
    return (tour_length < infinity ? ARROW_TRUE : ARROW_FALSE);
}

int
baltsp_shake_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                      int min_cost, int max_cost, int i, int j)
{
    baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if((cost >= min_cost) && (cost <= max_cost))
        return 0;
    else
    {
        int pos = -1;
        if(!arrow_problem_info_cost_index(data->info, cost, &pos))
        {
            arrow_print_error("Could not find cost in ordered cost list!");
            return data->infinity;
        }
        return cost + data->random_list[pos] + 1;
    }        
}

int
baltsp_shake_initialize(arrow_btsp_fun *fun)
{ 
    int val;
    baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
    
    /* Need to generate a new list of random numbers */
    arrow_bintree tree;
    arrow_bintree_init(&tree);
    while(tree.size < data->random_list_length)
    {
        val = arrow_util_random_between(data->random_min, data->random_max);
        arrow_bintree_insert(&tree, val);
    }
    arrow_bintree_to_array(&tree, data->random_list);
    arrow_bintree_destruct(&tree);
    
    return ARROW_SUCCESS;
}

void
baltsp_shake_destruct(arrow_btsp_fun *fun)
{
    if(fun->data != NULL)
    {
        baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
        free(data->random_list);
        
        free(fun->data);
        fun->data = NULL;
    }
}

int
baltsp_ib_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, int i, int j)
{
    int cost = base_problem->get_cost(base_problem, i, j);
    int infinity = base_problem->size * (max_cost + 1);
    
    if(cost >= min_cost)
        return cost;
    else
        return infinity;
}

int
baltsp_ib_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                   int min_cost, int max_cost, double tour_length, int *tour)
{
    int infinity = base_problem->size * (max_cost + 1);
    return (tour_length < infinity ? ARROW_TRUE : ARROW_FALSE);
}

int
baltsp_dt2_get_cost(arrow_btsp_fun *fun, arrow_problem *base_problem,
                    int min_cost, int max_cost, int i, int j)
{
    baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
    int cost = base_problem->get_cost(base_problem, i, j);
    
    if((cost >= min_cost) && (cost <= max_cost))
    {
        int pos = -1;
        if(!arrow_problem_info_cost_index(data->info, cost, &pos))
        {
            arrow_print_error("Could not find cost in ordered cost list!");
            return data->infinity;
        }
        return max_cost - cost + data->random_list[pos];
    }
    else
    {
        int M = (data->random_list[0] + max_cost - min_cost);
        int randnum = arrow_util_random_between(data->random_min, data->random_max);
        return M * base_problem->size + randnum;
    }
}

int
baltsp_dt2_initialize(arrow_btsp_fun *fun)
{ 
    int i, j, temp;
    
    /* This procedure is the same as the shake one, so reuse this code */
    if(!baltsp_shake_initialize(fun))
    {
        arrow_print_error("Could not create random list");
        return ARROW_FAILURE;
    }
    
    baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
    
    /* Now we need to reverse the list */
    for(i = 0; i < (data->info->cost_list_length / 2); i++)
    {
        j = data->info->cost_list_length - i - 1;
        temp = data->random_list[j];
        data->random_list[j] = data->random_list[i];
        data->random_list[i] = temp;
    }
        
    return ARROW_SUCCESS;
}

int
baltsp_dt2_feasible(arrow_btsp_fun *fun, arrow_problem *base_problem, 
                    int min_cost, int max_cost, double tour_length, int *tour)
{
    baltsp_shake_data *data = (baltsp_shake_data *)fun->data;
    int M = (data->random_list[0] + max_cost - min_cost) ;
    double maxlen = base_problem->size * M * 1.0;    
    return (tour_length < maxlen ? ARROW_TRUE : ARROW_FALSE);
}
