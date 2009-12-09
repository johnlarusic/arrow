/**********************************************************doxygen*//** @file
 * @brief   Bottleneck Assignment Problem solver.
 *
 * Solves the bottleneck assignment problem (BAP) on the given input file.
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"
#include "baltsp.h"

/* Global variables */
char *program_name;             /**< Program name */
char *input_file = NULL;        /**< Given input TSPLIB file */
char *xml_file = NULL;
int deep_copy = ARROW_FALSE;
int btsp_min_cost = -1;
int btsp_max_cost = -1;
int mstsp_min_cost = -1;

/* Program options */
#define NUM_OPTS 6
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'x', "xml", "File to write XML output to",
        ARROW_OPTION_STRING, &xml_file, ARROW_FALSE, ARROW_TRUE},
    {'d', "deep-copy", "stores data in full cost-matrix",
        ARROW_OPTION_INT, &deep_copy, ARROW_FALSE, ARROW_FALSE},
        
    {'t', "btsp-min-cost", "min cost in BTSP tour",
        ARROW_OPTION_INT, &btsp_min_cost, ARROW_FALSE, ARROW_TRUE},
    {'u', "btsp-max-cost", "max cost in BTSP tour",
        ARROW_OPTION_INT, &btsp_max_cost, ARROW_FALSE, ARROW_TRUE},
    {'v', "mstsp-min-cost", "min cost in MSTSP tour",
        ARROW_OPTION_INT, &mstsp_min_cost, ARROW_FALSE, ARROW_TRUE}
};
char *desc = "Bottleneck assignment problem solver";
char *usage = "-i tsplib.tsp [options] ";

/* Main method */
int 
main(int argc, char *argv[])
{   
    int ret = ARROW_SUCCESS;
    arrow_problem *problem;
    arrow_problem input_problem;
    arrow_problem_info info;
    int lb_result;
    double start_time, end_time, lb_time;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;

    /* Read the problem file */
    problem = &input_problem;
    if(!arrow_problem_read(input_file, problem))
        return EXIT_FAILURE;
    
    /* Get problem information */
    if(!arrow_problem_info_get(problem, ARROW_FALSE, &info))
        return EXIT_FAILURE;
    
    /* Solve BalTSP LB */
    start_time = arrow_util_zeit();
    if(!arrow_balanced_tsp_lb(problem, &info, btsp_min_cost, btsp_max_cost, mstsp_min_cost, &lb_result))
    {
        arrow_print_error("Could not solve BalTSP LB on file.");
        return EXIT_FAILURE;
    }
    end_time = arrow_util_zeit();
    lb_time = end_time - start_time;
    
    printf("\nBalTSP LB Solution: %d\n", lb_result);
    printf("Total Time: %5.2f\n", lb_time);
    
    /* Print out XML file if required */
    if(xml_file != NULL)
    {
        FILE *xml;
        if(!(xml = fopen(xml_file, "w")))
        {
            arrow_print_error("Could not open xml file for writing");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        fprintf(xml, "<arrow_bound type=\"BalTSP\" ");
        fprintf(xml, "problem_file=\"%s\"", input_file);
        fprintf(xml, " command_args=\"");
        arrow_util_print_program_args(argc, argv, xml);
        fprintf(xml, "\">\n");
        fprintf(xml, "    <objective_value>%d</objective_value>\n", lb_result);
        fprintf(xml, "    <total_time>%.5f</total_time>\n", lb_time);
        fprintf(xml, "</arrow_bound>\n");
        
        fclose(xml);
    }
        
CLEANUP:
    arrow_problem_destruct(&input_problem);
    arrow_problem_info_destruct(&info);
    return ret;
}
