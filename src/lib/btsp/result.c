/**********************************************************doxygen*//** @file
 * @brief   BTSP result structure and methods
 *
 * Bottleneck traveling salesman problem results structure.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"
#include "btsp.h"

/****************************************************************************
 * Public function implementations
 ****************************************************************************/
int
arrow_btsp_result_init(arrow_problem *problem, arrow_btsp_result *result)
{
    int i;
    if(!arrow_util_create_int_array(problem->size, &(result->tour)))
    {
        result = NULL;
        return ARROW_FAILURE;
    }
    result->found_tour = ARROW_FALSE;
    result->obj_value = INT_MAX;
    result->tour_length = DBL_MAX;
    result->bin_search_steps = 0;
    
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        result->solver_attempts[i] = 0;
        result->solver_time[i] = 0.0;
    }
    
    result->total_time = 0.0;
    return ARROW_SUCCESS;
}

void 
arrow_btsp_result_destruct(arrow_btsp_result *result)
{
    if(result->tour != NULL)
        free(result->tour);
}

void
arrow_btsp_result_print_xml(arrow_btsp_result *result, FILE *out)
{
    int i;
    double avg_time;
    
    arrow_xml_element_bool("found_tour", result->found_tour, out);

    if(result->found_tour)
    {
        arrow_xml_element_int("objective_value", result->obj_value, out);
        arrow_xml_element_double("tour_length", result->tour_length, out);
    }
    else
    {
        arrow_xml_element_int("objective_value", -1, out);
        arrow_xml_element_double("tour_length", -1.0, out);
    }
    arrow_xml_element_bool("optimal", result->optimal, out);
    arrow_xml_element_int("bin_search_steps", result->bin_search_steps, out);

    arrow_xml_element_open("solver_info", out);
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        if(result->solver_attempts[i] > 0)
        {
            arrow_xml_element_start("solver", out);
            
            arrow_xml_attribute_int("solver_type", i, out);
            arrow_xml_attribute_start("solver_name", out);
            arrow_tsp_short_name(i, out);
            arrow_xml_attribute_end(out);
            arrow_xml_element_end(out);
            
            avg_time = result->solver_time[i] 
                / (result->solver_attempts[i] * 1.0);
            arrow_xml_element_int("attempts", result->solver_attempts[i], 
                                  out);
            arrow_xml_element_double("avg_time", avg_time, out);            
            
            arrow_xml_element_close("solver", out);
        }
    }
    arrow_xml_element_close("solver_info", out);
    
    arrow_xml_element_double("btsp_total_time", result->total_time, out);
}

void
arrow_btsp_result_print_pretty(arrow_btsp_result *result, FILE *out)
{
    int i;
    double avg_time;
    
    fprintf(out, "Found Tour: %s\n", (result->found_tour ? "Yes" : "No"));
    if(result->found_tour)
    {
        fprintf(out, "Max. Cost: %d\n", result->obj_value);
        fprintf(out, "Tour Length: %.0f\n", result->tour_length);
    }
    
    fprintf(out, "Optimal?: %s\n", 
            (result->optimal == ARROW_TRUE ? "Yes" : "???"));
    fprintf(out, "Binary Search Steps: %d\n", result->bin_search_steps);
    
    fprintf(out, "Solver Information:\n");
    for(i = 0; i < ARROW_TSP_SOLVER_COUNT; i++)
    {
        if(result->solver_attempts[i] > 0)
        {
            avg_time = result->solver_time[i] / 
                (result->solver_attempts[i] * 1.0);
            
            fprintf(out, " - ");
            arrow_tsp_long_name(i, out);
            fprintf(out, "\n");
            fprintf(out, "   - Calls: %d\n", result->solver_attempts[i]);
            fprintf(out, "   - Avg Time: %.2f\n", avg_time);
        }
    }
    
    fprintf(out, "Total BTSP Time: %.2f\n", result->total_time);
}
