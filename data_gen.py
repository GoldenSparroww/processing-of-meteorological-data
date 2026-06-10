#!/usr/bin/env python3

import csv
import math
import random
from pathlib import Path
from datetime import date, timedelta

SEED = 42

BASE_STATIONS = "data/base_gen/stations_base_gen.csv"
BASE_MEASUREMENTS = "data/base_gen/measurements_base_gen.csv"

START_YEAR = 1990
END_YEAR = 2024

CONFIGS = {
    "small": {
        "output_dir": "data/meteodata",
        "station_multiplier": 1.0,
    },
    "medium": {
        "output_dir": "data/meteodata_medium",
        "station_multiplier": 2.5,
    },
    "large": {
        "output_dir": "data/meteodata_large",
        "station_multiplier": 7.5,
    },
}


def load_stations(path):
    stations = []

    with open(path, encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter=";")

        for row in reader:
            stations.append({
                "id": int(row["id"]),
                "name": row["name"],
                "latitude": float(row["latitude"]),
                "longitude": float(row["longitude"])
            })

    return stations


def seasonal_temperature(day_of_year, latitude):
    phase_shift = 110

    annual_cycle = (
        11.5
        + 12.5 *
        math.sin(
            2 * math.pi *
            ((day_of_year - phase_shift) / 365.25)
        )
    )

    lat_effect = -(latitude - 49.5) * 1.2

    return annual_cycle + lat_effect


def yearly_climate_shift(year):
    warming = (year - 1990) * 0.03

    rng = random.Random(year + 1000)

    yearly_noise = rng.gauss(0, 0.8)

    return warming + yearly_noise


def create_pseudo_stations(base_stations, multiplier):
    result = list(base_stations)

    if multiplier <= 1:
        return result

    target_count = int(len(base_stations) * multiplier)

    next_id = max(x["id"] for x in base_stations) + 1

    rng = random.Random(SEED)

    while len(result) < target_count:
        src = rng.choice(base_stations)

        lat_offset = rng.uniform(-0.05, 0.05)
        lon_offset = rng.uniform(-0.08, 0.08)

        result.append({
            "id": next_id,
            "name": f"{src['name']} - GEN {next_id}",
            "latitude": src["latitude"] + lat_offset,
            "longitude": src["longitude"] + lon_offset,
            "source_station": src["id"]
        })

        next_id += 1

    return result


def write_stations(stations, output_file):
    with open(output_file, "w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f, delimiter=";")

        writer.writerow([
            "id",
            "name",
            "latitude",
            "longitude"
        ])

        for s in stations:
            writer.writerow([
                s["id"],
                s["name"],
                round(s["latitude"], 6),
                round(s["longitude"], 6)
            ])


def daterange(start_date, end_date):
    current = start_date

    while current <= end_date:
        yield current
        current += timedelta(days=1)


def station_offset(station):
    seed = station["id"] * 1337

    rng = random.Random(seed)

    return rng.gauss(0, 1.8)


def generate_measurements(stations, output_file):
    start_date = date(START_YEAR, 1, 1)
    end_date = date(END_YEAR, 12, 31)

    with open(output_file, "w", encoding="utf-8", newline="") as f:

        writer = csv.writer(f, delimiter=";")

        writer.writerow([
            "station_id",
            "ordinal",
            "year",
            "month",
            "day",
            "value"
        ])

        for station in stations:

            ordinal = 0

            base_offset = station_offset(station)

            rng = random.Random(SEED + station["id"])

            for d in daterange(start_date, end_date):

                doy = d.timetuple().tm_yday

                temp = seasonal_temperature(
                    doy,
                    station["latitude"]
                )

                temp += yearly_climate_shift(d.year)

                temp += base_offset

                weather_noise = rng.gauss(0, 2.7)

                if rng.random() < 0.015:
                    weather_noise += rng.choice([
                        -8,
                        -7,
                        -6,
                        6,
                        7,
                        8
                    ])

                temp += weather_noise

                value = round(temp, 1)

                writer.writerow([
                    station["id"],
                    ordinal,
                    d.year,
                    d.month,
                    d.day,
                    value
                ])

                ordinal += 1


def build_dataset(config_name, config):

    print(f"\nGenerating {config_name}...")

    out_dir = Path(config["output_dir"])
    out_dir.mkdir(parents=True, exist_ok=True)

    base_stations = load_stations(BASE_STATIONS)

    stations = create_pseudo_stations(
        base_stations,
        config["station_multiplier"]
    )

    stations_file = out_dir / "stations.csv"
    measurements_file = out_dir / "measurements.csv"

    write_stations(stations, stations_file)

    generate_measurements(
        stations,
        measurements_file
    )

    station_size = stations_file.stat().st_size / 1024
    measurement_size = measurements_file.stat().st_size / 1024

    print(
        f"stations: {len(stations)} "
        f"({station_size:.1f} KB)"
    )

    print(
        f"measurements: {measurement_size:.1f} KB"
    )


def main():

    random.seed(SEED)

    for name, cfg in CONFIGS.items():
        build_dataset(name, cfg)

    print("\nDone.")


if __name__ == "__main__":
    main()