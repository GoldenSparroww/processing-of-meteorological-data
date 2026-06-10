# processing-of-meteorological-data – Average Temperatures

This application processes and analyzes a dataset of average daily temperatures from meteorological stations in the Czech Republic. The input consists of two CSV files: `stanice.csv` (a list of stations with ID, name, and coordinates) and `mereni.csv` (aggregated daily measurements). The data may come in differently sized sets, with larger sets containing artificially generated stations.

## Goals

The application performs several data‑processing steps:

- **Data preprocessing**
  - removing stations with fewer than 5 consecutive years of measurements
  - removing stations with an average of fewer than 100 values per year

- **Detection of year‑to‑year anomalies**
  - an anomaly is recorded if the average temperature of a given month differs between two consecutive years by more than 75% of the range between the minimum and maximum of all values for that month
  - results are stored in `anomalies.csv` (station ID, month, year, temperature difference)

- **Computation of average monthly temperatures**
  - for each station and each month

- **Generation of SVG maps**
  - 12 maps (`january.svg` … `december.svg`) with an outline map of the Czech Republic
  - stations are drawn as points colored according to temperature (blue → yellow → red)
  - colors are linearly scaled to the global minimum and maximum temperatures
  - station coordinates are transformed using linear interpolation

## Data

Due to size constraints, no data is provided directly. However, a Python script is included that deterministically generates 3 datasets. The application can run on these datasets and can also be tested using pytest.

Before any action, it is required to generate those "pseudo-random" data, to work with. Do that, by running `data_gen.py`.

This could run for several minutes, and it will generate 3 suits of data. The output should look something like this:

```cmd
(.venv) PS C:\Users\Josh\processing-of-meteorological-data> data_gen.py"

Generating small...
stations: 512 (17.8 KB)
measurements: 156926.8 KB

Generating medium...
stations: 1280 (55.8 KB)
measurements: 397869.4 KB

Generating large...
stations: 3840 (186.3 KB)
measurements: 1221255.2 KB

Done.
```

## Running the application

> Make sure you got already generated data.

Compilation process is standard cmake execution (`mkdir build`, `cd build`, `make ..` etc.)

- You might need to install additional libraries if needed

The compiled program is executed from the command line and supports both serial and parallel versions:

```bash
upp_sp1.exe .\data\meteodata_large\stations.csv .\data\meteodata_large\measurements.csv --serial
upp_sp1.exe .\data\meteodata_large\stations.csv .\data\meteodata_large\measurements.csv --parallel
```

## Reference environment

The project is buildable and runnable in the following environments:

- Windows (+ MSVC or MinGW-gcc)
- MS Visual Studio 2022 (Community)
