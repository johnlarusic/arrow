/**********************************************************doxygen*//** @file
 * @brief   Cost matrix transformation functions.
 *
 * Cost matrix transformation functions for the bottleneck traveling salesman
 * problem (BTSP).
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "arrow.h"

/****************************************************************************
 * Private function prototypes
 ****************************************************************************/
/**
 *  @brief  Applies basic BTSP function to the cost matrix of the old problem
 *          to create the new problem (shallow copy).
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                    int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Applies basic BTSP function to the cost matrix of the old problem
 *          to create the new problem (deep copy).
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                 int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Destructs a basic BTSP function structure
 *  @param  fun [out] the function structure to destruct
 */
void
basic_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Determines if the given tour is feasible or not.
 *  @param  fun [in] function structure
 *  @param  problem [in] the problem to check against
 *  @param  tour_length [in] the length of the given tour
 *  @param  tour [in] the tour in node-node format
 *  @return ARROW_TRUE if the tour is feasible, ARROW_FALSE if not
 */
int
basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem, 
               double tour_length, int *tour);

/**
 *  @brief  BTSP to TSP cost transformation
 *  @param  cost [in] the old cost
 *  @param  delta [in] delta parameter
 *  @return The transformed cost C[i,j] between nodes i and j
 */
static int
basic_cost(int cost, int delta);

/**
 *  @brief  Basic BTSP edge length function for Euclidean data (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
basic_euclid_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Basic BTSP edge length function for 2D Euclidean distances 
 *          rounded up  (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
basic_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Basic BTSP edge length function for geographical distances (GEO)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
basic_geographic_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Basic BTSP edge length function for att48/att532 problems (ATT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
basic_att_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Basic BTSP edge length function for explict matrix data (EXPLICIT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
basic_matrix_edgelen(int i, int j, CCdatagroup *dat);


/**
 *  @brief  Applies constrained BTSP function to the cost matrix of the old 
 *          problem to create the new problem (shallow copy)
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
constrained_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                          int delta, arrow_problem *new_problem);

/**
 *  @brief  Applies constrained BTSP function to the cost matrix of the old 
 *          problem to create the new problem (deep copy)
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
constrained_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                       int delta, arrow_problem *new_problem);

/**
 *  @brief  Destructs constrained BTSP function structure
 *  @param  fun [out] the function structure to destruct
 */
void
constrained_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Constrained BTSP cost transformation
 *  @param  cost [in] the old cost
 *  @param  delta [in] delta parameter
 *  @param  infinity [in] value to use as infinity
 *  @return The transformed cost C[i,j] between nodes i and j
 */
static int
constrained_cost(int cost, int delta, int infinity);

/**
 *  @brief  Constrained BTSP edge length function for Euclidean data (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
constrained_euclid_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Constrained BTSP edge length function for 2D Euclidean distances 
 *          rounded up  (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
constrained_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Constrained BTSP edge length function for geographical distances 
 *          (GEO)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
constrained_geographic_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Constrained BTSP edge length function for att48/att532 problems 
 *          (ATT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
constrained_att_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Constrained BTSP edge length function for explict matrix data 
 *          (EXPLICIT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
constrained_matrix_edgelen(int i, int j, CCdatagroup *dat);

/* These functions are essentially copies of those found in Concorde for
 * computing edge lengths.  Until such time Concorde makes it easy for
 * user-defined edge length functions to be used, we have to live with the
 * pain of redefining these functions for every TSPLIB type.
 */
/**
 *  @brief  Truncates decimal places from a given floating point number.
 *  @param  x [in] the floating point number to truncate
 *  @return The truncated number.
 */
static double
dtrunc(double x);

/**
 *  @brief  Edge length function for Euclidean data (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
euclid_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for 2D Euclidean distances rounded up 
 *          (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for geographical distances (GEO)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
geographic_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for att48 and att532 problems (ATT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
att_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for explict matrix data (EXPLICIT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
matrix_edgelen(int i, int j, CCdatagroup *dat);


/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
int
arrow_btsp_fun_apply(arrow_btsp_fun *fun, arrow_problem *old_problem, 
                     int delta, arrow_problem *new_problem)
{
    
    CCutil_init_datagroup(&(new_problem->data));
    new_problem->size = old_problem->size;
    new_problem->name = old_problem->name;
    new_problem->shallow = fun->shallow;
    new_problem->get_cost = old_problem->get_cost;
    
    if(fun->shallow == ARROW_TRUE)
    {
        /* Create a shallow copy of the data */
        arrow_util_CCdatagroup_shallow_copy(&(old_problem->data), 
                                            &(new_problem->data));
    }
    else
    {
        /* Create structure to hold new cost matrix */
        int ret;
        ret = arrow_util_CCdatagroup_init_matrix(old_problem->size, 
                                                 &(new_problem->data));
        if(ret != ARROW_SUCCESS)
            return ARROW_ERROR_FATAL;
    }
    
    /* Apply the function to the cost matrix */
    fun->apply(fun, old_problem, delta, new_problem);
    
    return ARROW_SUCCESS;
}

