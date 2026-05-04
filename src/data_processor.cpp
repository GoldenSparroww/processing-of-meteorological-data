#include <set>
#include <map>
#include <cmath>
#include <limits>
#include "../include/data_processor.h"

// TODO
void precalculate_stations(Station& station) {
    for (const auto& m : station.measurements) {
        // Aggregate month -> year
        station.monthly_yearly_stats[m.month][m.year].sum += m.value;
        station.monthly_yearly_stats[m.month][m.year].count++;

        // Aggregate month
        station.monthly_stats[m.month].sum += m.value;
        station.monthly_stats[m.month].count++;
    }
}

void filter_stations(std::vector<Station>& stations, bool is_parallel) {
    // Mask to detect items to delete
    std::vector<char> keep(stations.size(), 0);

    #pragma omp parallel for if(is_parallel) default(none) shared(stations, keep)
    for (size_t i = 0; i < stations.size(); ++i) {
        Station& station = stations[i];

        // Set guarantees: uniqueness, descending order
        std::set<int> unique_years;
        for (const auto& m : station.measurements) {
            unique_years.insert(m.year);
        }

        if (unique_years.size() < 5) {
            continue;
        }

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
            continue;
        }

        int min_year = *unique_years.begin();
        int max_year = *unique_years.rbegin();
        int period_length = max_year - min_year + 1;

        double average = static_cast<double>(station.measurements.size()) / static_cast<double>(period_length);

        if (average < 100.0) {
            continue;
        }

        precalculate_stations(station);

        // If station passes, mark as 1 -> keep
        keep[i] = 1;
    }

    // Apply mask
    std::vector<Station> filtered;
    filtered.reserve(stations.size());

    // NOTE: Following for cycle; The parallelization didn't pay off; performance dropped by 89%
    for (size_t i = 0; i < stations.size(); ++i) {
        if (keep[i]) {
            filtered.push_back(std::move(stations[i]));
        }
    }

    stations = std::move(filtered);
}

std::vector<Anomaly> detect_anomalies(const std::vector<Station>& stations, bool is_parallel) {
    std::vector<Anomaly> anomalies;

    #pragma omp parallel for if(is_parallel) default(none) shared(stations, anomalies)
    for (size_t i = 0; i < stations.size(); ++i) {
        const Station& station = stations[i];

        // Systematically find anomalies
        for (int month = 1; month <= 12; ++month) {
            auto month_it = station.monthly_yearly_stats.find(month);
            if (month_it == station.monthly_yearly_stats.end()) continue;

            const auto& yearly_data = month_it->second;
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
                        #pragma omp critical
                        {
                            anomalies.push_back({station.id, month, curr_year, diff});
                        }
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

// TODO
std::vector<Station> hashmap_to_vector(std::unordered_map<int, Station>& map) {
    std::vector<Station> vector;
    vector.reserve(map.size());

    for (auto& pair : map) {
        vector.push_back(std::move(pair.second));
    }

    return vector;
}