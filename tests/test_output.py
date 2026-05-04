import os
import subprocess
import filecmp
import tempfile
import pytest
import xml.etree.ElementTree as ET
import csv

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

def compare_csv_unordered(file1, file2):
    """Compare two CSVs independently on order of records."""
    with open(file1, 'r', encoding='utf-8') as f1, open(file2, 'r', encoding='utf-8') as f2:
        reader1 = list(csv.reader(f1))
        reader2 = list(csv.reader(f2))
        return sorted(reader1) == sorted(reader2)

def sort_xml_node(node):
    """Recursively sort XML nodes to ensure deterministic order."""
    node[:] = sorted(node, key=lambda child: (child.tag, str(sorted(child.attrib.items()))))
    for child in node:
        sort_xml_node(child)

def compare_svg_unordered(file1, file2):
    """Compare two SVGs independently on order of XML nodes."""
    try:
        tree1 = ET.parse(file1)
        tree2 = ET.parse(file2)
        root1 = tree1.getroot()
        root2 = tree2.getroot()

        sort_xml_node(root1)
        sort_xml_node(root2)

        return ET.tostring(root1) == ET.tostring(root2)
    except Exception as e:
        print(f"Error when parsing XML: {e}")
        return False

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

            if filename.endswith(".svg"):
                is_match = compare_svg_unordered(generated, reference)
            elif filename.endswith(".csv"):
                is_match = compare_csv_unordered(generated, reference)
            else:
                is_match = filecmp.cmp(generated, reference, shallow=False)

            assert is_match, f"File mismatch: {filename}"

@pytest.mark.serial
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_serial(compiled_executable, paths, size):
    run_and_verify(compiled_executable, paths, size, "--serial")

@pytest.mark.parallel
@pytest.mark.parametrize("size", ["small", "medium", "large"])
def test_parallel(compiled_executable, paths, size):
    run_and_verify(compiled_executable, paths, size, "--parallel")