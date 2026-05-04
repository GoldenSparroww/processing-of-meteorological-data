#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H

#include <unordered_map>
#include "data_types.h"

// Remove stations which does not fulfil the needs
void filter_stations(std::vector<Station>& stations, bool is_parallel);

// Get anomalies from stations
std::vector<Anomaly> detect_anomalies(const std::vector<Station>& stations, bool is_parallel);

// Convert hashmap of stations to vector
std::vector<Station> hashmap_to_vector(std::unordered_map<int, Station>& map);

#endif