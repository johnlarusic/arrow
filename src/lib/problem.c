/**********************************************************doxygen*//** @file
 * @brief   Functions for working with problem data.
 *
 * Function implemenations for working with problem data, generally 
 * manipulating the arrow_problem data structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Determines if the given file is a symmetric TSPLIB file.
 *  @param  file_name [in] the path to the TSPLIB file
 *  @return ARROW_TRUE if the file is for symmetric data, ARROW_FALSE if not
 */
int
is_symmetric(char *file_name);

/**
 *  @brief  Determines if the given file is an asymmetric TSPLIB file.
 *  @param  file_name [in] the path to the TSPLIB file
 *  @return ARROW_TRUE if the file is for asymmetric data, ARROW_FALSE if not
 */
int
is_asymmetric(char *file_name);

/**
 *  @brief  Reads an asymmetric TSPLIB file (*.atsp).
 *  @param  file_name [in] the path to the TSPLIB file
 *  @param  problem [out] problem structure
 */
int
read_atsp(char *file_name, arrow_problem *problem);

/**
 *  @brief  Edge length function for full matrix data
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
fullmatrix_edgelen(int i, int j, CCdatagroup *dat);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_problem_read(char *file_name, arrow_problem *problem)
{    
    if(is_symmetric(file_name) == ARROW_TRUE)
    {    
        arrow_debug("Reading symmetric TSPLIB file...\n");
        int size;
        if(CCutil_gettsplib(file_name, &size, &(problem->data)) != 0)
        {
            arrow_print_error("Unable to read TSPLIB file using Concorde\n");
            return ARROW_FAILURE;
        }
        problem->size = size;
        problem->symmetric = ARROW_TRUE;
    }
    else if(is_asymmetric(file_name) == ARROW_TRUE)
    {
        arrow_debug("Reading asymmetric TSPLIB file...\n");
        if(read_atsp(file_name, problem) != ARROW_SUCCESS)
        {
            arrow_print_error("Unable to read ATSP file");
            return ARROW_FAILURE;
        }
        problem->symmetric = ARROW_FALSE;
    }
    else
    {
        arrow_print_error("Input file type not supported.");
        return ARROW_FAILURE;
    }
    
    /* Strip out the problem file name from the path */
    char *s = strrchr(file_name, '/');
    if(s != NULL)
        sprintf(problem->name, "%s", s + 1);
    else
        sprintf(problem->name, "%s", file_name);
    
    problem->shallow = ARROW_FALSE;
    problem->get_cost = arrow_problem_get_cost;
    
    return ARROW_SUCCESS;
}

void
arrow_problem_destruct(arrow_problem *problem)
{
    if(!problem->shallow)
    {
        if(&(problem->data) != NULL)
        {
            CCutil_freedatagroup(&(problem->data));
        }
    }
    else
    {
        if(problem->data.userdat.data != NULL)
            free(problem->data.userdat.data);
    }
}

int
arrow_problem_info_get(arrow_problem *problem, arrow_problem_info *info)
{
    arrow_bintree tree;
    int ret;
    int i, j;
    int cost, min_cost, max_cost;
    
    ret = ARROW_SUCCESS;
    arrow_bintree_init(&tree);
    min_cost = INT_MAX;
    max_cost = INT_MIN;
        
    for(i = 0; i < problem->size; i++)
    {
        j = (problem->symmetric == ARROW_TRUE ? i + 1 : 0);
        for(; j < problem->size; j++)
        {
            if(i != j)
            {
                /* Add to binary tree */
                cost = problem->get_cost(problem, i, j);
                ret = arrow_bintree_insert(&tree, cost);
                if(ret != ARROW_SUCCESS) goto CLEANUP;
            
                /* Determine min and max costs */
                if(cost < min_cost) min_cost = cost;
                if(cost > max_cost) max_cost = cost;
            }
        }
    }
    
    /* Convert tree to array */
    arrow_bintree_to_array(&tree, &(info->cost_list));
    info->cost_list_length = tree.size;
    info->min_cost = min_cost;
    info->max_cost = max_cost;
    
CLEANUP:
    arrow_bintree_destruct(&tree);
    return ret;
}

void
arrow_problem_info_destruct(arrow_problem_info *info)
{
    if(info->cost_list != NULL)
        free(info->cost_list);
}

