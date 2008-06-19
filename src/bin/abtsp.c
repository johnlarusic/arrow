/**********************************************************doxygen*//** @file
 * @brief   Asymmetric Bottleneck TSP heuristic.
 *
 * Runs the Bottleneck TSP heuristic on the given asymmetric input file.  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "arrow.h"

/* Global variables */
char *input_file = NULL;
char *xml_file = NULL;
int random_restarts = -1;
int stall_count = -1;
int kicks = -1;
int confirm_sol = ARROW_FALSE;
int supress_ebst = ARROW_FALSE;
int find_short_tour = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;
int basic_attempts = ARROW_DEFAULT_BASIC_ATTEMPTS;

/* Program options */
#define NUM_OPTS 11
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

    arrow_problem atsp_problem;
    arrow_problem problem;
    arrow_problem_info info;
    arrow_tsp_lk_params lk_params;
    arrow_btsp_fun fun_basic;
    arrow_btsp_result result;
    arrow_btsp_params btsp_params;
    
    int infinity = INT_MAX;
    double start_time = arrow_util_zeit();
    double end_time;
    double bbssp_time = -1.0;
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
        
    /* Try and read the problem file */
    if(!arrow_problem_read(input_file, &atsp_problem))
        return EXIT_FAILURE;
    if(atsp_problem.symmetric)
    {
        arrow_print_error("Solver only works on symmetric matrices.");
        return EXIT_FAILURE;
    }
    
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(&atsp_problem, &info))
        return EXIT_FAILURE;
    printf("Number of unique costs: %d\n", info.cost_list_length);
    printf("Unique cost list: ");
    int i;
    for(i = 0; i < info.cost_list_length; i++)
        printf("%d, ", info.cost_list[i]);
    printf("EOL\n");
    
    /* Create transformed problem */
    infinity = info.max_cost * atsp_problem.size + 1;
    arrow_debug("infinity = %d\n", infinity);
    if(!arrow_problem_abtsp_to_sbtsp(&atsp_problem, infinity, &problem))
    {
        arrow_print_error("Could not create symmetric transformation.");
        arrow_problem_destruct(&atsp_problem);
        return EXIT_FAILURE;
    }

    /* Determine if we need to call the BBSSP to find a lower bound */
    if(lower_bound < 0)
    {
        lower_bound = info.min_cost;
        // TODO: Insert call to a ABTSP lower bound here
    }
    if(info.max_cost < upper_bound)
    {
        upper_bound = info.max_cost;
    }
    
    /* Setup LK parameters structure */
    arrow_tsp_lk_params_init(&problem, &lk_params);
    if(random_restarts >= 0)    lk_params.random_restarts  = random_restarts;
    if(stall_count >= 0)        lk_params.stall_count      = stall_count;
    if(kicks >= 0)              lk_params.kicks            = kicks;
        
    /* Setup necessary function structures */
    if(arrow_btsp_fun_basic_atsp(ARROW_FALSE, &fun_basic) != ARROW_SUCCESS)
        return EXIT_FAILURE;
    #define SOLVE_STEPS 1
    arrow_btsp_solve_plan steps[SOLVE_STEPS] = 
    {
       {
           ARROW_BTSP_SOLVE_PLAN_BASIC,     /* plan_type */
           ARROW_FALSE,                     /* use_exact_solver? */
           fun_basic,                       /* fun (cost matrix function) */
           lk_params,                       /* LK parameters */
           ARROW_FALSE,                     /* upper_bound_update? */
           basic_attempts                   /* attempts */
       }
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
    
    /* Solve BTSP */
    arrow_btsp_result_init(&problem, &result);
    if(!arrow_btsp_solve(&problem, &info, &btsp_params, &result))
    {
        arrow_print_error("Could not solve BTSP on file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    end_time = arrow_util_zeit() - start_time;
    
    /* Transform solution from symmetric solution to asymmetric solution */
    if(result.found_tour)
    {
        result.tour_length += atsp_problem.size * infinity;
    }
    
    /* Final output */
    printf("\nFound Tour: %s\n", (result.found_tour ? "Yes" : "No"));
    if(result.found_tour)
    {
        printf("Max. Cost: %d\n", result.obj_value);
        printf("Tour Length: %.0f\n", result.tour_length);
    }
    printf("Initial Lower Bound: %d\n", lower_bound);
    if(bbssp_time >= 0.0)
        printf("BBSSP Time: %.2f\n", bbssp_time);        
    printf("Optimal?: %s\n", (result.optimal == ARROW_TRUE ? "Yes" : "???"));
    printf("Binary Search Steps: %d\n", result.bin_search_steps);
    printf("Lin-Kernighan Calls: %d\n", result.linkern_attempts);
    if(result.linkern_attempts > 0)
        printf("Avg. Lin-Kernighan Time: %.2f\n", 
            result.linkern_time / (result.linkern_attempts * 1.0));
    printf("Exact TSP Calls: %d\n", result.exact_attempts);
    if(result.exact_attempts > 0)
        printf("Avg. Exact TSP Time: %.2f\n", 
            result.exact_time / (result.exact_attempts * 1.0));
    printf("Total BTSP Time: %.2f\n", result.total_time);
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

        fprintf(xml, "    <found_tour>%s</found_tour>\n", 
                (result.found_tour ? "true" : "false"));
        fprintf(xml, "    <objective_value>%d</objective_value>\n", 
                (result.found_tour ? result.obj_value : -1));
        fprintf(xml, "    <tour_length>%.0f</tour_length>\n", 
                (result.found_tour ? result.tour_length : -1.0));
        fprintf(xml, "    <optimal>%s</optimal>\n", 
                (result.optimal ? "true" : "false"));
        fprintf(xml, "    <lower_bound>%d</lower_bound>\n", lower_bound);
        fprintf(xml, "    <lower_bound_time>%.2f</lower_bound_time>\n",
                bbssp_time);
        fprintf(xml, "    <binary_search_steps>%d</binary_search_steps>\n",
                result.bin_search_steps);
        fprintf(xml, "    <linkern_attempts>%d</linkern_attempts>\n",
                result.linkern_attempts);
        fprintf(xml, "    <linkern_avg_time>%.2f</linkern_avg_time>\n", 
                (result.linkern_attempts > 0 
                 ? result.linkern_time / (result.linkern_attempts * 1.0) 
                 : 0.0));
        fprintf(xml, "    <exact_tsp_attempts>%d</exact_tsp_attempts>\n",
                result.exact_attempts);
        fprintf(xml, "    <exact_tsp_avg_time>%.2f</exact_tsp_avg_time>\n", 
                (result.exact_attempts > 0 
                 ? result.exact_time / (result.exact_attempts * 1.0) 
                 : 0.0));
        fprintf(xml, "    <btsp_total_time>%.2f</btsp_total_time>\n",
                result.total_time);
        fprintf(xml, "    <total_time>%.2f</total_time>\n", end_time);
        fprintf(xml, "</arrow_btsp>\n");
        
        fclose(xml);
    }
    
CLEANUP:
    arrow_btsp_result_destruct(&result);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_btsp_fun_destruct(&fun_basic);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_tsp_lk_params_destruct(&lk_params); 
    arrow_problem_destruct(&problem); 
    arrow_problem_destruct(&atsp_problem);
    return ret;
}