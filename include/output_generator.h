#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H

#include <vector>
#include <string>
#include "data_types.h"

constexpr std::string OUTPUT_CSV_NAME = "vykyvy.csv";

// Write anomalies into new CSV file
void export_anomalies(const std::vector<Anomaly>& anomalies, const std::string& filePath);

// Generate 12 SVG maps
void generate_maps(const std::vector<Station>& stations, bool is_parallel);

#endif