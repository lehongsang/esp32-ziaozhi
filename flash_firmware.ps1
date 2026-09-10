param (
    [string]$Port = "COM3"
)

$env:IDF_PATH = "C:\Espressif\frameworks\esp-idf-v5.5.5"
$env:IDF_PYTHON_ENV_PATH = "C:\Espressif\python_env\idf5.5_py3.11_env"
$env:PATH = "C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20260121\xtensa-esp-elf\bin;C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\idf-exe\1.0.3;C:\Espressif\python_env\idf5.5_py3.11_env\Scripts;C:\Espressif\frameworks\esp-idf-v5.5.5\tools;" + $env:PATH

Write-Host ">>> Dang nap firmware vao cong $Port ..." -ForegroundColor Cyan
python -m esptool --chip esp32s3 -p $Port -b 921600 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 build/merged-binary.bin
