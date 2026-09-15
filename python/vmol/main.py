"""Run the viewer with specified command-line arguments and/or molecule data and capture the output."""

from types import SimpleNamespace
import ctypes
from ctypes import c_int, c_double, c_char_p
import functools
from warnings import warn
from pprint import pformat


__all__ = ["Vmol"]


c_double_p = ctypes.POINTER(c_double)
c_int_p = ctypes.POINTER(c_int)


class mol_t(ctypes.Structure):  # ruff: ignore[invalid-class-name]
    """C structure for the input molecule data, containing the number of atoms, charge array, coordinate array, and name.

    Declared in src/mol/mol.h as
    ```
        typedef struct {
          double * r;
          int    * q;
          char   * name;
          int      n;
        } mol;
    ```
    """

    _fields_ = (
            ("r",    c_double_p),
            ("q",    c_int_p),
            ("name", c_char_p),
            ("n",    c_int),
            )


class vibr_t(ctypes.Structure):  # ruff: ignore[invalid-class-name]
    """C structure for the input molecule data, containing the number of atoms, charge array, coordinate array, and name.

    Declared in src/v/v.h as
    ```
        typedef struct {
          double * freq;
          double * ints;
          double * disp;
          double * mass;
          int      n;
        } vibr_t;
    ```
    """

    _fields_ = (
            ("freq", c_double_p),
            ("ints", c_double_p),
            ("disp", c_double_p),
            ("mass", c_double_p),
            ("n",    c_int),
            )


ARGS_T = (c_int, ctypes.POINTER(ctypes.c_char_p))
INP_MOLS_T = (c_int, ctypes.POINTER(mol_t))


def mol2struct(get_element, mol):
    """Convert a molecule to the expected C structure for input.

    Currently supports two formats for the input molecule:

        1) Dictionary with the following keys:
           - 'q': a 1D array-like of integers or strings representing the atomic numbers or element symbols,
           - 'r': a 2D array-like of floats with shape (len(q), 3) representing the atomic coordinates,
           - 'name' (optional): a string representing the name of the molecule.

        2) ase.atoms.Atoms-like object with the following attributes:
           - `numbers`: a 1D array of integers representing the atomic numbers,
           - `positions`: a 2D array of floats with shape (len(q), 3) representing the atomic coordinates.

    Args:
        get_element (callable): A function that takes a byte string representing an element symbol
                                and returns its atomic number as an integer.
        mol (dict or ase.atoms.Atoms-like): The molecule to convert.

    Returns:
        mol_t: An instance of `mol_t` with the fields set according to the input molecule.

    Raises:
        TypeError: If mol is neither a dictionary nor an ase.atoms.Atoms-like.
        ValueError: If the required keys are missing or their values have wrong shapes.
    """
    import numpy as np  # ruff: ignore[import-outside-top-level]

    if isinstance(mol, dict):
        q = mol.get('q')
        r = mol.get('r')
        name = mol.get('name')
        if q is None or r is None:
            msg = "mol must contain 'q' and 'r' for charge and coordinate arrays, respectively"
            raise ValueError(msg)

    elif hasattr(mol, 'positions') and hasattr(mol, 'numbers'):
        r = mol.positions
        q = mol.numbers
        name = str(mol)

    else:
        msg = f"mol must be a dictionary or ase.atoms.Atoms-like, but got {type(mol)}"
        raise TypeError(msg)

    q_tmp = np.asarray(q)
    n = len(q_tmp)
    if q_tmp.ndim != 1:
        msg = f"q must be a 1D array, but has shape {q_tmp.shape}"
        raise ValueError(msg)

    r = np.require(r, dtype=c_double, requirements=['C_CONTIGUOUS', 'OWNDATA'])
    if r.shape != (n, 3):
        msg = f"r must be a 2D array with shape ({n}, 3), but has shape {r.shape}"
        raise ValueError(msg)
    r = r.flatten()

    try:
        q = np.require(q, dtype=c_int, requirements=['C_CONTIGUOUS', 'OWNDATA'])
    except ValueError:
        q = list(q)
        for i, qi in enumerate(q):
            if isinstance(qi, str):
                q[i] = get_element(qi.encode('utf-8'))
            elif isinstance(qi, bytes):
                q[i] = get_element(qi)
        q = np.require(q, dtype=c_int, requirements=['C_CONTIGUOUS', 'OWNDATA'])

    if not isinstance(name, bytes):
        name = str(name).encode('utf-8')

    n = c_int(n)
    q = q.ctypes.data_as(c_int_p)
    r = r.ctypes.data_as(c_double_p)
    in_str = mol_t(n=n, q=q, r=r, name=name)
    in_str._keepalive = (n, q, r, name)  # keep strong references
    return in_str


