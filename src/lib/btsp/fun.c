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
 * Private structures
 ****************************************************************************/
typedef struct fun_data
{
    arrow_btsp_fun *fun;
    arrow_problem *base_problem;
    int delta;
} fun_data;


/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
int
apply_shallow(arrow_btsp_fun *fun, arrow_problem *old_problem, 
              int delta, arrow_problem *new_problem);

int
apply_deep(arrow_btsp_fun *fun, arrow_problem *old_problem, 
           int delta, arrow_problem *new_problem);
           
int 
fun_get_cost(arrow_problem *this, int i, int j);

void 
fun_destruct(arrow_problem *this);

/**
 *  @brief  Retrieves cost between nodes i and j from Concorde's structure.
 *  @param  problem [in] pointer to arrow_problem structure
 *  @param  i [in] id of starting node
 *  @param  j [in] id of ending node
 *  @return cost between node i and node j
 */
int 
cc_get_cost(arrow_problem *problem, int i, int j);

/**
 *  @brief  Destructs Concorde's data structure
 *  @param  problem [in] pointer to arrow_problem structure
 */
void
cc_destruct(arrow_problem *problem);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_apply(arrow_btsp_fun *fun, arrow_problem *old_problem, 
                     int delta, arrow_problem *new_problem)
{
    new_problem->size = old_problem->size;
    new_problem->symmetric = old_problem->symmetric;
    new_problem->shallow = fun->shallow;
    strcpy(new_problem->name, old_problem->name);    
    
    if(fun->shallow)
        return apply_shallow(fun, old_problem, delta, new_problem);
    else
        return apply_deep(fun, old_problem, delta, new_problem);
}

void
arrow_btsp_fun_destruct(arrow_btsp_fun *fun)
{
    fun->destruct(fun);
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
apply_shallow(arrow_btsp_fun *fun, arrow_problem *old_problem, 
              int delta, arrow_problem *new_problem)
{
    fun_data *data = NULL;
    if((data = malloc(sizeof(fun_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for fun_data");
        return ARROW_FAILURE;
    }
    
    data->fun = fun;
    data->base_problem = old_problem;
    data->delta = delta;
    
    new_problem->type = ARROW_PROBLEM_DATA_BTSP_FUN;
    new_problem->data = (void *)data;
    new_problem->get_cost = fun_get_cost;
    new_problem->destruct = fun_destruct;
    
    return ARROW_SUCCESS;
}

int
apply_deep(arrow_btsp_fun *fun, arrow_problem *old_problem, 
           int delta, arrow_problem *new_problem)
{
    int i, j;
    int size = old_problem->size;    
    
    CCdatagroup *dat = NULL;
    if((dat = malloc(sizeof(CCdatagroup))) == NULL)
    {
        arrow_print_error("Could not allocate memory for CCdatagroup");
        return ARROW_FAILURE;
    }
    
    if(!arrow_util_CCdatagroup_init_matrix(size, dat))
    {
        arrow_print_error("Could not allocate memory for CC matrix");
        free(dat);
        return ARROW_FAILURE;
    }
    
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < i; j++)
            dat->adj[i][j] = fun->get_cost(fun, old_problem, delta, i, j);
    }
    
    new_problem->type = ARROW_PROBLEM_DATA_CONCORDE;
    new_problem->data = (void *)dat;
    new_problem->get_cost = cc_get_cost;
    new_problem->destruct = cc_destruct;
/*    
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            if(i != j)
                arrow_debug("C[%d,%d] = %d/%d/%d; ", i, j, old_problem->get_cost(old_problem, i, j),
                    fun->get_cost(fun, old_problem, delta, i, j), new_problem->get_cost(new_problem, i, j));
        }
        arrow_debug("\n");
    }
    arrow_debug("\n");
*/
    return ARROW_SUCCESS;
}

int 
fun_get_cost(arrow_problem *this, int i, int j)
{
    fun_data *d = (fun_data *)(this->data);
    return d->fun->get_cost(d->fun, d->base_problem, d->delta, i, j);
}

void 
fun_destruct(arrow_problem *this)
{ }

int 
cc_get_cost(arrow_problem *problem, int i, int j)
{
    CCdatagroup *dat = (CCdatagroup *)problem->data;    
    return dat->edgelen(i, j, dat);
}

void
cc_destruct(arrow_problem *problem)
{    
    if(problem->data != NULL)
    {
        CCdatagroup *dat = (CCdatagroup *)problem->data;
        CCutil_freedatagroup(dat);
    }
}
