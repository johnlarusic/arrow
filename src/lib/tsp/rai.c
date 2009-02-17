/**********************************************************doxygen*//** @file
 *  @brief   Random arbitrary insertion TSP heuristic.
 *
 *  Random arbitrary insertion (RAI) TSP heuristic.
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "tsp.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Given a set of nodes to insert, inserts them as cheaply
 *          as possible into a partial tour.
 *  @param  problem [in] problem data
 *  @param  solve_btsp [in] if ARROW_TRUE, attempts to construct tour by
 *              minimizing the maximum cost, otherwise constructs tour by
 *              minimizing the largest cost
 *  @param  node_list [in] the list of nodes to insert into the partial tour
 *  @param  list_size [in] the number of nodes in the node list
 *  @param  tour [out] initially a partial tour, at termination is a
 *              full tour of n nodes
 *  @param  length [out] the length of the tour (or largest cost in tour
 *              if solve_btsp = ARROW_TRUE)
 *  @param  ins_list [out] a temporary array of pointers to linked list items
 */
void
construct_tour(arrow_problem *problem, int solve_btsp, int *node_list, 
               int list_size, arrow_llist *tour, double *length,
               arrow_llist_item **ins_list);

/**
 *  @brief  Attempts to improve upon the given tour by removing a random path
 *          and reinserting the removed nodes in a random order as cheaply as
 *          possible..
 *  @param  problem [in] problem data
 *  @param  solve_btsp [in] if ARROW_TRUE, attempts to construct tour by
 *              minimizing the maximum cost, otherwise constructs tour by
 *              minimizing the largest cost
 *  @param  best_tour [out] the best tour found so far; if a better tour is
 *              found this pointer will be changed to point to the new best
 *              tour
 *  @param  length [out] the length of the tour (or largest cost in tour
 *              if solve_btsp = ARROW_TRUE)
 *  @param  tour [out] initially a partial tour, at termination is a
 *              full tour of n nodes
 *  @param  ins_list [out] a temporary array of pointers to linked list items
 *  @param  node_list [out] temporary array of integers of size n
 */           
void
improve_tour(arrow_problem *problem, int solve_btsp, arrow_llist *best_tour, 
            double *length, arrow_llist *tour, arrow_llist_item **ins_list, 
            int *node_list);

/**
 *  @brief  Returns the min of the two values
 *  @param  i [in] first number
 *  @param  j [in] second number
 *  @return the smallest of i, j
 */
int
min2(int i, int j);

/**
 *  @brief  Returns the max of the two values
 *  @param  i [in] first number
 *  @param  j [in] second number
 *  @return the largest of i, j
 */
int
max2(int i, int j);

/**
 *  @brief  Returns the max of the three values
 *  @param  i [in] first number
 *  @param  j [in] second number
 *  @param  k [in] third number
 *  @return the largest of i, j, k
 */