def make_array(x, element_type, convert_func=lambda x: x):
    """Convert a list of Python objects to a C array of the specified element type.

    Args:
        x (object or list[object]): An object or a list thereof to convert to a C array.
        element_type (ctypes type): The type of the elements in the C array.
        convert_func (callable, optional): A function that takes a Python object and converts it to the appropriate C type.
                                           Defaults to the identity function.

    Returns:
        tuple: A tuple containing the number of elements, the C array, and a list of the converted elements to keep alive.
    """
    if not isinstance(x, list):
        x = [x]
    n = len(x)
    x_keepalive = [convert_func(i) for i in x]
    x = (element_type * n)(*x_keepalive)
    return c_int(n), x, x_keepalive


class Hooked:
    """A descriptor that calls a hook function whenever the attribute is set to a new value."""

    def __set_name__(self, owner, name):
        self.private_name = "_" + name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        return getattr(obj, self.private_name, None)

    def __set__(self, obj, value):
        setattr(obj, self.private_name, value)
        obj._call_hook()


class VmolFunctions:

    so = Hooked()
    f = SimpleNamespace()
    lib = None

    def __init__(self, so=None):
        self.so = so

    def __repr__(self):
        return f"Vmol(so={self.so!r})"

    def __bool__(self):
        return self.so is not None

    def _check_so(self):
        if self.so is None:
            msg = "shared library path is not set, cannot run the viewer"
            raise ValueError(msg)

    def _call_hook(self):
        """Re-declare functions when self.so is set to a new value."""
        if self.so is None:
            self.lib = None
            self._reset_functions()
        elif self.lib is None or self.lib._name != self.so:
            try:
                self.lib = ctypes.cdll.LoadLibrary(self.so)
            except OSError as e:
                msg = f"Failed to load shared library: {e}, keeping the old value {self.so}"
                warn(msg, RuntimeWarning, stacklevel=3)
                self.so = None
                return
            self._declare_functions()

    def _reset_functions(self):
        self.f.__dict__.clear()

    def _declare(self, name, *, argtypes, restype, errcheck=None):
        """Declare a function from the shared library with the given argument and return types.

        Args:
            name (str): The name of the function to declare.
            argtypes (list of ctypes types): The argument types of the function.
            restype (ctypes type): The return type of the function.
            errcheck (callable, optional): A function that takes the result, the function, and the arguments,
                                           and returns the processed result.

        Returns:
            callable: The function with the specified arguments and return types.
        """
        if not hasattr(self.lib, name):
            msg = f"Function '{name}' not found in the shared library '{self.so}'"
            warn(msg, RuntimeWarning, stacklevel=2)
            return None

        func = getattr(self.lib, name)
        func.argtypes = argtypes
        func.restype = restype
        if errcheck:
            func.errcheck = errcheck
        return func

    def _declare_functions(self):

        def errcheck(result, _func, args):
            ret = args[-1]._obj.value
            out = result.decode('utf-8').rstrip('\n') if result else None
            self.f.free()
            return ret, out

        self.f.free        = self._declare('free_out_str', argtypes=None,       restype=None)
        self.f.get_element = self._declare('get_element',  argtypes=[c_char_p], restype=c_int)

        self.f.main_raw        = self._declare('main', argtypes=ARGS_T, restype=c_int)
        self.f.main_out_raw    = self._declare('main_wrap_out',    argtypes=[*ARGS_T, c_int_p],
                                               restype=c_char_p, errcheck=errcheck)
        self.f.main_in_out_raw = self._declare('main_wrap_in_out', argtypes=[*ARGS_T, *INP_MOLS_T, vibr_t, c_int_p],
                                               restype=c_char_p, errcheck=errcheck)
        self.f.main_in_raw = self._declare('main_wrap_in', argtypes=[*ARGS_T, *INP_MOLS_T, vibr_t], restype=c_int)

        self.f.main        = self._convert_in(self.f.main_raw)
        self.f.main_out    = self._convert_in(self.f.main_out_raw, last_arg_ret_code=True)
        self.f.main_in_out = self._convert_in(self.f.main_in_out_raw, add_molecules=True, convert_vib=True, last_arg_ret_code=True)
        self.f.main_in     = self._convert_in(self.f.main_in_raw, add_molecules=True, convert_vib=True)

    def _convert_in(self, func, add_molecules=False, convert_vib=False, last_arg_ret_code=False):
        """Decorate a function to convert Python arguments to the expected C types.

        The function is expected to take an integer and a pointer to an array of C strings as its first 2 arguments,
        which represent the argument count and argument values, respectively, i.e.,
        ```
        void func(int argc, char ** argv, ...)
        ```
        The decorator will convert a list of Python strings passed as the first argument
        to the wrapped function into the appropriate C types before calling the original function, i.e.,
        ```
        def wrapped_func(argv: list[str], ...)
        ```

        If `add_molecules` is True, the function is also expected to take an integer
        and a pointer to an array of `mol_t` structures as additional arguments after the first 2,
        which represent the number of molecules and the molecule data, respectively.
        The decorator will convert a molecule or a list of molecules passed as the 3rd argument to the wrapped function:
        ```
        void func(int argc, char ** argv, int nmol, mol_t * mols, ...) -> def wrapped_func(argv: list[str], mols: object or list[object], ...)
        ```

        If `convert_vib` is also True, the function is expected to take a `vibr_t` argument after the first 4.
        The decorator will convert a dictionary passed as the 5th argument to the wrapped function:
        ```
        void func(int argc, char ** argv, int nmol, mol_t * mols, vibr_t vib, ...) -> def wrapped_func(argv: list[str], mols: object or list[object], vib: dict, ...)
        ```

        If `last_arg_ret_code` is True, the decorator also passes a pointer to an integer
        as the last argument to the original function to store the return code, i.e.,
        ```
        void func(int argc, char ** argv, ..., int * ret) -> def wrapped_func(argv: list[str], ...)
        ```
        The `func.errcheck` function is supposed to take care of the return value.

        Args:
            func (callable): The function to wrap.
            add_molecules (bool, optional): Whether the function also takes molecule data as additional arguments after the first two.
            convert_vib (bool, optional): Whether the function also takes normal modes data as additional argument after the first four.
            last_arg_ret_code (bool, optional): If the last argument is `int *` to store the return value in.

        Returns:
            callable: A wrapped function.

        Raises:
            TypeError: If func is not a ctypes function.
            ValueError: If its attributes are wrong.
        """
        def begin_differently(x, y):
            if x is None:
                return y is not None
            if len(x) < len(y):
                return True
            return any(xi!=yi for xi, yi in zip(x[:len(y)], y, strict=True))

        if func is None:
            return None
        if not isinstance(func, ctypes._CFuncPtr):
            msg = "function should be a ctypes function"
            raise TypeError(msg)

        if begin_differently(func.argtypes, ARGS_T):
            msg = f"function must have a signature that starts with\n{pformat(ARGS_T)}\nto convert the input list of strings, but got\n{pformat(func.argtypes)}"
            raise ValueError(msg)

        if add_molecules and begin_differently(func.argtypes[len(ARGS_T):], INP_MOLS_T):
            msg = f"function must have a signature that starts with\n{pformat(ARGS_T + INP_MOLS_T)}\nto convert the input list of strings and molecule data, but got\n{pformat(func.argtypes)}"
            raise ValueError(msg)

        if convert_vib:
            if add_molecules:
                if func.argtypes[len(ARGS_T)+len(INP_MOLS_T)] != vibr_t:
                    msg = f"function must have a signature that starts with\n{pformat(ARGS_T + INP_MOLS_T + (vibr_t,))}\nto convert the input list of strings, molecule data, and normal modes data, but got\n{pformat(func.argtypes)}"
                    raise ValueError(msg)
            else:
                msg = f"{convert_vib=} is incompatible with {add_molecules=}"
                raise ValueError(msg)

        if last_arg_ret_code and func.argtypes[-1] != c_int_p:
            msg = f"return code argument must be a pointer to an integer ({c_int_p}), but got {func.argtypes[-1]}"
            raise ValueError(msg)

        @functools.wraps(func)
        def myinner(*args):
            args = list(args)
            argv = args[0]
            argc, argv, _argv = make_array(args[0], c_char_p, convert_func=lambda x: x.encode('utf-8'))
            args.pop(0)
            if add_molecules:
                nmol, mols, keep_mols = make_array(args[0], mol_t, convert_func=lambda x: mol2struct(self.f.get_element, x))
                args.pop(0)
                if convert_vib:
                    args[0] = vib2struct(keep_mols[-1].n, args[0])
                args = [nmol, mols, *args]
            args = [argc, argv, *args]
            if last_arg_ret_code:
                ret = c_int(0)
                args.append(ctypes.byref(ret))
            return func(*args)
        return myinner


