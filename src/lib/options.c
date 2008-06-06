/**********************************************************doxygen*//** @file
 *  @brief   Helper for parsing program options
 *
 *  For helping parse all those pesky program options!
 *
 *  @author  John LaRusic
 *  @ingroup lib
 ****************************************************************************/
#include "arrow.h"

void
print_usage(char *program_name, char *usage);

void
print_version(char *program_name);

void
print_help(int num_opts, arrow_option options[], char *description);

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_options_parse(int num_opts, arrow_option options[], char *description, 
                    char *usage, int argc, char *argv[], int *opt_ind)
{
    int i, ret, idx = 0;

    char *short_options;
    struct option *long_options;
    int *given;
    
    if(argc == 1)
    {
        print_usage(argv[0], usage);
        return ARROW_FAILURE;
    }
    
    /* Allocate memory to required structures */
    if((short_options = malloc((num_opts * 2 + 2) * sizeof(char))) == NULL)
    {
        arrow_print_error("Error allocating memory for short_options");
        return ARROW_FAILURE;
    }
    if((long_options = malloc((num_opts + 2) * sizeof(struct option))) == NULL)
    {
        arrow_print_error("Error allocating memory for long_options");
        return ARROW_FAILURE;
    }
    if((given = malloc(num_opts * sizeof(int))) == NULL)
    {
        arrow_print_error("Error allocating memory for given");
        return ARROW_FAILURE;
    }
    
    /* Setup structures */
    for(i = 0; i < num_opts; i++)
    {
        if((options[i].short_option == 'h') ||
           (options[i].short_option == 'V'))
        {
            arrow_print_error("h/V option specified.\n");
            return ARROW_FAILURE;
        }
        
        short_options[idx++] = options[i].short_option;
        long_options[i].name = options[i].long_option;
        long_options[i].flag = 0;
        long_options[i].val = options[i].short_option;
        
        if(options[i].arg_required)
        {
            short_options[idx++] = ':';
            long_options[i].has_arg = required_argument;
        }
        else
        {
            if(options[i].data_type != ARROW_OPTION_INT)
            {
                arrow_print_error("Options with no argument must be ints");
                ret = ARROW_FAILURE;
                goto CLEANUP;
            }
            long_options[i].has_arg = no_argument;
        }
        
        given[i] = ARROW_FALSE;
    }
    short_options[idx++] = 'h';
    long_options[num_opts].name = "help";
    long_options[num_opts].flag = 0;
    long_options[num_opts].val = 'h';
        
    short_options[idx++] = 'V';
    long_options[num_opts + 1].name = "version";
    long_options[num_opts + 1].flag = 0;
    long_options[num_opts + 1].val = 'V';
    
    short_options[idx++] = '\0';
    
    /* Process options with getopt_long */
    int opt; int opt_idx = 0; int found = ARROW_FALSE;
    while(1)
    {
        opt = getopt_long(argc, argv, short_options, long_options, &opt_idx);
        if(opt == -1) break;
            
        if(opt == 'h')
        {
            print_usage(argv[0], usage);
            print_help(num_opts, options, description);
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        else if(opt == 'V')
        {
            print_version(argv[0]);
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
        
        for(i = 0; i < num_opts; i++)
        {
            if(options[i].short_option == opt)
            {
                if(options[i].arg_required)
                {
                    switch(options[i].data_type)
                    {
                        case ARROW_OPTION_INT:
                            *((int *)(options[i].data_ptr)) = atoi(optarg);
                            break;
                        case ARROW_OPTION_DOUBLE:
                            *((double *)(options[i].data_ptr)) = atof(optarg);
                            break;
                        case ARROW_OPTION_STRING:
                            *((char **)(options[i].data_ptr)) = optarg;
                            break;
                        default:
                            arrow_print_error("Unknown data type");
                            ret = ARROW_FAILURE;
                            goto CLEANUP;
                    }   
                }
                else
                {
                    *((int *)options[i].data_ptr) = ARROW_TRUE;
                }
                given[i] = ARROW_TRUE;
            }
            found = ARROW_TRUE;
        }
        
        if(!found)
        {
            fprintf(stderr, "Unknown argument: '%c'\n");
            ret = ARROW_FAILURE;
            goto CLEANUP;
        }
    }
    
    /* Check for required arguments */
    for(i = 0; i < num_opts; i++)
    {
        if(options[i].opt_required && !given[i])
        {
            fprintf(stderr, "Missing argument: '-%c'/'--%s'\n", 
                    options[i].short_option, options[i].long_option);
            ret = ARROW_FAILURE;
            goto CLEANUP;    
        }
    }
    
    if(opt_ind != NULL)
        *opt_ind = optind;
    
    ret = ARROW_SUCCESS;
CLEANUP:    
    free(short_options);
    free(long_options);
    
    return ret;
}

void
print_usage(char *program_name, char *usage)
{
    printf("Usage: %s %s\n", program_name, usage);
}

void
print_version(char *program_name)
{
    printf("%s (Arrow %s)\n", program_name, ARROW_VERSION);
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
print_help(int num_opts, arrow_option options[], char *description)
{
    int i;
    char *format;
    printf("%s\n", description);
    printf("\n");
    printf("Options:\n");
    for(i = 0; i < num_opts; i++)
    {
        if(options[i].arg_required)
            format = "  -%c [arg], --%s=[arg]:\n\t%s\n";
        else
            format = "  -%c, --%s:\n\t%s\n";
        printf(format, options[i].short_option, options[i].long_option,
               options[i].help_message);
    }
    printf("\n");
    printf("Report bugs to <johnlr@gmail.com>.\n");
}