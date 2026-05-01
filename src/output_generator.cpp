#include <fstream>
#include <stdexcept>
#include <limits>
#include <iomanip>
#include <iostream>
#include "../include/output_generator.h"
#include "../include/czmap_svg.h"

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

void generate_maps(const std::unordered_map<int, Station>& stations) {
    std::string svg_template = CZ_MAP_SVG_TEMPLATE;

    // Remove </svg>, so we can add elements
    size_t pos = svg_template.rfind("</svg>");
    if (pos != std::string::npos) {
        svg_template.erase(pos);
    } else {
        throw std::runtime_error("Invalid SVG fromat (missing </svg>)");
    }

    // 2D hashtable: station ID -> (month number -> avg. temp.)
    std::unordered_map<int, std::unordered_map<int, double>> station_monthly_averages;

    double global_min = std::numeric_limits<double>::max();
    double global_max = std::numeric_limits<double>::lowest();

    for (const auto& pair : stations) {
        const Station& st = pair.second;

        for (const auto& m : st.measurements) {
            if (m.value < global_min) global_min = m.value;
            if (m.value > global_max) global_max = m.value;
        }

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

    const double lon_min = 12.102209054269062;
    const double lon_max = 18.866923511078615;
    const double lat_max = 51.03806105663445;
    const double lat_min = 48.521003814763994;

    const double SVG_WIDTH = 1412.0;
    const double SVG_HEIGHT = 809.0;

    const std::string mesice[12] = {
        "leden", "unor", "brezen", "duben", "kveten", "cerven",
        "cervenec", "srpen", "zari", "rijen", "listopad", "prosinec"
    };

    // Generate maps for each month
    for (int month = 1; month <= 12; ++month) {
        std::string filename = mesice[month - 1] + ".svg";
        std::ofstream out_file(filename, std::ios::binary);

        out_file << svg_template; // Write basic map without closing tag

        // (stations) points writing
        for (const auto& pair : stations) {
            const Station& st = pair.second;

            if (station_monthly_averages[st.id].find(month) == station_monthly_averages[st.id].end()) {
                continue;
            }

            double avg_temp = station_monthly_averages[st.id][month];

            double x = ((st.longitude - lon_min) / (lon_max - lon_min)) * SVG_WIDTH;
            double y = ((lat_max - st.latitude) / (lat_max - lat_min)) * SVG_HEIGHT;

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