def check_shape(x, name, shape):
    if x.shape != shape:
        msg = f"{name} must be a {len(shape)}D array with shape {shape}, but has shape {x.shape}"
        raise ValueError(msg)


def check_dims(x, name, ndim):
    if x.ndim !=ndim:
        msg = f"{name} must be a {ndim}D array, but has shape {x.shape}"
        raise ValueError(msg)


def vib2struct(nat, vib=None):
    """Convert a normal modes dictionary to the expected C structure for input.

    The dictionary has to have the following keys:
        - 'freq': a 1D array-like of floats representing the harmonic frequencies (wave numbers) in cm-1,
        - 'disp': a 3D array-like of floats with shape (len(freq), nat, 3) representing the atom displacements,
                which will be renormalized to 1.
    Optional keys:
        - 'mass': a 1D array-like of floats with shape (len(freq),) for requced massed in amu,
        - 'ints': a 1D array-like of floats with shape (len(freq),) for intensities in km/mole.

    Args:
        nat (int): Number of atoms in a molecule.
        vib (dict, optional): The dictionary to convert.

    Returns:
        vibr_t: An instance of `vibr_t` with the fields set.  If missing, the optional fields are set to 0,
            If the input dictionary is None, everything is set to 0/NULL.

    Raises:
        TypeError: If mol is neither a dictionary nor None.
    """
    if vib is None:
        return vibr_t(n=c_int(0), disp=None, freq=None, ints=None, mass=None)

    if not isinstance(vib, dict):
        msg = f"vib must be None or a dictionary, but got {type(vib)}"
        raise TypeError(msg)

    import numpy as np  # ruff: ignore[import-outside-top-level]

    def check_array(key, required=True, shape=None, ndim=None, dtype=c_double):
        if not required and shape is None:
            msg = "shape should be known for non-required members"
            raise RuntimeError(msg)
        x = vib.get(key, None)
        if x is None:
            if required:
                msg = f"vib must contain '{key}'"
                raise ValueError(msg)
            x = np.zeros(shape, dtype=dtype)
            msg = f"vib does not contain '{key}'"
            warn(msg, RuntimeWarning, stacklevel=3)
        else:
            try:
                x = np.require(x, dtype=dtype, requirements=['C_CONTIGUOUS', 'OWNDATA'])
            except ValueError as e:
                msg = f"cannot convert '{key}' to an array of {dtype}: {e}"
                raise ValueError(msg) from None
            if shape is not None:
                check_shape(x, key, shape)
            if ndim is not None:
                check_dims(x, key, ndim)
        return x

    vdict = {'freq': check_array('freq', ndim=1)}
    nvib = len(vdict['freq'])

    vdict['disp'] = check_array('disp', shape=(nvib, nat, 3))
    disp_norm = np.linalg.norm(vdict['disp'].reshape(nvib, -1), axis=1)
    vdict['disp'] /= disp_norm[:,None,None]
    vdict['disp'] = vdict['disp'].flatten()

    for key in ['mass', 'ints']:
        vdict[key] = check_array(key, shape=(nvib,), required=None)

    nvib = c_int(nvib)
    for key, val in vdict.items():
        vdict[key] = val.ctypes.data_as(c_double_p)

    v = vibr_t(n=nvib, disp=vdict['disp'], freq=vdict['freq'], ints=vdict['ints'], mass=vdict['mass'])
    v._keepalive = (nvib, vdict['disp'], vdict['freq'], vdict['ints'], vdict['mass'])  # keep strong references
    return v