void
arrow_problem_print(arrow_problem *problem)
{
    int i, j, k, count;
    int group_size = 8;
    
    printf("Problem Cost Matrix:\n");
    printf("----------------------------------------");
    printf("--------------------------------------\n");
    
    /* Print cost matrix out into groups so they will (normally) fit into
     * an 80-character width terminal. */
    k = 0;
    
    arrow_debug("Problem Size: %d\n", problem->size);
    while(k < problem->size)
    {
        /* Handle special case for last column */
        if(problem->size - k < group_size)
            count = problem->size - k;
        else
            count = group_size;
        
        /* Print out the header */
        for(j = k; j < k + count; j++)
            printf("\t[j=%d]", j);   
        printf("\n");

        /* Print out rows */
        for(i = 0; i < problem->size; i++)
        {
            printf("[i=%d]", i); fflush(stdin);
            for(j = k; j < k + count; j++)
            {
                if(i == j)
                    printf("\t-");
                else
                    printf("\t%d", problem->get_cost(problem, i, j));
                fflush(stdin);
            }
            printf("\n");
        }
        printf("\n");

        k += count;
    }
}

inline int 
arrow_problem_get_cost(arrow_problem *problem, int i, int j)
{
    return problem->data.edgelen(i, j, &(problem->data));
}

int
arrow_problem_read_tour(char *file_name, int size, int *tour)
{
    if(CCutil_getcycle_tsplib(size, file_name, tour) == CONCORDE_SUCCESS)
        return ARROW_SUCCESS;
    else
        return ARROW_ERROR_FATAL;
}

int
arrow_problem_abtsp_to_sbtsp(arrow_problem *old_problem, int infinity, 
                             arrow_problem *new_problem)
{
    int i, j;
    int n = old_problem->size;
    int ret = ARROW_SUCCESS;

    sprintf(new_problem->name, "%s", old_problem->name);
    new_problem->shallow = ARROW_FALSE;
    new_problem->size = n * 2;
    new_problem->symmetric = ARROW_TRUE;
    new_problem->get_cost = arrow_problem_get_cost;
    
    CCdatagroup *dat = &(new_problem->data);
    
    ret = arrow_util_CCdatagroup_init_matrix(n * 2, dat);
    if(ret != ARROW_SUCCESS)
    {
        return ARROW_FAILURE;
    }

    for(i = 0; i < new_problem->size; i++)
    {
        for(j = 0; j <= i; j++)
        {
            if((i < n) || (j >= n))
                dat->adj[i][j] = infinity;
            else if(i == j + n)
                dat->adj[i][j] = -1 * infinity;
            else
                dat->adj[i][j] = old_problem->get_cost(old_problem, j, i - n);
        }
    }
    
    return ret;
}


/****************************************************************************
 * Private function implementations
 ****************************************************************************/
/* These regular expressions check to see if the given file name ends
   with tsp (for symmetric TSPLIB files) or atsp (for asymmetric TSPLIB
   files).  Someday, perhaps, I'll bother to actually read the TSPLIB
   file to see what the TYPE is rather than go by the file extension. */
int
is_symmetric(char *file_name)
{
    return arrow_util_regex_match(file_name, "(.*\\.([tT][sS][pP])$)");
}

int
is_asymmetric(char *file_name)
{
    return arrow_util_regex_match(file_name, "(.*\\.([aA][tT][sS][pP])$)");
}

