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
 * Private structures
 ****************************************************************************/
/**
 *  @brief  Concorde userdat structure for constrained cost matrix function.
 */
typedef struct constrained_data
{
    CCdatagroup *dat;   /**< existing Concorde data structure */
    int infinity;       /**< value to use for "infinity" */
    int delta;          /**< delta value */
} constrained_data;

/**
 *  @brief  Shake information.
 */
typedef struct shake_info
{
    arrow_problem *problem;     /**< original problem data */
    arrow_problem_info *info;   /**< original problem info */
    int rand_min;               /**< smallest random number to generate */
    int rand_max;               /**< largest random number to generate */
    int infinity;               /**< value to use for "infinity" */
} shake_info;


/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Applies constrained BTSP function to the cost matrix of the old 
 *          problem to create the new problem (shallow copy)
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
constrained_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                          int delta, arrow_problem *new_problem);

/**
 *  @brief  Applies constrained BTSP function to the cost matrix of the old 
 *          problem to create the new problem (deep copy)
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
constrained_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                       int delta, arrow_problem *new_problem);

/**
 *  @brief  Destructs constrained BTSP function structure
 *  @param  fun [out] the function structure to destruct
 */
void
constrained_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Concorde edge length function for the constrained cost matrix 
 *          function.  Returns the cost C[i,j].
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde data structure.
 */
static int 
constrained_edgelen(int i, int j, struct CCdatagroup *dat);

/**
 *  @brief  Applies constrained shake BTSP function to the cost matrix of 
 *          the old problem to create the new problem (deep copy)
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
constrained_shake_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                             int delta, arrow_problem *new_problem);

/**
 *  @brief  Destructs constrained shake BTSP function structure
 *  @param  fun [out] the function structure to destruct
 */
void
constrained_shake_destruct(arrow_btsp_fun *fun);

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
constrained_shake_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
                           int delta, double tour_length, int *tour);

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
               int delta, double tour_length, int *tour)
{
    return (tour_length <= fun->feasible_length ? ARROW_TRUE : ARROW_FALSE);
}


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_constrained(int shallow, double feasible_length, int infinity, 
                           arrow_btsp_fun *fun)
{
    if((fun->data = malloc(sizeof(int))) == NULL)
    {
        arrow_print_error("Error allocating memory for fun->data (int).");
        return ARROW_FAILURE;
    }
    *((int*)fun->data) = infinity;
    
    if(shallow)
    {
        fun->shallow = ARROW_TRUE;
        fun->apply = constrained_shallow_apply;
    }
    else
    {
        fun->shallow = ARROW_FALSE;
        fun->apply = constrained_deep_apply;
    }
    fun->feasible_length = feasible_length;
    fun->destruct = constrained_destruct;
    fun->feasible = basic_feasible;
    
    return ARROW_SUCCESS;
}

int
arrow_btsp_fun_constrained_shake(int shallow, double feasible_length, 
                                 int infinity, int rand_min, int rand_max,
                                 arrow_problem *problem, 
                                 arrow_problem_info *info, 
                                 arrow_btsp_fun *fun)
{
    if(shallow)
    {
        arrow_print_error("Not supported.\n");
        return ARROW_FAILURE;
    }
    else
    {
        fun->shallow = ARROW_FALSE;
        fun->apply = constrained_shake_deep_apply;
    }
    
    /* Create data structure for function */
    shake_info *data;
    if((fun->data = malloc(sizeof(shake_info))) == NULL)
    {
        arrow_print_error("Error allocating memory for fun->data.");
        return ARROW_FAILURE;
    }
    data = (shake_info *)(fun->data);
    
    /* Set structure parameters */
    data->problem = problem;
    data->info = info;
    data->rand_min = rand_min;
    data->rand_max = rand_max;
    data->infinity = infinity;
    
    fun->feasible_length = feasible_length;
    fun->destruct = constrained_shake_destruct;
    fun->feasible = constrained_shake_feasible;
    
    return ARROW_SUCCESS;
}

