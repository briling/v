from setuptools import setup, Extension
import subprocess
import os
import tempfile
import shutil

VERSION="3.0rc1"

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
    return version.strip().decode().replace('-', '+', 1)


def check_compilation():
    """Check if C compiler and X libraries are available.

    Thanks to https://stackoverflow.com/questions/16549893/programatically-testing-for-openmp-support-from-a-python-setup-script
    """
    omp_test = 'void main() { }'
    tmpdir = tempfile.mkdtemp()
    curdir = os.getcwd()
    os.chdir(tmpdir)
    filename = r'test.c'
    with open(filename, 'w') as file:
        file.write(omp_test)
    with open(os.devnull, 'w') as fnull:
        result = subprocess.call(['cc', '-lX11', '-lXpm', filename], stdout=fnull, stderr=fnull)
    os.chdir(curdir)
    shutil.rmtree(tmpdir)
    return not result


if __name__ == '__main__':
    # TODO
    #check_compilation()

    all_c_files = [os.path.join(root, file) for root, dirs, files in os.walk('../src') for file in files if file.endswith('.c')]

    INCL=['-I../src/'+i for i in ['mol', 'math', 'v', 'sym']]
    try:
        GIT_HASH = subprocess.check_output(['git', 'rev-parse', 'HEAD']).strip().decode()
        GIT_BRANCH = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip().decode()
    except subprocess.CalledProcessError:
        GIT_HASH = "unknown"
        GIT_BRANCH = "unknown"
    VERSION_FLAGS = [f'-DGIT_HASH="{GIT_HASH}"',
                     f'-DGIT_BRANCH="{GIT_BRANCH}"',
                     f'-DBUILD_USER="{os.getenv('USER')}@{os.getenv('HOSTNAME')}"',
                     f'-DBUILD_DIRECTORY="{os.getcwd()}"']

    setup(
        version=get_git_version_hash(),
        ext_modules=[Extension('vmol.v',
                               all_c_files,
                               extra_compile_args=['-std=gnu11', '-O2', ] + INCL + VERSION_FLAGS,
                               extra_link_args=['-lX11', '-lXpm']),
                    ],
    )
