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
    arrow_problem problem;
    arrow_problem_info info;
    int i;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Try and read the problem file and its info */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(!arrow_problem_info_get(&problem, &info))
        return EXIT_FAILURE;
        
    /* Create array for storing all these numbers */
    int BUFFER_SIZE = 15;
    char** str_cost_list;
    char* str_cost_list_space;
    
    str_cost_list = CC_SAFE_MALLOC(info.cost_list_length, char *);
    str_cost_list_space = CC_SAFE_MALLOC(info.cost_list_length * BUFFER_SIZE, char);

    /* Create hash */
    for(i = 0; i < info.cost_list_length; i++)
    {
        str_cost_list[i] = str_cost_list_space + i * BUFFER_SIZE;
        sprintf(str_cost_list[i], "%d", info.cost_list[i]);
    }
    
    cmph_io_adapter_t *source = cmph_io_vector_adapter((char **)str_cost_list, info.cost_list_length);
    
    cmph_config_t *config = cmph_config_new(source);
  	cmph_t *hash = cmph_new(config);
  	cmph_config_destroy(config);
    
    char buffer[BUFFER_SIZE];
    for(i = 0; i < info.cost_list_length; i++)
    {
        int cost = info.cost_list[i];
        sprintf(buffer, "%d", cost);
        unsigned int k = cmph_search(hash, buffer, strlen(buffer));
        printf("%d\t%d\t%s\t%u\n", i, cost, str_cost_list[i], k);
    }
    printf("\n");
    
    cmph_destroy(hash);
    cmph_io_vector_adapter_destroy(source);
    
        
    /* Free up the structures */
    free(str_cost_list_space);
    free(str_cost_list);
    arrow_problem_info_destruct(&info);
    arrow_problem_destruct(&problem);
    return EXIT_SUCCESS;
}
