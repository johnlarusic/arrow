/**********************************************************doxygen*//** @file
 * @brief   Hash testing
 *
 * Tests hashing functions.  
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
char *desc = "Tests the hashing functions.";
char *usage = "-i tsplib.tsp";

/* Main Method */
int 
main(int argc, char *argv[])
{   
    arrow_problem problem;
    arrow_problem_info info;
    arrow_hash hash;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Try and read the problem file and its info */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(!arrow_problem_info_get(&problem, &info))
        return EXIT_FAILURE;

    /* Create hash */
    if(!arrow_hash_init(info.cost_list, info.cost_list_length, &hash))
    {
        arrow_print_error("Could not create hash.");
        return EXIT_FAILURE;
    }
    
    int i, j, cost, key;
    for(i = 0; i < problem.size; i++)
    {
        for(j = i + 1; j < problem.size; j++)
        {
            cost = problem.get_cost(&problem, i, j);
            key = arrow_hash_search(&hash, cost);
            printf("C[%d,%d] = %d; l[%d] = %d\n", 
                i, j, cost, key, info.cost_list[key]);
        }
    }
    
    /* Free up the structures */
    arrow_hash_destruct(&hash);
    arrow_problem_info_destruct(&info);
    arrow_problem_destruct(&problem);
    return EXIT_SUCCESS;
}
