/**********************************************************doxygen*//** @file
 * @brief   Hash testing
 *
 * Tests hashing functions.  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include <cmph.h>
#include "common.h"

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
    int i;
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
    
    /* Create hash table */
    arrow_hash_cost_list(info.cost_list, info.cost_list_length, &hash);
    
    /* Check to make sure the hash table is working properly */
    for(i = 0; i < info.cost_list_length; i++)
    {
        int cost = info.cost_list[i];
        unsigned int k = arrow_hash_search(&hash, cost);
        printf("%d\t%d\t%s\t%d\n", i, cost, hash.vector[i], k);
    }
    printf("\n");
        
    /* Free up the structures */
    arrow_hash_destruct(&hash);
    arrow_problem_info_destruct(&info);
    arrow_problem_destruct(&problem);
    return EXIT_SUCCESS;
}
