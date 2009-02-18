/**********************************************************doxygen*//** @file
 * @brief   Functions for working with problem data.
 *
 * Function implementations for working with problem data, generally 
 * manipulating the arrow_problem data structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/****************************************************************************
 * Private structures
 ****************************************************************************/
typedef struct full_matrix_data
{
    int **adj;      /**< 2D array of integers */
    int *adjspace;  /**< contiguous allocation of n*n integers */
} full_matrix_data;

typedef struct abtsp_data
{
    arrow_problem *base;  /**< base problem */
    int infinity;         /**< value to represent infinity */
} abtsp_data;

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
 *  @brief  Reads an symmetric TSPLIB file (*.tsp).
 *  @param  file_name [in] the path to the TSPLIB file
 *  @param  problem [out] problem structure
 */
int
read_stsp(char *file_name, arrow_problem *problem);

/**
 *  @brief  Retrieves cost between nodes i and j from Concorde's structure.
 *  @param  problem [in] pointer to arrow_problem structure
 *  @param  i [in] id of starting node
 *  @param  j [in] id of ending node
 *  @return cost between node i and node j
 */
int 
concorde_get_cost(arrow_problem *problem, int i, int j);

/**
 *  @brief  Destructs Concorde's data structure
 *  @param  problem [in] pointer to arrow_problem structure
 */
void
concorde_destruct(arrow_problem *problem);

/**
 *  @brief  Reads an asymmetric TSPLIB file (*.atsp).
 *  @param  file_name [in] the path to the TSPLIB file
 *  @param  problem [out] problem structure
 */
int
read_atsp(char *file_name, arrow_problem *problem);

/**
 *  @brief  Edge length function for full matrix data
 *  @param  problem [in] pointer to arrow_problem structure
 *  @param  i [in] id of starting node
 *  @param  j [in] id of ending node
 *  @return The cost between nodes i and j
 */
int 
full_matrix_get_cost(arrow_problem *problem, int i, int j);

/**
 *  @brief  Destructs data structure of a full matrix problem.
 *  @param  problem [in] pointer to arrow_problem structure
 */
void
full_matrix_destruct(arrow_problem *problem);

/**
 *  @brief  Edge length function for ABTSP->SBTSP data
 *  @param  problem [in] pointer to arrow_problem structure
 *  @param  i [in] id of starting node
 *  @param  j [in] id of ending node
 *  @return The cost between nodes i and j
 */
int 
abtsp_get_cost(arrow_problem *problem, int i, int j);

/**
 *  @brief  Destructs data structure of a ABTSP->SBTSP problem.
 *  @param  problem [in] pointer to arrow_problem structure
 */
void
abtsp_destruct(arrow_problem *problem);


/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_problem_read(char *file_name, arrow_problem *problem)
{   
    int ret = ARROW_FAILURE;
    
    if(is_symmetric(file_name))
        ret = read_stsp(file_name, problem);
    else if(is_asymmetric(file_name))
        ret = read_atsp(file_name, problem);
    
    if(ret == ARROW_FAILURE)
    {
        arrow_print_error("Unable to read '%s'", file_name);
        return ARROW_FAILURE;
    }
        
    /* We're going to build a problem name from the file name. */
    /* Start by only considering stuff after the last "/" */
    char *s = strrchr(file_name, '/');
    if(s != NULL)
        sprintf(problem->name, "%s", s + 1);
    else
        sprintf(problem->name, "%s", file_name);
        
    /* Now replace "." characters with "_" characters */
    s = strchr(problem->name, '.');
    while(s != NULL)
    {
        *s = '_';
        s = strchr(problem->name, '.');
    }
        
    return ret;
}

void
arrow_problem_destruct(arrow_problem *problem)
{
    if(problem->data != NULL)
    {
        problem->destruct(problem);
        problem->data = NULL;
    }
}

int
arrow_problem_info_get(arrow_problem *problem, int create_hash,
                       arrow_problem_info *info)
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
    arrow_bintree_to_new_array(&tree, &(info->cost_list));
    info->cost_list_length = tree.size;
    info->min_cost = min_cost;
    info->max_cost = max_cost;
    
    /* (optionally) create hash table */
    info->hash.num_keys = 0;
    if(create_hash)
    {
        arrow_hash_cost_list(info->cost_list, info->cost_list_length, 
                             &(info->hash));
    }
    
CLEANUP:
    arrow_bintree_destruct(&tree);
    return ret;
}

void
arrow_problem_info_destruct(arrow_problem_info *info)
{
    if(info->cost_list != NULL)
        free(info->cost_list);
    if(info->hash.num_keys > 0)
        arrow_hash_destruct(&(info->hash));
}

