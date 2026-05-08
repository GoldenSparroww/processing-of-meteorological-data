#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_LOADER_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_LOADER_H

#include <string>
#include <unordered_map>
#include "data_types.h"

/**
 * @brief Loads station data from a given CSV file.
 * * @param filePath Path to the CSV file containing station definitions.
 * @param is_parallel Flag indicating whether the operation should be executed in parallel.
 * @return std::unordered_map<int, Station> Map of stations, where the key is the station ID.
 */
std::unordered_map<int, Station> load_stations(const std::string& filePath, bool is_parallel);

/**
 * @brief Loads temperature measurements from a CSV file and assigns them to the corresponding stations.
 * * @param filePath Path to the CSV file containing measurement data.
 * @param stations Reference to the map of previously loaded stations.
 * @param is_parallel Flag indicating whether the operation should be executed in parallel.
 */
void load_measurements(const std::string& filePath, std::unordered_map<int, Station>& stations, bool is_parallel);

#endif