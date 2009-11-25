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
 *  @param  max_index [in/out] largest index to bother searching (can change if
 *              lower bounds infeasible for some value)
 *  @param  btsp_lbs [out] array of BTSP bounds for each cost
 *  @param  best_gap [out] smallest gap bound calculated
 *  @param  best_low [out] lower cost value for the best gap bound
 *  @param  best_high [out] upper cost value for the best gap bound
 *  @param  total_time [out] total time spent calculating lower bound values
 */
int
btsp_bounds(arrow_problem *problem, arrow_problem_info *info, int *max_index, 
            int *btsp_lbs, int *best_gap, int *best_low, int *best_high, 
            double *total_time);

/**
 *  @brief  Calculates a sorted tree of unique BalTSP lower bounds
 *  @param  info [in] problem info
 *  @param  btsp_lbs [out] array of BTSP bounds for each cost
 *  @param  max_index [in] largest index to bother searching
 *  @param  tree [out] sorted tree of unique BalTSP lower bounds
 */
int
baltsp_bounds_tree(arrow_problem_info *info, int *btsp_lbs, int max_index, 
                   arrow_bintree *tree);

/**
 *  @brief  Fills the tiers of linked lists corresponding to BalTSP
 *          lower bounds
 *  @param  info [in] problem info
 *  @param  btsp_lbs [out] array of BTSP bounds for each cost
 *  @param  max_index [in] largest index to bother searching
 *  @param  baltsp_lbs [in] array of BalTSP lower bounds
 *  @param  num_baltsp_lbs [in] size of baltsp_lbs array
 *  @param  tiers [out] array of linked lists to be filled
 */
int
fill_tiers(arrow_problem_info *info, int *btsp_lbs, int max_index, 
           int *baltsp_lb, int num_baltsp_lbs, arrow_llist *tiers);

/**
 *  @brief  Initializes tiers
 *  @param  num_tiers [in] number of tiers to create
 *  @param  tiers [out] array of linked lists to create
 */
int
init_tiers(int num_tiers, arrow_llist **tiers);

/**
 *  @brief  Destructs tiers
 *  @param  num_tiers [in] number of tiers to destroy
 *  @param  tiers [out] array of linked lists to destroy
 */
void
destruct_tiers(int num_tiers, arrow_llist **tiers);

/**
 *  @brief  Fills cost_order with a good order for processing them with the 
 *          IB-algorithm.
 *  @param  info [in] problem info
 *  @param  max_index [in] largest index to bother searching
 *  @param  btsp_lbs [in] array of BTSP bounds for each cost
 *  @param  cost_order [out] specifically ordered array of costs
 */
