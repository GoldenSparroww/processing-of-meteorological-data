#include <set>
#include <map>
#include <cmath>
#include <limits>
#include "../include/data_processor.h"

void filterStations(std::unordered_map<int, Station>& stations) {
    for (auto it = stations.begin(); it != stations.end(); ) {
        const Station& station = it->second;

        // Set guarantees, that years are going to be sorted (ascending)
        std::set<int> unique_years;
        for (const auto& m : station.measurements) {
            unique_years.insert(m.year);
        }

        // If station got less than 5 years of measurements, then end (fast check)
        if (unique_years.size() < 5) {
            it = stations.erase(it);
            continue;
        }

        // Check if those 5 years were continuous
        int max_continuous = 1;
        int current_continuous = 1;
        int prev_year = -1;

        for (int year : unique_years) {
            if (prev_year != -1) {
                if (year == prev_year + 1) {
                    current_continuous++;
                } else {
                    if (current_continuous > max_continuous) {
                        max_continuous = current_continuous;
                    }
                    current_continuous = 1;
                }
            }
            prev_year = year;
        }
        if (current_continuous > max_continuous) {
            max_continuous = current_continuous;
        }

        if (max_continuous < 5) {
            it = stations.erase(it);
            continue;
        }

        // Check if station had at least an average of 100 mesurements per year (>= 100)
        double average = static_cast<double>(station.measurements.size()) / static_cast<double>(unique_years.size());

        if (average < 100.0) {
            it = stations.erase(it);
            continue;
        }

        // If station succeed, move to next
        ++it;
    }
}

std::vector<Anomaly> detectAnomalies(const std::unordered_map<int, Station>& stations) {
    std::vector<Anomaly> anomalies;

    struct SumCount {
        double sum = 0.0;   // Sum of values for this MONTH and this YEAR
        int count = 0;      // Count of measurements of this MONTH and this YEAR
    };

    for (const auto& pair : stations) {
        const Station& station = pair.second;

        // MONTH -> (YEAR -> SUM and COUNT)
        // (map makes it in order)
        std::unordered_map<int, std::map<int, SumCount>> monthly_stats;

        // Just aggregate
        for (const auto& m : station.measurements) {
            monthly_stats[m.month][m.year].sum += m.value;
            monthly_stats[m.month][m.year].count++;
        }

        // Systematicly find anomalies
        for (int month = 1; month <= 12; ++month) {
            // If month measurement data does not exist, return .end()
            if (monthly_stats.find(month) == monthly_stats.end()) continue;

            const auto& yearly_data = monthly_stats[month];
            if (yearly_data.size() < 2) continue;   // Cannot compare

            std::map<int, double> yearly_averages;
            double min_avg = std::numeric_limits<double>::max();
            double max_avg = std::numeric_limits<double>::lowest();

            // avg/min/max
            for (const auto& year_pair : yearly_data) {
                int year = year_pair.first;
                double avg = year_pair.second.sum / year_pair.second.count;
                yearly_averages[year] = avg;

                if (avg < min_avg) min_avg = avg;
                if (avg > max_avg) max_avg = avg;
            }

            double threshold = 0.75 * (max_avg - min_avg);

            // Anomalies from year to year
            auto it = yearly_averages.begin();
            int prev_year = it->first;
            double prev_avg = it->second;
            ++it;

            while (it != yearly_averages.end()) {
                int curr_year = it->first;
                double curr_avg = it->second;

                // Years must be next to each other, if they are not, they are skipped
                if (curr_year == prev_year + 1) {
                    double diff = std::abs(curr_avg - prev_avg);

                    if (diff > threshold) {
                        anomalies.push_back({station.id, month, curr_year, diff});
                    }
                }
                prev_year = curr_year;
                prev_avg = curr_avg;
                ++it;
            }
        }
    }

    return anomalies;
}