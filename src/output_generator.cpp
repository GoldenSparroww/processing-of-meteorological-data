#include <fstream>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <algorithm>
#include "../include/output_generator.h"
#include "../include/czmap_svg.h"

constexpr double LON_MIN = 12.102209054269062;
constexpr double LON_MAX = 18.866923511078615;
constexpr double LAT_MAX = 51.03806105663445;
constexpr double LAT_MIN = 48.521003814763994;
constexpr double SVG_WIDTH = 1412.0;
constexpr double SVG_HEIGHT = 809.0;
constexpr std::string MONTHS[12] = {
    "leden", "unor", "brezen", "duben", "kveten", "cerven",
    "cervenec", "srpen", "zari", "rijen", "listopad", "prosinec"
};
std::string SVG_TEMPLATE = CZ_MAP_SVG_TEMPLATE;

// TODO
void export_anomalies(const std::vector<Anomaly>& anomalies, const std::string& filePath) {
    std::ofstream file(filePath, std::ios::binary);

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
             << std::fixed << std::setprecision(2) << anomaly.diff << "\n";
    }
}

void generate_maps(const std::vector<Station>& stations, bool is_parallel) {
    // Remove </svg>, so we can add elements
    size_t pos = SVG_TEMPLATE.rfind("</svg>");
    if (pos != std::string::npos) {
        SVG_TEMPLATE.erase(pos);
    } else {
        throw std::runtime_error("Invalid SVG fromat (missing </svg>)");
    }

    // 2D hashtable: station ID -> (month number -> avg. temp.)
    std::unordered_map<int, std::unordered_map<int, double>> station_monthly_averages;

    double global_min = std::numeric_limits<double>::max();
    double global_max = std::numeric_limits<double>::lowest();

    #pragma omp parallel for if(is_parallel) default(none) shared(stations) reduction(min: global_min) reduction(max: global_max)
    for (size_t i = 0; i < stations.size(); ++i) {
        const Station& st = stations[i];

        for (const auto& m : st.measurements) {
            if (m.value < global_min) global_min = m.value;
            if (m.value > global_max) global_max = m.value;
        }
    }

    // TODO
    // Fill with averages
    for (const auto& st : stations) {
        for (const auto& m_pair : st.monthly_stats) {
            int month = m_pair.first;
            double avg = m_pair.second.sum / m_pair.second.count;
            station_monthly_averages[st.id][month] = avg;
        }
    }

    std::cout << "Global min: " << global_min << '\n';
    std::cout << "Global max: " << global_max << '\n';

    double temp_range = global_max - global_min;
    if (temp_range == 0) temp_range = 1; // zero division secure

    //std::ostream& out_stream = std::cout;
    // Generate maps for each month
    #pragma omp parallel for if(is_parallel) default(none) \
    shared(stations, station_monthly_averages, global_min, temp_range, \
    LON_MIN, LON_MAX, LAT_MAX, LAT_MIN, SVG_WIDTH, SVG_HEIGHT, MONTHS, SVG_TEMPLATE)
    for (int month = 1; month <= 12; ++month) {
        std::string filename = MONTHS[month - 1] + ".svg";
        std::ofstream out_file(filename, std::ios::binary);

        out_file << SVG_TEMPLATE; // Write basic map without closing tag

        // (stations) points writing
        for (const auto& st : stations) {

            if (station_monthly_averages[st.id].find(month) == station_monthly_averages[st.id].end()) {
                continue;
            }

            double avg_temp = station_monthly_averages[st.id][month];

            double x = ((st.longitude - LON_MIN) / (LON_MAX - LON_MIN)) * SVG_WIDTH;
            double y = ((LAT_MAX - st.latitude) / (LAT_MAX - LAT_MIN)) * SVG_HEIGHT;

            double t = (avg_temp - global_min) / temp_range;
            int r, g, b;

            if (t < 0.5) {
                double norm = t / 0.5;
                r = static_cast<int>(norm * 255);
                g = static_cast<int>(norm * 255);
                b = static_cast<int>((1.0 - norm) * 255);
            } else {
                double norm = (t - 0.5) / 0.5;
                r = 255;
                g = static_cast<int>((1.0 - norm) * 255);
                b = 0;
            }

            out_file << "<circle cx=\"" << x << "\" cy=\"" << y
                     << "\" r=\"3\" fill=\"rgb(" << r << "," << g << "," << b << ")\" "
                     << "stroke=\"black\" stroke-width=\"0.5\" />\n";
        }

        out_file << "</svg>";
    }
}