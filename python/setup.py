from setuptools import setup, Extension
import subprocess
import os
import sys
import tempfile
import shutil
from pathlib import Path


VERSION="3.0rc6"

def run_git(args):
    try:
        r = subprocess.run( ["git", *args], check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        return r.stdout.strip() or None
    except Exception:
        return None


def get_git_version_hash():
    """Get tag/hash of the latest commit.

    Thanks to https://gist.github.com/nloadholtes/07a1716a89b53119021592a1d2b56db8
    """
    version = os.getenv("VMOL_GIT_DESCRIBE") or run_git(["describe", "--tags", "--dirty"]) or f"{VERSION}+unknown"
    return version.replace('+', '-').replace('-', '+', 1)


def rel_posix(path):
    return os.path.relpath(path, start=setup_dir).replace(os.sep, "/")


def have_x11():
    cc = os.getenv("CC", "cc")
    code = b"#include <X11/Xlib.h>\n#include <X11/xpm.h>\nint main(void){return 0;}\n"
    inc, lib = get_x11_config()
    cmd = [cc, "-x", "c", "-", "-o", os.devnull, "-lX11", "-lXpm"]
    cmd += [f"-I{d}" for d in inc]
    cmd += [f"-L{d}" for d in lib]
    try:
        subprocess.run(cmd, input=code, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return True
    except Exception:
        return False


def get_x11_config():
    # On Linux, X11 headers/libs are in standard system paths — nothing extra needed.
    # On macOS, we additionally probe common non-default install prefixes.
    if sys.platform != 'darwin':
        return [], []
    try:
        inc = subprocess.run(
            ['pkg-config', '--cflags-only-I', 'x11', 'xpm'],
            check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
        ).stdout.split()
        lib = subprocess.run(
            ['pkg-config', '--libs-only-L', 'x11', 'xpm'],
            check=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True,
        ).stdout.split()
        return [f[2:] for f in inc if f.startswith('-I')], \
               [f[2:] for f in lib if f.startswith('-L')]
    except Exception:
        pass
    for prefix in ['/opt/X11', '/opt/homebrew', '/usr/local']:
        if Path(f'{prefix}/include/X11/Xlib.h').exists():
            return [f'{prefix}/include'], [f'{prefix}/lib']
    return [], []


setup_dir = Path(__file__).parent
src_dir = setup_dir.parent / "src"

c_files = [rel_posix(p) for p in src_dir.rglob("*.c")]
include_dirs = sorted({ rel_posix(h.parent) for h in src_dir.rglob("*.h") })
if not c_files:
    raise RuntimeError(f"No C sources found under {src_dir}")

GIT_HASH   = os.getenv("VMOL_GIT_HASH")   or run_git(["rev-parse", "HEAD"])                 or "unknown"
GIT_BRANCH = os.getenv("VMOL_GIT_BRANCH") or run_git(["rev-parse", "--abbrev-ref", "HEAD"]) or "unknown"

version_flags = [f'-DGIT_HASH="{GIT_HASH}"',
                 f'-DGIT_BRANCH="{GIT_BRANCH}"',
                 f'-DBUILD_USER="{os.getenv("USER")}@{os.getenv("HOSTNAME")}"',
                 f'-DBUILD_DIRECTORY="{os.getcwd()}"']

x11_inc, x11_lib = get_x11_config()
build_with_x11 = have_x11()

x11_only_sources = {
    rel_posix(src_dir / "v" / "x.c"),
    rel_posix(src_dir / "v" / "ac3_draw.c"),
    rel_posix(src_dir / "v" / "loop.c"),
    rel_posix(src_dir / "v" / "xinput.c"),
}
headless_only_sources = {
    rel_posix(src_dir / "v" / "x_no_x11.c"),
    rel_posix(src_dir / "v" / "ac3_draw_no_x11.c"),
}

if build_with_x11:
    c_files = [f for f in c_files if f not in headless_only_sources]
    libraries = ['X11', 'Xpm']
    define_macros = []
else:
    c_files = [f for f in c_files if f not in x11_only_sources]
    libraries = []
    define_macros = [('NO_X11', '1')]
    print("X11 not detected; building vmol in headless mode (NO_X11=1).", file=sys.stderr)

ignore_warnings = ['-Wno-sign-compare', '-Wno-format-truncation', '-Wno-format']

setup(
    version=get_git_version_hash(),
    include_package_data=True,
    ext_modules=[Extension('vmol.v',
                           sources=c_files,
                           include_dirs=include_dirs + x11_inc,
                           library_dirs=x11_lib,
                           define_macros=define_macros,
                           libraries = libraries,
                           extra_compile_args=['-std=gnu11', '-O2'] + ignore_warnings + version_flags,
                           extra_link_args=[]),
                ],
)
