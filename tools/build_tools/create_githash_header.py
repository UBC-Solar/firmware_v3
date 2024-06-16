import subprocess

print("Creating githash.h file...")
# Get the root directory of the git repository
repo_root = subprocess.check_output(['git', 'rev-parse', '--show-toplevel']).decode('utf-8').strip()

# Get the current git commit hash
commit_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('utf-8').strip()

# Check if the working directory is dirty
try:
    subprocess.check_call(['git', 'diff', '--quiet'])
    suffix = ' '
except subprocess.CalledProcessError:
    suffix = '*'

# Create the githash.h file and write the commit hash to it
with open(f'{repo_root}/libraries/githash/githash.h', 'w') as file:
    file.write(f'#define GITHASH "{commit_hash}{suffix}"\n')