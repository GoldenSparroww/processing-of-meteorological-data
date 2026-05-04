#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <omp.h>
#include "../include/data_loader.h"
#include "../include/data_types.h"

namespace {
    // Read file into std::string buffer
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

    // Parse station record from buffer
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

    // Parse measurement record from buffer
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

std::unordered_map<int, Station> load_stations(const std::string& filePath) {
    std::unordered_map<int, Station> stations;
    std::string buffer = read_file_to_buffer(filePath);
    size_t end_idx = buffer.size();

    // Skip header
    size_t current_pos = buffer.find('\n');
    current_pos = (current_pos != std::string::npos) ? current_pos + 1 : end_idx;

    // Iterate through lines
    while (current_pos < end_idx) {
        Station station = parse_station_line(buffer, current_pos, end_idx);
        stations[station.id] = station;
    }

    return stations;
}

void load_measurements(const std::string& filePath, std::unordered_map<int, Station>& stations) {
    std::string buffer = read_file_to_buffer(filePath);
    size_t end_idx = buffer.size();

    // Skip header
    size_t current_pos = buffer.find('\n');
    current_pos = (current_pos != std::string::npos) ? current_pos + 1 : end_idx;

    // Iterate through lines
    while (current_pos < end_idx) {
        int parsed_station_id;
        Measurement m;
        parse_measurement_line(buffer, current_pos, end_idx, parsed_station_id, m);

        auto it = stations.find(parsed_station_id);
        if (it != stations.end()) {
            it->second.measurements.push_back(m);
        }
    }
}

std::unordered_map<int, Station> load_stations_parallel(const std::string& filePath) {
    std::unordered_map<int, Station> stations;
    std::string buffer = read_file_to_buffer(filePath);

    int num_threads = omp_get_max_threads();
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

    #pragma omp parallel default(none) shared(buffer, chunk_boundaries, stations)
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

void load_measurements_parallel(const std::string& filePath, std::unordered_map<int, Station>& stations) {
    std::string buffer = read_file_to_buffer(filePath);

    int num_threads = omp_get_max_threads();
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

    #pragma omp parallel default(none) shared(buffer, chunk_boundaries, stations)
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