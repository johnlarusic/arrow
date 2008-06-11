/**********************************************************doxygen*//** @file
 *  @brief   TSP solver and Lin-Kernighan heuristic.
 *
 *  Wrapper for calling Concorde's TSP solver and Lin-Kernighan heuristic.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
int
build_initial_tour(arrow_problem *problem, CCedgegengroup *plan, 
                   CCrandstate *rstate, int *initial_tour);
 

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_tsp_result_init(arrow_problem *problem, arrow_tsp_result *result)
{
    if(!arrow_util_create_int_array(problem->size, &(result->tour)))
    {
        result = NULL;
        return ARROW_ERROR_FATAL;
    }
    result->found_tour = ARROW_FALSE;
    result->obj_value = -1.0;
    result->total_time = 0.0;
    return ARROW_SUCCESS;
}

void 
arrow_tsp_result_destruct(arrow_tsp_result *result)
{
    if(result->tour != NULL)
    {
        free(result->tour);
        result->tour = NULL;
    }
}

void
arrow_tsp_lk_params_init(arrow_problem *problem, arrow_tsp_lk_params *params)
{
    params->random_restarts = 0;
    params->stall_count = problem->size;
    params->kicks = (problem->size > 1000 ? 500 : problem->size / 2);
    params->kick_type = CC_LK_GEOMETRIC_KICK;
    params->time_bound = 0.0;
    params->length_bound = 0.0;
    params->initial_tour = NULL;
}

void 
arrow_tsp_lk_params_destruct(arrow_tsp_lk_params *params)
{
    if(params->initial_tour != NULL)
        free(params->initial_tour);
}

int 
arrow_tsp_exact_solve(arrow_problem *problem, int *initial_tour, 
                      arrow_tsp_result *result)
{
    int ret;
    int success;
    double start_time, end_time;
    CCrandstate rstate;
    
    CCutil_sprand((int)CCutil_real_zeit(), &rstate);
    start_time = arrow_util_zeit();
    ret = CCtsp_solve_dat(
            problem->size,          // int ncount
            &(problem->data),       // CCdatagroup *indat
            initial_tour,           // int *in_tour
            result->tour,           // int *out_tour
            NULL,                   // double *in_val
            &(result->obj_value),   // double *optval
            &success,               // int *success
            &(result->found_tour),  // int *foundtour
            problem->name,          // char *name
            NULL,                   // double *timebound
            NULL,                   // int *hit_timebound
            1,                      // int silent
            &rstate                 // CCrandstate *rstate
        );
    
    end_time = arrow_util_zeit();
    result->total_time = end_time - start_time;
        
    if(result->found_tour && success)
        return ARROW_SUCCESS;
    else
        return ARROW_FAILURE;
}

int
arrow_tsp_lk_solve(arrow_problem *problem, arrow_tsp_lk_params *params,
                   arrow_tsp_result *result)
{
    int ret = ARROW_SUCCESS;
    int cc_ret;
    int ecount;
    int *elist = (int *) NULL;
    int *cyc = (int *) NULL;
    int *bestcyc = (int *) NULL;
    int *tmp;
    int i;
    double val, bestval;
    double start_time, end_time;
    arrow_tsp_lk_params lk_params;
    CCedgegengroup plan;
    CCrandstate rstate;
    
    start_time = arrow_util_zeit();
    
    /* Some of the following code is heavily copied from the find_tour
       method in Concorde's TSP/concorde.c */
    arrow_debug("\nEntering arrow_tsp_lk_solve\n");
    
    /* Set default params if none are passed */
    if(params == NULL)
    {
        arrow_debug("No parameters set, so using default.\n");
        arrow_tsp_lk_params_init(problem, &lk_params);
    }
    else
    {
        arrow_debug("Using passed parameters.\n");
        lk_params = *params;
    }
    arrow_debug(" - random_restarts = %d\n", lk_params.random_restarts);
    arrow_debug(" - stall_count = %d\n", lk_params.stall_count);
    arrow_debug(" - kicks = %d\n", lk_params.kicks);
    arrow_debug(" - kick_type = %d\n", lk_params.kick_type);
    arrow_debug(" - time_bound = %1.2f\n", lk_params.time_bound);
    arrow_debug(" - length_bound = %.0f\n", lk_params.length_bound);
    
    CCutil_sprand((int)CCutil_real_zeit(), &rstate);
    
    /* cyc temporarily holds a tour found by the LK algorithm */
    cyc = CC_SAFE_MALLOC(problem->size, int);
    if(cyc == NULL)
    {
        arrow_print_error("Out of memory for cyc");
        return ARROW_FAILURE;
    }
    bestcyc = CC_SAFE_MALLOC(problem->size, int);
    if(cyc == NULL)
    {
        arrow_print_error("Out of memory for bestcyc");
        return ARROW_FAILURE;
    }
    
    /* We start by building up a set of "good" edges for the heuristic
       to chew upon */
    arrow_debug("Building set of 'good' edges... ");
    CCedgegen_init_edgegengroup(&plan);
    plan.quadnearest = 2;
    cc_ret = CCedgegen_edges(&plan, problem->size, &(problem->data), 
                             (double *) NULL, &ecount, &elist, 1, &rstate);
    if(cc_ret == CONCORDE_FAILURE)
    {
        arrow_print_error("CCedgegen_edges failed");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    plan.quadnearest = 0;
    arrow_debug("done.\n");
    
    /* If none is passed, we construct an initial starting tour */
    if(lk_params.initial_tour == NULL)
    {
        arrow_debug("Building initial tour... ");
        build_initial_tour(problem, &plan, &rstate, cyc);
        arrow_debug("done.\n");
    }
    else
    {
        cyc = lk_params.initial_tour;
    }
    
    /* Initial LK call */
    arrow_debug("Initial call to the LK algorithm...\n");
    cc_ret = CClinkern_tour(problem->size, &(problem->data), ecount, elist, 
                            params->stall_count, params->kicks,
                            cyc, bestcyc, &bestval, 1, 
                            params->time_bound, params->length_bound,
                            (char *) NULL, params->kick_type, &rstate);
    arrow_debug("Found tour of length '%.0f'.\n", bestval);
    if(cc_ret == CONCORDE_FAILURE)
    {
        arrow_print_error("CClinkern_tour failed");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Now begin the random restart phase if necessary */
    for(i = 0; (i < lk_params.random_restarts) 
        && (bestval > lk_params.length_bound); i++)
    {
        arrow_debug("Trial %d of %d to LK algorithm...\n", i + 1, 
                    lk_params.random_restarts);
        cc_ret = CClinkern_tour(problem->size, &(problem->data), ecount, 
                                elist, params->stall_count, params->kicks,
                                (int *) NULL, cyc, &val, 1, 
                                params->time_bound, params->length_bound,
                                (char *) NULL, params->kick_type, &rstate);
        arrow_debug("Found tour of length '%.0f'.\n", val);
        if(cc_ret == CONCORDE_FAILURE)
        {
            arrow_print_error("CClinkern_tour failed");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        if (val < bestval) {
            arrow_debug("Found a better tour (%.0f vs %.0f) so swap\n",
                        val, bestval);
            CC_SWAP (cyc, bestcyc, tmp);
            bestval = val;
        }
    }
    
    if((lk_params.random_restarts > 0) && (bestval > lk_params.length_bound))
    {
        arrow_debug("Final attempt to find tour...\n");
        cc_ret = CClinkern_tour(problem->size, &(problem->data), ecount, 
                                elist, params->stall_count, 2 * params->kicks,
                                bestcyc, result->tour, &(result->obj_value),
                                1, params->time_bound, params->length_bound,
                                (char *) NULL, params->kick_type, &rstate);
        arrow_debug("Found tour of length '%.0f'.\n", result->obj_value);
        if(cc_ret == CONCORDE_FAILURE)
        {
            arrow_print_error("CClinkern_tour failed");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
    }
    else
    {
        printf("Copying over found tour\n");
        result->obj_value = bestval;
        if(result->tour != NULL)
        {
            for (i = 0; i < problem->size; i++)
            {
                result->tour[i] = bestcyc[i];
            }
        }
        else
            arrow_debug("tour is NULL\n");
    }

CLEANUP:
    if(ret) result->found_tour = ARROW_TRUE;
    end_time = arrow_util_zeit();
    result->total_time = end_time - start_time;
    
    arrow_debug("Cleaning up...");
    CC_IFFREE(cyc, int);
    CC_IFFREE(elist, int);
    if(params == NULL) arrow_tsp_lk_params_destruct(&lk_params);
    arrow_debug("done.\n");
    return ret;
}

int
build_initial_tour(arrow_problem *problem, CCedgegengroup *plan, 
                   CCrandstate *rstate, int *initial_tour)
{
    int ret = ARROW_SUCCESS;
    int cc_ret;
    int tcount, istour;
    int *tlist = (int *) NULL;
    
    plan->tour.greedy = 1;
    cc_ret = CCedgegen_edges(plan, problem->size, &(problem->data), 
                             (double *) NULL, &tcount, &tlist, 1, rstate);
    if(cc_ret == CONCORDE_FAILURE)
    {
        arrow_print_error("CCedgegen_edges failed");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(tcount != problem->size) {
        arrow_print_error("Wrong edgeset from CCedgegen_edges");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }

    cc_ret = CCutil_edge_to_cycle(problem->size, tlist, &istour, initial_tour);
    if(cc_ret == CONCORDE_FAILURE)
    {
        arrow_print_error("CCutil_edge_to_cycle failed");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(istour == ARROW_FALSE) {
        arrow_print_error("Starting tour has an error\n");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
CLEANUP:
    CC_FREE(tlist, int);
    return ret;
}