from pathlib import Path

sdk = Path("sdkconfig").read_text(encoding="utf-8")
sdk_dict = {}
for line in sdk.splitlines():
    line = line.strip()
    if not line:
        continue
    if line.startswith("# CONFIG_") and line.endswith("is not set"):
        k = line.split()[1]
        sdk_dict[k] = "n"
    elif "=" in line and not line.startswith("#"):
        k, v = line.split("=", 1)
        sdk_dict[k] = v

defaults_files = [
    "sdkconfig.defaults",
    "sdkconfig.defaults.esp32s3",
    "build/xiaozhi-build.sdkconfig.defaults"
]

for df in defaults_files:
    p = Path(df)
    if not p.exists():
        continue
    print(f"=== Checking {df} ===")
    for line in p.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" in line:
            k, v = line.split("=", 1)
            actual = sdk_dict.get(k)
            if actual != v:
                print(f"MISMATCH: {k}: expected '{v}', actual '{actual}'")
