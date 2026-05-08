#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H

#include <unordered_map>
#include "data_types.h"

/**
 * @brief Filters out stations that do not meet the minimum data requirements.
 * * A station is kept only if it has at least 5 consecutive years of measurements
 * and averages at least 100 measurements per year within its active period.
 * * @param stations Reference to the vector of stations to be filtered (modified in place).
 * @param is_parallel Flag indicating whether the operation should be executed in parallel.
 */
void filter_stations(std::vector<Station>& stations, bool is_parallel);

/**
 * @brief Detects inter-annual temperature anomalies across all provided stations.
 * * An anomaly is defined as a year-to-year difference in the average monthly temperature
 * that exceeds 75% of the range between the historical maximum and minimum average
 * temperatures for that specific month at that station.
 * * @param stations Vector of valid stations with precalculated statistics.
 * @param is_parallel Flag indicating whether the operation should be executed in parallel.
 * @return std::vector<Anomaly> A collection of all detected anomalies.
 */
std::vector<Anomaly> detect_anomalies(const std::vector<Station>& stations, bool is_parallel);

/**
 * @brief Converts an unordered_map of stations into a contiguous vector.
 * * This operation moves the elements, leaving the original map in an empty/moved-from state.
 * * @param map The hashmap of stations to be converted.
 * @return std::vector<Station> A vector containing all stations from the map.
 */
std::vector<Station> hashmap_to_vector(std::unordered_map<int, Station>& map);

#endif