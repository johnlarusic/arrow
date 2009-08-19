/**********************************************************doxygen*//** @file
 * @brief   Balanced traveling salesman problem iterative bottleneck
 *          algorithm.
 *
 * Heuristic for solving the balanced traveling salesman problem iterative
 * bottleneck algorithm (BalTSP IB-Algorithm).
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
 *  @brief  Calculates the best BTSP lower bound
 *  @param  problem [in] problem structure
 *  @param  lower_bound [out] the best lower bound value
 */
int 
btsp_lower_bound(arrow_problem *problem, arrow_problem_info *info, int *lower_bound);

/**
 *  @brief  Calculates BTSP lower bounds for each unique cost
 *  @param  problem [in] problem structure
 *  @param  info [in] problem info
 *  @param  max_index [in] largest index to bother searching
 *  @param  gap_bound [out] array of gap bounds for each cost
 *  @param  best_gap [out] smallest gap bound calculated
 *  @param  best_low [out] lower cost value for the best gap bound
 *  @param  best_high [out] upper cost value for the best gap bound
 *  @param  total_time [out] total time spent calculating lower bound values
 */
int
btsp_bounds(arrow_problem *problem, arrow_problem_info *info, int max_index, 
            int *gap_bound, int *best_gap, int *best_low, int *best_high, 
            double *total_time);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_balanced_tsp_ib(arrow_problem *problem, 
                      arrow_problem_info *info,
                      arrow_btsp_params *params, 
                      int lb_only, int with_improvements,
                      arrow_bound_result *lb_result,
                      arrow_btsp_result *tour_result)
{
    int ret = ARROW_SUCCESS;
    int i, low;
    int best_lb_low, best_lb_high;
    int best_tour_low, best_tour_high;
    int low_val, max;
    int best_gap, cur_gap, lb_gap;
    int *btsp_lb;
    double start_time, end_time;
    arrow_btsp_fun fun_ib;
    arrow_problem *solve_problem;
    arrow_problem ib_problem;
    arrow_btsp_result cur_tour_result;
    
    /* Print out debug information */
    arrow_debug("LB Only? %s\n", (lb_only ? "Yes" : "No"));
    arrow_debug("With Improvements? %s\n", (with_improvements ? "Yes" : "No"));
    arrow_debug("Initial Lower Bound: %d\n", params->lower_bound);
    arrow_debug("Initial Upper Bound: %d\n", params->upper_bound);
    if(params->num_steps > 0)
    {
        arrow_debug("Total solve steps: %d\n", params->num_steps);
    }
    arrow_debug("\n");
    
    /* For future use, possibly, to handle asymmetric instances */
    solve_problem = problem;
    
    /* Setup result structures */
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
    lb_result->total_time = 0.0;
    
    best_tour_high = info->max_cost;
    best_tour_low = info->min_cost;
    
    /* Find high values */
    if(params->upper_bound == INT_MAX)
        params->upper_bound = info->max_cost;
    if(!arrow_problem_info_cost_index(info, params->upper_bound, &max))
    {
        arrow_debug("Could not find upper_bound in cost_list\n");
        arrow_debug(" - Using max_cost = %d\n", info->max_cost);
        max = info->cost_list_length - 1;
    }
    
    /* Create function for ignoring C[i,j] < low_val */
    if(!arrow_baltsp_fun_ib(ARROW_TRUE, &fun_ib))
    {
        arrow_print_error("Could not create IB transformation function\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Calculate BTSP bounds for each unique cost */
    if(!arrow_util_create_int_array(max + 1, &btsp_lb))
    {
        arrow_print_error("Could not create gap_bound array\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    if(!btsp_bounds(problem, info, max, btsp_lb, &(lb_result->obj_value), 
                    &best_lb_low, &best_lb_high, &(lb_result->total_time)))
    {
        arrow_print_error("Could not calculate gap bounds\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Print values out for debug purposes */
    arrow_debug("z_i\tLB\tGAP\n");
    for(i = 0; i <= max; i++)
    {
        arrow_debug("%d\t%d\t%d\n", info->cost_list[i], btsp_lb[i], 
                    btsp_lb[i] - info->cost_list[i]);
    }    
   
    /* Main loop */
    if(!lb_only)
    {
        arrow_debug("Starting iterative bottleneck search\n");
        low = 0;
        while(low <= max)
        {        
            low_val = info->cost_list[low];
            printf("C[i,j] >= %d:\n", low_val);
            
            tour_result->bin_search_steps++;
            
            lb_gap = btsp_lb[low] - low_val;
            best_gap = best_tour_high - best_tour_low;

            arrow_debug("LB-Gap %d vs. Best-Gap %d\n", lb_gap, best_gap);
            
            /* If the lower bounds look promising, then ask feasibility question */
            if(lb_gap < best_gap)
            {
                /* Create problem that ignores C[i,j] < low_val */
                if(!arrow_btsp_fun_apply(&fun_ib, solve_problem, low_val, info->max_cost, &ib_problem))
                {
                    arrow_debug("Could not create IB cost matrix\n");
                    ret = ARROW_FAILURE;
                    goto CLEANUP;
                }
                        
                arrow_debug("LB is promising, now trying to find a tour...\n");
                start_time = arrow_util_zeit();
                params->lower_bound = btsp_lb[low];
                if(!arrow_btsp_solve(&ib_problem, info, params, &cur_tour_result))
                {
                    arrow_debug("Error searching for BTSP tour\n");
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
                
                arrow_problem_destruct(&ib_problem);
            
                /* See if we improved the best known gap */
                cur_gap = cur_tour_result.max_cost - cur_tour_result.min_cost;
                if(cur_gap < best_gap)
                {
                    arrow_debug("Tour is better than the current best solution.\n");
                    tour_result->min_cost = cur_tour_result.min_cost;
                    tour_result->max_cost = cur_tour_result.max_cost;
                    tour_result->tour_length = cur_tour_result.tour_length;
                    tour_result->found_tour = ARROW_TRUE;
                
                    if(tour_result->tour != NULL)
                    {
                        for(i = 0; i < problem->size; i++)
                            tour_result->tour[i] = cur_tour_result.tour[i];
                    }
        
                    best_tour_low = cur_tour_result.min_cost;
                    best_tour_high = cur_tour_result.max_cost;
                    arrow_debug("Tour exists in [%d,%d] = %d\n", best_tour_low, best_tour_high,
                        best_tour_high - best_tour_low);
                }
                else
                {
                    arrow_debug("Tour is no better than the current best solution.\n");
                }
            }
            
            low++;
        
            if(btsp_lb[low] >= info->max_cost)
            {
                arrow_debug("Lower Bound is equal to max cost in problem so we can quit\n");
                break;
            }
        }
        arrow_debug("\n");
    }

CLEANUP:
    free(btsp_lb);
    arrow_btsp_fun_destruct(&fun_ib);
    arrow_btsp_result_destruct(&cur_tour_result);
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int 
btsp_lower_bound(arrow_problem *problem, arrow_problem_info *info, int *lower_bound)
{
    arrow_bound_result result;
    
    if(!arrow_bbssp_solve(problem, info, &result))
    {
        arrow_debug("Error finding BBSSP lower bound\n");
        return ARROW_FAILURE;
    }
    *lower_bound = result.obj_value;
    
    return ARROW_SUCCESS;
}

int
btsp_bounds(arrow_problem *problem, arrow_problem_info *info, int max_index, 
            int *btsp_lb, int *best_gap, int *best_low, int *best_high, 
            double *total_time)
{
    int ret = ARROW_SUCCESS;
    int i;
    int cost;
    int lb;
    int gap;
    double start_time;
    double end_time;
    arrow_btsp_fun fun_ib;
    arrow_problem ib_problem;
    
    /* Create function for ignoring C[i,j] < low_val */
    if(!arrow_baltsp_fun_ib(ARROW_TRUE, &fun_ib))
    {
        arrow_print_error("Could not create IB transformation function\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    *best_gap = INT_MAX;
    *total_time = 0.0;
    
    for(i = 0; i <= max_index; i++)
    {
        cost = info->cost_list[i];
        
        /* Create problem that ignores C[i,j] < low_val */
        if(!arrow_btsp_fun_apply(&fun_ib, problem, cost, info->max_cost, &ib_problem))
        {
            arrow_print_error("Could not create IB cost matrix\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        /* Calculate lower bound */
        start_time = arrow_util_zeit();
        if(!btsp_lower_bound(&ib_problem, info, &lb))
        {
            arrow_print_error("Error finding BTSP lower bound\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        end_time = arrow_util_zeit();
        *total_time += (end_time - start_time);
        
        /* Store calculated value for later, update best lower bound */
        gap = lb - cost;
        btsp_lb[i] = lb;    
        if(gap < *best_gap)
        {
            *best_low = cost;
            *best_high = lb;
            *best_gap = gap;
        }
        
        /* Destroy problem */
        arrow_problem_destruct(&ib_problem);
    }
    arrow_debug("\n");

CLEANUP:
    arrow_btsp_fun_destruct(&fun_ib);
    return ret;
}

int
gap_tiers(arrow_problem_info *info, int *gap_bound, int max_index, arrow_llist **tier)
{
    int ret = ARROW_SUCCESS;
    int i;
    int *gap;
    int gap_length;
    arrow_bintree tree;
    arrow_hash hash;
    
    /* Sort the unique gap values */
    arrow_bintree_init(&tree);        
    for(i = 0; i <= max_index; i++)
    {    
        if(!arrow_bintree_insert(&tree, gap_bound[i] - info->cost_list[i]))
        {
            arrow_print_error("Could not sort gap bounds");
            goto CLEANUP;
        }
    }
    
    /* Convert tree to an array */
    arrow_bintree_to_new_array(&tree, &gap);
    gap_length = tree.size;
    
    /* Create a hash of the unique values */
    if(!arrow_hash_cost_list(gap, gap_length, &hash))
    {
        arrow_print_error("Could not create hash of unique gap bounds");
        goto CLEANUP;
    }
    
    /* We create a tier for each unique gap bound */
    if((*tier = malloc(gap_length * sizeof(arrow_llist))) == NULL)
    {
        arrow_print_error("Error allocating memory for array of linked lists.");
        *tier = NULL;
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Initialize each tier of linked lists */
    for(i = 0; i < gap_length; i++)
        arrow_llist_init(&((*tier)[i]));

    

CLEANUP:
    arrow_hash_destruct(&hash);
    arrow_bintree_destruct(&tree);
    return ret;
}