class Vmol(VmolFunctions):
    """Run the viewer with specified command-line arguments and/or molecule data and capture the output."""

    def run(self, *, args=None, mols=None, vib=None, with_arg0=False):
        """Run the viewer with the given command-line arguments.

        If `args` is provided, it will be passed as command-line arguments to the main function.
        It can contain any arguments that the main function accepts, including the paths to the molecule files to read.
        If `with_arg0` is True, the first argument in `args` is treated as the program name.

        If `mols` is provided, the molecules(s) will be converted to the appropriate input structure(s)
        and passed to the main function, and the paths to the molecule files in `args` will be ignored.

        Args:
            args (list of str, optional): The command-line arguments to pass to the main function.
            mols (object or list[object], optional): An object or a list thereof representing the molecule(s).
                 See `mol2struct()` for the expected format.
            vib (dict, optional): A dictionary representing the normal modes. Used only with mols!=None.
                See `vib2struct()` for the expected format.
            with_arg0 (bool, optional): Whether the first argument in `args` is the program name (e.g., `sys.argv[0]`).
                      If False or if `args` is None or empty, the program name is automatically added as the first argument.
                      Otherwise, `args` is used as is. Defaults to False.

        Returns:
            int: The return code from the main function.
        """
        self._check_so()
        args = (args if with_arg0 else [self.so, *args]) if args else [self.so]
        return self.f.main_in(args, mols, vib) if mols else self.f.main(args)

    def capture(self, *, mols=None, args=None, vib=None, return_code=False):
        """Run the viewer with the given structure and/or command-line arguments and capture the output.

        If `args` is provided, it will be passed as command-line arguments to the main function.
        It can contain any arguments that the main function accepts, except for the program name,
        including the paths to the molecule files to read.

        If `mols` is provided, the molecules(s) will be converted to the appropriate input structure(s)
        and passed to the main function, and the paths to the molecule files in `args` will be ignored.

        Args:
            mols (object or list[object], optional): An object or a list thereof representing the molecule(s).
                 See `mol2struct()` for the expected format.
            vib (dict, optional): A dictionary representing the normal modes. Used only with mols!=None.
                See `vib2struct()` for the expected format.
            args (list of str, optional): Command-line arguments to pass to the main function (without the program name).
            return_code (bool, optional): Whether to return the return code along with the output. Defaults to False.

        Returns:
            If `return_code` is False:
                str: The output from the main function, decoded as UTF-8 and stripped of trailing newlines.
            If `return_code` is True:
                tuple of (int, str): the function return code and the output string.
        """
        self._check_so()
        args = [self.so, *args] if args else [self.so]
        ret, out = self.f.main_in_out(args, mols, vib) if mols else self.f.main_out(args)
        return (ret, out) if return_code else out
