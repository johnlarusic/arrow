/**********************************************************doxygen*//** @file
 * @brief   Useful utility functions.
 *
 * Useful utility functions that have general purpose throughout the library.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
inline int
arrow_util_create_int_array(int size, int **array)
{
    if((*array = malloc(size * sizeof(int))) == NULL)
    {
        arrow_print_error("Error allocating memory for int array.");
        *array = NULL;
        return ARROW_ERROR_FATAL;
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
        return ARROW_ERROR_FATAL;
    else        
        return ARROW_SUCCESS;
}