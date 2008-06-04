/**********************************************************doxygen*//** @file
 * @brief   Bottleneck Biconnected Spanning Subgraph solver.
 *
 * Solves the bottleneck biconnected spanning subgraph problem (BBSSP) problem
 * on the given input file.
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
print_xml_output(int obj_value, double total_time, int argc, char *argv[]);

/* Program options */
char *program_name;             /**< Program name */
char *input_file;               /**< Given input TSPLIB file */
int xml_output = ARROW_FALSE;   /**< Output output in XML format (or not) */

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
    
    /* Solve BBSSP */
    arrow_bbssp_result result;
    if(!arrow_bbssp_solve(&problem, &info, &result))
    {
        arrow_print_error("Could not solve BBSSP on file.\n");
    }
    printf("\nBBSSP Solution: %d\n", result.obj_value);
    printf("Total Time: %5.2f\n", result.total_time);
    
    /* Output result */
    if(xml_output)
    {
        arrow_util_restore_stdout(stdout_id);
        print_xml_output(result.obj_value, result.total_time, argc, argv);
    }
    
    /* Free up the structure */
    arrow_problem_destruct(&problem);
    arrow_problem_info_destruct(&info);
    
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
    printf("  -x, --xml                 outputs result in xml format\n");
    printf("\n");
    printf("Report bugs to <johnlr@gmail.com>.\n");
}

void 
print_version()
{
    printf("%s (Arrow BBSSP) 1.0\n", program_name);
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
        {"version", no_argument, 0, 'V'},
        {"xml", no_argument, 0, 'x'}
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
print_xml_output(int obj_value, double total_time, int argc, char *argv[])
{
    printf("<arrow_bbssp problem_file=\"%s\" command_args=\"", input_file);
    int i;
    for(i = 0; i < argc; i++)
    {
        if(i > 0) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\">\n");
    printf("    <objective_value>%d</objective_value>\n", obj_value);
    printf("    <total_time>%5.2f</total_time>\n", total_time);
    printf("</<arrow_bbssp>\n");
}