int
max3(int i, int j, int k);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int 
arrow_tsp_rai_solve(arrow_problem *problem, arrow_tsp_rai_params *params, 
                    arrow_tsp_result *result)
{
    int ret = ARROW_SUCCESS;
    int i, u, v;
    double length;
    int *order = NULL;
    int n = problem->size;
    double start_time, end_time;
    arrow_llist best_tour;
    arrow_llist tour;
    arrow_llist_item **ins_list = NULL;
    
    /* Print out RAI parameters */
    arrow_debug("RAI Parameters:\n");
    arrow_debug(" - Iterations: %d\n", params->iterations);
    arrow_debug(" - Solve BTSP?: %s\n", 
                (params->solve_btsp ? "Yes" : "No"));
    
    start_time = arrow_util_zeit();
    
    /* Initialize the arrays */
    if(!arrow_util_create_int_array(n, &order))
        return ARROW_FAILURE;
    if((ins_list = malloc(n * sizeof(arrow_llist_item *))) == NULL)
    {
        arrow_print_error("Error allocating memory for ins_list array.");
        ins_list = NULL;
        ret = ARROW_FAILURE;
        goto CLEANUP;
    }
        
    /* Start by creating a random permutation of numbers from 0 to n-1.  This
       order will be the order we process nodes for the initial tour */
    for(i = 0; i < n; i++) order[i] = i;
    arrow_util_permute_array(n, order);
    
    /* We store our constructed tour in a linked list.  Initially, we insert
       the last two nodes from our random permutation, and find the largest
       cost in the two node cycle. */
    u = order[n - 2];
    v = order[n - 1];
    arrow_llist_init(&best_tour);
    arrow_llist_init(&tour);
    arrow_llist_insert_tail(&best_tour, u);
    arrow_llist_insert_tail(&best_tour, v);

    /* Using a node-insertion heuristic, build a starting tour in the
       cheapest way possible. */
    construct_tour(problem, params->solve_btsp, order, n - 2, &best_tour, 
                   &length, ins_list);
    arrow_debug("Constructed initial tour (length: %.0f)\n", length);
    
    /* Now we randomly remove chunks from the best tour, replace them into
       a new tour, and see if we can get a better solution */
    for(i = 1; i <= params->iterations; i++)
    {
        /* See if we found a tour of length 0 (we stop then!) */
        if(length == 0.0)
            break;
        improve_tour(problem, params->solve_btsp, &best_tour, &length, &tour,
                     ins_list, order);
    }
    arrow_debug("Finished RAI iterations\n");
    
    end_time = arrow_util_zeit();
    
    /* Set results */
    result->total_time = end_time - start_time;
    result->obj_value = length;
    result->found_tour = ARROW_TRUE;
    arrow_llist_to_array(&best_tour, result->tour);    
    
CLEANUP:
    arrow_llist_destruct(&best_tour);
    arrow_llist_destruct(&tour);
    if(ins_list != NULL) free(ins_list);
    if(order != NULL) free(order);
    return ret;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
void
improve_tour(arrow_problem *problem, int solve_btsp, arrow_llist *best_tour, 
            double *length, arrow_llist *tour, arrow_llist_item **ins_list, 
            int *node_list)
{
    int j, u, v;
    double new_length;
    arrow_llist_item *node;
    
    /* Pick two nodes at random (can be the same node!) */
    u = arrow_util_random_between(0, problem->size - 1);
    v = arrow_util_random_between(0, problem->size - 1);
    
    /* Find u in the linked list */
    node = best_tour->head;
    while(node != NULL)
    {
        if(node->data == u)
            break;
        node = node->next;
    }
    
    /* Start inserting nodes from u to v into an array */
    j = 0;
    while(1)
    {
        node_list[j] = node->data;
        j++;
        
        if(node->data == v)
            break;    

        if(node->next == NULL)
            node = best_tour->head;
        else
            node = node->next;
    }
    
    /* Ensure linked list is empty */
    arrow_llist_destruct(tour);
    
    /* Insert the rest of the tour into a new linked list */
    if(node->next == NULL)
        node = best_tour->head;
    else
        node = node->next;

    while(1)
    {
        if(node->data == u)
            break;
        
        arrow_llist_insert_tail(tour, node->data);
        
        if(node->next == NULL)
            node = best_tour->head;
        else
            node = node->next;
    }
    
    /* Determine new insertion order */
    arrow_util_permute_array(j, node_list);
    
    /* Finally, reinsert all those removed nodes back into the tour */
    construct_tour(problem, solve_btsp, node_list, j, tour, 
                   &new_length, ins_list);
    
    /* See if we found a better tour */
    if(new_length < *length)
    {
        arrow_debug("Better tour found! (length: %.0f)\n", new_length);
        *length = new_length;
        arrow_llist_swap(best_tour, tour);
    }
}

void
construct_tour(arrow_problem *problem, int solve_btsp, int *node_list, 
               int list_size, arrow_llist *tour, double *length,
               arrow_llist_item **ins_list)
{
    int i, j, u, v, w;
    int cost, in_cost, out_cost;
    double best_cost, ins_cost;
    int alpha, beta;
    arrow_llist_item *node;
    
    /* If our partial tour has less than two nodes, then we'll pick off the
       last two nodes from the randomized node_list until we have at least two
       nodes to work with. */
    while(tour->size  < 2)
    {
        arrow_llist_insert_tail(tour, node_list[list_size - 1]);
        list_size--;
    }
    
    /* If we're solving for the BTSP, then we need to find the largest
       and second largest costs.  Otherwise, we want the total length. */
    node = tour->head;
    alpha = INT_MIN;
    beta = INT_MIN;
    *length = 0.0;
    while(node != NULL)
    {
        u = node->data;
        if(node->next == NULL)
            v = tour->head->data;
        else
            v = node->next->data;
        cost = problem->get_cost(problem, u, v);

        *length += cost;
        if(cost > alpha)
        {
            beta = alpha;
            alpha = cost;
        }
        else if(cost > beta)
        {
            beta = cost;
        }

        node = node->next;
    }
    
    /* Find the best place to insert the left over nodes */    
    for(i = 0; i < list_size; i++)
    {
        /* Te current node  is v, we want to insert between nodes u and w. */
        v = node_list[i];    
        
        best_cost = DBL_MAX;
        node = tour->head;
        while(node != NULL)
        {
            u = node->data;
            if(node->next == NULL)
                w = tour->head->data;
            else
                w = node->next->data;
            
            cost = problem->get_cost(problem, u, w);
            in_cost = problem->get_cost(problem, u, v);
            out_cost = problem->get_cost(problem, v, w);
            
            if(solve_btsp)
            {
                if(cost == alpha)
                    ins_cost = max3(beta, in_cost, out_cost);
                else
                    ins_cost = max3(alpha, in_cost, out_cost);
            }
            else
            {
                ins_cost = *length + in_cost + out_cost - cost;
            }
            
            /* When looking for the best spot to insert v, we keep a list of
               all the best spots (in ins_list).  Later on, we'll pick one at 
               random */
            if(ins_cost < best_cost)
            {
                best_cost = ins_cost;
                ins_list[0] = node;
                j = 1;
            }
            else if(ins_cost == best_cost)
            {
                ins_list[j] = node;
                j++;
            }
            
            node = node->next;
        }
        
        /* Of the available spots to place the node, we pick one randomly */
        j = arrow_util_random_between(0, j - 1);
        node = ins_list[j];
        arrow_llist_insert_after(tour, node, v);
        
        /* Update alpha and beta, and the length */
        node = tour->head;
        alpha = INT_MIN;
        beta = INT_MIN;
        *length = 0.0;
        while(node != NULL)
        {
            u = node->data;
            if(node->next == NULL)
                w = tour->head->data;
            else
                w = node->next->data;
            cost = problem->get_cost(problem, u, w);
            
            *length += cost;
            if(cost > alpha)
            {
                beta = alpha;
                alpha = cost;
            }
            else if(cost > beta)
            {
                beta = cost;
            }
            
            node = node->next;
        }
    }
    
    /* If we're solving BTSP, return the length as the largest cost */
    if(solve_btsp)
    {
        *length = alpha;
    }
}

int
min2(int i, int j)
{
    int min = i;
    if(min > j) min = j;
    return min;
}

int
max2(int i, int j)
{
    int max = i;
    if(max < j) max = j;
    return max;
}

int
max3(int i, int j, int k)
{
    int max = i;
    if(max < j) max = j;
    if(max < k) max = k;
    return max;
}

