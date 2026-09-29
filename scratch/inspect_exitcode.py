from pathlib import Path

content = Path(r"C:\Espressif\frameworks\esp-idf-v5.5.5\tools\cmake\build.cmake").read_text(encoding="utf-8", errors="ignore")
lines = content.splitlines()
for idx, line in enumerate(lines):
    if "__RERUN_EXITCODE" in line:
        start = max(0, idx - 10)
        end = min(len(lines), idx + 40)
        print("\n".join(lines[start:end]))
