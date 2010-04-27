/**********************************************************doxygen*//** @file
 * @brief   Tour information.
 *
 * Displays tour information for a given problem input and tour input  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"

/* Global variables */
char *input_file = NULL;
char *tour_file = NULL;

/* Program options */
#define NUM_OPTS 2
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'T', "tour", "TSPLIB tour input file",
        ARROW_OPTION_STRING, &tour_file, ARROW_TRUE, ARROW_TRUE},
};
char *desc = "Prints tour information";
char *usage = "-i tsplib.tsp -t tsplib.tour";

/****************************************************************************
 * Function implementations
 ****************************************************************************/
int 
main(int argc, char *argv[])
{   
    arrow_problem problem;
    int *tour = NULL;
    int ret = ARROW_SUCCESS;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Read problem */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    
    /* Create tour array */
    if(!arrow_util_create_int_array(problem.size, &tour))
    {
        arrow_print_error("Could not create tour array.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    
    /* Read tour file */
    if(!arrow_problem_read_tour(tour_file, problem.size, tour))
    {
        arrow_print_error("Could not read tour file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    printf("!!!\n");
    
    int cost;
    int max_cost = INT_MIN;
    int min_cost = INT_MAX;
    double length = 0.0;
    int i, u, v;
    
    int shift = (tour[0] == -1);
    
    for(i = 0; i < problem.size; i++)
    {
        u = tour[i] + shift;
        v = tour[(i + 1) % problem.size] + shift;
        cost = problem.get_cost(&problem, v, u);
        printf("C[%d,%d] = %d\n", u, v, cost);
        
        length += cost;
        if(cost > max_cost) max_cost = cost;
        if(cost < min_cost) min_cost = cost;   
    }
    
    printf("Tour Length: %.0f\n", length);
    printf("Max Cost:    %d\n", max_cost);
    printf("Min Cost:    %d\n", min_cost);
    
CLEANUP:
    arrow_problem_destruct(&problem);
    if(tour != NULL) free(tour);
    return ret;
}