int
read_atsp(char *file_name, arrow_problem *problem)
{
    /* This code is mostly borrowed (stolen?) from Concorde's routine,
       so credit to the Concorde guys who dealing with the pain of reading
       TSPLIB files in C. */
    char buf[256], key[256], field[256];
    char *p;
    FILE *in;
    int norm, i, j;
    CCdatagroup *dat = &(problem->data);
    
    if(!(in = fopen(file_name, "r"))) 
    {
        arrow_print_error("Unable to open file for input\n");
        return ARROW_ERROR_FATAL;
    }
    
    CCutil_init_datagroup(dat);
    
    while(fgets(buf, 254, in) != (char *)NULL)
    {
        p = buf;
        while (*p != '\0')
        {
            if (*p == ':') *p = ' ';
            p++;
        }
        p = buf;
        
        if(sscanf(p, "%s", key) != EOF)
        {
            p += strlen(key);
            while(*p == ' ') p++;
            
            if(!strcmp (key, "NAME"))
            {
                arrow_debug("Problem Name: %s", p);
            }
            else if(!strcmp (key, "TYPE"))
            {
                arrow_debug("Problem Type: %s", p);
                if(sscanf(p, "%s", field) == EOF || strcmp (field, "ATSP"))
                {
                    arrow_print_error("Not an ATSP problem");
                    return ARROW_ERROR_FATAL;
                }
            } 
            else if(!strcmp (key, "COMMENT"))
            {
                arrow_debug("%s", p);
            } 
            else if(!strcmp (key, "DIMENSION"))
            {
                if(sscanf(p, "%s", field) == EOF)
                {
                    arrow_print_error("ERROR in DIMENSION line");
                    return ARROW_ERROR_FATAL;
                }
                problem->size = atoi(field);
                arrow_debug("Number of Nodes: %d\n", problem->size);
            }
            else if(!strcmp (key, "EDGE_WEIGHT_TYPE"))
            {
                if(sscanf(p, "%s", field) == EOF)
                {
                    arrow_print_error("ERROR in EDGE_WEIGHT_TYPE line");
                    return ARROW_ERROR_FATAL;
                }
                if(!strcmp(field, "EXPLICIT")) {
                    printf ("Explicit Lengths (CC_MATRIXNORM)\n");
                    norm = CC_MATRIXNORM;
                }
                else
                {
                    arrow_print_error("ERROR: Not set up for given norm");
                    return ARROW_ERROR_FATAL;
                }
                
                if(CCutil_dat_setnorm(dat, norm)) 
                {
                    arrow_print_error("Could not setup MAXTRIXNORM");
                    return ARROW_ERROR_FATAL;
                }
                dat->edgelen = fullmatrix_edgelen;
            }
            else if(!strcmp (key, "EDGE_WEIGHT_FORMAT"))
            {
                if(sscanf (p, "%s", field) == EOF)
                {
                    arrow_print_error("ERROR in EDGE_WEIGHT_FORMAT line");
                    return ARROW_ERROR_FATAL;
                }
                if(strcmp(field, "FULL_MATRIX"))
                {
                    arrow_print_error("Cannot handle edge weight format");
                    return ARROW_ERROR_FATAL;
                }
            }
            else if(!strcmp(key, "EDGE_WEIGHT_SECTION"))
            {
                if(problem->size < 1) {
                    arrow_print_error("Dimension not specified");
                    return ARROW_ERROR_FATAL;
                }
                if(dat->adj != (int **)NULL)
                {            
                    arrow_print_error("A second NODE_COORD_SECTION?");
                    CCutil_freedatagroup(dat);
                    return ARROW_ERROR_FATAL;
                }
                if ((norm & CC_NORM_SIZE_BITS) == CC_MATRIX_NORM_SIZE)
                {
                    dat->adj = CC_SAFE_MALLOC(problem->size, int *);
                    dat->adjspace = 
                        CC_SAFE_MALLOC(problem->size * problem->size, int);

                    if(dat->adj == (int **) NULL ||
                        dat->adjspace == (int *) NULL) 
                    {
                        arrow_print_error("Could not setup adj/adjspace");
                        CCutil_freedatagroup(dat);
                        return ARROW_ERROR_FATAL;
                    }
                    
                    for(i = 0, j = 0; i < problem->size; i++)
                    {
                        dat->adj[i] = dat->adjspace + j;
                        j += problem->size;
                    }
                    
                    for(i = 0; i < problem->size; i++)
                    {
                        for(j = 0; j < problem->size; j++)
                        {
                            fscanf(in, "%d", &(dat->adj[i][j]));
                        }
                    }
                }
            }
            else if(!strcmp(key, "NODE_COORD_SECTION"))
            {
                arrow_print_error("Encountered NODE_COORD_SECTION\n");
                return ARROW_ERROR_FATAL;
            }
            else if(!strcmp(key, "FIXED_EDGES_SECTION"))
            {
                arrow_print_error("Not set up for fixed edges\n");
                return ARROW_ERROR_FATAL;
            }
        }
    }
    
    return ARROW_SUCCESS;
}

static int
fullmatrix_edgelen(int i, int j, CCdatagroup *dat)
{
    return dat->adj[i][j];
}
