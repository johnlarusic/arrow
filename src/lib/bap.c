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
 *  @brief  Initialize flow data structures.
 *  @param  problem [in] problem data
 *  @param  delta [in] delta parameter
 *  @param  s [in] source node index
 *  @param  t [in] sink node index
 *  @param  res [out] residual graph to construct
 *  @param  dist [out] distance labels
 *  @param  pred [out] predecessor labels
 */
void
initialize_flow_data(arrow_problem *problem, int delta, int s, int t,
                     int **res, int *m, int *dist, int *pred);

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

/*
 *  @brief  Ford-Fulkerson labeling algorithm for max-flow.
 *  @param  n [in] size of the graph
 *  @param  s [in] source node index
 *  @param  t [in] sink node index
 *  @param  res [out] the residual graph -- res[i][j] > 0 indicates the edge
 *              (i,j) exists in the residual graph; res[i][j] < 0 indicates
 *              flow is going through (i, j)
 *  @param  label [out] labels for each node
 *  @param  pred [out] indicates predecessors for each node
 *  @param  flow [out] holds the flow found from s to t
 *  @param  list [out] temporary list
 */                      
void
ford_fulkerson_labeling(int n, int s, int t, int **res, int *label,
                        int *pred, int *flow,  int *list);

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_bap_solve(arrow_problem *problem, arrow_problem_info *info, 
                arrow_bound_result *result)
{
    int ret = ARROW_SUCCESS;
    int i, j, m, stop;
    int low, high, median, delta, flow;
    double start_time, end_time;
    
    int n = problem->size * 2 + 2;
    int s = n - 2;
    int t = n - 1;
    
    int **res;
    int *res_space;
    int *dist;
    int *pred;
    int *list;
    
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
    if(!arrow_util_create_int_array(n, &list))
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
        flow = 0;
        
        initialize_flow_data(problem, delta, s, t, res, &m, dist, pred);

        stop = (int)(2 * pow(n, 2.0 / 3.0) + 0.5);
        m = (int)(sqrt(m * 1.0) + 0.5);
        if(m < stop) stop = m;

        shortest_augmenting_path(n, s, t, stop, res, dist, pred, &flow);
        if(flow < problem->size)
            ford_fulkerson_labeling(n, s, t, res, dist, pred, &flow, list);
        
        if(flow == problem->size)
            high = median;
        else
            low = median + 1;
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
    if(list != NULL) free(list);
    
    return ret;
}

/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
void
initialize_flow_data(arrow_problem *problem, int delta, int s, int t,
                     int **res, int *m, int *dist, int *pred)
{
    int i, j;
    
    *m = problem->size * 2;
    for(i = 0; i < problem->size; i++)
    {
        for(j = 0; j < problem->size; j++)
        {
            if((i != j) && (problem->get_cost(problem, i, j) <= delta))
            {
                res[i][j + problem->size] = 1;
                (*m)++;
            }
            else
            {
                res[i][j + problem->size] = 0;
            }

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

void
shortest_augmenting_path(int n, int s, int t, int stop, 
                         int **res, int *dist, int *pred, int *flow)
{
    int ret = ARROW_SUCCESS;
    int i, j, u, v;
    int min_d, admissable;
    
    i = s;
    while(dist[s] < stop)
    {
        min_d = n + 1;        
        admissable = ARROW_FALSE;
        
        for(j = 0; j < n; j++)
        {
            if(res[i][j] > 0)
            {
                if(dist[i] == dist[j] + 1)
                {
                    admissable = ARROW_TRUE;
                    pred[j] = i;
                    i = j;
                    
                    if(i == t)
                    {
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
                        i = s;
                    }
                    
                    break;
                }
                else
                {
                    if(dist[j] + 1 < min_d)
                        min_d = dist[j] + 1;
                }
            }
        }
        
        if(!admissable)
        {
            dist[i] = min_d;
            if(i != s) 
                i = pred[i];
        }
    }
}

void
ford_fulkerson_labeling(int n, int s, int t, int **res, int *label,
                        int *pred, int *flow,  int *list)
{
    int i, j, u, v;
    int idx = 0;
    
    label[t] = ARROW_TRUE;
    while(label[t])
    {
        for(i = 0; i < n; i++)
        {
            label[i] = ARROW_FALSE;
            pred[i] = -1;
        }
        
        label[s] = ARROW_TRUE;
        list[idx] = s; idx++;
        
        while(idx > 0)
        {
            i = list[idx - 1]; idx--;
            for(j = 0; j < n; j++)
            {
                if(res[i][j] > 0)
                {
                    if(!label[j])
                    {
                        label[j] = ARROW_TRUE;
                        pred[j] = i;
                        list[idx++] = j;
                    }
                }
            }
        }
        
        if(label[t])
        {
            (*flow)++;
            u = t;
            v = pred[u];
            while(u != s)
            {
                res[u][v] = 1;
                res[v][u] = -1;
                u = v;
                v = pred[u];
            }
        }
    }
}
