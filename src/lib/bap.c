/**********************************************************doxygen*//** @file
 *  @brief   Bottleneck assignment problem (BAP) implemenation.
 *
 *  Implemenation of the bottleneck assignment problem (BAP) that's used as a 
 *  lower bound for the Bottleneck TSP objective value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/*
 *  @brief  Shortest augmenting path algorithm for max-flow.
 *  @param  n [in] size of the graph
 *  @param  s [in] source node index
 *  @param  t [in] sink node index
 *  @param  stop [in] stop when d[s] >= stop
 *  @param  res [out] the residual graph -- res[i][j] > 0 indicates the edge
 *              (i,j) exists in the residual graph; res[i][j] < 0 indicates
 *              flow is going through (i, j)
 *  @param  dist [out] indicates distance labels for each node
 *  @param  pred [out] indicates predecessors for each node
 *  @param  flow [out] holds the flow found from s to t
 */
void
shortest_augmenting_path(int n, int s, int t, int stop, 
                         int **res, int *dist, int *pred, int *flow);

void
initialize_flow_data(arrow_problem *problem, int delta, int s, int t,
                     int **res, int *dist, int *pred)
{
    int i, j;
    for(i = 0; i < problem->size; i++)
    {
        for(j = 0; j < problem->size; j++)
        {
            if((i != j) && (problem->get_cost(problem, i, j) <= delta))
                res[i][j + problem->size] = 1;
            else
                res[i][j + problem->size] = 0;

            res[i][j] = 0;
            res[i + problem->size][j] = 0;
            res[i + problem->size][j + problem->size] = 0;
        }
        res[s][i] = 1;
        res[i][s] = 0;
        res[i][t] = 0;
        res[i + problem->size][t] = 1;
        res[t][i + problem->size] = 0;    
        
        dist[i] = 2;
        dist[i + problem->size] = 1;
        
        pred[i] = -1;
        pred[i + problem->size] = -1;
    }
    dist[s] = 3;  dist[t] = 0;
    pred[s] = -1; pred[t] = -1;
}

int
arrow_bap_solve(arrow_problem *problem, arrow_problem_info *info, 
                arrow_bound_result *result)
{
    int ret = ARROW_SUCCESS;
    int i, j;
    int low, high, median, delta, flow;
    double start_time, end_time;
    
    int n = problem->size * 2 + 2;
    int s = n - 2;
    int t = n - 1;
    
    int **res;
    int *res_space;
    int *dist;
    int *pred;
    
    start_time = arrow_util_zeit();
    
    /* Dynamic memory allocation GO! */
    if(!arrow_util_create_int_matrix(n, n, &res, &res_space))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    if(!arrow_util_create_int_array(n, &dist))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    if(!arrow_util_create_int_array(n, &pred))
    {
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }

    low = 0;
    high = info->cost_list_length - 1;
    while(low != high)
    {
        median = ((high - low) / 2) + low;
        delta = info->cost_list[median];
        arrow_debug("delta = %d\n", delta);        
        flow = 0;
        
        initialize_flow_data(problem, delta, s, t, res, dist, pred);
        shortest_augmenting_path(n, s, t, n, res, dist, pred, &flow);
        
        if(flow == problem->size)
        {
            high = median;
            arrow_debug(" - has a bipartite matching: \n");
            
            for(i = 0; i < problem->size; i++)
                for(j = problem->size; j < problem->size * 2; j++)
                    if(res[i][j] == -1)
                        arrow_debug("   - C[%d,\t%d] \t= %d\n", i, j - problem->size,
                            problem->get_cost(problem, i, j - problem->size));
        }
        else
        {
            arrow_debug(" - does not have a bipartite matching\n");
            arrow_debug(" - max flow is %d\n", flow);
            low = median + 1;
        }
        arrow_debug("\n");
    }
    end_time = arrow_util_zeit();
    
    /* Return the cost we converged to as the answer */
    result->obj_value = info->cost_list[low];
    result->total_time = end_time - start_time;
    
CLEANUP:
    if(dist != NULL) free(dist);
    if(pred != NULL) free(pred);
    if(res_space != NULL) free(res_space);
    if(res != NULL) free(res);
    
    return ret;
}

void
shortest_augmenting_path(int n, int s, int t, int stop, 
                         int **res, int *dist, int *pred, int *flow)
{
    int ret = ARROW_SUCCESS;
    int i, j, u, v;
    int min_d, admissable;
    
    //print_residual(n, res, dist, pred);
    
    i = s;
    while(dist[s] < stop)
    {
        //arrow_debug("current node: %d\n", i);
        min_d = n + 1;        
        admissable = ARROW_FALSE;
        
        for(j = 0; j < n; j++)
        {
            //arrow_debug(" - res[%d][%d] = %d\n", i, j, res[i][j]);
            if(res[i][j] > 0)
            {
                //arrow_debug("   - (dist[%d] = %d) vs ", i, dist[i]);
                //arrow_debug("(dist[%d] + 1 = %d)\n", j, dist[j] + 1);
                if(dist[i] == dist[j] + 1)
                {
                    //arrow_debug("   - arc is admissable -- advance!\n");
                    admissable = ARROW_TRUE;
                    pred[j] = i;
                    i = j;
                    
                    if(i == t)
                    {
                        //arrow_debug("   - reached t.  Augment!\n");
                        (*flow)++;
                        u = i;
                        v = pred[u];
                        while(u != s)
                        {
                            res[u][v] = 1;
                            res[v][u] = -1;
                            u = v;
                            v = pred[u];
                        }
                        //print_residual(n, res, dist, pred);
                        i = s;
                    }
                    
                    break;
                }
                else
                {
                    //arrow_debug("   - arc is NOT admissable\n");
                    //arrow_debug("     - dist[%d] = %d\n", j, dist[j]);
                    if(dist[j] + 1 < min_d)
                        min_d = dist[j] + 1;
                }
            }
        }
        
        if(!admissable)
        {
            //arrow_debug(" - No admissable arc found -- retreat!\n");
            //arrow_debug("   - min_d = %d\n", min_d);
            dist[i] = min_d;
            if(i != s) 
                i = pred[i];
        }
    }
    
    //arrow_debug("Leaving shortest_augmenting_path\n");
}