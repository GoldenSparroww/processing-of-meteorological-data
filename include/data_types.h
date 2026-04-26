#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATATYPES_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATATYPES_H

#include <string>
#include <vector>

struct Measurement {
    int year;
    int month;
    int day;
    double value;
};

struct Station {
    int id;
    std::string name;
    double latitude;
    double longitude;
    std::vector<Measurement> measurements;
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
};

#endif