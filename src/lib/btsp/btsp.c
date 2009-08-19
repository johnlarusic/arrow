/**********************************************************doxygen*//** @file
 * @brief   Bottleneck traveling salesman problem (BTSP) methods.
 *
 * Heuristic for solving the bottleneck traveling salesman problem (BTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_btsp_solve(arrow_problem *problem, arrow_problem_info *info,
                 arrow_btsp_params *params, arrow_btsp_result *result)
{    
    int ret = ARROW_SUCCESS;
    int is_feasible;
    int i, low, high, median, median_val;
    double start_time = arrow_util_zeit();
    
    arrow_btsp_result cur_result;
    arrow_btsp_result_init(problem, &cur_result);
    
    result->optimal = ARROW_FALSE;
    result->found_tour = ARROW_FALSE;

    /* Print out debug information */
    arrow_debug("Confirm solution?: %s\n", 
        params->confirm_sol == ARROW_TRUE ? "Yes" : "No");
    arrow_debug("Supress EBST?: %s\n", 
        params->supress_ebst == ARROW_TRUE ? "Yes" : "No");
    arrow_debug("Find short tour?: %s\n", 
        params->find_short_tour == ARROW_TRUE ? "Yes" : "No");
    arrow_debug("Initial Lower Bound: %d\n", params->lower_bound);
    arrow_debug("Initial Upper Bound: %d\n", params->upper_bound);  
    if(params->num_steps > 0)
    {
        arrow_debug("Total solve steps: %d\n", params->num_steps);
    }
    arrow_debug("\n");
    
    /* Start enhanced threshold heuristic */
    arrow_debug("Starting enhanced threshold heuristic\n");
    arrow_debug("Current solution: %d\n", result->max_cost);
    ret = arrow_btsp_feasible(problem, params->num_steps, params->steps, INT_MIN, 
                              params->lower_bound, &is_feasible, result);
    if(ret != ARROW_SUCCESS)
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(is_feasible)
    {
        arrow_debug("A tour was found!.\n");
        result->optimal = ARROW_TRUE;
        result->found_tour = ARROW_TRUE;
        goto CONFIRM;
    }
    
    /* Start enhanced binary search threshold heuristic */
    if(params->supress_ebst) goto CONFIRM;
    arrow_debug("\nStarting enhanced binary search threshold heuristic.\n");

    ret = arrow_util_binary_search(info->cost_list, info->cost_list_length,
                                   params->lower_bound, &low);
    if(!ret)
    {
        low--;
        arrow_debug("Lower bound not in cost list, so pick next lowest\n");
        arrow_debug("Next lowest cost is %d...", info->cost_list[low]);
    }
    
    int upper_bound = params->upper_bound;
    if(result->max_cost < upper_bound) upper_bound = result->max_cost;
    if(upper_bound == INT_MAX)
    {
        arrow_debug("Taking largest cost as upperbound...\n");
        high = info->cost_list_length - 1;
    }
    else
    {
        ret = arrow_util_binary_search(info->cost_list, info->cost_list_length,
                                       upper_bound, &high);
    }
    
    arrow_debug("Starting binary search.\n");
    while(low != high)
    {
        median = ((high - low) / 2) + low;
        median_val = info->cost_list[median];
        arrow_debug("low = %d; high = %d; median = %d\n", 
                    info->cost_list[low], info->cost_list[high], median_val);
        
        ret = arrow_btsp_feasible(problem, params->num_steps, params->steps, 
                                  INT_MIN, median_val, &is_feasible, &cur_result);
        if(ret != ARROW_SUCCESS)
        {
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        if(is_feasible)
        {
            arrow_debug("A tour was found!\n");
            result->found_tour = ARROW_TRUE;
            
            /* Check to see if we found a smaller solution than our median */
            if(cur_result.max_cost < median_val)
            {
                arrow_debug(" - Found a smaller solution than our median: %d\n",
                            cur_result.max_cost);
                arrow_util_binary_search(info->cost_list, 
                                         info->cost_list_length,
                                         cur_result.max_cost, &high);
            }
            else
            {
                high = median;
            }
            
            /* Copy over new best objective value and tour */
            result->min_cost = cur_result.min_cost;
            result->max_cost = cur_result.max_cost;
            result->tour_length = cur_result.tour_length;
            if(result->tour != NULL)
            {
                result->found_tour = ARROW_TRUE;
                for(i = 0; i < problem->size; i++)
                    result->tour[i] = cur_result.tour[i];
            }   
        }
        else
        {
            arrow_debug("A tour could not be found.\n");
            low = median + 1;
            
            /* Check to see if we found a smaller solution than our median */
            if(cur_result.max_cost < median_val)
            {
                arrow_debug(" - Found a smaller solution than our best: %d\n",
                            cur_result.max_cost);
                arrow_debug(" - Lower upper bound to this value\n");
                arrow_util_binary_search(info->cost_list, 
                                         info->cost_list_length,
                                         cur_result.max_cost, &high);
                                         
                /* Copy over new best objective value and tour */
                result->max_cost = cur_result.max_cost;
                if(result->tour != NULL)
                {
                    result->found_tour = ARROW_TRUE;
                    for(i = 0; i < problem->size; i++)
                        result->tour[i] = cur_result.tour[i];
                }
            }
            
            if(low > high) low = high;
        }
        arrow_debug("\n");
        
        /* Copy over other important information */
        result->bin_search_steps++;
        for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
        {
            result->solver_attempts[i] += cur_result.solver_attempts[i];
            result->solver_time[i] += cur_result.solver_time[i];
        }
    }

    /* Confirm the solution if required.  If we can find a Hamiltonian cycle
       using costs strictly less than our objective value, then our tour
       is not optimal. */
CONFIRM:
    if(params->confirm_sol)
    {
        arrow_debug("Confirming solution...\n");
        arrow_btsp_solve_plan confirm_plan_steps[1] = 
        {
           params->confirm_plan
        };
        
        printf("checking feasibility...\n");
        ret = arrow_btsp_feasible(problem, 1, confirm_plan_steps, INT_MIN, 
                                  result->max_cost - 1, &is_feasible, &cur_result);
        if(ret != ARROW_SUCCESS)
        {
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        if(is_feasible)
        {
            arrow_debug("Tour found -- solution not optimal.\n");
            result->found_tour = ARROW_TRUE;
            
            /* Copy over new best objective value and tour */
            result->max_cost = cur_result.max_cost;
            result->tour_length = cur_result.tour_length;
            
            if(result->tour != NULL)
            {
                result->found_tour = ARROW_TRUE;
                for(i = 0; i < problem->size; i++)
                    result->tour[i] = cur_result.tour[i];
            }
        }
        else
        {
            arrow_debug("Tour could not be found -- solution optimal!\n");
            result->optimal = ARROW_TRUE;
        }
        
        result->solver_attempts[ARROW_TSP_CC_EXACT] 
            += cur_result.solver_attempts[ARROW_TSP_CC_EXACT];
        result->solver_time[ARROW_TSP_CC_EXACT] 
            += cur_result.solver_time[ARROW_TSP_CC_EXACT];
    }

                           
CLEANUP:
    /* See if we've reached the lower bound to guarantee optimality */
    if(result->max_cost == params->lower_bound)
        result->optimal = ARROW_TRUE;
    result->total_time = arrow_util_zeit() - start_time;

    arrow_btsp_result_destruct(&cur_result);
    return ret;
}
