import subprocess
import pathlib
import glob
import shutil
import os

# Configuration
projects = [
    'bms/bms_master',
    'ecu/ecu_firmware'
]

root_dir = subprocess.check_output(['git', 'rev-parse', '--show-toplevel']).decode('ascii').strip()
build_cmd = f"pio run -d {root_dir}/components/"
elf_location = f".pio/build/*"
elf_destination = pathlib.Path(f"{root_dir}/tools/build_tools/builds")

# Remove build dir
shutil.rmtree(elf_destination)
elf_destination.mkdir()

# Build all projects
for project in projects:
    tmp_command = f"{build_cmd}/{project}".split(" ")
    out = subprocess.run(tmp_command, capture_output=True)

    try:
        out.check_returncode()
    except subprocess.CalledProcessError:
        print(f"{project} - build failed")
        continue

    print(f"{project} - buid passed")

    # Collect elf
    for src_file in glob.glob(f"{root_dir}/components/{project}/{elf_location}/firmware.elf"):
        filename = project.split('/')[-1]

        shutil.copy(f"{src_file}", f"{elf_destination}")
        os.rename(f"{elf_destination}/firmware.elf", f"{elf_destination}/{filename}.elf")