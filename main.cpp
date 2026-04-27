#include <iostream>
#include <string>

#include "data_processor.h"
#include "include/data_loader.h"

int main(const int argc, const char* argv[]) {
    // Sanity check
    if (argc != 4) {
        std::cerr << "Error: Invalid arguments." << std::endl;
        std::cerr << "Usage: upp_sp1.exe <csv_stations_file1> <csv_measurement_file2> <-s|--serial | -p|--parallel>" << std::endl;
        return EXIT_INVALID_ARGS_ERR;
    }

    const std::string file1 = argv[1];
    const std::string file2 = argv[2];
    bool useParallel = false;

    // Serial/parallel flag check
    const std::string flag = argv[3];
    if (flag == "-s" || flag == "--serial") {
        useParallel = false;
    } else if (flag == "-p" || flag == "--parallel") {
        useParallel = true;
    } else {
        std::cerr << "Error: Invalid flag '" << flag << "'. Flag is required." << std::endl;
        return EXIT_INVALID_ARGS_ERR;
    }

    // Execute logic
    try {
        auto stations = loadStations(file1);
        loadMeasurements(file2, stations);

        if (useParallel) {
            std::cout << "Executing parallel version..." << std::endl;
        } else {
            std::cout << "Executing serial version..." << std::endl;
            filterStations(stations);
            auto anomalies = detectAnomalies(stations);
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred. " << e.what() << std::endl;
        return EXIT_INPUT_FILE_ERR;
    }

    return EXIT_OK;
}
