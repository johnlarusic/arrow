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
 *  @brief  Applies function to the cost matrix of the old problem to create
 *          the new problem
 *  @param  fun [in] the cost matrix function
 *  @param  old_problem [in] the problem to apply the function to
 *  @param  delta [in] delta parameter
 *  @param  new_problem [out] the resulting new problem
 */
int
basic_shallow_apply(arrow_btsp_fun *fun, arrow_problem *old_problem,
                    int delta, arrow_problem *new_problem);
                    
/**
 *  @brief  Destructs a function structure
 *  @param  fun [out] the function structure to destruct
 */
void
basic_shallow_destruct(arrow_btsp_fun *fun);

/**
 *  @brief  Edge length function for Euclidean data (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
btsp_euclid_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for 2D Euclidean distances rounded up 
 *          (EUC_2D)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
btsp_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for geographical distances (GEO)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
btsp_geographic_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for att48 and att532 problems (ATT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
btsp_att_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Edge length function for explict matrix data (EXPLICIT)
 *  @param  i [in] node i
 *  @param  j [in] node j
 *  @param  dat [in] Concorde problem data structure
 *  @return The cost C[i,j] between nodes i and j
 */
static int
btsp_matrix_edgelen(int i, int j, CCdatagroup *dat);

/**
 *  @brief  Truncates decimal places from a given floating point number.
 *  @param  x [in] the floating point number to truncate
 *  @return The truncated number.
 */
static double
dtrunc(double x);


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
        arrow_print_error("Not yet implemented");
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
arrow_btsp_fun_basic_shallow(arrow_btsp_fun *fun)
{
    fun->data = NULL;
    fun->shallow = ARROW_TRUE;
    fun->apply = basic_shallow_apply;
    fun->destruct = basic_shallow_destruct;
    return ARROW_SUCCESS;
}

/****************************************************************************
 * Public function implemenations
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
            //new_data->edgelen = btsp_euclid_edgelen; 
            (new_problem->data).edgelen = btsp_euclid_edgelen;
            break;
        case CC_GEOGRAPHIC: 
            arrow_debug("GEO cost matrix\n");
            new_data->edgelen = btsp_geographic_edgelen; 
            break;
        case CC_ATT:
            arrow_debug("ATT cost matrix\n");
            new_data->edgelen = btsp_att_edgelen; 
            break;
        case CC_MATRIXNORM:
            arrow_debug("EXPLICIT cost matrix\n");
            new_data->edgelen = btsp_matrix_edgelen; 
            break;
        case CC_EUCLIDEAN_CEIL:
            arrow_debug("CEIL_2D cost matrix\n");
            new_data->edgelen = btsp_euclid_ceiling_edgelen; 
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

void
basic_shallow_destruct(arrow_btsp_fun *fun)
{ }

static int 
btsp_euclid_edgelen(int i, int j, CCdatagroup *dat)
{
    double t1 = dat->x[i] - dat->x[j], t2 = dat->y[i] - dat->y[j];
    int val = (int) (sqrt (t1 * t1 + t2 * t2) + 0.5);
    return (val <= dat->dsjrand_param ? 0 : val);
}

static int
btsp_euclid_ceiling_edgelen(int i, int j, CCdatagroup *dat)
{
    double t1 = dat->x[i] - dat->x[j], t2 = dat->y[i] - dat->y[j];
    int val = (int) (ceil (sqrt (t1 * t1 + t2 * t2)));
    return (val <= dat->dsjrand_param ? 0 : val);
}

#define GH_PI (3.141592)
static int
btsp_geographic_edgelen(int i, int j, CCdatagroup *dat)
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
    val = (int) (6378.388 * acos (0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3))
                + 1.0);
    return (val <= dat->dsjrand_param ? 0 : val);
}

static int
btsp_att_edgelen(int i, int j, CCdatagroup *dat)
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
    return (val <= dat->dsjrand_param ? 0 : val);
}

static int
btsp_matrix_edgelen(int i, int j, CCdatagroup *dat)
{
    int val = (i > j ? (dat->adj[i])[j] : (dat->adj[j])[i]);
    return (val <= dat->dsjrand_param ? 0 : val);    
}

static double dtrunc(double x)
{
    int k;
    k = (int) x;
    x = (double) k;
    return x;
}