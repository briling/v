import ctypes
import vmol

def run(args):
    lib = ctypes.cdll.LoadLibrary(vmol.SO)
    main = lib.main

    main.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]
    main.restype = ctypes.c_int

    argv = [vmol.SO] + args
    argc = len(argv)
    argv = (ctypes.c_char_p * argc)(*[arg.encode('utf-8') for arg in argv])

    ret, output = vmol.stdout.capture(main, argc, argv)
    return ret, output
