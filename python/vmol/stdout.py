# taken from https://gist.github.com/denilsonsa/9c8f5c44bf2038fd000f

import sys
import os
import tempfile
import io
import ctypes
import ctypes.util

class FILE(ctypes.Structure):
    pass

FILE_p = ctypes.POINTER(FILE)

libc = ctypes.CDLL(ctypes.util.find_library('c'), use_errno=True)
cfflush = libc.fflush
cfflush.argtypes = (FILE_p,)
cfflush.restype = ctypes.c_int

csetbuf = libc.setbuf
csetbuf.argtypes = (FILE_p, ctypes.c_char_p)
csetbuf.restype = None

cstdout = FILE_p.in_dll(libc, 'stdout')

def capture(func, *args, **kwargs):

    sys.stdout.flush()
    cfflush(cstdout)

    with tempfile.TemporaryFile(buffering=0) as temp:

        fd = sys.stdout.fileno()
        prev_sys_stdout = sys.stdout
        prev_stdout_fd = os.dup(fd)
        os.close(fd)
        os.dup2(temp.fileno(), fd)

        temp_wrapper = io.TextIOWrapper(temp, encoding='utf8', line_buffering=True, write_through=True)
        sys.stdout = temp_wrapper
        csetbuf(cstdout, None)

        ret = func(*args, **kwargs)

        cfflush(cstdout)
        os.dup2(prev_stdout_fd, fd)
        os.close(prev_stdout_fd)
        sys.stdout = prev_sys_stdout

        temp_wrapper.seek(0)
        return ret, temp_wrapper.read()
