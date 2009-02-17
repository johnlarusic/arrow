/**********************************************************doxygen*//** @file
 * @brief   Useful utility functions.
 *
 * Useful utility functions that have general purpose throughout the library.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
inline int
arrow_util_create_int_array(int size, int **array)
{
    if((*array = malloc(size * sizeof(int))) == NULL)
    {
        arrow_print_error("Error allocating memory for int array.");
        *array = NULL;
        return ARROW_FAILURE;
    }
    return ARROW_SUCCESS;
}

inline int
arrow_util_create_int_matrix(int rows, int cols, int ***matrix, int **space)
{
    int i;
    
    if((*matrix = malloc(rows * sizeof(int *))) == NULL)
    {
        arrow_print_error("Could not allocate int * array for matrix.");
        *matrix = NULL;
        *space = NULL;
        return ARROW_FAILURE;
    }
 
    if(!arrow_util_create_int_array(rows * cols, space))
    {
        arrow_print_error("Could not allocate int array for matrix space.");
        free(*matrix);
        *matrix = NULL;
        return ARROW_FAILURE;
    }
 
    for(i = 0; i < rows; i++)
    {
        (*matrix)[i] = (*space) + (i * cols);
    }
    
    return ARROW_SUCCESS;
}

inline void
arrow_util_print_error(const char *file_name, int line_num,
                       const char *message)
{
    fprintf(stderr, "%s:%d: %s\n", file_name, line_num, message);
}

inline double
arrow_util_zeit()
{
    return CCutil_zeit();
}

void
arrow_util_redirect_stdout_to_file(const char *filename, int *old_stream)
{
    fpos_t pos;
    fgetpos(stdout, &pos);
    *old_stream = dup(fileno(stdout));
    freopen(filename, "w", stdout);
}

void
arrow_util_restore_stdout(int old_stream)
{
    fpos_t pos;
    fflush(stdout);
    dup2(old_stream, fileno(stdout));
    close(old_stream);
    clearerr(stdout);
    fsetpos(stdout, &pos); /* for C9X */
}

void
arrow_util_CCdatagroup_shallow_copy(CCdatagroup *from, CCdatagroup *to)
{
    to->edgelen        = from->edgelen;
    to->x              = from->x;
    to->y              = from->y;
    to->z              = from->z;
    to->adj            = from->adj;
    to->adjspace       = from->adjspace;
    to->len            = from->len;
    to->lenspace       = from->lenspace;
    to->degree         = from->degree;
    to->norm           = from->norm;
    to->dsjrand_param  = from->dsjrand_param;
    to->default_len    = from->default_len;
    to->sparse_ecount  = from->sparse_ecount;
    to->gridsize       = from->gridsize;
    to->dsjrand_factor = from->dsjrand_factor;
    to->rhdat          = from->rhdat;
    to->userdat        = from->userdat;
    to->ndepot         = from->ndepot;
    to->orig_ncount    = from->orig_ncount;
    to->depotcost      = from->depotcost;
    to->orig_names     = from->orig_names;
}

int
arrow_util_CCdatagroup_init_matrix(int size, CCdatagroup *dat)
{   
    CCutil_init_datagroup(dat);
    if(CCutil_dat_setnorm(dat, CC_MATRIXNORM))
    {
        arrow_print_error("Couldn't set norm to MATRIXNORM");
        return ARROW_FAILURE;
    }
    
    dat->adj = CC_SAFE_MALLOC(size, int *);
    dat->adjspace = CC_SAFE_MALLOC(size * (size + 1) / 2, int);
    
    if(dat->adj == (int **)NULL || dat->adjspace == (int *)NULL)
    {
        CCutil_freedatagroup(dat);
        arrow_print_error("Couldn't create adj/adjspace arrays");
        return ARROW_FAILURE;
    }
    
    int i, j;
    for(i = 0, j = 0; i < size; i++)
    {
        dat->adj[i] = dat->adjspace + j;
        j += (i + 1);
    }
    
    return ARROW_SUCCESS;
}

int
arrow_util_binary_search(int *array, int size, int element, int *pos)
{
    int low = 0;
    int high = size -1;
    int median;
    
    while(low != high)
    {
        median = ((high - low) / 2) + low; 
        if(array[median] == element)
        {
            *pos = median;
            return ARROW_SUCCESS;
        }
        else if(array[median] > element)
        {
            high = median;   
        }
        else
        {
            low = median + 1; 
        }
    }
    
    *pos = low;
    if(array[low] != element)
        return ARROW_FAILURE;
    else        
        return ARROW_SUCCESS;
}

int
arrow_util_regex_match(char *string, char *pattern)
{
    regex_t re;
    int match;
    int status;
 
    if((status = regcomp(&re, pattern, REG_EXTENDED)) != 0)
        return ARROW_FALSE;
    match = regexec(&re, string, 0, NULL, 0);
    regfree(&re);
    
    if(match == 0)
        return ARROW_TRUE;
    else
        return ARROW_FALSE;
}

void
arrow_util_print_program_args(int argc, char *argv[], FILE *out)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        if(i > 0) fprintf(out, " ");
        fprintf(out, "%s", argv[i]);
    }
}

void
arrow_util_random_seed(int seed)
{
    if(seed == 0)
        srand(time(0));
    else
        srand((unsigned int)seed);
}

inline int
arrow_util_random()
{
    return rand();
}

inline int
arrow_util_random_between(int min, int max)
{
    /*return (rand() % (max - min)) + min; */
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void
arrow_util_permute_array(int size, int *array)
{
    int i, c;
    for(i = 0; i < size; i++)
    {
        c = arrow_util_random_between(0, size - 1 - i);
        int t = array[i]; 
        array[i] = array[i + c];
        array[i + c] = t;
    }
}

void
arrow_util_write_tour(arrow_problem *problem, char *comment, int *tour, 
                      FILE *out)
{
    int i;
    
    fprintf(out, "NAME : %s\n", problem->name);
    fprintf(out, "TYPE : TOUR\n");    
    fprintf(out, "DIMENSION: %d\n", problem->size);
    
    if(comment != NULL)
        fprintf(out, "COMMENT : %s\n", comment);
    
    fprintf(out, "TOUR_SECTION\n");
    for(i = 0; i < problem->size; i++)
        fprintf(out, "%d\n", tour[i]);
    fprintf(out, "-1\n");
}

void
arrow_util_sbtsp_to_abstp_tour(arrow_problem *problem, int *old_tour,
                               int *new_tour)
{
    int i, u, v, cost;
    int n = problem->size / 2;
    int dir = problem->get_cost(problem, old_tour[0], old_tour[1]);
    
    for(i = 0; i < problem->size; i++)
    {
        u = old_tour[i];
        v = old_tour[(i + 1) % problem->size];
        cost = problem->get_cost(problem, u, v);
    }
    
    for(i = 0; i < n; i++)
    {
        if(dir >= 0)
            new_tour[i] = old_tour[2 * i];
        else
            new_tour[n - i - 1] = old_tour[2 * i];
    }
}