void
arrow_btsp_fun_destruct(arrow_btsp_fun *fun)
{
    fun->destruct(fun);
}

int
arrow_btsp_fun_basic(int shallow, arrow_btsp_fun *fun)
{
    fun->data = NULL;
    if(shallow == ARROW_TRUE)
    {
        fun->shallow = ARROW_TRUE;
        fun->apply = basic_shallow_apply;
    }
    else
    {
        fun->shallow = ARROW_FALSE;
        fun->apply = basic_deep_apply;
    }
    fun->feasible_length = 0;
    
    fun->destruct = basic_destruct;
    fun->feasible = basic_feasible;
    
    return ARROW_SUCCESS;
}

int
arrow_btsp_fun_constrained(int shallow, int infinity, arrow_btsp_fun *fun)
{
    if((fun->data = malloc(sizeof(int))) == NULL)
    {
        arrow_print_error("Error allocating memory for fun->data (int).");
        return ARROW_ERROR_FATAL;
    }
    *((int*)fun->data) = infinity;
    
    if(shallow == ARROW_TRUE)
    {
        fun->shallow = ARROW_TRUE;
        fun->apply = constrained_shallow_apply;
    }
    else
    {
        fun->shallow = ARROW_TRUE;
        fun->apply = constrained_deep_apply;
    }

    fun->destruct = constrained_destruct;
    fun->feasible = basic_feasible;
    
    return ARROW_SUCCESS;
}


/****************************************************************************
 * Private function implemenations
 ****************************************************************************/
int
basic_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                    int delta, arrow_problem *new_problem)
{
    int norm;
    CCdatagroup *old_data = &(old_problem->data);
    CCdatagroup *new_data = &(new_problem->data);
    CCutil_dat_getnorm(old_data, &norm);
    switch(norm)
    {
        case CC_EUCLIDEAN: 
            arrow_debug("EUC_2D cost matrix\n");
            (new_problem->data).edgelen = basic_euclid_edgelen;
            break;
        case CC_GEOGRAPHIC: 
            arrow_debug("GEO cost matrix\n");
            new_data->edgelen = basic_geographic_edgelen; 
            break;
        case CC_ATT:
            arrow_debug("ATT cost matrix\n");
            new_data->edgelen = basic_att_edgelen; 
            break;
        case CC_MATRIXNORM:
            arrow_debug("EXPLICIT cost matrix\n");
            new_data->edgelen = basic_matrix_edgelen; 
            break;
        case CC_EUCLIDEAN_CEIL:
            arrow_debug("CEIL_2D cost matrix\n");
            new_data->edgelen = basic_euclid_ceiling_edgelen; 
            break;
        default:
            arrow_print_error("Edge weight type not supported\n");
            return ARROW_ERROR_FATAL;
            break;
    }
    
    arrow_debug("Delta is %d\n", delta);
    new_data->dsjrand_param = delta;
    return ARROW_SUCCESS;
}

int
basic_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                 int delta, arrow_problem *new_problem)
{
    int i, j;
    for(i = 0; i < new_problem->size; i++)
        for(j = 0; j <= i; j++)
            new_problem->data.adj[i][j] = 
                basic_cost(old_problem->get_cost(old_problem, i, j), delta);
    return ARROW_SUCCESS;
}

void
basic_destruct(arrow_btsp_fun *fun)
{ }

int
basic_feasible(arrow_btsp_fun *fun, arrow_problem *problem,
               double tour_length, int *tour)
{
    return (tour_length <= fun->feasible_length ? ARROW_TRUE : ARROW_FALSE);
}

static int
basic_cost(int cost, int delta)
{
    return (cost <= delta ? 0 : cost);
}

static int 
basic_euclid_edgelen(int i, int j, CCdatagroup *dat)
{
    return basic_cost(euclid_edgelen(i, j, dat),
                      dat->dsjrand_param);
}

static int
basic_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat)
{
    return basic_cost(euclid_ceiling_edgelen(i, j, dat),
                      dat->dsjrand_param);
}

static int
basic_geographic_edgelen(int i, int j, CCdatagroup *dat)
{
    return basic_cost(geographic_edgelen(i, j, dat),
                      dat->dsjrand_param);
}

static int
basic_att_edgelen(int i, int j, CCdatagroup *dat)
{
    return basic_cost(att_edgelen(i, j, dat),
                      dat->dsjrand_param);
}

static int
basic_matrix_edgelen(int i, int j, CCdatagroup *dat)
{
    return basic_cost(matrix_edgelen(i, j, dat),
                      dat->dsjrand_param);  
}


