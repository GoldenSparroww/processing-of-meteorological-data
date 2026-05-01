import os
import subprocess
import filecmp
import tempfile
import pytest

# Project paths
TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_test")
EXEC_NAME = "upp_sp1.exe"

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

@pytest.fixture(scope="session")
def compiled_executable():
    """Builds the C++ project via CMake once per test session."""
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    try:
        # Configure and build
        subprocess.run(["cmake", "-S", PROJECT_ROOT, "-B", BUILD_DIR, "-G", "MinGW Makefiles"],
                       check=True, capture_output=True, text=True)
        subprocess.run(["cmake", "--build", BUILD_DIR],
                       check=True, capture_output=True, text=True)
    except subprocess.CalledProcessError as e:
        pytest.exit(f"CMake build failed:\n{e.stderr}")

    # Locate binary in build directory
    exec_path = None
    for root, _, files in os.walk(BUILD_DIR):
        if EXEC_NAME in files:
            exec_path = os.path.join(root, EXEC_NAME)
            break

    if not exec_path:
        pytest.exit(f"Binary {EXEC_NAME} not found.")

    return exec_path

def run_and_verify(exec_path, dataset_size, mode):
    """Executes the binary and compares output files with references."""
    folder_name = DATASETS[dataset_size]
    stanice_csv = os.path.join(PROJECT_ROOT, "data", folder_name, "stanice.csv")
    mereni_csv = os.path.join(PROJECT_ROOT, "data", folder_name, "mereni.csv")
    ref_dir = os.path.join(TESTS_DIR, "results", folder_name)

    # Run in isolation using a temporary directory
    with tempfile.TemporaryDirectory() as temp_out_dir:
        command = [exec_path, stanice_csv, mereni_csv, mode]
        process = subprocess.run(command, cwd=temp_out_dir, capture_output=True, text=True)

        assert process.returncode == 0, f"Runtime error: {process.stderr}"

        for filename in EXPECTED_FILES:
            generated = os.path.join(temp_out_dir, filename)
            reference = os.path.join(ref_dir, filename)

            assert os.path.exists(generated), f"Missing output: {filename}"
            assert filecmp.cmp(generated, reference, shallow=False), f"Mismatch in {filename}"

@pytest.mark.serial
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_serial(compiled_executable, size):
    run_and_verify(compiled_executable, size, "--serial")

@pytest.mark.parallel
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_parallel(compiled_executable, size):
    run_and_verify(compiled_executable, size, "--parallel")