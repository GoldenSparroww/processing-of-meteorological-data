#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_TYPES_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

/**
 * @brief Represents a single daily temperature measurement.
 */
struct Measurement {
    int year;           ///< Year of the measurement
    int month;          ///< Month of the measurement (1-12)
    int day;            ///< Day of the measurement
    double value;       ///< Average daily temperature in Celsius
};

/**
 * @brief Helper structure used for statistical aggregation (e.g., calculating averages).
 */
struct MeasurementStats {
    double sum = 0.0;           ///< Sum of all aggregated values
    int count = 0;              ///< Number of aggregated values
};

/**
 * @brief Represents a meteorological station and all its associated data.
 */
struct Station {
    int id;                                         ///< Unique identifier of the station
    std::string name;                               ///< Name of the station
    double latitude;                                ///< Latitude coordinate
    double longitude;                               ///< Longitude coordinate
    std::vector<Measurement> measurements;          ///< List of all daily measurements

    /// Precalculated statistics for anomaly detection: Month -> (Year -> Stats)
    std::unordered_map<int, std::map<int, MeasurementStats>> monthly_yearly_stats;

    /// Precalculated overall monthly statistics for map generation: Month -> Stats
    std::unordered_map<int, MeasurementStats> monthly_stats;
};

/**
 * @brief Represents a detected inter-annual temperature anomaly.
 */
struct Anomaly {
    int station_id;     ///< ID of the station where the anomaly occurred
    int month;          ///< Month in which the anomaly was detected
    int year2;          ///< The second year of the consecutive year pair (e.g., 2001 for 2000-2001)
    double diff;        ///< The absolute temperature difference between the two years
};

/**
 * @brief Application exit codes for standardized error handling.
 */
enum ErrorCode {
    EXIT_OK = EXIT_SUCCESS,             ///< Successful execution
    EXIT_INVALID_ARGS_ERR = 1,          ///< Invalid command line arguments
    EXIT_INPUT_FILE_ERR = 2,            ///< Error reading input files
    EXIT_NOT_ENOUGH_MEMORY_ERR = 3,     ///< Memory allocation failed
    EXIT_GENERAL_ERR = 100,             ///< Unspecified general error
};

#endif