/**********************************************************doxygen*//** @file
 *  @brief  Bottleneck strongly connected spanning subgraph problem 
 *          implemenation.
 *
 *  Implemenation of the bottleneck strongly connected spanning subgraph 
 *  problem (BSCSSP) that's used as a lower bound for the Bottleneck TSP
 *  objective value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "lb.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Determines if the given graph is strongly connected.
 *  @param  problem [in] the problem instance
 *  @param  delta [in] delta parameter
 *  @param  result [out] the result!
 */
int
strongly_connected(arrow_problem *problem, int delta, int *result);

/**
 *  @brief  Performs a recursive depth-first search to test for connectivity
 *  @param  problem [in] the problem instance
 *  @param  delta [in] delta parameter
 *  @param  i [in] node index to search from
 *  @param  transpose [in] if true, calculates costs with transposed cost
 *              matrix
 *  @param  visited [out] array that marks nodes that have been visited
 */
void
strongly_connected_dfs(arrow_problem *problem, int delta, int i, 
                       int transpose, int *visited);
                       
/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_bscssp_solve(arrow_problem *problem, arrow_problem_info *info, 
                   arrow_bound_result *result)
{
    int ret;
    int low, high, median;
    int connected;
    double start_time, end_time;
    
    /* Start binary search */
    start_time = arrow_util_zeit();
    low = 0;
    high = info->cost_list_length - 1;
    while(low != high)
    {
        median = ((high - low) / 2) + low;
        
        /* Determine is graph is biconnected if we consider only costs less
         * than or equal to the median value. */
        ret = strongly_connected(problem, info->cost_list[median], 
                                 &connected);
        
        if(ret == ARROW_FAILURE)
        {
            result->obj_value = -1;
            return ARROW_FAILURE;
        }  
        else
        {
            if(connected == ARROW_TRUE)
                high = median;
            else
                low = median + 1;
        }
    }
    end_time = arrow_util_zeit();
    
    /* Return the cost we converged to as the answer */
    result->obj_value = info->cost_list[low];
    result->total_time = end_time - start_time;
    return ARROW_SUCCESS;
}

int
strongly_connected(arrow_problem *problem, int delta, int *result)
{
    int i;
    int ret = ARROW_SUCCESS;
    int *visited = NULL;
    
    /* Initialize arrays */
    ret = arrow_util_create_int_array(problem->size, &visited);
    if(ret == ARROW_FAILURE) goto CLEANUP; 
    
    /* We start by assuming the graph is strongly connected */
    *result = ARROW_TRUE;
    
    /* Perform DFS from node 0 to see if we can reach every node */
    for(i = 0; i < problem->size; i++)
        visited[i] = 0;

    strongly_connected_dfs(problem, delta, 0, ARROW_FALSE, visited);
    
    for(i = 0; i < problem->size; i++)
    {
        if(!visited[i])
        {
            *result = ARROW_FALSE;
            goto CLEANUP;
        }
        visited[i] = 0; /* Clear values for next DFS */
    }
    
    /* Now perform the same search with the transposed cost matrix */
    strongly_connected_dfs(problem, delta, 0, ARROW_TRUE, visited);
    
    for(i = 0; i < problem->size; i++)
    {
        if(!visited[i])
        {
            *result = ARROW_FALSE;
            goto CLEANUP;
        }
    }
    
CLEANUP:
    if(visited != NULL) free(visited);   
    return ret;
}

void
strongly_connected_dfs(arrow_problem *problem, int delta, int i, 
                       int transpose, int *visited)
{
    int j, cost;
    visited[i] = ARROW_TRUE;
    
    for(j = 0; j < problem->size; j++)
    {
        if((i != j) && (!visited[j]))
        {
            if(transpose)
                cost = problem->get_cost(problem, j, i);
            else
                cost = problem->get_cost(problem, i, j);
            
            if(cost <= delta)
            {
                strongly_connected_dfs(problem, delta, j, transpose, visited);
            }
        }
    }
}
