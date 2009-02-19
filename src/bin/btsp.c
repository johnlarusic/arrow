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
char *tour_file = NULL;
int edge_infinity = -1;
int random_restarts = -1;
int stall_count = -1;
int kicks = -1;
int confirm_sol = ARROW_FALSE;
int supress_ebst = ARROW_FALSE;
int find_short_tour = ARROW_FALSE;
int supress_hash = ARROW_FALSE;
int deep_copy = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;
int basic_attempts = 3;
int shake_1_attempts = 1;
int shake_1_rand_min = 0;
int shake_1_rand_max = -1;
int random_seed = 0;

/* Program options */
#define NUM_OPTS 18
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'x', "xml", "file to write XML output to",
        ARROW_OPTION_STRING, &xml_file, ARROW_FALSE, ARROW_TRUE},
    {'T', "tour", "file to write tour to",
        ARROW_OPTION_STRING, &tour_file, ARROW_FALSE, ARROW_TRUE},
        
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
    {'H', "supress-hash", "do not create hash table",
        ARROW_OPTION_INT, &supress_hash, ARROW_FALSE, ARROW_FALSE},
    {'d', "deep-copy", "stores data in full cost-matrix",
        ARROW_OPTION_INT, &deep_copy, ARROW_FALSE, ARROW_FALSE},
        
    {'l', "lower-bound", "initial lower bound",
        ARROW_OPTION_INT, &lower_bound, ARROW_FALSE, ARROW_TRUE},
    {'u', "upper-bound", "initial upper bound",
        ARROW_OPTION_INT, &upper_bound, ARROW_FALSE, ARROW_TRUE},
    {'a', "basic-attempts", "number of basic attempts",
        ARROW_OPTION_INT, &basic_attempts, ARROW_FALSE, ARROW_TRUE},
        
    {'b', "shake-1-attempts", "number of shake type I attempts",
        ARROW_OPTION_INT, &shake_1_attempts, ARROW_FALSE, ARROW_TRUE},
    {'1', "shake-1-rand-min", "min value for shake type I random numbers",
        ARROW_OPTION_INT, &shake_1_rand_min, ARROW_FALSE, ARROW_TRUE},
    {'2', "shake-1-rand-max", "max value for shake type I random numbers",
        ARROW_OPTION_INT, &shake_1_rand_max, ARROW_FALSE, ARROW_TRUE},
        
    {'g', "random-seed", "random number generator seed",
        ARROW_OPTION_INT, &random_seed, ARROW_FALSE, ARROW_TRUE}
};
char *desc = "Bottleneck traveling salesman problem (BTSP) solver";
char *usage = "-i tsplib.tsp [options]";