/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
constrained_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                          int delta, arrow_problem *new_problem)
{
    CCdatagroup *old_data = &(old_problem->data);
    CCdatagroup *new_data = &(new_problem->data);
    constrained_data *user;
    
    if((new_data->userdat.data = malloc(sizeof(constrained_data))) == NULL)
    {
        arrow_print_error("Error allocating memory for constrained_data.");
        return ARROW_FAILURE;
    }
    user = (constrained_data *)(new_data->userdat.data);
    user->dat = old_data;
    user->delta = delta;
    user->infinity = *((int*)(fun->data));
    
    new_data->userdat.edgelen = constrained_edgelen;
    CCutil_dat_setnorm(new_data, CC_USER);
    
    return ARROW_SUCCESS;
}

int
constrained_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                       int delta, arrow_problem *new_problem)
{
    int i, j, cost;
    int infinity = *((int*)(fun->data));
    for(i = 0; i < new_problem->size; i++)
    {
        for(j = 0; j <= i; j++)
        {
            cost = old_problem->get_cost(old_problem, i, j);
            new_problem->data.adj[i][j] = (cost <= delta ? cost : infinity);
        }
    }

    return ARROW_SUCCESS;
}

void
constrained_destruct(arrow_btsp_fun *fun)
{ 
    if((int*)(fun->data) != NULL)
    {
        free((int*)(fun->data));
        fun->data = NULL;
    }
}

static int 
constrained_edgelen(int i, int j, struct CCdatagroup *dat)
{
    constrained_data *user = (constrained_data *)(dat->userdat.data);
    int old_cost = user->dat->edgelen(i, j, user->dat);
    return (old_cost <= user->delta ? old_cost : user->infinity);
}

int
constrained_shake_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                             int delta, arrow_problem *new_problem)
{
    int i, j, cost, rand_val, pos;
    int *rand_list;
    int ret = ARROW_SUCCESS;
    arrow_bintree tree;
    
    shake_info *data = (shake_info *)(fun->data);
    int list_length = data->info->cost_list_length;
    
    /* Generate an ordered list of random numbers */
    arrow_bintree_init(&tree);
    while(tree.size < list_length)
    {
        rand_val = arrow_util_random_between(data->rand_min, data->rand_max);
        arrow_bintree_insert(&tree, rand_val);
    }
    if(!arrow_bintree_to_array(&tree, &rand_list))
    {
        arrow_print_error("Could not transform tree into int array");
        arrow_bintree_destruct(&tree);
        return ARROW_FAILURE;
    }
            
    /* Fill cost matrix */
    for(i = 0; i < new_problem->size; i++)
    {
        for(j = 0; j <= i; j++)
        {
            cost = old_problem->get_cost(old_problem, i, j);
            if(i == j)
            {
                new_problem->data.adj[i][j] = 0;
            }
            else if(cost <= delta)
            {
                if(!arrow_util_binary_search(data->info->cost_list, 
                                             list_length, cost, &pos))
                {
                    arrow_print_error("Could not find cost in cost list!");
                    ret = ARROW_FAILURE;
                    goto CLEANUP;
                }
                rand_val = rand_list[pos];
                new_problem->data.adj[i][j] = cost + rand_val;
            }
            else
            {
                new_problem->data.adj[i][j] = data->infinity;
            }
        }
    }

CLEANUP:
    arrow_bintree_destruct(&tree);
    free(rand_list);
    return ret;
}

void
constrained_shake_destruct(arrow_btsp_fun *fun)
{ 
    if((shake_info *)(fun->data) != NULL)
    {
        free(fun->data);
        fun->data = NULL;
    }
}

int
constrained_shake_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
                           int delta, double tour_length, int *tour)
{
    int i, u, v, cost;
    arrow_problem *old_problem = ((shake_info *)(fun->data))->problem;
    double actual_length = 0.0;
    for(i = 0; i < old_problem->size; i++)
    {
        u = tour[i];
        v = tour[(i + 1) % old_problem->size];
        cost = old_problem->get_cost(old_problem, u, v);
        
        if(cost > delta)
            return ARROW_FALSE;
        
        actual_length += cost;
        if(actual_length > fun->feasible_length)
            return ARROW_FALSE;
    }
    
    return ARROW_TRUE;
}