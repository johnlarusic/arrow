/**********************************************************doxygen*//** @file
 * @brief   Balanced traveling salesman problem double threshold algorithm.
 *
 * Heuristic for solving the balanced traveling salesman problem double
 * threshold algorithm (BalTSP DT-Algorithm).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "lb.h"
#include "btsp.h"
#include "baltsp.h"


/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Determines if a tour is feasible given min/max costs based upon
 *          lower bounds for the BTSP.
 *  @param  problem [in] problem structure
 *  @param  min_cost [in] min_cost to consider for active edges
 *  @param  max_cost [in] max_cost to consider for active edges
 *  @param  is_feasible [out] ARROW_TRUE if tour feasibly could exist
 */
int 
dt2_lb_feasible(arrow_problem *problem, int min_cost, int max_cost, 
                int *is_feasible);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_balanced_tsp_dt2(arrow_problem *problem, 
                       arrow_problem_info *info,
                       arrow_baltsp_params *params,
                       double *lb_time,
                       arrow_btsp_result *tour_result)
{
    int ret = ARROW_SUCCESS;
    int is_feasible = ARROW_FALSE;
    int i, low, high;
    int p, q;
    int best_tour_low, best_tour_high;
    int low_val, high_val, max;
    int best_gap, cur_gap;
    double start_time, end_time;
    arrow_problem asym_problem;
    arrow_problem *solve_problem;
    arrow_btsp_result cur_tour_result;
    
    /* Print out debug information */
    arrow_debug("With Improvements? %s\n", (params->with_improvements ? "Yes" : "No"));
    arrow_debug("BalTSP Lower Bound: %d\n", params->lower_bound);
    arrow_debug("BTSP Min Cost: %d\n", params->btsp_min_cost);
    arrow_debug("BTSP Max Cost: %d\n", params->btsp_max_cost);
    arrow_debug("MSTSP Min Cost: %d\n", params->mstsp_min_cost);
    arrow_debug("Total solve steps: %d\n", params->num_steps);
    arrow_debug("\n");
    

    /* Determine if we need to convert a symmetric instance into an asymmetric instance */
    solve_problem = problem;
    if(!problem->symmetric)
    {
        if(!arrow_problem_abtsp_to_sbtsp(params->deep_copy, problem, params->infinity, &asym_problem))
        {
            arrow_print_error("Could not create symmetric transformation.");
            return EXIT_FAILURE;
        }
        solve_problem = &asym_problem;
    }
    
    /* Setup result structures */
    *lb_time = 0.0;
    arrow_btsp_result_init(solve_problem, &cur_tour_result);
    tour_result->optimal = ARROW_FALSE;
    tour_result->found_tour = ARROW_FALSE;
    tour_result->total_time = 0.0;
    tour_result->bin_search_steps = 0;
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        tour_result->solver_attempts[i] = 0;
        tour_result->solver_time[i] = 0.0;
    }    
    
    /* Find low/high/max values */
    low = 0; best_tour_low = 0;
    if(!arrow_problem_info_cost_index(info, params->btsp_min_cost, &low))
    {
        arrow_debug("Could not find btsp_min_cost in cost_list\n");
        arrow_debug(" - Using low = 0\n");
        low = 0;
    }
    if(!arrow_problem_info_cost_index(info, params->btsp_max_cost, &high))
    {
        arrow_debug("Could not find btsp_max_cost in cost_list\n");
        arrow_debug(" - Using high = 0\n");
        high = 0;
    }
    best_tour_low = params->btsp_min_cost;
    best_tour_high = params->btsp_max_cost;
    
    if(best_tour_high - best_tour_low == params->lower_bound)
    {
        arrow_debug("The BTSP solution is an optimal BalTSP solution!\n");
        goto CLEANUP;
    }

    if(params->mstsp_min_cost == INT_MAX)
        params->mstsp_min_cost = info->max_cost;
    if(!arrow_problem_info_cost_index(info, params->mstsp_min_cost, &max))
    {
        arrow_debug("Could not find mstsp_min_cost in cost_list\n");
        arrow_debug(" - Using max_cost = %d\n", info->max_cost);
        max = info->cost_list_length - 1;
    }
    
    low++;
    printf("low = %d; high = %d; max = %d\n", low, high, max);
    
    /* Main loop */
    arrow_debug("Starting balanced search [%d,%d]\n", info->cost_list[low], info->cost_list[high]);
    while((low <= high) && (low <= max) && (high < info->cost_list_length))
    {   
        /* Check to see if we're under the time bound */
        if(tour_result->total_time > params->timebound)
        {
            arrow_debug("Reached timebound of %.0fs.\n", params->timebound);
            break;
        }
        
        low_val = infgito->cost_list[low];
        high_val = info->cost_list[high];
        printf("%d <= C[i,j] <= %d: ", low_val, high_val);
                
        tour_result->bin_search_steps++;
        
        /* First check if lower bounds are feasible */
        start_time = arrow_util_zeit();
        if(!dt2_lb_feasible(problem, low_val, high_val, &is_feasible))
        {
            arrow_debug("Error checking lower bound feasibility\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        end_time = arrow_util_zeit();
        *lb_time += end_time - start_time;
        
        /* If the lower bounds check out, then ask feasibility question */
        if(is_feasible)
        {            
            arrow_debug("LB is feasible, now trying to find a tour...\n");
            start_time = arrow_util_zeit();
            if(!arrow_btsp_feasible(solve_problem, params->num_steps, params->steps,
                                    low_val, high_val, &is_feasible, &cur_tour_result))
            {
                arrow_debug("Error checking tour feasibility\n");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
            end_time = arrow_util_zeit();
            tour_result->total_time += end_time - start_time;
            
            for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
            {
                tour_result->solver_attempts[i] += cur_tour_result.solver_attempts[i];
                tour_result->solver_time[i] += cur_tour_result.solver_time[i];
            }
        }
        
        if(is_feasible)
        {
            arrow_debug("Found a feasible tour!\n");
            arrow_debug("  - Tour MinCost = %d, MaxCost = %d\n", cur_tour_result.min_cost, cur_tour_result.max_cost);
            
            best_gap = best_tour_high - best_tour_low;
            cur_gap = cur_tour_result.max_cost - cur_tour_result.min_cost; 
            
            /* Find the indicates for the min and max cost in tour */
            if(!arrow_problem_info_cost_index(info, cur_tour_result.min_cost, &p))
            {
                arrow_print_error("Could not find min_cost in cost_list\n");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
            if(!arrow_problem_info_cost_index(info, cur_tour_result.max_cost, &q))
            {
                arrow_print_error("Could not find max_cost in cost_list\n");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
            
            if(cur_gap < best_gap)
            {
                arrow_debug("Tour is better than the current best solution.\n");
                
                /* Copy over new best objective value and tour */
                tour_result->min_cost = cur_tour_result.min_cost;
                tour_result->max_cost = cur_tour_result.max_cost;
                tour_result->tour_length = cur_tour_result.tour_length;
            
                if(tour_result->tour != NULL)
                {
                    tour_result->found_tour = ARROW_TRUE;
                    if(problem->symmetric)
                    {
                        for(i = 0; i < problem->size; i++)
                            tour_result->tour[i] = cur_tour_result.tour[i];
                    }
                    else
                    {
                        arrow_util_sbtsp_to_abstp_tour(solve_problem, cur_tour_result.tour, 
                                                       tour_result->tour);
                        tour_result->tour_length += problem->size * params->infinity;
                    }
                }
        
                best_tour_low = cur_tour_result.min_cost;
                best_tour_high = cur_tour_result.max_cost;
                
                if(cur_gap == params->lower_bound)
                {
                    arrow_debug("We found a gap equal to the lower bound!\n");
                    break;
                }
            }
            
            /* Theorem 1 Improvement */
            if(params->with_improvements)
            {
                if(best_tour_high - best_tour_low + params->mstsp_min_cost <= high_val)
                {
                    arrow_debug("Tour is heuristically optimal.\n");
                    break;
                }
            }
            
            low = p + 1;
            
            /* Theorem 2 Improvement */
            if(params->with_improvements)
            {
                while(info->cost_list[high] - info->cost_list[low] > best_gap)
                    low++;
            }
            
            while(info->cost_list[high] - info->cost_list[low] < params->lower_bound)
                high++;
        }
        else
        {
            high++;
            
            /* Theorem 2 Improvement */
            /*
            if(params->with_improvements)
            {
                while(info->cost_list[high] - info->cost_list[low] >= best_gap)
                    low++;
            }
            */
        }
        
        arrow_debug("low = %d, high = %d\n", low, high);
    }
    
    arrow_debug("\n");

CLEANUP:
    if(!problem->symmetric)
        arrow_problem_destruct(&asym_problem);
    arrow_btsp_result_destruct(&cur_tour_result);
    return ret;
}

int 
dt2_lb_feasible(arrow_problem *problem, int min_cost, int max_cost, 
                int *is_feasible)
{
    *is_feasible = ARROW_FALSE;
        
    if(!arrow_bbssp_biconnected(problem, min_cost, max_cost, is_feasible))
    {
        printf("Could not solve BBSSP for balanced LB.\n");
        return ARROW_FAILURE;
    }
    if(!*is_feasible)
    {
        printf("BBSSP is infeasible.\n");
        return ARROW_SUCCESS;
    }

    if(!arrow_bap_has_assignment(problem, min_cost, max_cost, is_feasible))
    {
        arrow_print_error("Could not solve BAP for balanced LB\n");
        return ARROW_FAILURE;
    }
    if(!*is_feasible)
    {
        printf("BAP is infeasible.\n");
        return ARROW_SUCCESS;
    }

    if(!problem->symmetric)
    {
        if(!arrow_bscssp_connected(problem, min_cost, max_cost, is_feasible))
        {
            arrow_print_error("Could not solve BSCSSP for balanced LB\n");
            return ARROW_FAILURE;
        }
        if(!*is_feasible)
        {
            printf("BSCSSP is infeasible.\n");
            return ARROW_SUCCESS;
        }
    }
    
    return ARROW_SUCCESS;
}
