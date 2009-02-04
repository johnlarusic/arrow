/**********************************************************doxygen*//** @file
 * @brief   Minimal perfect hashing functions
 *
 * Methods for generating a minimal perfect hash.
 *
 * @author  John LaRusic
 * @ingroup lib
 ****************************************************************************/
#include "common.h"

int
arrow_hash_cost_list(int *cost_list, int cost_list_length, arrow_hash *hash)
{
    int i;

    hash->num_keys = (cmph_uint32)cost_list_length;
    
    /* Memory allocation for vector holding the keys */
    hash->vector = CC_SAFE_MALLOC(hash->num_keys, char *);
    hash->vector_space = 
        CC_SAFE_MALLOC(hash->num_keys * ARROW_HASH_BUFFER_LENGTH, char);
    
    /* Populate vector */
    for(i = 0; i < hash->num_keys; i++)
    {
        hash->vector[i] = hash->vector_space + i * ARROW_HASH_BUFFER_LENGTH;
        sprintf(hash->vector[i], "%d", cost_list[i]);
    }
    
    /* Create hash with CMPH */
    hash->source = cmph_io_vector_adapter((char **)hash->vector, hash->num_keys);
    cmph_config_t *config = cmph_config_new(hash->source);
  	hash->data = cmph_new(config);
  	cmph_config_destroy(config);
  	
    return ARROW_SUCCESS;
}

void
arrow_hash_destruct(arrow_hash *hash)
{
    hash->num_keys = 0;
    cmph_destroy(hash->data);
    cmph_io_vector_adapter_destroy(hash->source);
    free(hash->vector_space);
    free(hash->vector);
}

unsigned int
arrow_hash_search(arrow_hash *hash, int key)
{
    char buffer[ARROW_HASH_BUFFER_LENGTH];
    sprintf(buffer, "%d", key);
    return cmph_search(hash->data, buffer, strlen(buffer));
}


