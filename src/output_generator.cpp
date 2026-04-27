#include <fstream>
#include <stdexcept>
#include "../include/output_generator.h"

void exportAnomalies(const std::vector<Anomaly>& anomalies, const std::string& filePath) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file for output CSV: " + filePath);
    }

    // Header
    file << "station_id;month;year;diff\n";

    // Data
    for (const auto& anomaly : anomalies) {
        file << anomaly.station_id << ";"
             << anomaly.month << ";"
             << anomaly.year2 << ";"
             << anomaly.diff << "\n";
    }
}