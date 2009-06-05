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
#include "baltsp.h"

/* Global variables */
char *input_file = NULL;
char *xml_file = NULL;
char *tour_file = NULL;
int edge_infinity = -1;
int random_restarts = -1;
int stall_count = -1;
int kicks = -1;
int supress_hash = ARROW_FALSE;
int deep_copy = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;
int basic_attempts = 3;
int shake_1_attempts = 1;
int shake_1_rand_min = 0;
int shake_1_rand_max = -1;
int random_seed = 0;
int lb_only;

/* Program options */
#define NUM_OPTS 17
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
    {'H', "supress-hash", "do not create hash table",
        ARROW_OPTION_INT, &supress_hash, ARROW_FALSE, ARROW_FALSE},
    {'d', "deep-copy", "stores data in full cost-matrix",
        ARROW_OPTION_INT, &deep_copy, ARROW_FALSE, ARROW_FALSE},
    {'I', "infinity", "value to use as infinity",
        ARROW_OPTION_INT, &edge_infinity, ARROW_FALSE, ARROW_TRUE},
        
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
        ARROW_OPTION_INT, &random_seed, ARROW_FALSE, ARROW_TRUE},
        
    {'L', "lb-only", "only check lower bound gap",
        ARROW_OPTION_INT, &lb_only, ARROW_FALSE, ARROW_FALSE}
};
char *desc = "Balanced traveling salesman problem (BalTSP) solver";
char *usage = "-i tsplib.tsp [options]";

