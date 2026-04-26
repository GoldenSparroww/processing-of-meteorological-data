#ifndef PROCESSING_OF_METEOROLOGICAL_DATA_DATALOADER_H
#define PROCESSING_OF_METEOROLOGICAL_DATA_DATALOADER_H

#include <string>
#include <unordered_map>
#include "data_types.h"

// Nacte stanice ze souboru a vrati je v mape (klic je id stanice)
std::unordered_map<int, Station> loadStations(const std::string& filePath);

// Nacte mereni ze souboru a priradi je k jiz nactenym stanicim
void loadMeasurements(const std::string& filePath, std::unordered_map<int, Station>& stations);

#endif