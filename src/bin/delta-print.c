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
int delta = 0;                  /**< Delta value */

/* Program options */
#define NUM_OPTS 2
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'d', "delta", "Delta value", 
        ARROW_OPTION_INT, &delta, ARROW_TRUE, ARROW_TRUE}
};
char *desc = "Prints cost matrix for delta feasibility problem";
char *usage = "-i tsplib.tsp -d #";

/* Main Method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int stdout_id;
    int edge_infinity;
    char comment[100];
    arrow_problem *problem;
    arrow_problem input_problem;
    arrow_problem asym_problem;
    arrow_problem delta_problem;
    arrow_problem_info info;
    arrow_btsp_fun fun_basic;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Need to supress output from Concorde */
    //arrow_util_redirect_stdout_to_file(ARROW_DEV_NULL, &stdout_id);
    
    /* Try and read the problem file and its info */    
    if(!arrow_problem_read(input_file, &input_problem))
        return EXIT_FAILURE;
    problem = &input_problem;
        
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(problem, ARROW_TRUE, &info))
        return EXIT_FAILURE;
    edge_infinity = info.max_cost + 1;
        
    if(problem->symmetric)
    {
        if(arrow_btsp_fun_basic(ARROW_FALSE, &fun_basic) != ARROW_SUCCESS)
            return EXIT_FAILURE;
    }
    else
    {
        if(!arrow_problem_abtsp_to_sbtsp(ARROW_FALSE, problem, edge_infinity, &asym_problem))
            return EXIT_FAILURE;
        if(!arrow_btsp_fun_asym_shift(ARROW_FALSE, edge_infinity, &fun_basic))
            return EXIT_FAILURE;
        problem = &asym_problem;
    }

    /* Apply function! */
    if(!arrow_btsp_fun_apply(&fun_basic, problem, delta, &delta_problem))
    {
        arrow_print_error("Could not apply function to problem.\n");
        return EXIT_FAILURE;
    }
    
    /* Restore output */
    //arrow_util_restore_stdout(stdout_id);
    
    /* Generate comment */
    sprintf(comment, "Delta is %d, Infinity is %d", delta, edge_infinity);
    
    /* Print problem file out */
    arrow_util_write_problem(&delta_problem, comment, stdout);
    
    /* Free up the structure */
    arrow_problem_destruct(&delta_problem);
    arrow_btsp_fun_destruct(&fun_basic);
    if(!input_problem.symmetric)
        arrow_problem_destruct(&asym_problem);
    arrow_problem_destruct(&input_problem);
    return ret;
}
