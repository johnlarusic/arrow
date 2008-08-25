/**********************************************************doxygen*//** @file
 *  @brief   Contrained bottleneck assignment problem algorithm
 *
 *  Implemenation of the contrained bottleneck assignment problem (CBAP) 
 *  that's used as a lower bound for the contrained BTSP objective value.
 *
 *  NOTE: This code is an almost verbatim copy of C++ code written by Jonker
 *  (a copy may be found at http://www.magiclogic.com/assignment.html).
 *  The code will not compile along side Concorde, and requires a full cost
 *  matrix (which we need to avoid for large problems).  Therefore, the best
 *  option was to implement a C version using our problem data structure.
 *
 *  This code is explained in the paper:
 *  "A Shortest Augmenting Path Algorithm for Dense and Sparse Linear 
 *  Assignment Problems," Computing 38, 325-340, 1987 by
 *  R. Jonker and A. Volgenant, University of Amsterdam.
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
init_data(int n, int **x, int **y, int **pi, int **d, int **pred, int **label,
          arrow_heap *heap);

void
destruct_data(int **x, int **y, int **pi, int **d, int **pred, int **label,
              arrow_heap *heap);

double
lap(arrow_problem *problem, int delta, int *x, int *y, 
    int *pi, int *d, int *pred, int *label, arrow_heap *heap);

void
dijkstra(arrow_problem *problem, int delta, int *x, int *y, int *pi, int s, 
         int *t, int *d, int *pred, int *label, arrow_heap *heap);

void
augment(arrow_problem *problem, int s, int t, int *pred, int *x, int *y);


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
arrow_cbap_lap(arrow_problem *problem, double *result)
{
    int ret;
    int n = problem->size * 2;
    int *x, *y, *pi, *d, *pred, *label;
    arrow_heap heap;
    
    ret = init_data(n, &x, &y, &pi, &d, &pred, &label, &heap);
    *result = lap(problem, INT_MAX, x, y, pi, d, pred, label, &heap);

    destruct_data(&x, &y, &pi, &d, &pred, &label, &heap);
    return ret;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
int
init_data(int n, int **x, int **y, int **pi, int **d, int **pred, int **label,
          arrow_heap *heap)
{
    if(!arrow_util_create_int_array(n, x))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, y))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, pi))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, d))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, pred))
        goto CLEANUP;
    if(!arrow_util_create_int_array(n, label))
        goto CLEANUP;
    if(!arrow_heap_init(heap, n))
        goto CLEANUP;
    
    return ARROW_SUCCESS;
    
CLEANUP:
    destruct_data(x, y, pi, d, pred, label, heap);
    return ARROW_FAILURE;
}

void
destruct_data(int **x, int **y, int **pi, int **d, int **pred, int **label,
              arrow_heap *heap)
{
    if(*x != NULL) free(*x);
    if(*y != NULL) free(*y);
    if(*pi != NULL) free(*pi);
    if(*d != NULL) free(*d);
    if(*pred != NULL) free(*pred);
    if(*label != NULL) free(*label);
    arrow_heap_destruct(heap);
}

double
lap(arrow_problem *problem, int delta, int *x, int *y, 
    int *pi, int *d, int *pred, int *label, arrow_heap *heap)
{
    int i, j, t;
    int n = problem->size;
    
    /* Initialization */
    for(i = 0; i < 2 * n; i++)
    {
        x[i] = -1;
        y[i] = -1;
        pi[i] = 0;
    }
        
    for(i = 0; i < n; i++)
    {
        /* Find the shortest path from i to any demand node t */
        dijkstra(problem, delta, x, y, pi, i, &t, d, pred, label, heap);
        
        /* If we cannot reach a demand node then problem's infeasible */
        if(t == -1) return -1.0;
        
        /* Update reduced costs */
        for(j = 0; j < 2 * n; j++)
        {
            if(label[j])
                pi[j] = pi[j] - d[j] + d[t];
        }
        
        /* Augment! */
        augment(problem, i, t, pred, x, y);
    }
    
    /* Calculate total cost of assignment */
    double cost = 0.0;
    for(i = 0; i < n; i++)
    {
        j = x[i] - n;
        cost += problem->get_cost(problem, i, j);
    }
    return cost;
}

void
dijkstra(arrow_problem *problem, int delta, int *x, int *y, int *pi, int s, 
         int *t, int *d, int *pred, int *label, arrow_heap *heap)
{
    int i, j, u, v;
    int start, stop, admissable;
    int cost, red_cost;
    int n = problem->size;
    
    /* Initialization */
    arrow_heap_empty(heap);
    for(i = 0; i < 2 * n; i++)
    {
        d[i] = INT_MAX;
        pred[i] = -1;
        label[i] = 0;
    }
    d[s] = 0;
    arrow_heap_insert(heap, 0, s);
    
    /* Main loop for Dijkstra */
    while(heap->size > 0)
    {
        i = arrow_heap_get_min(heap);
        arrow_heap_delete_min(heap);
        
        if(i < n)
        {
            start = n;
            stop = 2 * n - 1;
        }
        else
        {
            start = 0;
            stop = n - 1;
        } 

        label[i] = 1;
        
        /* If we've reached an unassigned demand node, we can stop */
        if((i >= n) && (y[i] == -1))
        {
            *t = i;
            return;
        }
            
        /* Iterate through neighbouring nodes */
        u = (i < n ? i : i - n);
        for(j = start; j <= stop; j++)
        {
            v = (j < n ? j : j - n);
            
            if(i < n)
                admissable = (x[i] != j);
            else
                admissable = (x[j] == i);
            
            if((u != v) && (admissable) && (!label[j]))
            {
                if(i >= n)
                    cost = problem->get_cost(problem, v, u);
                else
                    cost = problem->get_cost(problem, u, v);
                                
                if(cost <= delta)
                {
                    if(i >= n) cost = cost * -1;
                    red_cost = cost - pi[i] + pi[j];
                    
                    if(red_cost < 0)
                    {
                        arrow_print_error("Negative reduced cost!");
                        *t = -1;
                        return;
                    }
                    
                    if(red_cost + d[i] < d[j])
                    {
                        d[j] = red_cost + d[i];
                        pred[j] = i;
                        
                        if(arrow_heap_in(heap, j))
                            arrow_heap_change_key(heap, d[j], j);
                        else
                            arrow_heap_insert(heap, d[j], j);
                    }
                }
            }
        }
    }
    
    /* At this point, we've not encountered an unassigned demand node */
    *t = -1;
}

void
augment(arrow_problem *problem, int s, int t, int *pred, int *x, int *y)
{
    int u, v;
    v = t;
    while(v != s)
    {
        u = pred[v];
        x[u] = v;
        y[v] = u;        
        v = u;
    }
}


