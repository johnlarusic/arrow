/**********************************************************doxygen*//** @file
 * @brief   Functions for working with problem data.
 *
 * Function implemenations for working with problem data, generally 
 * manipulating the arrow_problem data structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "arrow.h"


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_problem_read(char *file_name, arrow_problem *problem)
{
    int size;
    
    /* Get Concorde to read and populate its data structure */
    if(CCutil_gettsplib(file_name, &size, &(problem->data)) != 0)
    {
        arrow_print_error("Unable to read TSPLIB file using Concorde\n");
        return ARROW_ERROR_FATAL;
    }

    problem->size = size;
    problem->symmetric = ARROW_TRUE;
    problem->shallow = ARROW_FALSE;
    problem->name = NULL;
    problem->get_cost = arrow_problem_get_cost;
    
    return ARROW_SUCCESS;
}

void
arrow_problem_destruct(arrow_problem *problem)
{
    /* Free Concorde's data structure */
    if(problem->shallow == ARROW_FALSE)
    {
        if(&(problem->data) != NULL)
        {
            CCutil_freedatagroup(&(problem->data));
        }
    }
}

int
arrow_problem_info_get(arrow_problem *problem, arrow_problem_info *info)
{
    arrow_bintree tree;
    int ret;
    int i, j;
    int cost, min_cost, max_cost;
    
    ret = ARROW_SUCCESS;
    arrow_bintree_init(&tree);
    min_cost = INT_MAX;
    max_cost = INT_MIN;
        
    for(i = 0; i < problem->size; i++)
    {
        for(j = i + 1; j < problem->size; j++)
        {
            /* Add to binary tree */
            cost = problem->get_cost(problem, i, j);
            ret = arrow_bintree_insert(&tree, cost);
            if(ret != ARROW_SUCCESS) goto CLEANUP;
            
            /* Determine min and max costs */
            if(cost < min_cost) min_cost = cost;
            if(cost > max_cost) max_cost = cost;
        }
    }
    
    /* Convert tree to array */
    arrow_bintree_to_array(&tree, &(info->cost_list));
    info->cost_list_length = tree.size;
    info->min_cost = min_cost;
    info->max_cost = max_cost;
    
CLEANUP:
    arrow_bintree_destruct(&tree);
    return ret;
}

void
arrow_problem_info_destruct(arrow_problem_info *info)
{
    if(info->cost_list != NULL)
        free(info->cost_list);
}

void
arrow_problem_print(arrow_problem *problem)
{
    int i, j, k, count;
    int group_size = 8;
    
    printf("Problem Cost Matrix:\n");
    printf("----------------------------------------");
    printf("--------------------------------------\n");
    
    /* Print cost matrix out into groups so they will (normally) fit into
     * an 80-character width terminal. */
    k = 0;
    
    arrow_debug("Problem Size: %d\n", problem->size);
    while(k < problem->size)
    {
        /* Handle special case for last column */
        if(problem->size - k < group_size)
            count = problem->size - k;
        else
            count = group_size;
        
        /* Print out the header */
        for(j = k; j < k + count; j++)
            printf("\t[j=%d]", j);   
        printf("\n");

        /* Print out rows */
        for(i = 0; i < problem->size; i++)
        {
            printf("[i=%d]", i);
            for(j = k; j < k + count; j++)
            {
                if(i == j)
                    printf("\t-");
                else
                    printf("\t%d", problem->get_cost(problem, i, j));
            }
            printf("\n");
        }
        printf("\n");

        k += count;
    }
        
}

inline int 
arrow_problem_get_cost(arrow_problem *problem, int i, int j)
{
    return problem->data.edgelen(i, j, &(problem->data));
}

int
arrow_problem_read_tour(char *file_name, int size, int *tour)
{
    if(CCutil_getcycle_tsplib(size, file_name, tour) == CONCORDE_SUCCESS)
        return ARROW_SUCCESS;
    else
        return ARROW_ERROR_FATAL;
}