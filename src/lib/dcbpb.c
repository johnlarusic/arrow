/**********************************************************doxygen*//** @file
 *  @brief   Degree constarined bottleneck paths bound
 *
 *  Implemenation of the degree constrained bottleneck paths bound (DCBPB) 
 *  used as a lower bound for the Bottleneck TSP objective value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/**
 *  @param  Solves the all-pairs bottleneck paths problem (a simple
 *          modification of the Floyd-Warshall alg for all-pairs shortest
 *          paths).
 *  @param  problem [in] problem data
 *  @param  ignore [in] vertex number to ignore
 *  @param  b [out] array will hold bottleneck path value for each
 *              pair of source/sink nodes
 *  @param  delta [out] the smallest cost among all bottleneck paths
 */
void
bottleneck_paths(arrow_problem *problem, int ignore, int **b, int *delta);

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_dcbpb_solve(arrow_problem *problem, arrow_bound_result *result)
{
    int ret = ARROW_SUCCESS;
    int i, j, k;
    int delta, max_tree, min_node;
    int in_cost, out_cost;
    int n = problem->size;
    double start_time, end_time;    
    
    start_time = arrow_util_zeit();
    
    int **b;
    int *b_space;
    if(!arrow_util_create_int_matrix(n, n, &b, &b_space))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    result->obj_value = INT_MAX;
    
    for(i = 0; i < n; i++)
    {
        bottleneck_paths(problem, i, b, &delta);
        
        min_node = INT_MAX;
        for(j = 0; j < n; j++)
        {
            if(i != j)
            {
                for(k = j; k < n; k++)
                {
                    if((i != k) && (j != k))
                    {
                        /* Assume we fix (i, j) and (k, i) arcs */
                        max_tree = delta;
                        
                        out_cost = problem->get_cost(problem, i, j);
                        if(max_tree < out_cost) 
                            max_tree = out_cost;

                        in_cost = problem->get_cost(problem, k, i);
                        if(max_tree < in_cost) 
                            max_tree = in_cost;
                        
                        if(max_tree < min_node)
                            min_node = max_tree;
                            
                        /* Now assume we fix (i, k) and (j, i) arcs */
                        max_tree = delta;
                        
                        out_cost = problem->get_cost(problem, i, k);
                        if(max_tree < out_cost) 
                            max_tree = out_cost;

                        in_cost = problem->get_cost(problem, j, i);
                        if(max_tree < in_cost) 
                            max_tree = in_cost;
                        
                        if(max_tree < min_node)
                            min_node = max_tree; 
                    }
                }
            }
        }
        
        if(min_node < result->obj_value)
            result->obj_value = min_node;
    }
    end_time = arrow_util_zeit();

    result->total_time = end_time - start_time;
    
CLEANUP:
    if(b_space != NULL) free(b_space);
    if(b != NULL) free(b);
    return ret;
}

/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
void
bottleneck_paths(arrow_problem *problem, int ignore, int **b, int *delta)
{
    int i, j, k, max;
    
    /* Initialize! */
    for(i = 0; i < problem->size; i++)
    {
        for(j = 0; j < problem->size; j++)
            b[i][j] = problem->get_cost(problem, i, j);
        b[i][i] = INT_MAX;
    }
    
    /* Dynamic programming technique GO! */
    for(k = 0; k < problem->size; k++)
    {
        if(k != ignore)
        {
            for(i = 0; i < problem->size; i++)
            {
                if((i != ignore) && (k != i))
                {
                    for(j = 0; j < problem->size; j++)
                    {
                        if((j != ignore) && (k != j) && (i != j))
                        {
                            max = (b[i][k] > b[k][j] ? b[i][k] : b[k][j]);
                            if(max < b[i][j])
                                b[i][j] = max;
                        }
                    }
                }
            }
        }
    }
    
    /* Find the largest bottleneck path cost */
    *delta = INT_MIN;
    for(i = 0; i < problem->size; i++)
    {
        if(i != ignore)
        {
            for(j = 0; j < problem->size; j++)
            {
                if((j != ignore) && (i != j))
                {
                    if(*delta < b[i][j])
                        *delta = b[i][j];
                }
            }
        }
    }
}