#include <iostream>
#include <string>
#include <chrono>
#include "include/data_loader.h"
#include "include/data_processor.h"
#include "include/output_generator.h"
#include "include/data_types.h"

/**
 * Test method
 * Currently only for debugging purpose (checking files somehow manually)
 * @param anomalies
 */
void deterministic_sort(std::vector<Anomaly>& anomalies) {
    std::sort(anomalies.begin(), anomalies.end(), [](const Anomaly& a, const Anomaly& b) {
        if (a.station_id != b.station_id) {
            return a.station_id < b.station_id;
        }
        if (a.month != b.month) {
            return a.month < b.month;
        }
        return a.year2 < b.year2;
    });
}

int main(const int argc, const char* argv[]) {
    // Check number of arguments
    if (argc != 4) {
        std::cerr << "Error: Invalid number of arguments.\n";
        std::cerr << "Usage: ./upp_sp1 <stations.csv> <measurements.csv> <--serial | --parallel>\n";
        return EXIT_INVALID_ARGS_ERR;
    }

    const std::string fileStations = argv[1];
    const std::string fileMeasurements = argv[2];
    bool is_parallel = false;

    // Check mode flag
    const std::string flag = argv[3];
    if (flag == "-s" || flag == "--serial") {
        is_parallel = false;
    } else if (flag == "-p" || flag == "--parallel") {
        is_parallel = true;
    } else {
        std::cerr << "Error: Invalid flag '" << flag << "'. Expected --serial or --parallel.\n";
        return EXIT_INVALID_ARGS_ERR;
    }

    try {
        // Start stopwatch
        auto start_time = std::chrono::high_resolution_clock::now();

        // Data loading (common for both versions)
        auto stations = load_stations(fileStations);
        load_measurements(fileMeasurements, stations);

        if (is_parallel) {
            std::cout << "Starting parallel version...\n";
        } else {
            std::cout << "Starting serial version...\n";
        }

        // Process data
        filter_stations(stations, is_parallel);
        std::cout << stations.size() << "\n";
        auto anomalies = detect_anomalies(stations, is_parallel);
        deterministic_sort(anomalies);
        export_anomalies(anomalies, "vykyvy.csv");
        generate_maps(stations, is_parallel);

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