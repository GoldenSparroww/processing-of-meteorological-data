#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <omp.h>
#include <chrono>
#include <iostream>
#include "../include/data_loader.h"
#include "../include/data_types.h"

namespace {
    /**
     * @brief Reads the entire content of a file into a string buffer.
     * * @param filePath Path to the file to be read.
     * @return std::string A string containing the entire file content.
     * @throws std::runtime_error If the file cannot be opened or read.
     */
    inline std::string read_file_to_buffer(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer;
        buffer.resize(size);
        if (!file.read(&buffer[0], size)) {
            throw std::runtime_error("Error reading file: " + filePath);
        }
        return buffer;
    }

    /**
     * @brief Parses a single station record from a CSV string buffer.
     * * Extracts ID, name, latitude, and longitude. Advances the `current_pos`
     * to the beginning of the next line.
     * * @param buffer The string buffer containing the CSV data.
     * @param current_pos Reference to the current parsing position in the buffer (updated during parsing).
     * @param end_idx The maximum index in the buffer to parse up to (chunk boundary).
     * @return Station The parsed station object.
     */
    inline Station parse_station_line(const std::string& buffer, size_t& current_pos, size_t end_idx) {
        Station station;

        // 1. ID
        int id = 0;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            id = id * 10 + (buffer[current_pos] - '0');
            current_pos++;
        }
        current_pos++;
        station.id = id;

        // 2. Name
        std::string name;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            name += buffer[current_pos];
            current_pos++;
        }
        current_pos++;
        station.name = name;

        // 3. Latitude
        double latitude = 0.0;
        bool lat_negative = false;
        if (current_pos < end_idx && buffer[current_pos] == '-') {
            lat_negative = true;
            current_pos++;
        }
        while (current_pos < end_idx && buffer[current_pos] != '.' && buffer[current_pos] != ';') {
            latitude = latitude * 10.0 + (buffer[current_pos] - '0');
            current_pos++;
        }
        if (current_pos < end_idx && buffer[current_pos] == '.') {
            current_pos++;
            double fraction = 0.1;
            while (current_pos < end_idx && buffer[current_pos] >= '0' && buffer[current_pos] <= '9') {
                latitude += (buffer[current_pos] - '0') * fraction;
                fraction *= 0.1;
                current_pos++;
            }
        }
        if (lat_negative) {
            latitude = -latitude;
        }
        station.latitude = latitude;

        if (current_pos < end_idx && buffer[current_pos] == ';') {
            current_pos++;
        }

        // 4. Longitude
        double longitude = 0.0;
        bool lon_negative = false;
        if (current_pos < end_idx && buffer[current_pos] == '-') {
            lon_negative = true;
            current_pos++;
        }
        while (current_pos < end_idx && buffer[current_pos] != '.' && buffer[current_pos] != '\r' && buffer[current_pos] != '\n') {
            longitude = longitude * 10.0 + (buffer[current_pos] - '0');
            current_pos++;
        }
        if (current_pos < end_idx && buffer[current_pos] == '.') {
            current_pos++;
            double fraction = 0.1;
            while (current_pos < end_idx && buffer[current_pos] >= '0' && buffer[current_pos] <= '9') {
                longitude += (buffer[current_pos] - '0') * fraction;
                fraction *= 0.1;
                current_pos++;
            }
        }
        if (lon_negative) {
            longitude = -longitude;
        }
        station.longitude = longitude;

        // Skip line endings
        while (current_pos < end_idx && buffer[current_pos] != '\n') {
            current_pos++;
        }
        if (current_pos < end_idx && buffer[current_pos] == '\n') {
            current_pos++;
        }

        return station;
    }

    /**
     * @brief Parses a single measurement record from a CSV string buffer.
     * * Extracts station ID, year, month, day, and temperature value. The ordinal
     * number is skipped. Advances the `current_pos` to the beginning of the next line.
     * * @param buffer The string buffer containing the CSV data.
     * @param current_pos Reference to the current parsing position in the buffer (updated during parsing).
     * @param end_idx The maximum index in the buffer to parse up to (chunk boundary).
     * @param[out] parsed_station_id Reference where the parsed station ID will be stored.
     * @param[out] m Reference to the Measurement struct to be populated with parsed data.
     */
    inline void parse_measurement_line(const std::string& buffer, size_t& current_pos, size_t end_idx, int& parsed_station_id, Measurement& m) {
        // 1. Station ID
        parsed_station_id = 0;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            parsed_station_id = parsed_station_id * 10 + (buffer[current_pos] - '0');
            current_pos++;
        }
        current_pos++;

        // 2. Ordinal (skipped)
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            current_pos++;
        }
        current_pos++;

        // 3. Year
        int year = 0;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            year = year * 10 + (buffer[current_pos] - '0');
            current_pos++;
        }
        current_pos++;
        m.year = year;

        // 4. Month
        int month = 0;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            month = month * 10 + (buffer[current_pos] - '0');
            current_pos++;
        }
        current_pos++;
        m.month = month;

        // 5. Day
        int day = 0;
        while (current_pos < end_idx && buffer[current_pos] != ';') {
            day = day * 10 + (buffer[current_pos] - '0');
            current_pos++;
        }
        current_pos++;
        m.day = day;

        // 6. Value
        double value = 0.0;
        bool is_negative = false;
        if (current_pos < end_idx && buffer[current_pos] == '-') {
            is_negative = true;
            current_pos++;
        }
        while (current_pos < end_idx && buffer[current_pos] != '.' && buffer[current_pos] != '\r' && buffer[current_pos] != '\n') {
            value = value * 10.0 + (buffer[current_pos] - '0');
            current_pos++;
        }
        if (current_pos < end_idx && buffer[current_pos] == '.') {
            current_pos++;
            double fraction = 0.1;
            while (current_pos < end_idx && buffer[current_pos] >= '0' && buffer[current_pos] <= '9') {
                value += (buffer[current_pos] - '0') * fraction;
                fraction *= 0.1;
                current_pos++;
            }
        }
        if (is_negative) {
            value = -value;
        }
        m.value = value;

        // Skip line endings
        while (current_pos < end_idx && buffer[current_pos] != '\n') {
            current_pos++;
        }
        if (current_pos < end_idx && buffer[current_pos] == '\n') {
            current_pos++;
        }
    }
}

