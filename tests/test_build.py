import subprocess
import tempfile
import pytest

@pytest.mark.build
def test_compilation_basic(paths):
    """Verify that the project can be successfully compiled."""
    with tempfile.TemporaryDirectory() as tmp_build:
        config_res = subprocess.run(
            ["cmake", "-S", paths["root"], "-B", tmp_build, "-G", "MinGW Makefiles"],
            capture_output=True, text=True
        )
        assert config_res.returncode == 0, f"Configuration failed: {config_res.stderr}"

        build_res = subprocess.run(
            ["cmake", "--build", tmp_build],
            capture_output=True, text=True
        )
        assert build_res.returncode == 0, f"Compilation failed: {build_res.stderr}"

@pytest.mark.build
def test_compilation_strict(paths):
    """Verify compilation with strict flags and no warnings."""
    with tempfile.TemporaryDirectory() as tmp_build:
        strict_flags = "-Wall -Wextra -Werror"
        config_res = subprocess.run(
            ["cmake", "-S", paths["root"], "-B", tmp_build, "-G", "MinGW Makefiles", f"-DCMAKE_CXX_FLAGS={strict_flags}"],
            capture_output=True, text=True
        )
        assert config_res.returncode == 0

        build_res = subprocess.run(
            ["cmake", "--build", tmp_build],
            capture_output=True, text=True
        )
        assert build_res.returncode == 0, f"Compilation with warnings failed:\n{build_res.stderr}"