int
constrained_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                          int delta, arrow_problem *new_problem)
{
    int norm;
    CCdatagroup *old_data = &(old_problem->data);
    CCdatagroup *new_data = &(new_problem->data);
    CCutil_dat_getnorm(old_data, &norm);
    switch(norm)
    {
        case CC_EUCLIDEAN: 
            arrow_debug("EUC_2D cost matrix\n"); 
            new_data->edgelen = constrained_euclid_edgelen;
            break;
        case CC_GEOGRAPHIC: 
            arrow_debug("GEO cost matrix\n");
            new_data->edgelen = constrained_geographic_edgelen; 
            break;
        case CC_ATT:
            arrow_debug("ATT cost matrix\n");
            new_data->edgelen = constrained_att_edgelen; 
            break;
        case CC_MATRIXNORM:
            arrow_debug("EXPLICIT cost matrix\n");
            new_data->edgelen = constrained_matrix_edgelen; 
            break;
        case CC_EUCLIDEAN_CEIL:
            arrow_debug("CEIL_2D cost matrix\n");
            new_data->edgelen = constrained_euclid_ceiling_edgelen; 
            break;
        default:
            arrow_print_error("Edge weight type not supported\n");
            return ARROW_ERROR_FATAL;
            break;
    }
    
    arrow_debug("Delta is %d\n", delta);
    new_data->dsjrand_param = delta;
    new_data->default_len = *((int*)(fun->data));
    return ARROW_SUCCESS;
}

int
constrained_deep_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                       int delta, arrow_problem *new_problem)
{
    int i, j;
    for(i = 0; i < new_problem->size; i++)
        for(j = 0; j <= i; j++)
            new_problem->data.adj[i][j] = 
                constrained_cost(old_problem->get_cost(old_problem, i, j), 
                                 delta, *((int*)(fun->data)));
    return ARROW_SUCCESS;
}

void
constrained_destruct(arrow_btsp_fun *fun)
{ 
    if((int*)(fun->data) != NULL)
    {
        free((int*)(fun->data));
        ((int*)(fun->data)) = NULL;
    }
}

static int
constrained_cost(int cost, int delta, int infinity)
{
    return (cost <= delta ? cost : infinity);
}

static int 
constrained_euclid_edgelen(int i, int j, CCdatagroup *dat)
{
    return constrained_cost(euclid_edgelen(i, j, dat), 
                            dat->dsjrand_param, dat->default_len);
}

static int
constrained_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat)
{
    return constrained_cost(euclid_ceiling_edgelen(i, j, dat), 
                            dat->dsjrand_param, dat->default_len);
}

static int
constrained_geographic_edgelen(int i, int j, CCdatagroup *dat)
{
    return constrained_cost(geographic_edgelen(i, j, dat), 
                            dat->dsjrand_param, dat->default_len);
}

static int
constrained_att_edgelen(int i, int j, CCdatagroup *dat)
{
    return constrained_cost(att_edgelen(i, j, dat), 
                            dat->dsjrand_param, dat->default_len);
}

static int
constrained_matrix_edgelen(int i, int j, CCdatagroup *dat)
{
    return constrained_cost(matrix_edgelen(i, j, dat), 
                            dat->dsjrand_param, dat->default_len);
}


static double dtrunc(double x)
{
    int k;
    k = (int) x;
    x = (double) k;
    return x;
}

static int 
euclid_edgelen(int i, int j, CCdatagroup *dat)
{
    double t1 = dat->x[i] - dat->x[j], t2 = dat->y[i] - dat->y[j];
    return (int) (sqrt (t1 * t1 + t2 * t2) + 0.5);
}

static int
euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat)
{
    double t1 = dat->x[i] - dat->x[j], t2 = dat->y[i] - dat->y[j];
    return (int) (ceil (sqrt (t1 * t1 + t2 * t2)));
}

#define GH_PI (3.141592)
static int
geographic_edgelen(int i, int j, CCdatagroup *dat)
{
    double deg, min;
    double lati, latj, longi, longj;
    double q1, q2, q3;
    int val;
    double x1 = dat->x[i], x2 = dat->x[j], yy1 = dat->y[i], yy2 = dat->y[j];

    deg = dtrunc (x1);
    min = x1 - deg;
    lati = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (x2);
    min = x2 - deg;
    latj = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;

    deg = dtrunc (yy1);
    min = yy1 - deg;
    longi = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;
    deg = dtrunc (yy2);
    min = yy2 - deg;
    longj = GH_PI * (deg + 5.0 * min / 3.0) / 180.0;

    q1 = cos (longi - longj);
    q2 = cos (lati - latj);
    q3 = cos (lati + latj);
    return (int) (6378.388 * acos (0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3))
                + 1.0);
}

static int
att_edgelen(int i, int j, CCdatagroup *dat)
{
    double xd = dat->x[i] - dat->x[j];
    double yd = dat->y[i] - dat->y[j];
    double rij = sqrt ((xd * xd + yd * yd) / 10.0);
    double tij = dtrunc (rij);
    int val;

    if (tij < rij)
        val = (int) tij + 1;
    else
        val = (int) tij;
    return val;
}

static int
matrix_edgelen(int i, int j, CCdatagroup *dat)
{
    return (i > j ? (dat->adj[i])[j] : (dat->adj[j])[i]);
}