std::unordered_map<int, Station> load_stations(const std::string& filePath, bool is_parallel) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::unordered_map<int, Station> stations;
    std::string buffer = read_file_to_buffer(filePath);

    // Set thread count based on is_parallel
    int num_threads = is_parallel ? omp_get_max_threads() : 1;
    std::vector<size_t> chunk_boundaries(num_threads + 1, 0);

    // Calculate thread boundaries
    size_t first_newline = buffer.find('\n');
    chunk_boundaries[0] = (first_newline != std::string::npos) ? first_newline + 1 : buffer.size();
    chunk_boundaries[num_threads] = buffer.size();

    size_t chunk_size = buffer.size() / num_threads;
    for (int i = 1; i < num_threads; ++i) {
        size_t estimated_pos = i * chunk_size;
        size_t actual_pos = buffer.find('\n', estimated_pos);
        chunk_boundaries[i] = (actual_pos != std::string::npos) ? actual_pos + 1 : buffer.size();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    std::cout << "\t- stations (serial part I/O): " << elapsed.count() << " ms\n";

    #pragma omp parallel if(is_parallel) num_threads(num_threads) default(none) shared(buffer, chunk_boundaries, stations)
    {
        int thread_id = omp_get_thread_num();
        size_t current_pos = chunk_boundaries[thread_id];
        size_t end_idx = chunk_boundaries[thread_id + 1];

        std::unordered_map<int, Station> local_stations;

        // Process thread-specific chunk
        while (current_pos < end_idx) {
            Station station = parse_station_line(buffer, current_pos, end_idx);
            local_stations[station.id] = station;
        }

        // Merge results
        #pragma omp critical
        {
            for (auto& pair : local_stations) {
                stations[pair.first] = pair.second;
            }
        }
    }

    return stations;
}

void load_measurements(const std::string& filePath, std::unordered_map<int, Station>& stations, bool is_parallel) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::string buffer = read_file_to_buffer(filePath);

    // Set thread count based on is_parallel
    int num_threads = is_parallel ? omp_get_max_threads() : 1;
    std::vector<size_t> chunk_boundaries(num_threads + 1, 0);

    // Calculate thread boundaries
    size_t first_newline = buffer.find('\n');
    chunk_boundaries[0] = (first_newline != std::string::npos) ? first_newline + 1 : buffer.size();
    chunk_boundaries[num_threads] = buffer.size();

    size_t chunk_size = buffer.size() / num_threads;
    for (int i = 1; i < num_threads; ++i) {
        size_t estimated_pos = i * chunk_size;
        size_t actual_pos = buffer.find('\n', estimated_pos);
        chunk_boundaries[i] = (actual_pos != std::string::npos) ? actual_pos + 1 : buffer.size();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    std::cout << "\t- measurements (serial part I/O): " << elapsed.count() << " ms\n";

    #pragma omp parallel if(is_parallel) num_threads(num_threads) default(none) shared(buffer, chunk_boundaries, stations)
    {
        int thread_id = omp_get_thread_num();
        size_t current_pos = chunk_boundaries[thread_id];
        size_t end_idx = chunk_boundaries[thread_id + 1];

        std::unordered_map<int, std::vector<Measurement>> local_data;

        // Process thread-specific chunk
        while (current_pos < end_idx) {
            int parsed_station_id;
            Measurement m;
            parse_measurement_line(buffer, current_pos, end_idx, parsed_station_id, m);
            local_data[parsed_station_id].push_back(m);
        }

        // Merge results
        #pragma omp critical
        {
            for (auto& pair : local_data) {
                int st_id = pair.first;
                auto it = stations.find(st_id);

                if (it != stations.end()) {
                    it->second.measurements.insert(
                        it->second.measurements.end(),
                        pair.second.begin(),
                        pair.second.end()
                    );
                }
            }
        }
    }
}