from setuptools import setup, Extension
import subprocess
import os
import tempfile
import shutil
from pathlib import Path


VERSION="3.0rc2"

def get_git_version_hash():
    """Get tag/hash of the latest commit.

    Thanks to https://gist.github.com/nloadholtes/07a1716a89b53119021592a1d2b56db8
    """
    try:
        p = subprocess.Popen(["git", "describe", "--tags", "--dirty"], stdout=subprocess.PIPE)
    except OSError:
        return VERSION + "+unknown"
    version = p.communicate()[0]
    if not version.strip():
        return VERSION + "+unknown"
    print(version)
    return version.strip().decode().replace('+', '-').replace('-', '+', 1)


if __name__ == '__main__':

    setup_dir = Path(__file__).parent
    src_dir = setup_dir.parent / "src"

    def rel_posix(path):
        return os.path.relpath(path, start=setup_dir).replace(os.sep, "/")

    c_files = [rel_posix(p) for p in src_dir.rglob("*.c")]
    include_dirs = sorted({ rel_posix(h.parent) for h in src_dir.rglob("*.h") })

    try:
        GIT_HASH = subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip().decode()
        GIT_BRANCH = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip().decode()
    except subprocess.CalledProcessError:
        GIT_HASH = "unknown"
        GIT_BRANCH = "unknown"
    VERSION_FLAGS = [f'-DGIT_HASH="{GIT_HASH}"',
                     f'-DGIT_BRANCH="{GIT_BRANCH}"',
                     f'-DBUILD_USER="{os.getenv("USER")}@{os.getenv("HOSTNAME")}"',
                     f'-DBUILD_DIRECTORY="{os.getcwd()}"']

    setup(
        version=get_git_version_hash(),
        ext_modules=[Extension('vmol.v',
                               sources=c_files,
                               include_dirs=include_dirs,
                               libraries = ['X11', 'Xpm'],
                               extra_compile_args=['-std=gnu11', '-O2', ] + VERSION_FLAGS,
                               extra_link_args=[]),
                    ],
    )
