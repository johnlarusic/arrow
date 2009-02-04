/**********************************************************doxygen*//** @file
 * @brief   Bottleneck TSP heuristic.
 *
 * Runs the Bottleneck TSP heuristic on the given input file.  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"
#include "lb.h"
#include "tsp.h"
#include "btsp.h"

/* Global variables */
char *input_file = NULL;
char *xml_file = NULL;
int random_restarts = -1;
int stall_count = -1;
int kicks = -1;
int confirm_sol = ARROW_FALSE;
int supress_ebst = ARROW_FALSE;
int find_short_tour = ARROW_FALSE;
int supress_hash = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;
int basic_attempts = 1;

/* Program options */
#define NUM_OPTS 12
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'x', "xml", "file to write XML output to",
        ARROW_OPTION_STRING, &xml_file, ARROW_FALSE, ARROW_TRUE},
        
    {'r', "restarts", "number of random restarts",
        ARROW_OPTION_INT, &random_restarts, ARROW_FALSE, ARROW_TRUE},
    {'s', "stall-count", "max number of 4-swaps w/o progress",
        ARROW_OPTION_INT, &stall_count, ARROW_FALSE, ARROW_TRUE},
    {'k', "kicks", "number of 4-swap kicks",
        ARROW_OPTION_INT, &kicks, ARROW_FALSE, ARROW_TRUE},
    {'c', "confirm-solution", "confirm solution with exact solver",
        ARROW_OPTION_INT, &confirm_sol, ARROW_FALSE, ARROW_FALSE},
    {'e', "supress-ebst", "supress binary search",
        ARROW_OPTION_INT, &supress_ebst, ARROW_FALSE, ARROW_FALSE},
    {'S', "find-short-tour", "finds a (relatively) short BTSP tour",
        ARROW_OPTION_INT, &find_short_tour, ARROW_FALSE, ARROW_FALSE},
    {'h', "supress-hash", "do not create hash table",
        ARROW_OPTION_INT, &supress_hash, ARROW_FALSE, ARROW_FALSE},
        
    {'l', "lower-bound", "initial lower bound",
        ARROW_OPTION_INT, &lower_bound, ARROW_FALSE, ARROW_TRUE},
    {'u', "upper-bound", "initial upper bound",
        ARROW_OPTION_INT, &upper_bound, ARROW_FALSE, ARROW_TRUE},
    {'a', "basic-attempts", "number of basic attempts",
        ARROW_OPTION_INT, &basic_attempts, ARROW_FALSE, ARROW_TRUE}
};
char *desc = "Bottleneck traveling salesman problem (BTSP) solver";
char *usage = "-i tsplib.tsp [options]";

/* Main method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;

    arrow_problem problem;
    arrow_problem_info info;
    arrow_tsp_cc_lk_params lk_params;
    arrow_btsp_fun fun_basic;
    arrow_btsp_result result;
    arrow_btsp_params btsp_params;
    
    double start_time = arrow_util_zeit();
    double end_time;
    double bbssp_time = -1.0;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
        
    /* Try and read the problem file */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(!problem.symmetric)
    {
        arrow_print_error("Solver only works on symmetric matrices.");
        return EXIT_FAILURE;
    }
    
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(&problem, !supress_hash, &info))
        return EXIT_FAILURE;
    printf("Number of unique costs: %d\n", info.cost_list_length);

    /* Determine if we need to call the BBSSP to find a lower bound */
    if(lower_bound < 0)
    {
        printf("Solving BBSSP to find a lower bound\n");
        arrow_bound_result bbssp_result;
        if(!arrow_bbssp_solve(&problem, &info, &bbssp_result))
        {
            arrow_print_error("Could not solve BBSSP on file.\n");
            arrow_problem_destruct(&problem);
            return EXIT_FAILURE;
        }
        lower_bound = bbssp_result.obj_value;
        bbssp_time = bbssp_result.total_time;
        printf("BBSSP lower bound is %d\n.", lower_bound);
    }
    
    /* Setup LK parameters structure */
    arrow_tsp_cc_lk_params_init(&problem, &lk_params);
    if(random_restarts >= 0)    lk_params.random_restarts  = random_restarts;
    if(stall_count >= 0)        lk_params.stall_count      = stall_count;
    if(kicks >= 0)              lk_params.kicks            = kicks;
        
    /* Setup necessary function structures */
    if(arrow_btsp_fun_basic(ARROW_FALSE, &fun_basic) != ARROW_SUCCESS)
        return EXIT_FAILURE;
    #define SOLVE_STEPS 1
    arrow_btsp_solve_plan steps[SOLVE_STEPS] = 
    {
       {
           ARROW_TSP_CC_LK,                 /* TSP solver */
           (void *)&lk_params,              /* TSP solver parameters */
           fun_basic,                       /* fun (cost matrix function) */
           basic_attempts                   /* attempts */
       }
    };
    arrow_btsp_solve_plan confirm_plan = 
    {
       ARROW_TSP_CC_EXACT,
       NULL,
       fun_basic,
       1
    };
    
    
    /* Setup BTSP parameters structure */
    arrow_btsp_params_init(&btsp_params);
    btsp_params.confirm_sol         = confirm_sol;
    btsp_params.supress_ebst        = supress_ebst;
    btsp_params.find_short_tour     = find_short_tour;
    btsp_params.lower_bound         = lower_bound;
    btsp_params.upper_bound         = upper_bound;
    btsp_params.num_steps           = SOLVE_STEPS;
    btsp_params.steps               = steps;
    btsp_params.confirm_plan        = confirm_plan;
    
    /* Setup BTSP results structure */
    arrow_btsp_result_init(&problem, &result);
    if(!arrow_btsp_solve(&problem, &info, &btsp_params, &result))
    {
        arrow_print_error("Could not solve BTSP on file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    end_time = arrow_util_zeit() - start_time;
    
    /* Final output */
    arrow_btsp_result_print_pretty(&result, stdout);
    printf("Initial Lower Bound: %d\n", lower_bound);
    if(bbssp_time >= 0.0)
        printf("BBSSP Time: %.2f\n", bbssp_time);
    printf("Total Time: %.2f\n", end_time);
        
    /* Xml output */
    if(xml_file != NULL)
    {
        FILE *xml;
        if(!(xml = fopen(xml_file, "w")))
        {
            arrow_print_error("Could not open xml file for writing");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        fprintf(xml, "<arrow_btsp problem_file=\"%s\"", input_file);
        fprintf(xml, " command_args=\"");
        arrow_util_print_program_args(argc, argv, xml);
        fprintf(xml, "\">\n");

        arrow_btsp_result_print_xml(&result, xml);
        
        fprintf(xml, "    <total_time>%.2f</total_time>\n", end_time);
        fprintf(xml, "</arrow_btsp>\n");
        
        fclose(xml);
    }
    
CLEANUP:
    arrow_btsp_result_destruct(&result);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_btsp_fun_destruct(&fun_basic);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_tsp_cc_lk_params_destruct(&lk_params); 
    arrow_problem_destruct(&problem);
    return ret;
}
