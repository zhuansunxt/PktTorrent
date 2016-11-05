/**
 * @file io.h
 * @brief io related functions.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef IO_H
#define IO_H

#include "core/global.h"

/**
 * @brief assemble chunks requested in the current session.
 * @param master_data_file Data source.
 * @param master_chunk_map hash -> id in master_data_file
 * @param chunk_map Requested chunks in the form of hash -> user specified id.
 * @param output_file Output file.
 * @param g Global state.
 *
 * Assumes all the data is already filled into
 *   g->g_config->chunks->master_data_file.
 * Pick input id's (offset) by looking up g->g_config->master_chunk_map
 * Pick output id's (offset) by looking up g->g_session->chunk_map.
 * Fill assembled data into g->g_sesion->output_file.
 */
void assemble_chunks(const char* master_data_file, const map_t master_chunk_map,
                     const char* output_file, const map_t chunk_map) ;

#endif // IO_H
