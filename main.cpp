#include <iostream>
#include <string>
#include <chrono>
#include "include/data_loader.h"
#include "include/data_processor.h"
#include "include/output_generator.h"
#include "include/data_types.h"

int main(const int argc, const char* argv[]) {
    // Check number of arguments
    if (argc != 4) {
        std::cerr << "Error: Invalid number of arguments.\n";
        std::cerr << "Usage: ./upp_sp1 <stations.csv> <measurements.csv> <--serial | --parallel>\n";
        return EXIT_INVALID_ARGS_ERR;
    }

    const std::string fileStations = argv[1];
    const std::string fileMeasurements = argv[2];
    bool useParallel = false;

    // Check mode flag
    const std::string flag = argv[3];
    if (flag == "-s" || flag == "--serial") {
        useParallel = false;
    } else if (flag == "-p" || flag == "--parallel") {
        useParallel = true;
    } else {
        std::cerr << "Error: Invalid flag '" << flag << "'. Expected --serial or --parallel.\n";
        return EXIT_INVALID_ARGS_ERR;
    }

    try {
        // Start stopwatch
        auto start_time = std::chrono::high_resolution_clock::now();

        // Data loading (common for both versions)
        auto stations = loadStations(fileStations);
        loadMeasurements(fileMeasurements, stations);

        if (useParallel) {
            std::cout << "Starting parallel version...\n";
        } else {
            std::cout << "Starting serial version...\n";
            filterStations(stations);
            auto anomalies = detectAnomalies(stations);
            exportAnomalies(anomalies, "vykyvy.csv");
            generateMaps(stations, "/mnt/c/Users/Rayaxir/Desktop/czmap.svg");
        }

        // Stop stopwatch and calculate duration
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

        std::cout << "Processing completed. Total time: " << elapsed.count() << " ms\n";

    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << '\n';
        return EXIT_GENERAL_ERR;
    }

    return EXIT_OK;
}