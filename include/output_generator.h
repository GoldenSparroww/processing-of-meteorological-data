#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H

#include <vector>
#include <string>
#include <unordered_map>
#include "data_types.h"

// Write anomalies into new CSV file
void exportAnomalies(const std::vector<Anomaly>& anomalies, const std::string& filePath);

// Generate 12 SVG maps
void generateMaps(const std::unordered_map<int, Station>& stations, const std::string& templatePath);

#endif