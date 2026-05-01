import os
import subprocess
import pytest

# Central path definitions
TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_test")
EXEC_NAME = "upp_sp1.exe"

@pytest.fixture(scope="session")
def paths():
    """Provides access to project paths."""
    return {
        "root": PROJECT_ROOT,
        "tests": TESTS_DIR,
        "build": BUILD_DIR,
        "exec_name": EXEC_NAME,
        "data": os.path.join(PROJECT_ROOT, "data")
    }

@pytest.fixture(scope="session")
def compiled_executable(paths):
    """Builds the project once for the entire test session."""
    if not os.path.exists(paths["build"]):
        os.makedirs(paths["build"])

    try:
        subprocess.run(["cmake", "-S", paths["root"], "-B", paths["build"], "-G", "MinGW Makefiles"],
                       check=True, capture_output=True, text=True)
        subprocess.run(["cmake", "--build", paths["build"]],
                       check=True, capture_output=True, text=True)
    except subprocess.CalledProcessError as e:
        pytest.exit(f"CMake build failed:\n{e.stderr}")

    exec_path = None
    for root, _, files in os.walk(paths["build"]):
        if paths["exec_name"] in files:
            exec_path = os.path.join(root, paths["exec_name"])
            break

    if not exec_path:
        pytest.exit(f"Binary {paths['exec_name']} not found.")

    return exec_path