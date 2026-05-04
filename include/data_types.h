#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_TYPES_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

struct Measurement {
    int year;
    int month;
    int day;
    double value;
};

// Helper struct for aggregation
struct MeasurementStats {
    double sum = 0.0;
    int count = 0;
};

struct Station {
    int id;
    std::string name;
    double latitude;
    double longitude;
    std::vector<Measurement> measurements;

    // Precalculate data for anomalies detection (Month -> (Year -> Stats))
    std::unordered_map<int, std::map<int, MeasurementStats>> monthly_yearly_stats;
    // Precalculate statistics needed later form map generation (Month -> Stats)
    std::unordered_map<int, MeasurementStats> monthly_stats;
};

struct Anomaly {
    int station_id;
    int month;
    int year2;
    double diff;
};

enum ErrorCode {
    EXIT_OK = EXIT_SUCCESS,
    EXIT_INVALID_ARGS_ERR = 1,
    EXIT_INPUT_FILE_ERR = 2,
    EXIT_NOT_ENOUGH_MEMORY_ERR = 3,
    EXIT_GENERAL_ERR = 100,
};

#endif