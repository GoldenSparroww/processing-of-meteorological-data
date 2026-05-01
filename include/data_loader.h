#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_LOADER_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_LOADER_H

#include <string>
#include <unordered_map>
#include "data_types.h"

// Loads stations data from a given filepath and returns them in hashmap (key is station ID)
std::unordered_map<int, Station> load_stations(const std::string& filePath);

// Loads measurements data from a given filepath and assigns them to leaded stations
void load_measurements(const std::string& filePath, std::unordered_map<int, Station>& stations);

#endif