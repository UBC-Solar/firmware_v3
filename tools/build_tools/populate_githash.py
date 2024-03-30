import subprocess

def is_repo_changed(repo_path) -> bool:
    try:
        output = subprocess.check_output(["git", "-C", repo_path, "status", "--porcelain"]).decode("utf-8")
        # If there are any changes, the output won't be empty
        return bool(output.strip())
    except subprocess.CalledProcessError:
        # If there's an error running the command, assume repository has changed
        return True
    
def get_git_revision_short_hash() -> str:
    return subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()

def get_headerfile_location() -> str:
    repo_path = subprocess.check_output(['git', 'rev-parse', '--show-toplevel']).decode('ascii').strip()
    return repo_path + "/libraries/github/githash.h"
    
def write_headerfile(file_path: str, hash: str):
    content = """\
    #ifndef __GITHASH_H__
    #define __GITHASH_H__

    // GITHASH is always 8 chars long and contains the 7 char git hash
    // and either a space or star depending on if the commit is dirty
    static const char githash[] = "{}";

    #endif // __GITHASH_H__
    """
    
    with open(file_path, 'w') as file:
        file.write(content.format(hash))
        file.close()   

repo_path = "./"
special_dirty_character = "*"
special_clean_character = " "

hash = ""
header_path = ""

# Format hash
short_hash = get_git_revision_short_hash()
dirty = is_repo_changed(repo_path)

if(dirty):
    hash = short_hash+special_dirty_character
else:
    hash = short_hash+special_clean_character

# Get path to header file (libraries)
header_path = get_headerfile_location()
    
# Write hash to headerfile
write_headerfile(header_path, hash)

print("===SUCCESSFULLY WROTE GITHASH===")
    
    
    