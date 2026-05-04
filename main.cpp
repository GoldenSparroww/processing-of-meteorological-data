#include <iostream>
#include <string>
#include <chrono>
#include "include/data_loader.h"
#include "include/data_processor.h"
#include "include/output_generator.h"
#include "include/data_types.h"

int main(const int argc, const char* argv[]) {
    // Check arguments
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
        is_parallel ? std::cout << "Starting parallel version...\n" : std::cout << "Starting serial version...\n";

        // Data loading
        std::unordered_map<int, Station> stations_map = load_stations(fileStations, is_parallel);
        load_measurements(fileMeasurements, stations_map, is_parallel);

        auto stations_vector = hashmap_to_vector(stations_map);
        stations_map.clear();

        auto end_time_ = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_ = end_time_ - start_time;
        std::cout << "Data loaded in: " << elapsed_.count() << " ms\n";

        // Process data
        filter_stations(stations_vector, is_parallel);
        std::cout << stations_vector.size() << "\n";
        auto anomalies = detect_anomalies(stations_vector, is_parallel);
        export_anomalies(anomalies, OUTPUT_CSV_NAME);
        generate_maps(stations_vector, is_parallel);

        // Stop stopwatch and calculate duration
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        std::cout << "Processing completed. Total time: " << elapsed.count() << " ms\n";
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "A critical error occurred. Not enough memory. (" << e.what() << ")\n";
        return EXIT_NOT_ENOUGH_MEMORY_ERR;
    }
    catch (const std::exception& e) {
        std::cerr << "A critical error occurred. " << e.what() << '\n';
        return EXIT_GENERAL_ERR;
    }

    return EXIT_OK;
}