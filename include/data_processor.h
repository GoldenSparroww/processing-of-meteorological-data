#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATA_PROCESSOR_H

#include <unordered_map>
#include "data_types.h"

// Remove stations which does not fulfil the needs
void filterStations(std::unordered_map<int, Station>& stations);

#endif