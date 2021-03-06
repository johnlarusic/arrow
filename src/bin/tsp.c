/**********************************************************doxygen*//** @file
 * @brief   Traveling Salesman Problem solver.
 *
 * Solves the traveling salesman problem (TSP) on the given input file.  This
 * is really nothing more than a wrapper to Concorde, and is for testing
 * purposes only.  Use Concorde's executable for access to solve options.
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"
#include "lb.h"
#include "tsp.h"
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

/****************************************************************************
 * Function implementations
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
    
    /* Solve TSP */
    arrow_tsp_result result;
    if(!arrow_tsp_result_init(&problem, &result))
        arrow_print_error("Could not initialize result structure.\n");
    
    if(!arrow_tsp_cc_exact_solve(&problem, NULL, &result))
        arrow_print_error("Could not solve TSP on file.\n");
    
    printf("\nFound Tour: %d\n", result.found_tour);
    printf("Tour length: %5.0f\n", result.obj_value);
    printf("Total Time: %5.2f\n", result.total_time);
    
    /* Free up the structures */
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
    char *short_options = "hV";
    struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'}
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
