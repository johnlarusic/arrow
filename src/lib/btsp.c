/**********************************************************doxygen*//** @file
 * @brief   Bottleneck traveling salesman problem (BTSP) methods.
 *
 * Heuristic for solving the bottleneck traveling salesman problem (BTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Solves the feasibility problem which attempts to determine if 
 *          there is a Hamiltonian cycle using costs <= delta.
 *  @param  problem [in] problem to solve
 *  @param  num_steps [in] total number of steps in solve plan
 *  @param  steps [in] solve plan step details
 *  @param  delta [in] delta parameter for feasibility problem.
 *  @param  tour_exists [out] true if a feasible tour exists, false otherwise
 *  @param  result [out] resulting BTSP tour found
 */
int
feasible(arrow_problem *problem, int num_steps, arrow_btsp_solve_plan *steps, 
         int delta, int *tour_exists, arrow_btsp_result *result);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_result_init(arrow_problem *problem, arrow_btsp_result *result)
{
    if(!arrow_util_create_int_array(problem->size, &(result->tour)))
    {
        result = NULL;
        return ARROW_FAILURE;
    }
    result->found_tour = ARROW_FALSE;
    result->obj_value = INT_MAX;
    result->tour_length = DBL_MAX;
    result->bin_search_steps = 0;
    result->linkern_attempts = 0;
    result->linkern_time = 0.0;
    result->exact_attempts = 0;
    result->exact_time = 0.0;
    result->total_time = 0.0;
    return ARROW_SUCCESS;
}

void 
arrow_btsp_result_destruct(arrow_btsp_result *result)
{
    if(result->tour != NULL)
        free(result->tour);
}

void
arrow_btsp_params_init(arrow_btsp_params *params)
{
    params->confirm_sol = ARROW_FALSE;
    params->supress_ebst = ARROW_FALSE;
    params->find_short_tour = ARROW_FALSE;
    params->lower_bound = 0;
    params->upper_bound = INT_MAX;
    params->num_steps = 0;
}

void 
arrow_btsp_params_destruct(arrow_btsp_params *params)
{
    if(params->num_steps > 0)
    {
        int i;
        for(i = 0; i < params->num_steps; i++)
        {
            arrow_btsp_solve_plan_destruct(&(params->steps[i]));
        }
    }
}

void
arrow_btsp_solve_plan_init(arrow_btsp_solve_plan *plan)
{
    plan->plan_type = -1;
    plan->attempts = 0;
}

void 
arrow_btsp_solve_plan_destruct(arrow_btsp_solve_plan *plan)
{
    arrow_btsp_fun_destruct(&(plan->fun));
    arrow_tsp_lk_params_destruct(&(plan->lk_params));
}

