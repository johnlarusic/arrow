/**********************************************************doxygen*//** @file
 * @brief   Sub-problem generator.
 *
 * Generates a sub-problem from a larger one by outputing data for nodes on
 * the interval [start, end].
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"

/* Global variables */
char *program_name;             /**< Program name */
char *input_file = NULL;        /**< Given input TSPLIB file */
int start = 0;                  /**< Starting city index */
int end = 1;                    /**< Ending city index */

/* Program options */
#define NUM_OPTS 3
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'s', "start", "TSPLIB input file", 
        ARROW_OPTION_INT, &start, ARROW_FALSE, ARROW_TRUE},
    {'e', "end", "TSPLIB input file", 
        ARROW_OPTION_INT, &end, ARROW_TRUE, ARROW_TRUE}
};
char *desc = "Generates a sub-problem from a larger one";
char *usage = "-i tsplib.tsp -s # -e #";

/* Main Method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int stdout_id;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Need to supress output from Concorde */
    arrow_util_redirect_stdout_to_file(ARROW_DEV_NULL, &stdout_id);
    
    /* Try and read the problem file and its info */
    arrow_problem problem;
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    
    /* Restore output */
    arrow_util_restore_stdout(stdout_id);
    
    if((start >= 0) && (start <= end) && (end <= problem.size))
    {
        printf("NAME: %s[%d,%d]\n", problem.name, start, end);
        printf("TYPE: %s\n", (problem.symmetric ? "TSP" : "ATSP"));
        printf("DIMENSION: %d\n", end - start + 1);
        printf("EDGE_WEIGHT_TYPE: EXPLICIT\n");
        printf("EDGE_WEIGHT_FORMAT: FULL_MATRIX\n");
        printf("EDGE_WEIGHT_SECTION\n");
        
        int i, j;
        for(i = start; i <= end; i++)
        {
            for(j = start; j <= end; j++)
            {
                printf("%d\t", problem.get_cost(&problem, i, j));
            }
            printf("\n");
        } 
        printf("EOF\n");
    }
    else
    {
        arrow_print_error("Invalid start/end bounds!\n");
        ret = EXIT_FAILURE;
    }
    
    /* Free up the structure */
    arrow_problem_destruct(&problem);
    return ret;
}
