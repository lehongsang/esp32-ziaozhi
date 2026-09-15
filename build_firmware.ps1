$env:IDF_PATH = "C:\Espressif\frameworks\esp-idf-v5.5.5"
$env:IDF_PYTHON_ENV_PATH = "C:\Espressif\python_env\idf5.5_py3.11_env"
$env:PYTHONIOENCODING = "utf-8"
$env:PATH = "C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin;C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\idf-exe\1.0.3;C:\Espressif\python_env\idf5.5_py3.11_env\Scripts;C:\Espressif\frameworks\esp-idf-v5.5.5\tools;" + $env:PATH

python scripts/build.py freenove-esp32s3-display-2.8-lcd --language vi-VN $args
