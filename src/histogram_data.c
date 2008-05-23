/**********************************************************doxygen*//** @file
 * @brief   Edge length histogram data collector.
 *
 * Prints out a list of every edge length present in given problem.  Used in
 * conjunction with a Python script for generating a histogram plot.
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

/****************************************************************************
 * Function implemenations
 ****************************************************************************/
int 
main(int argc, char *argv[])
{   
    int stdout_id;
    
    /* Read program arguments */
    program_name = argv[0];
    read_args(argc, argv);
    
    /* Need to supress output from Concorde */
    arrow_util_redirect_stdout_to_file(ARROW_DEV_NULL, &stdout_id);
    
    /* Try and read the problem file and its info */
    arrow_problem problem;
    if(!arrow_problem_read(input_file, &problem))
    {
        arrow_print_error("Could not read input file.\n");
    }
    
    /* Restore output */
    arrow_util_restore_stdout(stdout_id);
    
    int i, j;
    for(i = 0; i < problem.size; i++)
    {
        for(j = i + 1; j < problem.size; j++)
        {
            printf("%d\n", problem.get_cost(&problem, i, j));
        }
    }
    
    /* Free up the structure */
    arrow_problem_destruct(&problem);
    
    return EXIT_SUCCESS;
}

void 
print_help()
{
    print_usage(program_name);
    printf("\n");
    printf("Compiles a list of the number of times a given edge length\n");
    printf("appears in the data set.  Used in conjunction with a Python\n");
    printf("script for generating a histogram plot.\n");
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
    printf("%s (Arrow histogram) 1.0\n", program_name);
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
    char *short_options = "hVx";
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
        arrow_print_error("No problem file specified.\n");
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


