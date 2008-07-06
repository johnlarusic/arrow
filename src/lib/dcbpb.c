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
 *  @brief  Solves the all-pairs bottleneck paths problem (a simple
 *          modification of the Floyd-Warshall alg for all-pairs shortest
 *          paths).
 *  @param  problem [in] problem data
 *  @param  ignore [in] vertex number to ignore
 *  @param  b [out] array will hold bottleneck path value for each
 *              pair of source/sink nodes
 */
void
bottleneck_paths(arrow_problem *problem, int ignore, int **b);

/*
 *  @brief  Returns the max of the four values
 *  @param  i [in] first number
 *  @param  j [in] second number
 *  @param  k [in] third number
 *  @param  l [in] fourth number
 *  @return the largest of i, j, k, l
 */
int
max(int i, int j, int k, int l);

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_dcbpb_solve(arrow_problem *problem, arrow_bound_result *result)
{
    int ret = ARROW_SUCCESS;
    int i, j, k, l;
    int bottleneck, max_tree, min_node;
    int in_cost, out_cost;
    int n = problem->size;
    double start_time, end_time;    
    
    start_time = arrow_util_zeit();
    
    int *alpha;
    int *gamma;
    if(!arrow_util_create_int_array(n, &alpha))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(!arrow_util_create_int_array(n, &gamma))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    int **b;
    int *b_space;
    if(!arrow_util_create_int_matrix(n, n, &b, &b_space))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    
    bottleneck = INT_MIN;

    for(i = 0; i < n; i++)
    {
        bottleneck_paths(problem, i, b);
        min_node = INT_MAX;
        
        /*
        printf("P^%d\t", i);
        for(j = 0; j < n; j++)
            if(j != i)
                printf("[%d]\t", j);
        printf("\n");
        
        for(j = 0; j < n; j++)
        {
            if(j != i)
            {
                printf("[%d]\t", j);
                for(k = 0; k < n; k++)
                {
                    if((k != j) && (k != i))
                    {
                        printf("%d\t", b[j][k]);
                    }
                    else if(j == k)
                        printf("-\t");
                }
                printf("\n");
            }
        }
        */
        
        /* Find the largest bottleneck path values leaving (alpha[j]) and 
           entering (gamma[j]) of nodes j and k */
        for(j = 0; j < n; j++)
        {
            if(j != i)
            {
                alpha[j] = INT_MIN;
                gamma[j] = INT_MIN;
             
                for(k = 0; k < n; k++)
                {
                    if((k != j) && (k != i))
                    {
                        if(alpha[j] < b[j][k]) alpha[j] = b[j][k];
                        if(gamma[j] < b[k][j]) gamma[j] = b[k][j];
                    }
                }
            }
        }
        
        /*
        for(j = 0; j < n; j++)
        {
            if(j != i)
                printf("alpha[%d] = %d; gamma[%d] = %d\n", j, alpha[j], j, gamma[j]);
        }  
        printf("\n");
        */
        
        for(j = 0; j < n; j++)
        {
            if(j != i)
            {
                out_cost = problem->get_cost(problem, i, j);
                if(out_cost < min_node)
                {
                    for(k = 0; k < n; k++)
                    {
                        if((k != i))
                        {
                            in_cost = problem->get_cost(problem, k, i);
                            if(in_cost < min_node)
                            {
                                max_tree = 
                                    max(alpha[j], gamma[k], out_cost, in_cost);
                                if(max_tree < min_node) min_node = max_tree;
                            }
                        }
                    }
                }
            }
        }
        
        if(bottleneck < min_node)
            bottleneck = min_node;
    }
    end_time = arrow_util_zeit();

    result->obj_value = bottleneck;
    result->total_time = end_time - start_time;
    
CLEANUP:
    if(alpha != NULL) free(alpha);
    if(gamma != NULL) free(gamma);
    if(b_space != NULL) free(b_space);
    if(b != NULL) free(b);
    return ret;
}

/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
void
bottleneck_paths(arrow_problem *problem, int ignore, int **b)
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
}

int
max(int i, int j, int k, int l)
{
    int max = i;
    if(max < j) max = j;
    if(max < k) max = k;
    if(max < l) max = l;
    return max;
}
