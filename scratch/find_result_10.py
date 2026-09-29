from pathlib import Path

content = Path(r"C:\Espressif\frameworks\esp-idf-v5.5.5\tools\cmake\build.cmake").read_text(encoding="utf-8", errors="ignore")
for idx, line in enumerate(content.splitlines()):
    if "10" in line or "kconf" in line.lower() or "result" in line.lower():
        if "10" in line or "confgen" in line:
            start = max(0, idx - 10)
            end = min(len(content.splitlines()), idx + 10)
            print("--- match ---")
            print("\n".join(content.splitlines()[start:end]))
