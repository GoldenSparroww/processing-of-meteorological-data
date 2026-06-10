#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_OUTPUT_GENERATOR_H

#include <vector>
#include <string>
#include "data_types.h"

constexpr std::string OUTPUT_CSV_NAME = "anomalies.csv";

/**
 * @brief Exports the detected temperature anomalies to a CSV file.
 * * @param anomalies Vector of anomalies to be written to the output file.
 * @param filePath Path to the destination CSV file.
 */
void export_anomalies(const std::vector<Anomaly>& anomalies, const std::string& filePath);

/**
 * @brief Generates 12 SVG maps (one for each month) visualizing average temperatures.
 * * Stations are plotted as points on a map of the Czech Republic, colored on a scale
 * from blue (global minimum temperature) to red (global maximum temperature).
 * * @param stations Vector of stations containing measurement data.
 * @param is_parallel Flag indicating whether the map generation should run in parallel.
 */
void generate_maps(const std::vector<Station>& stations, bool is_parallel);

#endif