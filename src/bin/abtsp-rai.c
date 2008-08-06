/**********************************************************doxygen*//** @file
 * @brief   Asymmetric Bottleneck TSP heuristic using RAI heuristic.
 *
 * Runs the Bottleneck TSP heuristic on the given asymmetric input file.  
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
int iterations = -1;
int supress_ebst = ARROW_FALSE;
int solve_btsp = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;
int basic_attempts = 1;

/* Program options */
#define NUM_OPTS 9
arrow_option options[NUM_OPTS] = 
{
    {'i', "input", "TSPLIB input file", 
        ARROW_OPTION_STRING, &input_file, ARROW_TRUE, ARROW_TRUE},
    {'x', "xml", "file to write XML output to",
        ARROW_OPTION_STRING, &xml_file, ARROW_FALSE, ARROW_TRUE},
    {'T', "tour", "file to write tour to",
        ARROW_OPTION_STRING, &tour_file, ARROW_FALSE, ARROW_TRUE},
    
    {'r', "iterations", "number of RAI iterations",
        ARROW_OPTION_INT, &iterations, ARROW_FALSE, ARROW_TRUE},
    {'b', "solve-btsp", "use BTSP formulation in RAI algorithm",
        ARROW_OPTION_INT, &solve_btsp, ARROW_FALSE, ARROW_FALSE},
        
    {'e', "supress-ebst", "supress binary search",
        ARROW_OPTION_INT, &supress_ebst, ARROW_FALSE, ARROW_FALSE},
        
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
    int i, u, v, cost, costp;

    arrow_problem problem;
    arrow_problem_info info;
    arrow_btsp_fun fun_basic;
    arrow_btsp_result result;
    arrow_tsp_rai_params rai_params;
    arrow_btsp_params btsp_params;
    
    double start_time = arrow_util_zeit();
    double end_time;
    double bbssp_time = -1.0;
    
    arrow_util_random_seed(0);
    
    /* Read program arguments */
    if(!arrow_options_parse(NUM_OPTS, options, desc, usage, argc, argv, NULL))
        return EXIT_FAILURE;
        
    /* Try and read the problem file */
    if(!arrow_problem_read(input_file, &problem))
        return EXIT_FAILURE;
    if(problem.symmetric)
    {
        arrow_print_error("Solver only works on asymmetric matrices.");
        return EXIT_FAILURE;
    }
    
    /* Gather basic info about the problem */
    if(!arrow_problem_info_get(&problem, &info))
        return EXIT_FAILURE;
    printf("Num costs in problem: %d\n", info.cost_list_length);
    printf("Max cost in problem:  %d\n", info.max_cost);
    
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
    
    /* Set up RAI parameters */
    if(iterations < 0) iterations = problem.size * problem.size;
    rai_params.iterations = iterations;
    rai_params.solve_btsp = solve_btsp;
        
    /* Setup necessary function structures */
    if(arrow_btsp_fun_basic(ARROW_TRUE, &fun_basic) != ARROW_SUCCESS)
        return EXIT_FAILURE;
    #define SOLVE_STEPS 1
    arrow_btsp_solve_plan steps[SOLVE_STEPS] = 
    {
       {
           ARROW_TSP_RAI,                   /* TSP solver */
           (void *)&rai_params,             /* TSP solver parameters */
           fun_basic,                       /* fun (cost matrix function) */
           basic_attempts                   /* attempts */
       }
    };
    
    /* Setup BTSP parameters structure */
    arrow_btsp_params_init(&btsp_params);
    btsp_params.confirm_sol         = 0;
    btsp_params.supress_ebst        = supress_ebst;
    btsp_params.find_short_tour     = 0;
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
    
    if(result.found_tour)
    {
        /* Sanity check */
        for(i = 0; i < problem.size; i++)
        {
            u = result.tour[i];
            v = result.tour[(i + 1) % problem.size];
            cost = problem.get_cost(&problem, u, v);
            
            if(cost > result.obj_value)
            {
                arrow_print_error("Found tour is no good! WTF?\n");
                fprintf(stderr, "C[%d,%d] = %d, ", u, v, cost);
                ret = EXIT_FAILURE;
                goto CLEANUP;
            }
            else if(cost < 0)
            {
                arrow_print_error("Negative edge in tour! WTF?\n");
                fprintf(stderr, "C[%d,%d] = %d, ", u, v, cost);
                ret = EXIT_FAILURE;
                goto CLEANUP;
            }
            
            result.tour_length += cost;
        }
        printf("Tour passes sanity check\n");
        
        /* Tour output */
        if(tour_file != NULL)
        {
            FILE *tour;
            char comment[256];
        
            if(!(tour = fopen(tour_file, "w")))
            {
                arrow_print_error("Could not open tour file for writing");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
        
            sprintf(comment, "ABTSP Tour; Length %.0f, Max Cost %d.",
                result.tour_length, result.obj_value);
        
            arrow_util_write_tour(&problem, comment, result.tour, tour);
        
            fclose(tour);
        }
    }
    
    /* Final output */
    arrow_btsp_result_print_pretty(&result, stdout);
        printf("Initial Lower Bound: %d\n", lower_bound);       
    
    printf("Total Time: %.2f\n", end_time);
    
    /* Xml output */
    if(xml_file != NULL)
    {
        FILE *xml;
        if(!(xml = fopen(xml_file, "w")))
        {
            arrow_print_error("Could not open xml file for writing");
            ret = EXIT_FAILURE;
            goto CLEANUP;
        }
        
        fprintf(xml, "<arrow_btsp problem_file=\"%s\"", input_file);
        fprintf(xml, " command_args=\"");
        arrow_util_print_program_args(argc, argv, xml);
        fprintf(xml, "\">\n");

        arrow_btsp_result_print_xml(&result, xml);
        
        fprintf(xml, "<total_time>%.5f</total_time>\n", end_time);
        fprintf(xml, "</arrow_btsp>\n");
        
        fclose(xml);
    }
    
CLEANUP:
    arrow_btsp_result_destruct(&result);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_btsp_fun_destruct(&fun_basic);
    arrow_btsp_params_destruct(&btsp_params);
    arrow_problem_destruct(&problem); 
    return ret;
}
