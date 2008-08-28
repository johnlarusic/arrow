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
/**
 *  @brief  Concorde userdat structure for basic cost matrix function.
 */
typedef struct basic_data
{
    CCdatagroup *dat;   /**< existing Concorde data structure */
    int delta;          /**< delta value */
} basic_data;

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
 *  @brief  Applies basic BTSP function to the cost matrix of the old problem
 *          to create the new problem (shallow copy).
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                    int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Applies basic BTSP function to the cost matrix of the old problem
 *          to create the new problem (deep copy).
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                 int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Destructs a basic BTSP function structure
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

/**
 *  @brief  Concorde edge length function for the basic cost matrix function.
 *          Returns the cost C[i,j].
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde data structure.
 */
static int 
basic_edgelen(int i, int j, struct CCdatagroup *dat);

/**
 *  @brief  Creates a copy of the user data.
 *  @param  ncount [in] number of nodes
 *  @param  in [in] source data group
 *  @param  out [out] destination data group
 */
static int
basic_copy(int ncount, struct CCdatagroup *in, struct CCdatagroup *out);

/**
 *  @brief  Applies controlled type I shake
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_shake_i_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                         int delta, arrow_problem *new_problem);

/**
 *  @brief  Destructs constrained shake structure
 *  @param  fun [out] the function structure to destruct
 */
void
basic_shake_i_destruct(arrow_btsp_fun *fun);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_basic(int shallow, arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if(shallow)
    {
        fun->shallow = ARROW_TRUE;
        fun->apply = basic_shallow_apply;
    }
    else
    {
        fun->shallow = ARROW_FALSE;
        fun->apply = basic_deep_apply;
    }
    fun->feasible_length = 0;
    fun->destruct = basic_destruct;
    fun->feasible = basic_feasible;
    
    return ARROW_SUCCESS;
}

int
arrow_btsp_fun_basic_shake_i(int shallow, int rand_min, int rand_max, 
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
        fun->apply = basic_shake_i_deep_apply;
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
    data->infinity = INT_MAX;
    
    fun->feasible_length = 0;
    fun->destruct = basic_shake_i_destruct;
    fun->feasible = basic_feasible;
    
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
basic_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                    int delta, arrow_problem *new_problem)
{
    CCdatagroup *old_data = &(old_problem->data);
    CCdatagroup *new_data = &(new_problem->data);
    basic_data *user;
    
    if((new_data->userdat.data = malloc(sizeof(basic_data))) == NULL)
    {
        arrow_print_error("Error allocating memory for basic_data.");
        return ARROW_FAILURE;
    }
    user = (basic_data *)(new_data->userdat.data);
    user->dat = old_data;
    user->delta = delta;
        
    new_data->userdat.edgelen = basic_edgelen;
    new_data->userdat.copy_datagroup = basic_copy;
    CCutil_dat_setnorm(new_data, CC_USER);
    
    return ARROW_SUCCESS;
}

int
basic_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                 int delta, arrow_problem *new_problem)
{
    int i, j, cost;
    for(i = 0; i < new_problem->size; i++)
    {
        for(j = 0; j <= i; j++)
        {
            cost = old_problem->get_cost(old_problem, i, j);
            new_problem->data.adj[i][j] = (cost <= delta ? 0 : cost);
        }
    }
    return ARROW_SUCCESS;
}

void
basic_destruct(arrow_btsp_fun *fun)
{ }

int
basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
               int delta, double tour_length, int *tour)
{
    return (tour_length <= fun->feasible_length ? ARROW_TRUE : ARROW_FALSE);
}

static int 
basic_edgelen(int i, int j, struct CCdatagroup *dat)
{
    basic_data *user = (basic_data *)(dat->userdat.data);
    int old_cost = user->dat->edgelen(i, j, user->dat);
    return (old_cost <= user->delta ? 0 : old_cost);
}

static int
basic_copy(int ncount, struct CCdatagroup *in, struct CCdatagroup *out)
{
    basic_data *in_user;
    basic_data *out_user;
    
    if((out->userdat.data = malloc(sizeof(basic_data))) == NULL)
    {
        arrow_print_error("Error allocating memory for basic_data.");
        return 1;
    }
    
    in_user = (basic_data *)(in->userdat.data);
    out_user = (basic_data *)(out->userdat.data);
    
    out_user->dat = in_user->dat;
    out_user->delta = in_user->delta;
        
    out->userdat.edgelen = in->userdat.edgelen;
    out->userdat.copy_datagroup = in->userdat.copy_datagroup;
    CCutil_dat_setnorm(out, CC_USER);
    
    return 0;
}

int
basic_shake_i_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
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
            if((i == j) || (cost <= delta))
            {
                new_problem->data.adj[i][j] = 0;
            }
            else
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
        }
    }

CLEANUP:
    arrow_bintree_destruct(&tree);
    free(rand_list);
    return ret;
}

void
basic_shake_i_destruct(arrow_btsp_fun *fun)
{ 
    if((shake_info *)(fun->data) != NULL)
    {
        free(fun->data);
        fun->data = NULL;
    }
}
