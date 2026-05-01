#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H

#include <unordered_map>
#include "data_types.h"

// Remove stations which does not fulfil the needs
void filter_stations(std::unordered_map<int, Station>& stations);

// Get anomalies from stations
std::vector<Anomaly> detect_anomalies(const std::unordered_map<int, Station>& stations);

#endif