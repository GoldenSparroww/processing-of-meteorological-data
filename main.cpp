#include <iostream>
#include <string>
#include <chrono>
#include "include/data_loader.h"
#include "include/data_processor.h"
#include "include/output_generator.h"
#include "include/data_types.h"

/**
 * @brief Simple RAII timer class used for tracking execution time of code blocks.
 * * When a Timer object is created, it records the start time and prints a start message.
 * When it goes out of scope (is destroyed), it calculates the elapsed time and
 * prints the total duration in milliseconds.
 */
class Timer {
    std::string label;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

public:
    Timer(std::string l) : label(std::move(l)), start_time(std::chrono::high_resolution_clock::now()) {
        std::cout << "<< Starting " << label << "... >>" << std::endl;
    }

    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        std::cout << "<</ " << elapsed.count() << " ms >>\n";
    }
};

/**
 * @brief Main entry point of the application.
 * * This function coordinates the entire meteorological data processing pipeline:
 * 1. Validates command-line arguments.
 * 2. Loads stations and their temperature measurements from CSV files.
 * 3. Filters stations based on data continuity and frequency.
 * 4. Detects inter-annual temperature anomalies.
 * 5. Exports results to a CSV file and generates monthly SVG maps.
 * * Execution can be switched between serial and parallel modes (using OpenMP)
 * via a command-line flag.
 * * @param argc The count of command-line arguments.
 * @param argv The array of command-line arguments:
 * - argv[1]: Path to the stations CSV file.
 * - argv[2]: Path to the measurements CSV file.
 * - argv[3]: Mode flag (--serial or --parallel).
 * @return int Returns EXIT_OK (0) on success, or a specific error code from the ErrorCode enum.
 */
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
        std::unordered_map<int, Station> stations_map;
        {
            Timer timer("stations");
            stations_map = load_stations(fileStations, is_parallel);
        }
        {
            Timer timer("measurements");
            load_measurements(fileMeasurements, stations_map, is_parallel);
        }

        auto stations_vector = hashmap_to_vector(stations_map);
        stations_map.clear();

        // Process data
        {
            Timer timer("filter");
            filter_stations(stations_vector, is_parallel);

        }
        std::vector<Anomaly> anomalies;
        {
            Timer timer("anomalies");
            anomalies = detect_anomalies(stations_vector, is_parallel);
        }
        {
            Timer timer("export anomalies");
            export_anomalies(anomalies, OUTPUT_CSV_NAME);
        }
        {
            Timer timer("maps");
            generate_maps(stations_vector, is_parallel);
        }

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