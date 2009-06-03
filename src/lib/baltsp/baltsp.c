/**********************************************************doxygen*//** @file
 * @brief   Balanced traveling salesman problem (Balanced TSP) methods.
 *
 * Heuristic for solving the balanced traveling salesman problem 
 * (Balanced TSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "lb.h"
#include "btsp.h"
#include "baltsp.h"

#define TASK_FINE_LB 1
#define TASK_FIND_TOUR 2


/**
 *  @brief  Binary tree data structure.
 */
typedef struct balanced_tsp_lb
{
    int min_cost;
    int max_cost;
    double total_time;
} balanced_tsp_lb;

int balanced_lb_feasible(arrow_problem *problem, int min_cost, int max_cost, 
                         int *is_feasible);

int balanced_search(int task, arrow_problem *problem, arrow_problem_info *info,
                    arrow_btsp_params *params, void *result);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_balanced_tsp_solve(arrow_problem *problem, 
                         arrow_problem_info *info,
                         arrow_btsp_params *params, 
                         arrow_bound_result *lb_result,
                         arrow_btsp_result *tour_result)
{
    int ret = ARROW_SUCCESS;
    int is_feasible = ARROW_FALSE;
    balanced_tsp_lb bal_lb_result;
    
    /* Print out debug information */
    arrow_debug("Initial Lower Bound: %d\n", params->lower_bound);
    arrow_debug("Initial Upper Bound: %d\n", params->upper_bound);
    if(params->num_steps > 0)
    {
        arrow_debug("Total solve steps: %d\n", params->num_steps);
    }
    arrow_debug("\n");
    
    /* Find a good lower bound via a balanced search */
    arrow_debug("Trying to find a good lower bound...\n");
    ret = balanced_search(TASK_FINE_LB, problem, info, params, (void *)&bal_lb_result);
    if(ret != ARROW_SUCCESS)
    {
        arrow_print_error("Error trying when trying to find a lower bound\n");
        return ARROW_FAILURE;
    }
    lb_result->obj_value = bal_lb_result.max_cost - bal_lb_result.min_cost;
    lb_result->total_time = bal_lb_result.total_time;
    
    /* Try and find a tour using lower bound findings */
    arrow_debug("Trying to find a tour from lower bound...\n");
    tour_result->total_time = 0.0;
    ret = arrow_btsp_feasible(problem, params->num_steps, params->steps,
                             bal_lb_result.min_cost, bal_lb_result.max_cost,
                             &is_feasible, tour_result);
    if(ret != ARROW_SUCCESS)
    {
        arrow_print_error("Error trying to find a tour from lower bound\n");
        return ARROW_FAILURE;
    }
    if(is_feasible)
    {
        tour_result->optimal = ARROW_TRUE;
        tour_result->found_tour = ARROW_TRUE;
        return ARROW_SUCCESS;
    }
    
    /* Otherwise, do a balanced search to find a good tour */
    arrow_debug("Performing balanced search to find a good tour...\n");
    ret = balanced_search(TASK_FIND_TOUR, problem, info, params, (void *)&tour_result);
    if(ret != ARROW_SUCCESS)
    {
        arrow_print_error("Error trying when trying to find a good tour\n");
        return ARROW_FAILURE;
    }
    if(tour_result->max_cost - tour_result->min_cost == lb_result->obj_value)
    {
        tour_result->optimal = ARROW_TRUE;
        tour_result->found_tour = ARROW_TRUE;
    }
    
    return ARROW_SUCCESS;
}

int balanced_lb_feasible(arrow_problem *problem, int min_cost, int max_cost, 
                         int *is_feasible)
{
    *is_feasible = ARROW_FALSE;
    return ARROW_SUCCESS;
}

int balanced_search(int task, arrow_problem *problem, arrow_problem_info *info,
                    arrow_btsp_params *params, void *result)
{
    int ret = ARROW_SUCCESS;
    int is_feasible;
    int i, low, best_low, high, best_high, low_val, high_val;
    double start_time = arrow_util_zeit();
    double end_time;

    /* The result given back depends on the task */
    arrow_btsp_result *best_tour_result;
    arrow_btsp_result cur_tour_result;
    balanced_tsp_lb *best_lb_result;
    if(task == TASK_FIND_TOUR)
    {
        arrow_btsp_result_init(problem, &cur_tour_result);
        best_tour_result = (arrow_btsp_result *)result;
        best_tour_result->optimal = ARROW_FALSE;
        best_tour_result->found_tour = ARROW_FALSE;
    }
    else
    {
        best_lb_result = (balanced_tsp_lb *)result;
        best_lb_result->min_cost = info->cost_list[0];
        best_lb_result->max_cost = params->lower_bound;
    }
    
    /* Find low/high values */
    low = 1; best_low = 1;
    if(arrow_problem_info_cost_index(info, params->lower_bound, &high))
    {
        arrow_print_error("Could not find lower_bound in cost_list\n");
        goto CLEANUP;
    }
    best_high = high;
    
    /* Main loop */
    while((low <= high) && (info->cost_list[high] <= params->upper_bound))
    {
        low_val = info->cost_list[low];
        high_val = info->cost_list[high];
        
        /* Feasibility question depends on task */
        if(task == TASK_FIND_TOUR)
            ret = arrow_btsp_feasible(problem, params->num_steps, params->steps,
                                      low_val, high_val, &is_feasible, &cur_tour_result);
        else
            ret = balanced_lb_feasible(problem, low_val, high_val, &is_feasible);

        if(ret != ARROW_SUCCESS)
        {
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        if(is_feasible)
        {
            if(task == TASK_FIND_TOUR)
            {
                /* Copy over new best objective value and tour */
                best_tour_result->min_cost = cur_tour_result.min_cost;
                best_tour_result->max_cost = cur_tour_result.max_cost;
                best_tour_result->tour_length = cur_tour_result.tour_length;
                
                if(best_tour_result->tour != NULL)
                {
                    best_tour_result->found_tour = ARROW_TRUE;
                    for(i = 0; i < problem->size; i++)
                        best_tour_result->tour[i] = cur_tour_result.tour[i];
                }
                
                best_low = cur_tour_result.min_cost;
                best_high = cur_tour_result.max_cost;
            }
            else
            {
                best_lb_result->min_cost = low_val;
                best_lb_result->max_cost = high_val;
                best_low = low_val;
                best_high = high_val;
            }
            
            low++;
        }
        else
        {
            high++;
            while(info->cost_list[high] - info->cost_list[low] > best_high - best_low)
                low++;
        }
        
    }
    
CLEANUP:
    end_time = arrow_util_zeit() - start_time;
    if(task == TASK_FIND_TOUR)
    {
        best_tour_result->total_time += end_time;
        arrow_btsp_result_destruct(&cur_tour_result);
    }
    else
    {
        best_lb_result->total_time = end_time;
    }
    return ret;
}
