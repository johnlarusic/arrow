/**********************************************************doxygen*//** @file
 *  @brief   Bottleneck biconnected spanning subgraph problem implemenation.
 *
 *  Implemenation of the bottleneck biconnected spanning subgraph problem
 *  (BBSSP) that's used as a lower bound for the Bottleneck TSP objective
 *  value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Recursively searches for articulation points in the graph using
 *          only costs less than or equal to 'max_cost' out from the given
 *          node.
 *  @param  problem [in] problem data structure
 *  @param  max_cost [in] the largest cost to consider as being in the graph
 *  @param  node [in] the node to search outward from
 *  @param  depth_num [in] level at which the given node was first discovered
 *  @param  root_children [in] count of the number of children of the root
 *  @param  visited [out] indicates if a node has been visited or not
 *  @param  depth [out] indicates the discovery depth of a node
 *  @param  low [out] indicates a back edge for some descendent of a node
 *              (e.g. the discovery depth of the node closest to the root and
 *              reachable from the given node by following zero or more edges
 *              downward and then at most one back edge)
 *  @param  parent [out] indicates the closest "parent" of a node
 *  @param  art_point [out] indicates if the node is an articulation point
 */
int
find_art_points(arrow_problem *problem, int max_cost, int node, int depth_num, 
                int root_children, int *visited, int *depth, int *low, 
                int *parent, int *art_point);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_bbssp_solve(arrow_problem *problem, arrow_problem_info *info, 
                  arrow_bound_result *result)
{
    int ret;
    int low, high, median;
    int connected;
    double start_time, end_time;
    
    if(!problem->symmetric)
    {
        arrow_print_error("Solver only works on symmetric cost matrices.");
        return ARROW_FAILURE;
    }
    
    /* Start binary search */
    start_time = arrow_util_zeit();
    low = 0;
    high = info->cost_list_length - 1;
    while(low != high)
    {
        median = ((high - low) / 2) + low;
        
        /* Determine is graph is biconnected if we consider only costs less
         * than or equal to the median value. */
        ret = arrow_bbssp_biconnected(problem, info->cost_list[median],
                                      &connected);
        
        if(ret == ARROW_ERROR_FATAL)
        {
            result->obj_value = -1;
            return ARROW_FAILURE;
        }  
        else
        {
            if(connected == ARROW_TRUE)
                high = median;
            else
                low = median + 1;
        }
    }
    end_time = arrow_util_zeit();
    
    /* Return the cost we converged to as the answer */
    result->obj_value = info->cost_list[low];
    result->total_time = end_time - start_time;
    return ARROW_SUCCESS;
}


int
arrow_bbssp_biconnected(arrow_problem *problem, int max_cost, int *result)
{
    int ret;
    int u;
    int *visited;
    int *depth;
    int *low;
    int *parent;
    int *art_point;
    
    visited = NULL;
    depth = NULL;
    low = NULL;
    parent = NULL;
    art_point = NULL;

    /* Initialize all the necessary arrays */
    ret = arrow_util_create_int_array(problem->size, &visited);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP; 

    ret = arrow_util_create_int_array(problem->size, &depth);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP;

    ret = arrow_util_create_int_array(problem->size, &low);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP;

    ret = arrow_util_create_int_array(problem->size, &parent);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP;

    ret = arrow_util_create_int_array(problem->size, &visited);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP;

    ret = arrow_util_create_int_array(problem->size, &art_point);
    if(ret == ARROW_ERROR_FATAL) goto CLEANUP;

    /* Set the default values for each array */
    for(u = 0; u < problem->size; u++)
    {
        visited[u] = 0;
        depth[u] = -1;
        low[u] = INT_MAX;
        parent[u] = -1;
        art_point[u] = 0;
    }

    /* We start by assuming the graph is biconnected */
    *result = ARROW_TRUE;
    
    /* Find any and all articulation points from the first vertex */
    ret = find_art_points(problem, max_cost, 0, 0, 0, visited, depth, low, 
                          parent, art_point);
    
    /* Check for articulation points or unvisited nodes */
    for(u = 0; u < problem->size; u++)
    {
        if(!visited[u] || art_point[u])
            *result = ARROW_FALSE;
    }
    
CLEANUP:
    if(visited != NULL) free(visited);   
    if(depth != NULL) free(depth);   
    if(low != NULL) free(low); 
    if(parent != NULL) free(parent);
    if(art_point != NULL) free(art_point);   
    
    return ret;
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
find_art_points(arrow_problem *problem, int max_cost, int node, int depth_num, 
                int root_children, int *visited, int *depth, int *low, 
                int *parent, int *art_point)
{
    /* "u" is the current node, and "v" will represent adjecent nodes */
    int u, v;
    
    /* 
       Some explanation of these variables:
       - depth_num is the level at which we first discovered node u
       - visited[u] indicates if node u has been visited or not
       - depth[u] indicates the discovery depth of node u
       - low[u] = 
            min{d[u], d[w] |(v,w) is a back edge for some descendent v of u}
         i.e. the discovery depth of the vertex closest to the root and
         reachable from node u by following zero or more edges downward, and
         then at most one back edge  
    */
    u = node;
    visited[u] = 1;
    depth[u] = depth_num;
    low[u] = depth_num;
    depth_num++;
    
    /* Look at nodes adjacent to u */
    for(v = 0; v < problem->size; v++)
    {
        if(problem->get_cost(problem, u, v) <= max_cost)
        {
            /* If the node has yet to be visited, mark its parent as u */
            if(!visited[v])
            {
                parent[v] = u;
                
                /* If this is the parent, increase root_children count */
                if(parent[u] == -1) root_children++;
                
                /* Recurse on v */
                find_art_points(problem, max_cost, v, depth_num, 
                                root_children, visited, depth, low, 
                                parent, art_point);
                
                /* Update low[u] */
                if(low[v] < low[u]) low[u] = low[v];
                
                /* Check articulation point conditions */
                if(parent[u] == -1)
                {
                    if(root_children > 1)
                    {
                        art_point[u] = 1;
                    }
                }
                else if(low[v] >= depth[u])
                {
                    art_point[u] = 1;
                }
            }
            /* Check to see if edge is a back edge */
            else if(v != parent[u])
            {
                if(depth[v] < low[u]) low[u] = depth[v];
            }
        }
    }
    
    return ARROW_SUCCESS;
}
