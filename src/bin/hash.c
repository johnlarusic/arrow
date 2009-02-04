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
int supress_hash = ARROW_FALSE;

/* Program options */
#define NUM_OPTS 2
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'H', "supress-hash", "do not create hash table",
        ARROW_OPTION_INT, &supress_hash, ARROW_FALSE, ARROW_FALSE}
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
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Try and read the problem file and its info */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(!arrow_problem_info_get(&problem, !supress_hash, &info))
        return EXIT_FAILURE;
    
    /* Check to make sure the hash table is working properly */
    if(info.hash.num_keys > 0)
    {
        for(i = 0; i < info.cost_list_length; i++)
        {
            int cost = info.cost_list[i];
            unsigned int k = arrow_hash_search(&(info.hash), cost);
            printf("%d\t%d\t%s\t%d\n", i, cost, info.hash.vector[i], k);
        }
        printf("\n");
    }
    else
    {
        printf("Hash table not created as per user option.\n");
    }
        
    /* Free up the structures */
    arrow_problem_info_destruct(&info);
    arrow_problem_destruct(&problem);
    return EXIT_SUCCESS;
}
