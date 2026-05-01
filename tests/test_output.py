import os
import subprocess
import filecmp
import tempfile
import unittest

# Absolute paths
TESTS_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(TESTS_DIR)
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_test")
EXEC_NAME = "upp_sp1.exe"

# Expected output files
EXPECTED_FILES = [
    "vykyvy.csv",
    "leden.svg", "unor.svg", "brezen.svg", "duben.svg",
    "kveten.svg", "cerven.svg", "cervenec.svg", "srpen.svg",
    "zari.svg", "rijen.svg", "listopad.svg", "prosinec.svg"
]

# Dataset mapping
DATASETS = {
    "small": "meteodata",
    "medium": "meteodata_medium",
    "large": "meteodata_large"
}

class TestMeteorologicalData(unittest.TestCase):
    exec_path = None

    @classmethod
    def setUpClass(cls):
        """Runs once before all tests to configure and build the C++ code via CMake."""
        print("\n[+] Spouštím překlad C++ programu přes CMake...")

        if not os.path.exists(BUILD_DIR):
            os.makedirs(BUILD_DIR)

        config_cmd = ["cmake", "-S", PROJECT_ROOT, "-B", BUILD_DIR, "-G", "MinGW Makefiles"]
        try:
            subprocess.run(config_cmd, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print(f"\n[!] CHYBA PŘI KONFIGURACI CMAKE:\n{e.stderr}\n{e.stdout}")
            raise

        build_cmd = ["cmake", "--build", BUILD_DIR]
        try:
            subprocess.run(build_cmd, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print(f"\n[!] CHYBA PŘI KOMPILACI C++ KÓDU:\n{e.stderr}\n{e.stdout}")
            raise

        build_cmd = ["cmake", "--build", BUILD_DIR]
        subprocess.run(build_cmd, check=True, stdout=subprocess.DEVNULL)

        # Locate binary (cross-build system compatibility)
        for root, _, files in os.walk(BUILD_DIR):
            if EXEC_NAME in files:
                cls.exec_path = os.path.join(root, EXEC_NAME)
                break

        if not cls.exec_path or not os.path.exists(cls.exec_path):
            raise FileNotFoundError(f"Kompilace byla dokončena, ale spustitelný soubor {EXEC_NAME} nebyl nalezen ve složce {BUILD_DIR}.")

        print(f"[+] Překlad úspěšný. Binární soubor: {cls.exec_path}")

    def run_and_verify(self, dataset_size: str, mode: str):
        """Executes the compiled program and verifies its outputs."""
        folder_name = DATASETS[dataset_size]

        stanice_csv = os.path.join(PROJECT_ROOT, "data", folder_name, "stanice.csv")
        mereni_csv = os.path.join(PROJECT_ROOT, "data", folder_name, "mereni.csv")
        ref_dir = os.path.join(TESTS_DIR, "results", folder_name)

        self.assertTrue(os.path.exists(stanice_csv), f"Vstupní soubor nenalezen: {stanice_csv}")
        self.assertTrue(os.path.exists(mereni_csv), f"Vstupní soubor nenalezen: {mereni_csv}")
        self.assertTrue(os.path.exists(ref_dir), f"Referenční složka nenalezena: {ref_dir}")

        # Run isolated in a temp directory
        with tempfile.TemporaryDirectory() as temp_out_dir:
            command = [self.exec_path, stanice_csv, mereni_csv, mode]

            process = subprocess.run(command, cwd=temp_out_dir, capture_output=True, text=True)

            self.assertEqual(
                process.returncode, 0,
                f"Chyba běhu (Mód: {mode}, Data: {dataset_size}).\nStderr:\n{process.stderr}"
            )

            for filename in EXPECTED_FILES:
                generated_file = os.path.join(temp_out_dir, filename)
                reference_file = os.path.join(ref_dir, filename)

                self.assertTrue(
                    os.path.exists(generated_file),
                    f"Soubor nebyl vygenerován: {filename}"
                )
                self.assertTrue(
                    os.path.exists(reference_file),
                    f"Referenční soubor neexistuje: {reference_file}"
                )

                self.assertTrue(
                    filecmp.cmp(generated_file, reference_file, shallow=False),
                    f"Soubor {filename} se neshoduje s referencí (Mód: {mode}, Data: {dataset_size})."
                )

    # Serial version tests
    def test_serial_small(self):
        self.run_and_verify("small", "--serial")

    def test_serial_medium(self):
        self.run_and_verify("medium", "--serial")

    def test_serial_large(self):
        self.run_and_verify("large", "--serial")

    # Parallel version tests
    def test_parallel_small(self):
        self.run_and_verify("small", "--parallel")

    def test_parallel_medium(self):
        self.run_and_verify("medium", "--parallel")

    def test_parallel_large(self):
        self.run_and_verify("large", "--parallel")

if __name__ == '__main__':
    unittest.main(verbosity=2)