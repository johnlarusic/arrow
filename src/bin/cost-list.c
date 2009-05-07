/**********************************************************doxygen*//** @file
 * @brief   Prints cost matrix for delta feasibility problem.
 *
 * Prints cost matrix for delta feasibility problem.  For use for verifying
 * problem optimality with Concorde.
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/* Global variables */
char *program_name;             /**< Program name */
char *input_file = NULL;        /**< Given input TSPLIB file */
int lower_bound = -2;
int upper_bound = INT_MAX;

/* Program options */
#define NUM_OPTS 3
arrow_option options[NUM_OPTS] =
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'l', "lower-bound", "initial lower bound",
        ARROW_OPTION_INT, &lower_bound, ARROW_FALSE, ARROW_TRUE},
    {'u', "upper-bound", "initial upper bound",
        ARROW_OPTION_INT, &upper_bound, ARROW_FALSE, ARROW_TRUE}
};
char *desc = "Prints cost list for problem";
char *usage = "-i tsplib.tsp -d #";

/* Main Method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int low, high, i;
    arrow_problem problem;
    arrow_problem_info info;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
        
    /* Try and read the problem file and its info */    
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(!arrow_problem_info_get(&problem, ARROW_TRUE, &info))
        return EXIT_FAILURE;
        
    if(lower_bound > upper_bound)
    {
        arrow_print_error("Upper bound must be larger than lower bound\n");
        return EXIT_FAILURE;
    }
    
    if(lower_bound < 0)
        low = 0;
    else
        arrow_problem_info_cost_index(&info, lower_bound, &low);
    
    if(upper_bound == INT_MAX)
        high = info.cost_list_length - 1;
    else
        arrow_problem_info_cost_index(&info, upper_bound, &high);
    
    for(i = low; i <= high; i++)
        printf("%d\n", info.cost_list[i]);
    printf("TOTAL: %d\n", high - low);
    
    /* Free up the structure */
    arrow_problem_info_destruct(&info);
    arrow_problem_destruct(&problem);
    return ret;
}
