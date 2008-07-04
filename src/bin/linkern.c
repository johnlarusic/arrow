/**********************************************************doxygen*//** @file
 * @brief   Lin-Kernighan TSP heuristic.
 *
 * Runs the Lin-Kernighan TSP heuristic on the given input file.  This
 * is really nothing more than a wrapper to Concorde, and is for testing
 * purposes only.  Use Concorde's executable for access to solve options.
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

/* Program options */
char *program_name;             /**< Program name */
char *input_file;               /**< Given input TSPLIB file */
int random_restarts = -1;
int stall_count = -1;
int kicks = -1;

/****************************************************************************
 * Function implemenations
 ****************************************************************************/
int 
main(int argc, char *argv[])
{   
    /* Read program arguments */
    program_name = argv[0];
    read_args(argc, argv);
        
    /* Try and read the problem file and its info */
    arrow_problem problem;
    if(!arrow_problem_read(input_file, &problem))
    {
        arrow_print_error("Could not read input file.\n");
    }
    
    /* Setup Lk parameters structure */
    arrow_tsp_lk_params params;
    arrow_tsp_lk_params_init(&problem, &params);
    if(random_restarts >= 0)    params.random_restarts  = random_restarts;
    if(stall_count >= 0)        params.stall_count      = stall_count;
    if(kicks >= 0)              params.kicks            = kicks;
    
    /* Solve TSP */
    arrow_tsp_result result;
    if(!arrow_tsp_result_init(&problem, &result))
        arrow_print_error("Could not initialize result structure.\n");
    
    if(!arrow_tsp_lk_solve(&problem, &params, &result))
        arrow_print_error("Could not solve LK on file.\n");
    
    printf("\nFound Tour: %d\n", result.found_tour);
    printf("Tour length: %5.0f\n", result.obj_value);
    printf("Total Time: %5.2f\n", result.total_time);
    
    /* Free up the structures */
    arrow_tsp_lk_params_destruct(&params);
    arrow_tsp_result_destruct(&result);
    arrow_problem_destruct(&problem);
    
    return EXIT_SUCCESS;
}

void 
print_help()
{
    print_usage(program_name);
    printf("\n");
    printf("Runs the bottleneck biconnected spanning subgraph problem on\n");
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
    printf("%s (Arrow TSP) 1.0\n", program_name);
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
    char *short_options = "hVr:s:k:";
    struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {"restarts", required_argument, 0, 'r'},
        {"stalls", required_argument, 0, 's'},
        {"kicks", required_argument, 0, 'k'}
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
            case 'r':
                random_restarts = atoi(optarg);
                break;
            case 's':
                stall_count = atoi(optarg);
                break;
            case 'k':
                kicks = atoi(optarg);
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
