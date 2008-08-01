/**********************************************************doxygen*//** @file
 * @brief   Useful functions for writing XML.
 *
 * Useful functions for writing xml.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

/****************************************************************************
 * Public function implemenations
 ****************************************************************************/
inline void
arrow_xml_element_int(char *name, int value, FILE *out)
{
    fprintf(out, "<%s>%d</%s>\n", name, value, name);
}

inline void
arrow_xml_element_double(char *name, double value, FILE *out)
{
    fprintf(out, "<%s>%.5f</%s>\n", name, value, name);
}

inline void
arrow_xml_element_string(char *name, char *value, FILE *out)
{
    fprintf(out, "<%s>%s</%s>\n", name, value, name);
}

inline void
arrow_xml_element_bool(char *name, int value, FILE *out)
{
    fprintf(out, "<%s>%d</%s>\n", name, (value ? 1 : 0), name);
}

inline void
arrow_xml_attribute_int(char *name, int value, FILE *out)
{
    fprintf(out, " %s=\"%d\"", name, value);
}

inline void
arrow_xml_attribute_string(char *name, char *value, FILE *out)
{
    fprintf(out, " %s=\"%s\"", name, value);
}

inline void
arrow_xml_element_start(char *name, FILE *out)
{
    fprintf(out, "<%s", name);
}

inline void
arrow_xml_element_end(FILE *out)
{
    fprintf(out, ">\n");
}

inline void
arrow_xml_element_open(char *name, FILE *out)
{
    fprintf(out, "<%s>\n", name);
}

inline void
arrow_xml_element_close(char *name, FILE *out)
{
    fprintf(out, "</%s>\n", name);
}

inline void
arrow_xml_attribute_start(char *name, FILE *out)
{
    fprintf(out, " %s=\"", name);
}

inline void
arrow_xml_attribute_end(FILE *out)
{
    fprintf(out, "\"");
}
