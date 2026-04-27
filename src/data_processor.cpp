#include "../include/data_processor.h"
#include <set>

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