/**********************************************************doxygen*//** @file
 *  @brief   Constrained bottleneck spanning tree bound implemenation.
 *
 *  Implemenation of the Constrained bottleneck spanning tree (CBST) bound
 *  used as a lower bound for the symmetric constrained bottleneck TSP 
 *  objective value.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "lb.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
int
init_data(int n, arrow_heap *heap, int **cur_tree, int **in_heap, int **d);

void
destroy_data(arrow_heap *heap, int **cur_tree, int **in_heap, int **d);

/**
 *  @brief  Solves a (slightly modified!) minimum spanning tree problem.
 *  @param  problem [in] problem structure
 *  @param  C [in] largest cost in problem
 *  @param  exclude_cost [in] excludes costs less than this value except...
 *  @param  cur_tree [in] includes any edge costs found in this tree
 *  @param  tree [out] the resulting minimum spanning tree
 *  @param  max_cost [out] the largest cost in the resulting tree
 *  @param  length [out] the total length of the resulting tree
 *  @param  heap [out] a temporary heap structure
 *  @param  in_heap [out] a temporary integer array
 *  @param  d [out] a temporary integer array
 */
void
min_span_tree(arrow_problem *problem, int C, int exclude_cost,
              int *cur_tree, int *tree, int *max_cost, double *length,
              arrow_heap *heap, int *in_heap, int *d);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_cbst_solve(arrow_problem *problem, arrow_problem_info *info,
                 double max_length, arrow_bound_result *result)
{
    int ret = ARROW_SUCCESS;
    int *tree, *cur_tree, *in_heap, *d;
    int max_cost;
    double length;
    arrow_heap heap;
    
    double start_time, end_time;
    start_time = arrow_util_zeit();
    
    /* Allocate data */
    if(!arrow_util_create_int_array(problem->size, &tree))
    {
        arrow_print_error("Could not initialize tree array.");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    if(!init_data(problem->size, &heap, &cur_tree, &in_heap, &d))
    {
        arrow_print_error("Could not initialize MST data structures.");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    /* Initial MST call to determine feasibility and bottleneck value */
    min_span_tree(problem, info->max_cost, INT_MIN, cur_tree, tree,
                  &max_cost, &length, &heap, in_heap, d);
    /* See if given max_length is feasible */
    if(length > max_length)
    {
        arrow_print_error("max_length infeasible; must be >= %.0f", length);
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }

    arrow_debug("\nSpanning tree length: %.0f\n", length);

    end_time = arrow_util_zeit();
    
    result->obj_value = max_cost;
    result->total_time = end_time - start_time;    
    
CLEANUP:
    if(tree != NULL) free(tree);
    return ret;
}
 
int
arrow_cbst_mst_solve(arrow_problem *problem, arrow_problem_info *info,
                     int *tree, int *max_cost, double *length)
{
    arrow_heap heap;
    int  *cur_tree, *in_heap, *d;
    int ret = ARROW_SUCCESS;
    
    if(!init_data(problem->size, &heap, &cur_tree, &in_heap, &d))
    {
        arrow_print_error("Could not initialize MST data structures.");
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
    
    min_span_tree(problem, info->max_cost, INT_MIN, cur_tree, tree,
                  max_cost, length, &heap, in_heap, d);
    
CLEANUP:
    destroy_data(&heap, &cur_tree, &in_heap, &d);
    return ret;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
void
min_span_tree(arrow_problem *problem, int C, int exclude_cost,
              int *cur_tree, int *tree, int *max_cost, double *length,
              arrow_heap *heap, int *in_heap, int *d)
{
    int i, j, cost, p, tree_size;
    
    /* Node 0 gets inserted into an empty heap with cost 0.  All other nodes
       get inserted in with a cost of C + 1 */
    arrow_heap_empty(heap);
    arrow_heap_insert(heap, 0, 0);
    for(i = 1; i < problem->size; i++)
        arrow_heap_insert(heap, C + 1, i);
    
    /* Iniitalize arrays */
    for(i = 0; i < problem->size; i++)
    {
        tree[i] = -1;
        in_heap[i] = 1;
        d[i] = C + 1;
    }
    
    *max_cost = INT_MIN;
    *length = 0.0;
    tree_size = 0;
    
    /* Main loop */
    while(tree_size < problem->size - 1)
    {
        /*
        arrow_debug("HEAP:\n");
        arrow_heap_print(heap);
        arrow_debug("\n");
        */
        i = arrow_heap_get_min(heap);
        arrow_heap_delete_min(heap);
        in_heap[i] = 0;
        
        p = tree[i];
        if(p >= 0)
        {
            /* Insert (p,i) into tree, update length and max_cost */
            tree_size++;
            cost = problem->get_cost(problem, p, i);
            *length += cost;
            if(cost > *max_cost) *max_cost = cost;
            //arrow_debug(" - Add (%d,%d) to tree with cost %d\n", p, i, cost);
        }
        
        /* Iterate through adjacent nodes that are still in the heap */
        //arrow_debug(" - Examine neighbours of %d:\n", i);
        for(j = 0; j < problem->size; j++)
        {
            if((i != j) && (in_heap[j]))
            {
                cost = problem->get_cost(problem, i, j);
                /* We want to consider only adjacent nodes we can visit more
                   cheaply, and are not excluded by being less than the exclude
                   cost and not in the current solution */
                //arrow_debug("   - (%d,%d): cost: %d, d[%d] = %d, cur_tree[%d] = %d\n",
                //            i, j, cost, j, d[j], j, cur_tree[j]);
                if((d[j] > cost) && 
                   ((cost > exclude_cost) || (cur_tree[j] != i)))
                {
                    d[j] = cost;
                    tree[j] = i;
                    //arrow_debug("     - update d[%d] = %d\n", j, cost);
                    arrow_heap_change_key(heap, cost, j);
                }
            }
        }
    }
}

int
init_data(int n, arrow_heap *heap, int **cur_tree, int **in_heap, int **d)
{
    int i;
    
    if(!arrow_heap_init(heap, n))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, cur_tree))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, in_heap))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, d))
        goto CLEANUP;
        
    
    for(i = 0; i < n; i++)
    {
        (*cur_tree)[i] = 0;
        (*in_heap)[i] = 0;
        (*d)[i] = 0;
    }
    
    return ARROW_SUCCESS;

CLEANUP:
    destroy_data(heap, cur_tree, in_heap, d);
    return ARROW_FAILURE;
}

void
destroy_data(arrow_heap *heap, int **cur_tree, int **in_heap, int **d)
{
    arrow_heap_destruct(heap);
    if(*in_heap != NULL) free(*in_heap); *in_heap = NULL;
    if(*d != NULL) free(*d); *d = NULL;
    if(*cur_tree != NULL) free(*d); *cur_tree = NULL;
}
