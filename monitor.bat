@echo off
set PORT=COM3
if not "%1"=="" set PORT=%1

set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.5.5
set IDF_PYTHON_ENV_PATH=C:\Espressif\python_env\idf5.5_py3.11_env
set PATH=C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin;C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\idf-exe\1.0.3;C:\Espressif\python_env\idf5.5_py3.11_env\Scripts;C:\Espressif\frameworks\esp-idf-v5.5.5\tools;%PATH%

echo Dang mo Serial Monitor tren cong %PORT% (Nhan Ctrl+] de thoat)...
idf.py -p %PORT% monitor