/* Main method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int i, u, v, cost;

    arrow_problem input_problem;
    arrow_problem asym_problem;
    arrow_problem *problem;
    arrow_problem_info info;
    arrow_tsp_cc_lk_params lk_params;
    arrow_btsp_fun fun_basic;
    arrow_btsp_fun fun_shake_1;
    arrow_btsp_result result;
    arrow_btsp_params btsp_params;
    
    double start_time = arrow_util_zeit();
    double end_time;
    double lower_bound_time = -1.0;
    
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    
    /* Try and read the problem file.  We assume it's symmetric, but do some
       stick handling if it turns out to be asymmetric later. */
    problem = &input_problem;
    if(!arrow_problem_read(input_file, problem))
        return EXIT_FAILURE;
    
    
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(problem, !supress_hash, &info))
        return EXIT_FAILURE;
    printf("Num costs in problem: %d\n", info.cost_list_length);
    printf("Max cost in problem:  %d\n", info.max_cost);
    
    
    /* Extra processing for arguments */
    if(shake_1_rand_max < 0) 
        shake_1_rand_max = (input_problem.size * input_problem.size) + shake_1_rand_min;
    if(shake_1_rand_max - shake_1_rand_min < info.cost_list_length)
    {
        arrow_print_error("shake random interval not large enough");
        return EXIT_FAILURE;
    }
    
    
    /* Calculate a value for "infinity" if necessary */
    if(edge_infinity < 0)
        edge_infinity = (info.max_cost + shake_1_rand_max) * 2;
    else if(edge_infinity < info.max_cost)
    {
        arrow_print_error("Infinity value is not large enough\n");
        return EXIT_FAILURE;
    }
    printf("Infinity Value:       %d\n", edge_infinity);
    
    
    /* If the problem's asymmetric, create symmetric from transformation. */
    if(!problem->symmetric)
    {
        if(!arrow_problem_abtsp_to_sbtsp(deep_copy, problem, edge_infinity, &asym_problem))
        {
            arrow_print_error("Could not create symmetric transformation.");
            return EXIT_FAILURE;
        }
        problem = &asym_problem;
    }
    
        
    /* Initialize random number generator */
    arrow_util_random_seed(random_seed);


    /* Determine if we need to call the BBSSP to find a lower bound */
    if(lower_bound < 0)
    {
        /* TODO: Add support for other lower bounds here */
        printf("Solving BBSSP to find a lower bound\n");
        arrow_bound_result bbssp_result;
        if(!arrow_bbssp_solve(problem, &info, &bbssp_result))
        {
            arrow_print_error("Could not solve BBSSP on file.\n");
            return EXIT_FAILURE;
        }
        lower_bound = bbssp_result.obj_value;
        lower_bound_time = bbssp_result.total_time;
        printf("BBSSP lower bound is %d\n.", lower_bound);
    }
    
    
    /* Setup LK parameters structure */
    arrow_tsp_cc_lk_params_init(problem, &lk_params);
    if(random_restarts >= 0)    lk_params.random_restarts  = random_restarts;
    if(stall_count >= 0)        lk_params.stall_count      = stall_count;
    if(kicks >= 0)              lk_params.kicks            = kicks;
    
    if(!input_problem.symmetric)
        lk_params.length_bound = (edge_infinity * -1.0) * input_problem.size;


    /* Setup necessary function structures */
    if(arrow_btsp_fun_basic(ARROW_FALSE, &fun_basic) != ARROW_SUCCESS)
        return EXIT_FAILURE;
    if(!arrow_btsp_fun_shake_1(deep_copy, edge_infinity, shake_1_rand_min, shake_1_rand_max, &info, &fun_shake_1))
        return EXIT_FAILURE;
        
    #define SOLVE_STEPS 2
    arrow_btsp_solve_plan steps[SOLVE_STEPS] = 
    {
       {
           ARROW_TSP_CC_LK,                 /* TSP solver */
           (void *)&lk_params,              /* TSP solver parameters */
           fun_basic,                       /* fun (cost matrix function) */
           basic_attempts                   /* attempts */
       },
       {
           ARROW_TSP_CC_LK,                 /* TSP solver */
           (void *)&lk_params,              /* TSP solver parameters */
           fun_shake_1,                     /* fun (cost matrix function) */
           shake_1_attempts                 /* attempts */
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
    
    
    /* Setup BTSP results structure and solve BTSP */
    arrow_btsp_result_init(problem, &result);
    if(!arrow_btsp_solve(problem, &info, &btsp_params, &result))
    {
        arrow_print_error("Could not solve BTSP on file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    end_time = arrow_util_zeit() - start_time;
    
    
    /* If necessary, transform symmetric tour to equivalent asymmetric tour */
    if(result.found_tour && !input_problem.symmetric)
    {    
        int *actual_tour;
        if(!arrow_util_create_int_array(input_problem.size, &actual_tour))
        {
            arrow_print_error("Could not create actual_tour array.\n");
            ret = EXIT_FAILURE;
            goto CLEANUP;
        }
        arrow_util_sbtsp_to_abstp_tour(problem, result.tour, actual_tour);
        free(result.tour);
        result.tour = actual_tour;
        
        result.tour_length = 
            result.tour_length + input_problem.size * edge_infinity;
    }
    
    
    /* Perform sanity check on found tours, output them to file if requested */
    if(result.found_tour)
    {
        double actual_length = 0.0;
        
        for(i = 0; i < input_problem.size; i++)
        {
            u = result.tour[i];
            v = result.tour[(i + 1) % input_problem.size];
            cost = input_problem.get_cost(&input_problem, u, v);
            
            if(cost > result.obj_value)
            {
                fprintf(stderr, "Found tour is no good!\n");
                fprintf(stderr, "C[%d,%d] = %d\n", u, v, cost);
                ret = EXIT_FAILURE;
                goto CLEANUP;
            }
            else if(cost < 0)
            {
                fprintf(stderr, "Negative edge in tour -- is this okay?\n");
                fprintf(stderr, "C[%d,%d] = %d\n", u, v, cost);
            }

            actual_length += cost;
        }        
        
        if(result.tour_length != actual_length)
        {
            fprintf(stderr, "Returned tour length %.0f does not equal calculated length %.0f.\n",
                    result.tour_length, actual_length);
        }
    }
    
    
    /* Standard output */
    arrow_btsp_result_print_pretty(&result, stdout);
    printf("Initial Lower Bound: %d\n", lower_bound);
    if(lower_bound_time >= 0.0)
        printf("Lower Bound Time: %.2f\n", lower_bound_time);
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
    
    /* Tour file output */
    if(tour_file != NULL)
    {
        FILE *tour_out;
        char comment[256];
    
        if(!(tour_out = fopen(tour_file, "w")))
        {
            arrow_print_error("Could not open tour file for writing");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
    
        sprintf(comment, "BTSP Tour; Length %.0f, Max Cost %d.",
            result.tour_length, result.obj_value);
    
        arrow_util_write_tour(&input_problem, comment, result.tour, tour_out);
    
        fclose(tour_out);
    }
    
CLEANUP:
    arrow_btsp_result_destruct(&result);
    arrow_btsp_fun_destruct(&fun_basic);
    arrow_btsp_fun_destruct(&fun_shake_1);
    arrow_tsp_cc_lk_params_destruct(&lk_params);
    if(!input_problem.symmetric)
        arrow_problem_destruct(&asym_problem);
    arrow_problem_destruct(&input_problem);
    return ret;
}
