/**********************************************************doxygen*//** @file
 *  @brief   2-max bound implemenation.
 *
 *  Implemenation of the 2-max bound (2MB) used as a lower bound for the 
 *  Bottleneck TSP objective value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_2mb_solve(arrow_problem *problem, arrow_bound_result *result)
{
    /*
        Some explanation here: 
        - for symmetric problems we keep track of the smallest (alpha) and
          second smallest (beta) cost incident on each vertex and take the
          largest of all those values.  
        - for asymmetric problems we keep track of the smallest in-cost
          (alpha) and smallest out-cost (beta) incident on each vertex and
          take the largest of all those values.
    */
    int i, j, cost, alpha, beta;
    int max = INT_MIN;
    double start_time, end_time;
    
    start_time = arrow_util_zeit();
    
    for(i = 0; i < problem->size; i++)
    {
        alpha = INT_MAX;
        beta = INT_MAX;
        
        for(j = 0; j < problem->size; j++)
        {
            if(problem->symmetric)
            {
                cost = problem->get_cost(problem, i, j);
                if(cost < alpha)
                {
                    beta = alpha;
                    alpha = cost;
                }
                else if(cost < beta)
                {
                    beta = cost;
                }
            }
            else
            {
                cost = problem->get_cost(problem, i, j);
                if(cost < alpha) alpha = cost;
                
                cost = problem->get_cost(problem, j, i);
                if(cost < beta) beta = cost;
                    
            }
        }
        
        if(problem->symmetric)
        {
            if(max < beta) max = beta;
        }
        else
        {
            if(max < alpha) max = alpha;
            if(max < beta) max = beta;
        }
    }
    end_time = arrow_util_zeit();
    
    result->obj_value = max;
    result->total_time = end_time - start_time;
    return ARROW_SUCCESS;   
}
