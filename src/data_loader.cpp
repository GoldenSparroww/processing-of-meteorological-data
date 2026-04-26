#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include "../include/data_loader.h"
#include "../include/data_types.h"

std::unordered_map<int, Station> loadStations(const std::string& filePath) {
    std::unordered_map<int, Station> stations;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        Station station;

        // Expected format: id;name;latitude;longitude
        std::getline(ss, token, ';');
        station.id = std::stoi(token);

        std::getline(ss, token, ';');
        station.name = token;

        std::getline(ss, token, ';');
        station.latitude = std::stod(token);

        std::getline(ss, token, ';');
        station.longitude = std::stod(token);

        stations[station.id] = station;
    }

    return stations;
}

void loadMeasurements(const std::string& filePath, std::unordered_map<int, Station>& stations) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        // Expected format: station_id;ordinal;year;month;day;value
        std::getline(ss, token, ';');
        int station_id = std::stoi(token);

        std::getline(ss, token, ';');
        // Load 'ordinal' value, but do not store anywhere

        Measurement m;
        std::getline(ss, token, ';');
        m.year = std::stoi(token);

        std::getline(ss, token, ';');
        m.month = std::stoi(token);

        std::getline(ss, token, ';');
        m.day = std::stoi(token);

        std::getline(ss, token, ';');
        m.value = std::stod(token);

        // Find station by ID and append measurement into its vector
        auto it = stations.find(station_id);
        if (it != stations.end()) {
            it->second.measurements.push_back(m);
        }
    }
}