int 
arrow_btsp_solve(arrow_problem *problem, arrow_problem_info *info,
                 arrow_btsp_params *params, arrow_btsp_result *result)
{    
    int ret = ARROW_SUCCESS;
    int tour_exists;
    double start_time = arrow_util_zeit();
    
    if(!problem->symmetric)
    {
        arrow_print_error("Solver only works on symmetric cost matrices.");
        return ARROW_FAILURE;
    }
    
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
    arrow_debug("Current solution: %d\n", result->obj_value);
    ret = feasible(problem, params->num_steps, params->steps, 
                   params->lower_bound, &tour_exists, result);
    if(ret != ARROW_SUCCESS)
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(tour_exists == ARROW_TRUE)
    {
        arrow_debug("A tour was found!  Nothing left to do here.\n");
        result->optimal = ARROW_TRUE;
        result->found_tour = ARROW_TRUE;
        goto CLEANUP;
    }
    
    /* Start enhanced binary search threshold heuristic */
    if(params->supress_ebst) goto CLEANUP;
    arrow_debug("\nStarting enhanced binary search threshold heuristic.\n");
    int i, low, high, median, median_val;

    ret = arrow_util_binary_search(info->cost_list, info->cost_list_length,
                                   params->lower_bound, &low);
    if(!ret)
    {
        low--;
        arrow_debug("Lower bound not in cost list, so pick next lowest\n");
        arrow_debug("Next lowest cost is %d...", info->cost_list[low]);
    }
    
    int upper_bound = params->upper_bound;
    if(result->obj_value < upper_bound) upper_bound = result->obj_value;
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
        
        ret = feasible(problem, params->num_steps, params->steps, 
                       median_val, &tour_exists, &cur_result);
        if(ret != ARROW_SUCCESS)
        {
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        if(tour_exists == ARROW_TRUE)
        {
            arrow_debug("A tour was found!\n");
            result->found_tour = ARROW_TRUE;
            
            /* Check to see if we found a smaller solution than our median */
            if(cur_result.obj_value < median_val)
            {
                arrow_debug(" - Found a smaller solution than our median: %d\n",
                            cur_result.obj_value);
                arrow_util_binary_search(info->cost_list, 
                                         info->cost_list_length,
                                         cur_result.obj_value, &high);
            }
            else
            {
                high = median;
            }
            
            /* Copy over new best objective value and tour */
            result->obj_value = cur_result.obj_value;
            result->tour_length = cur_result.tour_length;
            if(result->tour != NULL)
            {
                for(i = 0; i < problem->size; i++)
                    result->tour[i] = cur_result.tour[i];
            }   
        }
        else
        {
            arrow_debug("A tour could not be found.\n");
            low = median + 1;
            
            /* Check to see if we found a smaller solution than our median */
            if(cur_result.obj_value < median_val)
            {
                arrow_debug(" - Found a smaller solution than our best: %d\n",
                            cur_result.obj_value);
                arrow_debug(" - Lower upper bound to this value\n");
                arrow_util_binary_search(info->cost_list, 
                                         info->cost_list_length,
                                         cur_result.obj_value, &high);
                                         
                /* Copy over new best objective value and tour */
                result->obj_value = cur_result.obj_value;
                if(result->tour != NULL)
                {
                    for(i = 0; i < problem->size; i++)
                        result->tour[i] = cur_result.tour[i];
                }
            }
            
            if(low > high) low = high;
        }
        arrow_debug("\n");
        
        /* Copy over other important information */
        result->bin_search_steps++;
        result->linkern_attempts    += cur_result.linkern_attempts;
        result->linkern_time        += cur_result.linkern_time;
        result->exact_attempts      += cur_result.exact_attempts;
        result->exact_time          += cur_result.exact_time;        
    }
                           
CLEANUP:
    /* See if we've reached the lower bound to guarantee optimality */
    if(result->obj_value == params->lower_bound)
    {
        result->optimal = ARROW_TRUE;
        result->found_tour = ARROW_TRUE;
    }
    result->total_time = arrow_util_zeit() - start_time;

    arrow_btsp_result_destruct(&cur_result);
    return ret;
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
feasible(arrow_problem *problem, int num_steps, arrow_btsp_solve_plan *steps, 
         int delta, int *tour_exists, arrow_btsp_result *result)
{
    int ret = ARROW_SUCCESS;
    int feasible = ARROW_FALSE;
    int u, v, cost, max_cost;
    double len;
    
    *tour_exists = ARROW_FALSE;
    result->found_tour = ARROW_FALSE;
    result->obj_value = INT_MAX;
    result->tour_length = DBL_MAX;
    result->linkern_attempts = 0;
    result->linkern_time = 0.0;
    result->exact_attempts = 0;
    result->exact_time = 0.0;
    result->total_time = 0.0;
    
    /* This holds the current tour that was found and its length */
    arrow_tsp_result tsp_result;
    arrow_tsp_result_init(problem, &tsp_result);
    
    arrow_debug("Feasibility problem for delta = %d\n", delta);
    int i, j, k;
    for(i = 0; i < num_steps; i++)
    {
        arrow_btsp_solve_plan *plan = &(steps[i]);
        arrow_btsp_fun *fun = &(plan->fun);
                
        arrow_debug("Step %d of %d (Type %d):\n", i + 1, num_steps, 
                    plan->plan_type);
        
        for(j = 1; j <= plan->attempts; j++)
        {
            arrow_debug("Attempt %d of %d:\n", j, plan->attempts);
            
            /* Create a new problem based upon the solve plan */
            arrow_problem new_problem;
            if(!arrow_btsp_fun_apply(fun, problem, delta, &new_problem))
            {
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
                                                    
            /* Call LK or Exact TSP solver on new problem */      
            if(plan->use_exact_solver)
            {
                ret = arrow_tsp_exact_solve(&new_problem, NULL, &tsp_result);
                (result->exact_attempts)++;
                result->exact_time += tsp_result.total_time;
            }
            else
            {
                ret = arrow_tsp_lk_solve(&new_problem, &(plan->lk_params), 
                                         &tsp_result);
                result->linkern_attempts++;
                result->linkern_time += tsp_result.total_time;
            }
            if(!ret)
            {
                ret = ARROW_FAILURE;
                arrow_tsp_result_destruct(&tsp_result);
                arrow_problem_destruct(&new_problem);
                goto CLEANUP;   
            }
            
            /* Determine if we have found a tour of feasible length or not */
            arrow_debug("Found a tour of length %.0f\n", 
                        tsp_result.obj_value);
            feasible = fun->feasible(fun, problem, delta, 
                                     tsp_result.obj_value, tsp_result.tour);
            if(feasible)
            {
                /* Set this tour to the output variables then exit */
                arrow_debug(" - tour found is feasible.\n");
                *tour_exists = ARROW_TRUE;
                result->obj_value = delta;

                len = 0.0;
                for(k = 0; k < problem->size; k++)
                {
                    u = tsp_result.tour[k];
                    v = tsp_result.tour[(k + 1) % problem->size];
                    len += problem->get_cost(problem, u, v);
                    if(result->tour != NULL)
                        result->tour[k] = tsp_result.tour[k];
                }
                arrow_debug(" - actual tour is of length %.0f\n", len);
                result->tour_length = len;
                    
                arrow_debug("Finished feasibility question.\n");
                goto CLEANUP;
            }
            else if(plan->upper_bound_update)
            {
                printf("Check to see if we found a better upper bound...\n");
                max_cost = INT_MIN;
                len = 0.0;
                for(k = 0; k < problem->size; k++)
                {
                    u = tsp_result.tour[k];
                    v = tsp_result.tour[(k + 1) % problem->size];
                    cost = problem->get_cost(problem, u, v);
                    len += cost;
                    if(cost > max_cost) max_cost = cost;
                }
                
                if(max_cost < result->obj_value)
                {
                    arrow_debug(" - tour is the best one found so far.\n");
                    arrow_debug(" - max. cost is %d\n", max_cost);
                    result->obj_value = max_cost;
                    arrow_debug(" - actual tour is of length %.0f\n", len);
                    result->tour_length = len;
                    if(result->tour != NULL)
                    {
                        for(k = 0; k < problem->size; k++)
                            result->tour[k] = tsp_result.tour[k];
                    }
                }
            }
            
            /* Clean up */
            arrow_problem_destruct(&new_problem);
        }
    }
    arrow_debug("Finished feasibility problem finding no feasible tours.\n");
    
CLEANUP:
    arrow_tsp_result_destruct(&tsp_result);
    return ret;
}


