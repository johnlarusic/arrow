/**********************************************************doxygen*//** @file
 * @brief   Wrapper for calling TSP solvers and heuristics.
 *
 * Wrapper for solvers and heuristics for the traveling salesman problem.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "tsp.h"

/**
 *  @brief  Solves TSP with Concorde's Lin-Kernighan heuristic.
 *  @param  tsp_solver [in] the TSP solver to use
 *  @param  problem [in] problem to solve
 *  @param  params [in] Lin-Kernighan params (can be NULL)
 *  @param  result [out] TSP solution
 */                  
int
arrow_tsp_solve(int tsp_solver, arrow_problem *problem, void *params,
                      arrow_tsp_result *result)
{
    switch(tsp_solver)
    {
        case ARROW_TSP_CC_EXACT:
            return arrow_tsp_cc_exact_solve(problem, (int *)params, result);
            break;
        case ARROW_TSP_CC_LK:
            return arrow_tsp_cc_lk_solve(problem, 
                                         (arrow_tsp_cc_lk_params *)params, 
                                         result);
            break;
        case ARROW_TSP_RAI:
            return arrow_tsp_rai_solve(problem,
                                       (arrow_tsp_rai_params *)params,
                                       result);
            break;
        default:
            arrow_print_error("TSP solver type %d not supported", tsp_solver);
            return ARROW_FAILURE;
    }
}

void
arrow_tsp_short_name(int tsp_solver, FILE *out)
{
    switch(tsp_solver)
    {
        case ARROW_TSP_CC_EXACT:
            fprintf(out, "cc_exact"); break;
        case ARROW_TSP_CC_LK:
            fprintf(out, "cc_lk"); break;
        case ARROW_TSP_RAI:
            fprintf(out, "rai"); break;            
        default:
            arrow_print_error("TSP solver type %d not supported", tsp_solver);
    }
}

void
arrow_tsp_long_name(int tsp_solver, FILE *out)
{
    switch(tsp_solver)
    {
        case ARROW_TSP_CC_EXACT:
            fprintf(out, "CC TSP Solver"); break;
        case ARROW_TSP_CC_LK:
            fprintf(out, "CC Lin-Kernighan"); break;
        case ARROW_TSP_RAI:
            fprintf(out, "RAI Solver"); break;
        default:
            arrow_print_error("TSP solver type %d not supported", tsp_solver);
    }
}