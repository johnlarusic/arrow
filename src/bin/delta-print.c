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
int solve_mstsp = ARROW_FALSE;
int deep_copy = ARROW_FALSE;
int edge_infinity = -1;

/* Program options */
#define NUM_OPTS 5
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'d', "delta", "Delta value", 
        ARROW_OPTION_INT, &delta, ARROW_TRUE, ARROW_TRUE},
    {'m', "solve-mstsp", "solves maximum scatter TSP",
        ARROW_OPTION_INT, &solve_mstsp, ARROW_FALSE, ARROW_FALSE},
    {'D', "deep-copy", "stores data in full cost-matrix",
        ARROW_OPTION_INT, &deep_copy, ARROW_FALSE, ARROW_FALSE},
    {'I', "infinity", "value to use as infinity",
        ARROW_OPTION_INT, &edge_infinity, ARROW_FALSE, ARROW_TRUE},        
};
char *desc = "Prints cost matrix for delta feasibility problem";
char *usage = "-i tsplib.tsp -d #";

/* Main Method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    //int stdout_id;
    int max_cost = INT_MAX;
    char comment[100];
    arrow_problem *problem;
    arrow_problem input_problem;
    arrow_problem mstsp_problem;
    arrow_problem asym_problem;
    arrow_problem delta_problem;
    arrow_btsp_fun fun_basic;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    /* Try and read the problem file */    
    if(!arrow_problem_read(input_file, &input_problem))
        return EXIT_FAILURE;
    problem = &input_problem;
    
    /* If we want to solve maximum scatter TSP, create BTSP equivalent. */
    if(solve_mstsp)
    {
        printf("Transforming MSTSP instance into equivalent BTSP instance.\n");
        max_cost = arrow_problem_max_cost(problem);
        if(!arrow_problem_mstsp_to_btsp(deep_copy, problem, max_cost, &mstsp_problem))
        {
            arrow_print_error("Could not create MSTSP->BTSP transformation.");
            return EXIT_FAILURE;
        }
        problem = &mstsp_problem;
        delta = max_cost - delta;
    }
    
    /* Calculate the value for "infinity" if necessary */
    if(edge_infinity < 0)
        edge_infinity = arrow_problem_max_cost(problem) + 1;
       
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
    if(solve_mstsp)
        sprintf(comment, "Delta is %d, Infinity is %d, Original Max Cost %d", delta, 
                edge_infinity, max_cost);
    else
        sprintf(comment, "Delta is %d, Infinity is %d", delta, edge_infinity);
        
    /* Print problem file out */
    arrow_util_write_problem(&delta_problem, comment, stdout);
    
    /* Free up the structure */
    arrow_problem_destruct(&delta_problem);
    arrow_btsp_fun_destruct(&fun_basic);
    if(!input_problem.symmetric)
        arrow_problem_destruct(&asym_problem);
    if(solve_mstsp)
        arrow_problem_destruct(&mstsp_problem);
    arrow_problem_destruct(&input_problem);
    return ret;
}