int
arrow_problem_info_cost_index(arrow_problem_info *info, int cost, int *pos)
{
    int ret = ARROW_SUCCESS;
    
    *pos = -1;
    if(info->hash.num_keys > 0)
    {
        *pos = (int)arrow_hash_search(&(info->hash), cost);
        if(*pos < 0)
            ret = ARROW_FAILURE;
    }
    else
    {
        if(!arrow_util_binary_search(info->cost_list, info->cost_list_length, 
                                     cost, pos))
        {
            arrow_print_error("Could not find cost in cost list");
            ret = ARROW_FAILURE;
        }
    }
    
    return ret;
}

void
arrow_problem_print(arrow_problem *problem, int pretty)
{
    int i, j, k, count;
    int group_size = 8;
    
    printf("Problem Cost Matrix:\n");
    printf("----------------------------------------");
    printf("--------------------------------------\n");
    printf("Problem Size:  %d\n", problem->size);
    printf("Shallow Data?: %s\n", (problem->shallow ? "Yes" : "No"));
    printf("Symmetric?:    %s\n", (problem->symmetric ? "Yes" : "No"));
    
    /* Print cost matrix out into groups so they will (normally) fit into
     * an 80-character width terminal. */
    if(pretty)
    {
        k = 0;
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
    else
    {
        /* Print out the header */
        for(i = 0; i < problem->size; i++)
            printf("\t[j=%d]", i);   
        printf("\n");
        
        /* Print out rows */
        for(i = 0; i < problem->size; i++)
        {
            printf("[i=%d]", i); fflush(stdin);
            for(j = 0; j < problem->size; j++)
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
    }
}

int
arrow_problem_read_tour(char *file_name, int size, int *tour)
{
    if(CCutil_getcycle_tsplib(size, file_name, tour) == CONCORDE_SUCCESS)
        return ARROW_SUCCESS;
    else
        return ARROW_FAILURE;
}

int
arrow_problem_abtsp_to_sbtsp(int shallow, arrow_problem *old_problem, 
                             int infinity, arrow_problem *new_problem)
{
    new_problem->size = old_problem->size * 2;
    new_problem->type = ARROW_PROBLEM_ABTSP_TO_SBTSP;
    new_problem->shallow = shallow;
    new_problem->symmetric = ARROW_TRUE;
    sprintf(new_problem->name, "%s", old_problem->name);
    
    abtsp_data *data = NULL;
    if((data = malloc(sizeof(abtsp_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for abtsp_data");
        return ARROW_FAILURE;
    }
    data->base = old_problem;
    data->infinity = infinity;    
    new_problem->data = data;

    new_problem->get_cost = abtsp_get_cost;
    new_problem->destruct = abtsp_destruct;
    
    return ARROW_SUCCESS;
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
read_stsp(char *file_name, arrow_problem *problem)
{
    int size;
    CCdatagroup *dat = NULL;
    
    arrow_debug("Reading symmetric TSPLIB file...\n");

    /* Allocate CCdatargoup structure for Concorde to chew on. */
    if((dat = malloc(sizeof(CCdatagroup))) == NULL)
    {
        arrow_print_error("Could not allocate memory for CCdatagroup");
        goto CLEANUP;
    }
    
    /* We're going to use Concorde to read the TSPLIB file!  Sneaky! 
       Recall that Concorde returns 1 on failure because it's silly. */
    if(CCutil_gettsplib(file_name, &size, dat))
    {
        arrow_print_error("Unable to read TSPLIB file using Concorde\n");
        goto CLEANUP;
    }
    
    problem->size = size;
    problem->type = ARROW_PROBLEM_DATA_CONCORDE;
    problem->symmetric = ARROW_TRUE;
    problem->data = (void *)dat;
    problem->shallow = ARROW_FALSE;
    problem->get_cost = concorde_get_cost;
    problem->destruct = concorde_destruct;
    
    return ARROW_SUCCESS;

CLEANUP:
    if(dat != NULL) free(dat);
    return ARROW_FAILURE;
}

int 
concorde_get_cost(arrow_problem *problem, int i, int j)
{
    CCdatagroup *dat = (CCdatagroup *)problem->data;    
    return dat->edgelen(i, j, dat);
}

void
concorde_destruct(arrow_problem *problem)
{    
    if(problem->data != NULL)
    {
        CCdatagroup *dat = (CCdatagroup *)problem->data;
        CCutil_freedatagroup(dat);
    }
}

int
read_atsp(char *file_name, arrow_problem *problem)
{
    int size = -1;
    full_matrix_data *data;
    
    arrow_debug("Reading asymmetric TSPLIB file...\n");
    
    /* Allocate CCdatargoup structure for Concorde to chew on. */
    if((data = malloc(sizeof(full_matrix_data))) == NULL)
    {
        arrow_print_error("Could not allocate memory for full_matrix_data");
        goto CLEANUP;
    }
    data->adj = NULL;
    data->adjspace = NULL;

    /* This code is mostly borrowed (stolen?) from Concorde's routine,
       so credit to the Concorde guys who dealing with the pain of reading
       TSPLIB files in C. */
    char buf[256], key[256], field[256];
    char *p;
    FILE *in;
    int norm, i, j;
    
    if(!(in = fopen(file_name, "r"))) 
    {
        arrow_print_error("Unable to open file for input\n");
        goto CLEANUP;
    }
    
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
                    goto CLEANUP;
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
                    goto CLEANUP;
                }
                size = atoi(field);
                arrow_debug("Number of Nodes: %d\n", size);
            }
            else if(!strcmp (key, "EDGE_WEIGHT_TYPE"))
            {
                if(sscanf(p, "%s", field) == EOF)
                {
                    arrow_print_error("ERROR in EDGE_WEIGHT_TYPE line");
                    goto CLEANUP;
                }
                if(!strcmp(field, "EXPLICIT")) {
                    printf ("Explicit Lengths (CC_MATRIXNORM)\n");
                    norm = CC_MATRIXNORM;
                }
                else
                {
                    arrow_print_error("ERROR: Not set up for given norm");
                    goto CLEANUP;
                }
            }
            else if(!strcmp (key, "EDGE_WEIGHT_FORMAT"))
            {
                if(sscanf (p, "%s", field) == EOF)
                {
                    arrow_print_error("ERROR in EDGE_WEIGHT_FORMAT line");
                    goto CLEANUP;
                }
                if(strcmp(field, "FULL_MATRIX"))
                {
                    arrow_print_error("Cannot handle edge weight format");
                    goto CLEANUP;
                }
            }
            else if(!strcmp(key, "EDGE_WEIGHT_SECTION"))
            {
                if(size < 1) {
                    arrow_print_error("Dimension not specified");
                    goto CLEANUP;
                }
                if(data->adj != (int **)NULL)
                {            
                    arrow_print_error("A second NODE_COORD_SECTION?");
                    goto CLEANUP;
                }
                if ((norm & CC_NORM_SIZE_BITS) == CC_MATRIX_NORM_SIZE)
                {
                    data->adj = CC_SAFE_MALLOC(size, int *);
                    data->adjspace = CC_SAFE_MALLOC(size * size, int);

                    if(data->adj == (int **) NULL ||
                        data->adjspace == (int *) NULL) 
                    {
                        arrow_print_error("Could not setup adj/adjspace");
                        goto CLEANUP;
                    }
                    
                    for(i = 0, j = 0; i < size; i++)
                    {
                        data->adj[i] = data->adjspace + j;
                        j += size;
                    }
                    
                    for(i = 0; i < size; i++)
                    {
                        for(j = 0; j < size; j++)
                        {
                            fscanf(in, "%d", &(data->adj[i][j]));
                        }
                    }
                }
            }
            else if(!strcmp(key, "NODE_COORD_SECTION"))
            {
                arrow_print_error("Encountered NODE_COORD_SECTION\n");
                goto CLEANUP;
            }
            else if(!strcmp(key, "FIXED_EDGES_SECTION"))
            {
                arrow_print_error("Not set up for fixed edges\n");
                goto CLEANUP;
            }
        }
    }

    problem->size = size;
    problem->type = ARROW_PROBLEM_DATA_FULL_MATRIX;
    problem->symmetric = ARROW_FALSE;
    problem->data = (void *)data;
    problem->shallow = ARROW_FALSE;
    problem->get_cost = full_matrix_get_cost;
    problem->destruct = full_matrix_destruct;
    return ARROW_SUCCESS;

CLEANUP:
    if(data->adjspace != NULL) free(data->adjspace);
    if(data->adj != NULL) free(data->adj);
    return ARROW_FAILURE;
}

int 
full_matrix_get_cost(arrow_problem *problem, int i, int j)
{
    full_matrix_data *data = (full_matrix_data *)problem->data;
    return data->adj[i][j];
}

void
full_matrix_destruct(arrow_problem *problem)
{   
    if(problem->data != NULL)
    {
        full_matrix_data *data = (full_matrix_data *)problem->data;
        free(data->adjspace);
        free(data->adj);
    }
}

int 
abtsp_get_cost(arrow_problem *this, int i, int j)
{
    if(j > i) return abtsp_get_cost(this, j, i);
    
    abtsp_data *data = (abtsp_data *)this->data;
    int n = data->base->size;
    
    if((i < n) || (j >= n))
        return data->infinity;
    else if(i == j + n)
        return -1 * data->infinity;
    else
        return data->base->get_cost(data->base, j, i - n);
}

void 
abtsp_destruct(arrow_problem *this)
{ 
    free(this->data);
}