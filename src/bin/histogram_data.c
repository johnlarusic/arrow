/**********************************************************doxygen*//** @file
 * @brief   Edge length histogram data collector.
 *
 * Prints out a list of every edge length present in given problem.  Used in
 * conjunction with a Python script for generating a histogram plot.
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "arrow.h"

/* Global variables */
char *program_name;             /**< Program name */
char *input_file = NULL;        /**< Given input TSPLIB file */

/* Program options */
#define NUM_OPTS 1
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE}
};
char *desc = "Prints a list of every cost in problem (for histogram.py)";
char *usage = "-i tsplib.tsp";

/* Main Method */
int 
main(int argc, char *argv[])
{   
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
    
    int i, j;
    for(i = 0; i < problem.size; i++)
    {
        for(j = i + 1; j < problem.size; j++)
        {
            printf("%d\n", problem.get_cost(&problem, i, j));
        }
    }
    
    /* Free up the structure */
    arrow_problem_destruct(&problem);
    return EXIT_SUCCESS;
}
