from pathlib import Path

idf_path = Path(r"C:\Espressif\frameworks\esp-idf-v5.5.5")
for f in idf_path.rglob("*.cmake"):
    try:
        content = f.read_text(encoding="utf-8", errors="ignore")
        if "Missing required kconfig option" in content:
            print(f"Found in {f}")
            for idx, line in enumerate(content.splitlines()):
                if "Missing" in line:
                    start = max(0, idx - 15)
                    end = min(len(content.splitlines()), idx + 15)
                    print("\n".join(content.splitlines()[start:end]))
    except Exception as e:
        pass
