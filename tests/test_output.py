import os
import subprocess
import filecmp
import tempfile
import pytest

EXPECTED_FILES = [
    "vykyvy.csv", "leden.svg", "unor.svg", "brezen.svg", "duben.svg",
    "kveten.svg", "cerven.svg", "cervenec.svg", "srpen.svg",
    "zari.svg", "rijen.svg", "listopad.svg", "prosinec.svg"
]

DATASETS = {
    "small": "meteodata",
    "medium": "meteodata_medium",
    "large": "meteodata_large"
}

def run_and_verify(exec_path, paths, dataset_size, mode):
    folder_name = DATASETS[dataset_size]
    stations_csv = os.path.join(paths["data"], folder_name, "stanice.csv")
    measurements_csv = os.path.join(paths["data"], folder_name, "mereni.csv")
    ref_dir = os.path.join(paths["tests"], "results", folder_name)

    with tempfile.TemporaryDirectory() as temp_out_dir:
        command = [exec_path, stations_csv, measurements_csv, mode]
        process = subprocess.run(command, cwd=temp_out_dir, capture_output=True, text=True)

        assert process.returncode == 0, f"Runtime error: {process.stderr}"

        for filename in EXPECTED_FILES:
            generated = os.path.join(temp_out_dir, filename)
            reference = os.path.join(ref_dir, filename)

            assert os.path.exists(generated), f"Missing output: {filename}"
            assert filecmp.cmp(generated, reference, shallow=False), f"File mismatch: {filename}"

@pytest.mark.serial
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_serial(compiled_executable, paths, size):
    run_and_verify(compiled_executable, paths, size, "--serial")

@pytest.mark.parallel
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_parallel(compiled_executable, paths, size):
    run_and_verify(compiled_executable, paths, size, "--parallel")