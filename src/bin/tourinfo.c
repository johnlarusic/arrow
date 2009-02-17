/**********************************************************doxygen*//** @file
 * @brief   Tour information.
 *
 * Displays tour information for a given problem input and tour input  
 *
 * @author  John LaRusic
 * @ingroup bin
 ****************************************************************************/
#include "common.h"

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
print_xml_output(double length, int max_cost, int min_cost, 
                 int argc, char *argv[]);

/* Program options */
char *program_name;             /**< Program name */
char *problem_file;             /**< Given TSPLIB problem file */
char *tour_file;                /**< Given TSPLIB tour file */
int xml_output = ARROW_FALSE;   /**< Output output in XML format (or not) */

/****************************************************************************
 * Function implementations
 ****************************************************************************/
int 
main(int argc, char *argv[])
{   
    arrow_problem problem;
    int *tour = NULL;
    int ret = ARROW_SUCCESS;
    int stdout_id;
    
    /* Read program arguments */
    program_name = argv[0];
    read_args(argc, argv);
    
    /* Supress output if required */
    if(xml_output) 
        arrow_util_redirect_stdout_to_file(ARROW_DEV_NULL, &stdout_id);
    
    /* Read problem file */
    if(!arrow_problem_read(problem_file, &problem))
    {
        arrow_print_error("Could not read problem file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    
    /* Create tour array */
    if(!arrow_util_create_int_array(problem.size, &tour))
    {
        arrow_print_error("Could not create tour array.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    
    /* Read tour file */
    if(!arrow_problem_read_tour(tour_file, problem.size, tour))
    {
        arrow_print_error("Could not read tour file.\n");
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }
    
    int cost;
    int max_cost = INT_MIN;
    int min_cost = INT_MAX;
    double length = 0.0;
    int i, u, v;
    
    for(i = 0; i < problem.size; i++)
    {
        u = tour[i];
        v = tour[(i + 1) % problem.size];
        cost = problem.get_cost(&problem, u, v);
        
        length += cost;
        if(cost > max_cost) max_cost = cost;
        if(cost < min_cost) min_cost = cost;   
    }
    
    if(xml_output)
    {
        arrow_util_restore_stdout(stdout_id);
        print_xml_output(length, max_cost, min_cost, argc, argv);
    }
    else
    {
        printf("Tour Length: %.0f\n", length);
        printf("Max Cost:    %d\n", max_cost);
        printf("Min Cost:    %d\n", min_cost);
    }
    
CLEANUP:
    arrow_problem_destruct(&problem);
    if(tour != NULL) free(tour);
    
    return ret;
}

void 
print_help()
{
    print_usage(program_name);
    printf("\n");
    printf("Displays basic information about a tour.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help                print this help, then exit\n");
    printf("  -V, --version             print version number, then exit\n");
    printf("  -x, --xml                 output in xml format\n");
    printf("\n");
    printf("Report bugs to <johnlr@gmail.com>.\n");
}

void 
print_version()
{
    printf("%s (Arrow Tour Information) 1.0\n", program_name);
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
    printf("Usage: %s [options] tsplib_problem tsplib_tour\n", program_name);
}

void 
read_args(int argc, char *argv[])
{
    int opt_idx;
    char opt;
    char *short_options = "hVxp:t:";
    struct option long_options[] =
    {
        {"help",                no_argument,        0, 'h'},
        {"version",             no_argument,        0, 'V'},
        {"xml",                 no_argument,        0, 'x'},
        {"problem",             required_argument,  0, 'p'},
        {"tour",                required_argument,  0, 't'}
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
                
            case 'p':
                problem_file = optarg;
                break;
            case 't':
                tour_file = optarg;
                break;
                
            default:
                printf("Option: %c\n", opt);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

void 
print_xml_output(double length, int max_cost, int min_cost, 
                 int argc, char *argv[])
{
    printf("<arrow_tour_info problem_file=\"%s\" tour_file=\"%s\" command_args=\"", 
           problem_file, tour_file);
    int i;
    for(i = 0; i < argc; i++)
    {
        if(i > 0) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\">\n");
    printf("    <length>%.0f</length>\n", length);
    printf("    <max_cost>%d</max_cost>\n", max_cost);
    printf("    <min_cost>%d</min_cost>\n", min_cost);
    printf("</<arrow_tour_info>\n");
}