int
find_cost_order(arrow_problem_info *info, int max_index, int *btsp_lbs, int *cost_order);

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_balanced_tsp_ib2(arrow_problem *problem, 
                       arrow_problem_info *info,
                       arrow_baltsp_params *params,
                       arrow_btsp_params *btsp_params,
                       double *lb_time,
                       arrow_btsp_result *tour_result)
{
    int ret = ARROW_SUCCESS;
    int i, k;
    int p1, q1, p2;
    int is_feasible;
    int best_tour_low, best_tour_high;
    int low, high;
    int low_val, max;
    int best_gap, cur_gap;
    int btsp_lb;
    double start_time, end_time;
    arrow_btsp_fun fun_ib;
    arrow_problem *solve_problem;
    arrow_problem ib_problem;
    arrow_btsp_result cur_tour_result;
    
    /* Print out debug information */
    arrow_debug("With Improvements? %s\n", (params->with_improvements ? "Yes" : "No"));
    arrow_debug("BalTSP Lower Bound: %d\n", params->lower_bound);
    arrow_debug("BTSP Min Cost: %d\n", params->btsp_min_cost);
    arrow_debug("BTSP Max Cost: %d\n", params->btsp_max_cost);
    arrow_debug("MSTSP Min Cost: %d\n", params->mstsp_min_cost);
    arrow_debug("Total solve steps: %d\n", params->num_steps);
    arrow_debug("\n");
    
    /* For future use, possibly, to handle asymmetric instances */
    solve_problem = problem;
    
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
    
    
    /* Create function for ignoring C[i,j] < low_val */
    if(!arrow_baltsp_fun_ib(ARROW_TRUE, &fun_ib))
    {
        arrow_print_error("Could not create IB transformation function\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    arrow_debug("Starting iterative bottleneck search\n");
    
    /* Main loop */
    arrow_debug("Starting balanced search [%d,...]\n", info->cost_list[low]);
    while((low <= max) && (high < info->cost_list_length))
    {
        printf("------------------------------------\n");
        low_val = info->cost_list[low];
        printf("C[i,j] >= %d: \n", low_val);
        
        tour_result->bin_search_steps++;
    
        best_gap = best_tour_high - best_tour_low;
    
        /* Create problem that ignores C[i,j] < low_val */
        if(!arrow_btsp_fun_apply(&fun_ib, solve_problem, low_val, info->max_cost, &ib_problem))
        {
            arrow_print_error("Could not create IB cost matrix\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        /* See if the BTSP lower bound is less than the current gap */
        if(!btsp_lower_bound(&ib_problem, info, &btsp_lb))
        {
            arrow_print_error("Could not solve BTSP lower bounds on IB cost matrix\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        arrow_debug("best_lb - low_val = %d - %d = %d vs best_gap = %d\n",
                    btsp_lb, low_val, btsp_lb - low_val, best_gap);
        if(btsp_lb - low_val > best_gap)
        {
            arrow_debug("BTSP lower bound says this index is not worth searching\n");
            low++;
        }
        else
        {
            start_time = arrow_util_zeit();
            btsp_params->lower_bound = btsp_lb;
            btsp_params->upper_bound = info->max_cost;
            if(!arrow_btsp_solve(&ib_problem, info, btsp_params, &cur_tour_result))
            {
                arrow_print_error("Error searching for BTSP tour\n");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
            end_time = arrow_util_zeit();
            tour_result->total_time += end_time - start_time;

            for(k = 0; k < ARROW_TSP_SOLVER_COUNT; k++)
            {
                tour_result->solver_attempts[k] += cur_tour_result.solver_attempts[k];
                tour_result->solver_time[k] += cur_tour_result.solver_time[k];
            }
    
            arrow_problem_destruct(&ib_problem);

            /* See if we improved the best known gap */
            if(cur_tour_result.found_tour)
            {
                arrow_debug("Tour exists in [%d,%d] = %d\n", cur_tour_result.min_cost, 
                    cur_tour_result.max_cost,
                    cur_tour_result.max_cost - cur_tour_result.min_cost);
            
                /* Find the indices for the min  and max costs in tour */
                arrow_debug("Starting BTSP search to find upper index\n");
                if(!arrow_problem_info_cost_index(info, cur_tour_result.min_cost, &p1))
                {
                    arrow_print_error("Could not find min_cost in cost_list\n");
                    ret = ARROW_FAILURE;
                    goto CLEANUP;
                }
                if(!arrow_problem_info_cost_index(info, cur_tour_result.max_cost, &q1))
                {
                    arrow_print_error("Could not find min_cost in cost_list\n");
                    ret = ARROW_FAILURE;
                    goto CLEANUP;
                }
                high = q1;
            
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
                        for(k = 0; k < problem->size; k++)
                            tour_result->tour[k] = cur_tour_result.tour[k];
                    }

                    best_tour_low = cur_tour_result.min_cost;
                    best_tour_high = cur_tour_result.max_cost;
                
                    if(cur_gap == params->lower_bound)
                    {
                        arrow_debug("We found a gap equal to the lower bound!\n");
                        break;
                    }
                }
                else
                {
                    arrow_debug("Tour is no better than the current best solution.\n");
                }
            
                /* Theorem 1 Improvement */
                if(params->with_improvements)
                {
                    if(best_tour_high - best_tour_low + params->mstsp_min_cost <= info->cost_list[high])
                    {
                        arrow_debug("Tour is heuristically optimal.\n");
                        break;
                    }
                }
            
                /* Solve a TSP to see if we can push the lower bound up */
                arrow_debug("Starting TSP search to raise lower index\n");
                start_time = arrow_util_zeit();
                if(!arrow_btsp_feasible(solve_problem, params->num_steps, params->steps,
                                        cur_tour_result.min_cost, cur_tour_result.max_cost, 
                                        &is_feasible, &cur_tour_result))
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
            
                p2 = p1;
                if(is_feasible)
                {
                    arrow_debug("Found a feasible tour!\n");
                    arrow_debug("  - Tour MinCost = %d, MaxCost = %d\n", cur_tour_result.min_cost, cur_tour_result.max_cost);
            
                    best_gap = best_tour_high - best_tour_low;
                    cur_gap = cur_tour_result.max_cost - cur_tour_result.min_cost; 
            
                    /* Find the indices for the min cost in tour */
                    if(!arrow_problem_info_cost_index(info, cur_tour_result.min_cost, &p2))
                    {
                        arrow_print_error("Could not find min_cost in cost_list\n");
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
                        if(best_tour_high - best_tour_low + params->mstsp_min_cost <= info->cost_list[high])
                        {
                            arrow_debug("Tour is heuristically optimal.\n");
                            break;
                        }
                    }
                }
            
                if(p2 > p1)
                    low = p2 + 1;
                else
                    low = p1 + 1;
            
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
                arrow_debug("Could not find tour\n");
                break;
            }
        }
        arrow_debug("\n");
    }
    arrow_debug("\n");

CLEANUP:
    arrow_btsp_fun_destruct(&fun_ib);
    arrow_btsp_result_destruct(&cur_tour_result);
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int 
btsp_lower_bound(arrow_problem *problem, arrow_problem_info *info, 
                 int *lower_bound)
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
btsp_bounds(arrow_problem *problem, arrow_problem_info *info, int *max_index, 
            int *btsp_lbs, int *best_gap, int *best_low, int *best_high, 
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
    
    for(i = 0; i <= *max_index; i++)
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
        
        /* A negative value indicates an infeasible lower bound, so we quit here */
        if(lb < 0)
        {
            arrow_debug("Negative value for the lower bound #%d, so quit\n", i);
            arrow_debug("Old Max Index Value: %d\n", *max_index);
            *max_index = i - 1;
            arrow_debug("New Max Index Value: %d\n", *max_index);
            break;
        }
        
        /* Store calculated value for later, update best lower bound */
        gap = lb - cost;
        btsp_lbs[i] = lb;    
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
baltsp_bounds_tree(arrow_problem_info *info, int *btsp_lbs, int max_index, 
                   arrow_bintree *tree)
{
    int i;
    
    /* Sort the unique gap values */
    arrow_bintree_init(tree);        
    for(i = 0; i <= max_index; i++)
    {    
        if(!arrow_bintree_insert(tree, btsp_lbs[i] - info->cost_list[i]))
        {
            arrow_print_error("Could not sort BalTSP lower bounds");
            arrow_bintree_destruct(tree);
            return ARROW_FAILURE;
        }
    }
    
    return ARROW_SUCCESS;
}

int
fill_tiers(arrow_problem_info *info, int *btsp_lbs, int max_index, 
           int *baltsp_lb, int num_baltsp_lbs, arrow_llist *tiers)
{
    int ret = ARROW_SUCCESS;
    int i;
    int cost;
    int gap;
    int idx;
    arrow_hash hash;
    arrow_llist *list;
    
    /* Create a hash of the unique values */
    if(!arrow_hash_cost_list(baltsp_lb, num_baltsp_lbs, &hash))
    {
        arrow_print_error("Could not create hash of unique BalTSP bounds");
        goto CLEANUP;
    }
    
    for(i = 0; i <= max_index; i++)
    {
        cost = info->cost_list[i];
        gap = btsp_lbs[i] - cost;
        idx = arrow_hash_search(&hash, gap);
        
        list = &(tiers[idx]);
        arrow_llist_insert_tail(list, i);
    }

CLEANUP:
    arrow_hash_destruct(&hash);
    return ret;
}

int
init_tiers(int num_tiers, arrow_llist **tiers)
{
    int i;
    arrow_llist *tier;
    
    /* Allocate memory */
    if((*tiers = malloc(num_tiers * sizeof(arrow_llist))) == NULL)
    {
        arrow_print_error("Error allocating memory for array of linked lists.");
        *tiers = NULL;
        return ARROW_FAILURE;
    }
    
    /* Initialize lists */
    for(i = 0; i < num_tiers; i++)
    {
        tier = &((*tiers)[i]);
        arrow_llist_init(tier);
    }
    
    return ARROW_SUCCESS;
}

void
destruct_tiers(int num_tiers, arrow_llist **tiers)
{
    int i;
    arrow_llist *tier;
    
    if(*tiers != NULL)
    {
        for(i = 0; i < num_tiers; i++)
        {
            tier = &((*tiers)[i]);
            arrow_llist_destruct(tier);
        }
        free(*tiers);
    }
}

int
find_cost_order(arrow_problem_info *info, int max_index, int *btsp_lbs, int *cost_order)
{
    int ret = ARROW_SUCCESS;
    int i, k;
    int idx;
    int *baltsp_lbs;
    int num_tiers;
    arrow_llist *tier;
    arrow_llist *tiers;
    arrow_bintree tree;
        
    /* Find a list of unique BalTSP lower bounds */
    if(!baltsp_bounds_tree(info, btsp_lbs, max_index, &tree))
    {
        arrow_print_error("Could not create BalTSP bounds list");
        return ARROW_FAILURE;
    }
    
    /* Create array for holding Balanced TSP lower bounds */
    if(!arrow_util_create_int_array(tree.size, &baltsp_lbs))
    {
        arrow_print_error("Could not create baltsp_lbs array\n");
        baltsp_lbs = NULL;
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Convert tree to an array */
    arrow_bintree_to_array(&tree, baltsp_lbs);
    num_tiers = tree.size;
    
    /* Create and fill the list of tiers with costs corresponding to each 
       BalTSP lower bound */
    if(!init_tiers(num_tiers, &tiers))
    {
        arrow_print_error("Could not create tiers of linked lists");
        tiers = NULL;
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(!fill_tiers(info, btsp_lbs, max_index, baltsp_lbs, num_tiers, tiers))
    {
        arrow_print_error("Could not fill tiers of linked lists");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Fill cost_order according to order in tiers */
    i = 0;
    for(k = 0; k < num_tiers; k++)
    {
        tier = &(tiers[k]);
        while(tier->size > 0)
        {
            arrow_llist_remove_head(tier, &idx);
            cost_order[i] = idx;
            i++;
        }
    }

    /*
    arrow_debug("Cost index order: ");
    for(i = 0; i <= max_index; i++)
        arrow_debug("%d, ", cost_order[i]);
    arrow_debug("EOL\n\n");
    */
         
CLEANUP:
    free(baltsp_lbs);
    destruct_tiers(num_tiers, &tiers);
    arrow_bintree_destruct(&tree);
    return ret;
}
