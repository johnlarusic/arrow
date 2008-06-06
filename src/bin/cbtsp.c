/**********************************************************doxygen*//** @file
 * @brief   Constrained Bottleneck TSP heuristic.
 *
 * Runs the Constrained Bottleneck TSP heuristic on the given input file.  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "arrow.h"
#include <getopt.h>

/**
 *  @brief  Prints help/usage message.
 */
void 
print_help();

/**
 *  @brief  Prints version message.
 */
void 
print_version();

/**
 *  @brief  Prints usage message.
 */
void 
print_usage();

/**
 *  @brief  Reads program arguments.
 */
void 
read_args(int argc, char *argv[]);

/**
 *  @brief  Prints output in XML format.
 */
void 
print_xml_output(arrow_btsp_result *result, double max_length, 
                 int lower_bound, double lower_bound_time, double total_time,
                 int argc, char *argv[]);

/* Program options */
char *program_name;             /**< Program name */
char *input_file;               /**< Given input TSPLIB file */
int xml_output = ARROW_FALSE;   /**< Output output in XML format (or not) */

double length = DBL_MAX;

int random_restarts = -1;
int stall_count = -1;
int kicks = -1;

int confirm_sol = ARROW_FALSE;
int supress_ebst = ARROW_FALSE;
int find_short_tour = ARROW_FALSE;
int lower_bound = -1;
int upper_bound = INT_MAX;

int basic_attempts = ARROW_DEFAULT_BASIC_ATTEMPTS;

/****************************************************************************
 * Function implemenations
 ****************************************************************************/
