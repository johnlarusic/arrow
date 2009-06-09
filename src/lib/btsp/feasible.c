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
arrow_btsp_feasible(arrow_problem *problem, int num_steps, 
                    arrow_btsp_solve_plan *steps, int min_cost, int max_cost, 
                    int *feasible, arrow_btsp_result *result)
{
    printf("Feasible?: %d <= C[i,j] <= %d\n", min_cost, max_cost);
    printf("is_symmetric = %d; size = %d;\n", problem->symmetric, problem->size);
    
    int ret = ARROW_SUCCESS;
    int i, j, k;
    int u, v;
    double len;
       
    *feasible = ARROW_FALSE;
    result->found_tour = ARROW_FALSE;
    result->max_cost = INT_MIN;
    result->max_cost = INT_MAX;
    result->tour_length = DBL_MAX;
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        result->solver_attempts[i] = 0;
        result->solver_time[i] = 0.0;
    }
    result->total_time = 0.0;
    
    /* This holds the current tour that was found and its length */
    arrow_tsp_result tsp_result;
    arrow_tsp_result_init(problem, &tsp_result);
    
    arrow_debug("Feasibility problem for %d <= C[i,j] <= %d\n", min_cost, max_cost);
    for(i = 0; i < num_steps; i++)
    {
        arrow_btsp_solve_plan *plan = &(steps[i]);
        arrow_btsp_fun *fun = &(plan->fun);
                
        arrow_debug("Step %d of %d:\n", i + 1, num_steps);
        
        for(j = 1; j <= plan->attempts; j++)
        {
            arrow_debug("Attempt %d of %d:\n", j, plan->attempts);
            
            /* Create a new problem based upon the solve plan */
            arrow_problem new_problem;
            if(!arrow_btsp_fun_apply(fun, problem, min_cost, max_cost, &new_problem))
            {
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
                                                    
            /* Call a TSP solver on new problem */      
            ret = arrow_tsp_solve(plan->tsp_solver, &new_problem, 
                                  plan->tsp_params, &tsp_result);
            if(!ret)
            {
                ret = ARROW_FAILURE;
                arrow_tsp_result_destruct(&tsp_result);
                arrow_problem_destruct(&new_problem);
                goto CLEANUP;   
            }
            
            result->solver_attempts[plan->tsp_solver] += 1;
            printf("TSP Solver Time: %.2f\n", tsp_result.total_time);
            result->solver_time[plan->tsp_solver] += tsp_result.total_time;            
            
            /* Determine if we have found a tour of feasible length or not */
            arrow_debug("Found a tour of length %.0f\n", 
                        tsp_result.obj_value);
            *feasible = fun->feasible(fun, problem, min_cost, max_cost, 
                                      tsp_result.obj_value, tsp_result.tour);
            if(*feasible)
            {
                /* Set this tour to the output variables then exit */
                arrow_debug(" - tour found is feasible.\n");
                result->found_tour = ARROW_TRUE;
                result->min_cost = min_cost;
                result->max_cost = max_cost;

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
                arrow_problem_destruct(&new_problem);
                goto CLEANUP;
            }
            
            /** TODO: Find a better way of doing this upper bound update */
            /*
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
                    result->found_tour = ARROW_TRUE;
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
            */
            
            /* Clean up */
            arrow_problem_destruct(&new_problem);
        }
    }
    arrow_debug("Finished feasibility problem finding no feasible tours.\n");
    
CLEANUP:
    arrow_tsp_result_destruct(&tsp_result);
    return ret;
}
