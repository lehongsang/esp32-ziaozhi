import subprocess
import sys

cmd = [
    r"C:\Espressif\python_env\idf5.5_py3.11_env\Scripts\python.exe",
    "-m",
    "idf_component_manager.prepare_components",
    "--project_dir=.",
    "--lock_path=dependencies.lock",
    "--sdkconfig_json_file=build/config/sdkconfig.json",
    "--interface_version=3",
    "prepare_dependencies",
    "--local_components_list_file=build/local_components_list.temp.yml",
    "--managed_components_list_file=build/managed_components_list.temp.cmake"
]

res = subprocess.run(cmd, capture_output=True, text=True)
print("Return code:", res.returncode)
print("STDOUT:", res.stdout)
print("STDERR:", res.stderr)