int 
main(int argc, char *argv[])
{   
    int ret = EXIT_SUCCESS;
    int stdout_id;
    double start_time, end_time, bbssp_time;
    
    start_time = arrow_util_zeit();
    bbssp_time = -1.0;
    
    /* Read program arguments */
    program_name = argv[0];
    read_args(argc, argv);
    
    /* Supress output if required */
    if(xml_output) 
        arrow_util_redirect_stdout_to_file(ARROW_DEV_NULL, &stdout_id);
        
    /* Try and read the problem file and its info */
    arrow_problem problem;
    arrow_problem_info info;
    if(!arrow_problem_read(input_file, &problem))
    {
        arrow_print_error("Could not read input file.\n");
    }
    if(!arrow_problem_info_get(&problem, &info))
    {
        arrow_print_error("Could not read problem info.\n");
    }
    
    /* Setup LK parameters structure */
    arrow_tsp_lk_params lk_params;
    arrow_tsp_lk_params_init(&problem, &lk_params);
    if(random_restarts >= 0)    lk_params.random_restarts  = random_restarts;
    if(stall_count >= 0)        lk_params.stall_count      = stall_count;
    if(kicks >= 0)              lk_params.kicks            = kicks;
    lk_params.length_bound = length;
    
    /* Setup necessary function structures */
    arrow_btsp_fun fun_constrained;
    if(arrow_btsp_fun_constrained_shallow(length + 1, 
                                          &fun_constrained) != ARROW_SUCCESS)
    {
        return EXIT_FAILURE;
    }
    
    arrow_btsp_solve_plan steps[] = {
       {ARROW_BTSP_SOLVE_PLAN_CONSTRAINED, ARROW_FALSE, 
           fun_constrained, lk_params, length, ARROW_FALSE, basic_attempts}
    };
    
    /* Determine if we need to call the BBSSP to find a lower bound */
    if(lower_bound < 0)
    {
        printf("Solving BBSSP to find a lower bound... ");
        arrow_bbssp_result bbssp_result;
        if(!arrow_bbssp_solve(&problem, &info, &bbssp_result))
        {
            arrow_print_error("Could not solve BBSSP on file.\n");
            return EXIT_FAILURE;
        }
        lower_bound = bbssp_result.obj_value;
        bbssp_time = bbssp_result.total_time;
        printf("done!  BBSSP lower bound is %d.\n", lower_bound);
    }
    
    /* Setup BTSP parameters structure */
    arrow_btsp_params btsp_params;
    arrow_btsp_params_init(&btsp_params);
    btsp_params.confirm_sol         = confirm_sol;
    btsp_params.supress_ebst        = supress_ebst;
    btsp_params.find_short_tour     = find_short_tour;
    btsp_params.lower_bound         = lower_bound;
    btsp_params.upper_bound         = upper_bound;
    btsp_params.num_steps           = 1;
    btsp_params.steps               = steps;
    
    /* Setup BTSP results structure */
    arrow_btsp_result result;
    arrow_btsp_result_init(&problem, &result);
    
    ret = arrow_btsp_solve(&problem, &info, &btsp_params, &result);
    if(ret != ARROW_SUCCESS)
    {
        arrow_print_error("Could not solve BTSP on file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    
    end_time = arrow_util_zeit() - start_time;
    
    int k, u, v;
    int cost, max_cost = INT_MIN;
    double len = 0.0;
    if(result.found_tour)
    {
        for(k = 0; k < problem.size; k++)
        {
            u = result.tour[k];
            v = result.tour[(k + 1) % problem.size];
            cost = problem.get_cost(&problem, u, v);
            len += cost;
            if(cost > max_cost) max_cost = cost;
            if(cost > result.obj_value)
            {
                fprintf(stderr, "FUCK. %s\n", input_file);
                return EXIT_FAILURE;
            }
        }
        printf("\nCHECK Tour Length: %.0f\n", len);
        printf("CHECK Max Cost: %d\n\n", max_cost);
    }
    
    
    /* Output result */
    if(xml_output)
    {
        arrow_util_restore_stdout(stdout_id);
        print_xml_output(&result, length, lower_bound, bbssp_time, end_time, 
                         argc, argv);
    }
    else
    {
        printf("\nFound Tour: %s\n", (result.found_tour ? "Yes" : "No"));
        if(result.found_tour)
        {
            printf("Max. Cost: %d\n", result.obj_value);
            printf("Tour Length: %.0f\n", result.tour_length);
        }
        printf("Initial Lower Bound: %d\n", lower_bound);
        if(bbssp_time >= 0.0)
            printf("BBSSP Time: %.2f\n", bbssp_time);        
        printf("Optimal?: %s\n", 
            (result.optimal == ARROW_TRUE ? "Yes" : "???"));
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
    }
    
CLEANUP:
    arrow_btsp_result_destruct(&result);
    /*arrow_btsp_fun_destruct(&fun_constrained);*/
    arrow_btsp_params_destruct(&btsp_params);
    arrow_tsp_lk_params_destruct(&lk_params);
    arrow_problem_destruct(&problem);
    
    return ret;
}

void 
print_help()
{
    print_usage(program_name);
    printf("\n");
    printf("Runs the bottleneck traveling salesman problem on\n");
    printf("the given TSPLIB file.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help                print this help, then exit\n");
    printf("  -V, --version             print version number, then exit\n");
    printf("\n");
    printf("Report bugs to <johnlr@gmail.com>.\n");
}

void 
print_version()
{
    printf("%s (Arrow BTSP) 1.0\n", program_name);
    printf("(c) Copyright 2006-2008\n");
    printf("    John LaRusic, Eric Aubanel, and Abraham Punnen.\n");
    printf("\n");
    printf("This is free software.  You may redistribute copies of it\n");
    printf("under the terms of the GNU General Public License \n");
    printf("    <http://www.gnu.org/licenses/gpl.html>.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
    printf("\n");
    printf("Written by John LaRusic.\n");
}

void 
print_usage()
{
    printf("Usage: %s [options] tsplib_file\n", program_name);
}

void 
read_args(int argc, char *argv[])
{
    int opt_idx;
    char opt;
    char *short_options = "hVxL:r:s:k:ceSl:u:e:";
    struct option long_options[] =
    {
        {"help",                no_argument,        0, 'h'},
        {"version",             no_argument,        0, 'V'},
        {"xml",                 no_argument,        0, 'x'},
        
        {"length",              required_argument,  0, 'L'},
        
        {"restarts",            required_argument,  0, 'r'},
        {"stalls",              required_argument,  0, 's'},
        {"kicks",               required_argument,  0, 'k'},
        
        {"confirm-solution",    no_argument,        0, 'c'},
        {"supress-ebst",        no_argument,        0, 'e'},
        {"find-short-tour",     no_argument,        0, 'S'},
        {"lower-bound",         required_argument,  0, 'l'},
        {"upper-bound",         required_argument,  0, 'u'},
        
        {"basic-attempts",      required_argument,  0, '1'}
    };

    while(1)
    {
        opt = getopt_long(argc, argv, short_options, long_options, &opt_idx);
        if(opt == -1) break;
        
        switch (opt)
        {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'V':
                print_version(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'x':
                xml_output = ARROW_TRUE;
                break;
            
            case 'L':
                length = atof(optarg);
                break;
            
            case 'r':
                random_restarts = atoi(optarg);
                break;
            case 's':
                stall_count = atoi(optarg);
                break;
            case 'k':
                kicks = atoi(optarg);
                break;
            
            case 'c':
                confirm_sol = ARROW_TRUE;
                break;
            case 'e':
                supress_ebst = ARROW_TRUE;
                break;
            case 'S':
                find_short_tour = ARROW_TRUE;
                break;
            case 'l':
                lower_bound = atoi(optarg);
                break;
            case 'u':
                upper_bound = atoi(optarg);
                break;
                
            case '1':
                basic_attempts = atoi(optarg);
                break;
            default:
                printf("Option: %c\n", opt);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    /* Read the file name to process */
    if(optind == argc)
    {
        arrow_print_error("No problem files specified.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    else if(optind < argc - 1)
    {
        arrow_print_error("Multiple input files specified.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    input_file = argv[optind];    
}

void 
print_xml_output(arrow_btsp_result *result, double max_length, 
                 int lower_bound, double lower_bound_time, double total_time,
                 int argc, char *argv[])
{
    printf("<arrow_cbtsp problem_file=\"%s\" command_args=\"", input_file);
    int i;
    for(i = 0; i < argc; i++)
    {
        if(i > 0) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\">\n");
    printf("    <max_length>%.0f</max_length>\n", max_length);
    
    printf("    <found_tour>%s</found_tour>\n", 
           (result->found_tour ? "true" : "false"));
    printf("    <objective_value>%d</objective_value>\n", 
           (result->found_tour ? result->obj_value : -1));
    printf("    <tour_length>%.0f</tour_length>\n", 
           (result->found_tour ? result->tour_length : -1.0));
    printf("    <optimal>%s</optimal>\n", 
           (result->optimal ? "true" : "false"));
    printf("    <lower_bound>%d</lower_bound>\n", 
           lower_bound);
    printf("    <lower_bound_time>%.2f</lower_bound_time>\n",
           lower_bound_time);
    printf("    <binary_search_steps>%d</binary_search_steps>\n",
           result->bin_search_steps);
    printf("    <linkern_attempts>%d</linkern_attempts>\n",
           result->linkern_attempts);
    printf("    <linkern_avg_time>%.2f</linkern_avg_time>\n", 
           (result->linkern_attempts > 0 ? 
            result->linkern_time / (result->linkern_attempts * 1.0) : 0.0));
    printf("    <exact_tsp_attempts>%d</exact_tsp_attempts>\n",
           result->exact_attempts);
    printf("    <exact_tsp_avg_time>%.2f</exact_tsp_avg_time>\n", 
           (result->exact_attempts > 0 ? 
            result->exact_time / (result->exact_attempts * 1.0) : 0.0));
    printf("    <btsp_total_time>%.2f</btsp_total_time>\n",
           result->total_time);
    printf("    <total_time>%.2f</total_time>\n", total_time);
    printf("</arrow_cbtsp>\n");
}