/* Main method */
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int i, u, v, cost;

    arrow_problem problem;
    arrow_problem_info info;
    arrow_tsp_cc_lk_params lk_params;
    arrow_btsp_fun fun_basic;
    arrow_btsp_fun fun_shake_1;
    arrow_bound_result lb_result;
    arrow_btsp_result tour_result;
    arrow_btsp_params btsp_params;
    
    double start_time = arrow_util_zeit();
    double end_time;
    double avg_time;
    
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
    
    
    /* Try and read the problem file. */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(&problem, !supress_hash, &info))
        return EXIT_FAILURE;
    printf("Num costs in problem: %d\n", info.cost_list_length);
    printf("Max cost in problem:  %d\n", info.max_cost);
    
    /* Extra processing for arguments */
    if(shake_1_rand_max < 0) 
        shake_1_rand_max = (problem.size * problem.size) + shake_1_rand_min;
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
        
    /* Initialize random number generator */
    arrow_util_random_seed(random_seed);

    if(lower_bound < 0)
        lower_bound = info.min_cost;
    if(upper_bound < lower_bound)
        upper_bound = info.max_cost;
    
    /* Setup LK parameters structure */
    arrow_tsp_cc_lk_params_init(&problem, &lk_params);
    if(random_restarts >= 0)    lk_params.random_restarts  = random_restarts;
    if(stall_count >= 0)        lk_params.stall_count      = stall_count;
    if(kicks >= 0)              lk_params.kicks            = kicks;
    
    if(!problem.symmetric)
        lk_params.length_bound = (edge_infinity * -1.0) * problem.size;

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
        
    /* Setup BTSP parameters structure */
    arrow_btsp_params_init(&btsp_params);
    btsp_params.confirm_sol         = ARROW_FALSE;
    btsp_params.supress_ebst        = ARROW_FALSE;
    btsp_params.find_short_tour     = ARROW_FALSE;
    btsp_params.lower_bound         = lower_bound;
    btsp_params.upper_bound         = upper_bound;
    btsp_params.num_steps           = SOLVE_STEPS;
    btsp_params.steps               = steps;
    btsp_params.infinity            = edge_infinity;
    btsp_params.deep_copy           = deep_copy;
    
    
    /* Setup BTSP results structure and solve BTSP */
    arrow_btsp_result_init(&problem, &tour_result);
    if(!arrow_balanced_tsp_solve(&problem, &info, &btsp_params, lb_only,
                                 &lb_result, &tour_result))
    {
        arrow_print_error("Could not solve BTSP on given problem.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    end_time = arrow_util_zeit() - start_time;
    
    
    /* Perform sanity check on found tours, output them to file if requested */
    if(tour_result.found_tour)
    {        
        for(i = 0; i < problem.size; i++)
        {
            u = tour_result.tour[i];
            v = tour_result.tour[(i + 1) % problem.size];
            cost = problem.get_cost(&problem, u, v);

            if((cost < tour_result.min_cost) || (cost > tour_result.max_cost))
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
        }   
    }
    
    /* Standard output */
    printf("Lower Bound: %d\n", lb_result.obj_value);
    printf("Lower Bound Time: %.2f\n", lb_result.total_time);      
    
    printf("Found Tour: %s\n", (tour_result.found_tour ? "Yes" : "No"));
    if(tour_result.found_tour)
    {
        printf("Obj. Value:  %d\n", tour_result.max_cost - tour_result.min_cost);
        printf("Min. Cost:   %d\n", tour_result.min_cost);
        printf("Max. Cost:   %d\n", tour_result.max_cost);
        printf("Tour Length: %.0f\n", tour_result.tour_length);
    }
    printf("Search steps: %d\n", tour_result.bin_search_steps);
    
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        if(tour_result.solver_attempts[i] > 0)
        {
            avg_time = tour_result.solver_time[i] / 
                (tour_result.solver_attempts[i] * 1.0);
            
            printf(" - ");
            arrow_tsp_long_name(i, stdout);
            printf("\n");
            printf("   - Calls: %d\n", tour_result.solver_attempts[i]);
            printf("   - Avg Time: %.2f\n", avg_time);
        }
    }
    
    printf("Total Tour Solve Time: %.2f\n", tour_result.total_time);
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
        
        fprintf(xml, "<arrow_baltsp ");
            
        fprintf(xml, "problem_file=\"%s\"", input_file);
        fprintf(xml, " command_args=\"");
        arrow_util_print_program_args(argc, argv, xml);
        fprintf(xml, "\">\n");
    
        arrow_xml_element_int("lower_bound", lb_result.obj_value, xml);
        arrow_xml_element_double("lower_bound_time", lb_result.total_time, xml);
    
        arrow_xml_element_bool("found_tour", tour_result.found_tour, xml);
        if(tour_result.found_tour)
        {
            arrow_xml_element_int("objective_value", tour_result.max_cost - tour_result.min_cost, xml);
            arrow_xml_element_int("tour_min_cost", tour_result.min_cost, xml);
            arrow_xml_element_int("tour_max_cost", tour_result.max_cost, xml);
            arrow_xml_element_double("tour_length", tour_result.tour_length, xml);
        }
        else
        {
            arrow_xml_element_int("objective_value", -1, xml);
            arrow_xml_element_int("tour_min_cost", -1, xml);
            arrow_xml_element_int("tour_max_cost", -1, xml);
            arrow_xml_element_double("tour_length", -1.0, xml);
        }

        arrow_xml_element_int("search_steps", tour_result.bin_search_steps, xml);

        arrow_xml_element_open("solver_info", xml);
        for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
        {
            if(tour_result.solver_attempts[i] > 0)
            {
                arrow_xml_element_start("solver", xml);
            
                arrow_xml_attribute_int("solver_type", i, xml);
                arrow_xml_attribute_start("solver_name", xml);
                arrow_tsp_short_name(i, xml);
                arrow_xml_attribute_end(xml);
                arrow_xml_element_end(xml);
            
                avg_time = tour_result.solver_time[i] / (tour_result.solver_attempts[i] * 1.0);
                arrow_xml_element_int("attempts", tour_result.solver_attempts[i], xml);
                arrow_xml_element_double("avg_time", avg_time, xml);            
            
                arrow_xml_element_close("solver", xml);
            }
        }
        arrow_xml_element_close("solver_info", xml);
    
        arrow_xml_element_double("tour_total_time", tour_result.total_time, xml);
        arrow_xml_element_double("total_time", end_time, xml);
        
        fprintf(xml, "</arrow_baltsp>\n");
        
        fclose(xml);
    }
    
    
    /* Tour file output */
    if((tour_file != NULL) && tour_result.found_tour)
    {
        FILE *tour_out;
        char comment[256];
    
        if(!(tour_out = fopen(tour_file, "w")))
        {
            arrow_print_error("Could not open tour file for writing");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
    
        sprintf(comment, "Balanced TSP Tour; Length %.0f, Min Cost %d, Max Cost %d.",
                tour_result.tour_length, tour_result.min_cost, tour_result.max_cost);
        
        arrow_util_write_tour(&problem, comment, tour_result.tour, tour_out);
    
        fclose(tour_out);
    }
    
    
    
CLEANUP:
    arrow_btsp_result_destruct(&tour_result);
    arrow_btsp_fun_destruct(&fun_basic);
    arrow_btsp_fun_destruct(&fun_shake_1);
    arrow_tsp_cc_lk_params_destruct(&lk_params);
    arrow_problem_destruct(&problem);
    return ret;
}