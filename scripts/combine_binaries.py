import subprocess
import os

# Paths (edit if needed)
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

stage2_bin = os.path.join(project_root, "stage2_bootloader", "STM32CubeIDE", "Debug", "stage2_bootloader.bin")
main_app_bin = os.path.join(project_root, "main_application", "STM32CubeIDE", "Debug", "main_application.bin")
output_bin = os.path.join(project_root, "binaries", "combined.bin")

# Offsets
stage2_offset = 0x0000  # 0x08000000
main_app_offset = 0x8000  # 0x08008000

# Ensure output directory exists
os.makedirs(os.path.dirname(output_bin), exist_ok=True)

# srec_cat command
srec_cat_path = r"C:\Program Files\srecord\bin\srec_cat.exe"
cmd = [
    srec_cat_path,
    stage2_bin, "-Binary", "-offset", str(stage2_offset),
    main_app_bin, "-Binary", "-offset", str(main_app_offset),
    "-o", output_bin, "-Binary"
]

# Run it
try:
    print(f"Combining binaries:\n  Stage 2: {stage2_bin}\n  Main App: {main_app_bin}\n  Output: {output_bin}")
    subprocess.run(cmd, check=True)
    print("✅ Combined binary created successfully.")
except subprocess.CalledProcessError as e:
    print(f"❌ Error combining binaries: {e}")
