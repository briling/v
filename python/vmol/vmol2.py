#!/usr/bin/env python
"""Run the vmol viewer with molecular data parsed by cclib."""

import sys
import os
import warnings

from vmol import vmol, _paths
from vmol.plugins import plugin_for
from vmol.plugins.orca import OrcaPlugin

try:
    import cclib
except ModuleNotFoundError:
    msg = "cclib not found. Install cclib to use this script."
    raise ModuleNotFoundError(msg) from None


VIEW_KEYS = ("freq", "opt", "sp", "irc", "scan", "geom")

_CCLIB2VMOL = {
    "vibdisps": "disp",
    "vibfreqs": "freq",
    "vibrmasses": "mass",
    "vibirs": "ints",
}


def _vib_from_data(data):
    vib = {dst: getattr(data, src, None) for src, dst in _CCLIB2VMOL.items()}
    if vib["disp"] is None or vib["freq"] is None:
        return None
    return vib


def _mols_from_data(data, parser):
    return [{"q": data.atomnos, "r": r, "name": str(parser)} for r in data.atomcoords]


def _peek_parser(path):
    """Open *path* with cclib without parsing contents.

    Returns:
        cclib parser instance.

    Raises:
        FileNotFoundError: If *path* does not exist.
        RuntimeError: If cclib cannot identify the file.
    """
    if not os.path.isfile(path):
        msg = f"File {path} not found."
        raise FileNotFoundError(msg)
    parser = cclib.io.ccopen(path)
    if parser is None:
        msg = f"Could not open {path} with cclib."
        raise RuntimeError(msg)
    return parser


def _parse_file(path):
    """Open *path* with cclib as ``(parser, data)``.

    Returns:
        tuple: ``(parser, data)``.
    """
    parser = _peek_parser(path)
    try:
        data = parser.parse()
    except Exception:  # noqa: BLE001
        msg = f"Error parsing {path} with cclib. Using partial data."
        warnings.warn(msg, RuntimeWarning, stacklevel=2)
        data = parser
    return parser, data


def read_mols_from_cclib(path):
    """Read molecular data from a file using cclib.

    Arguments:
        path (str): Path to the file to read.

    Returns:
        tuple containing:
            list[dict]: Dictionaries containing atomic numbers ('q'), coordinates ('r'),
                        and a name for each molecule, i.e. suitable argument for vmol.run().
            dict or None: Dictionary containing vibrational frequencies ('freq') and modes ('disp').

    Raises:
        RuntimeError: If cclib cannot parse the file or if no coordinates are found
            in the parsed data.
    """
    parser, data = _parse_file(path)
    if not (hasattr(data, "atomcoords") and hasattr(data, "atomnos") and len(data.atomcoords)):
        msg = f"No coordinates found in {path}."
        raise RuntimeError(msg)
    return _mols_from_data(data, parser), _vib_from_data(data)


def _collect_view_selectors(argv):
    """Parse ``view:`` / ``vib:`` tokens from *argv*.

    Returns:
        tuple: ``(view_keys, vib_mode, indices_to_pop)``.

    Raises:
        ValueError: If a ``view:`` key is unknown.
    """
    view_keys = []
    vib_mode = None
    to_pop = []
    for i, arg in enumerate(argv):
        if arg.startswith("view:"):
            key = arg.split(":", 1)[1].strip().lower()
            if key not in VIEW_KEYS:
                msg = (
                    f"Unknown view selector '{arg}'. Expected one of: "
                    + ", ".join(f"view:{k}" for k in VIEW_KEYS)
                )
                raise ValueError(msg)
            view_keys.append(key)
            to_pop.append(i)
        elif arg.startswith("vib:"):
            try:
                vib_mode = bool(int(arg.split(":", 1)[1]))
            except ValueError:
                continue
    return view_keys, vib_mode, to_pop


def _default_datasets(datasets):
    for ds in datasets:
        if ds.key == "freq":
            return [ds]
    return [datasets[0]]


def _prompt_datasets(datasets):
    print(f"Found {len(datasets)} dataset(s) in this file:")
    for i, ds in enumerate(datasets, start=1):
        print(f"  {i}) [{ds.key}] {ds.label}")
    print("Enter number(s), comma-separated list, or 'all': ", end="", flush=True)
    try:
        line = sys.stdin.readline()
    except EOFError:
        line = ""

    choice = line.strip().lower()
    if not choice:
        return _default_datasets(datasets)
    if choice in {"all", "*"}:
        return list(datasets)

    selected = []
    for part in choice.split(","):
        part = part.strip()
        if not part:
            continue
        try:
            idx = int(part)
        except ValueError:
            warnings.warn(f"Ignoring invalid choice '{part}'.", RuntimeWarning, stacklevel=2)
            continue
        if not 1 <= idx <= len(datasets):
            warnings.warn(f"Ignoring out-of-range choice {idx}.", RuntimeWarning, stacklevel=2)
            continue
        selected.append(datasets[idx - 1])
    return selected or _default_datasets(datasets)


def _select_by_keys(datasets, keys):
    by_key = {ds.key: ds for ds in datasets}
    selected = []
    for key in keys:
        if key not in by_key:
            warnings.warn(
                f"Requested view:{key} but that dataset is not available "
                f"(have: {', '.join(by_key)}).",
                RuntimeWarning,
                stacklevel=2,
            )
            continue
        selected.append(by_key[key])
    return selected


def _select_by_vib(datasets, vib_mode):
    if vib_mode:
        for ds in datasets:
            if ds.key == "freq":
                return [ds]
        warnings.warn("vib:1 requested but no frequencies found.", RuntimeWarning, stacklevel=2)
        return _default_datasets(datasets)

    for key in ("opt", "sp", "irc", "scan", "geom"):
        for ds in datasets:
            if ds.key == key:
                return [ds]
    return _default_datasets(datasets)


def _choose_datasets(datasets, view_keys, vib_mode):
    if len(datasets) == 1:
        return list(datasets)
    if view_keys:
        return _select_by_keys(datasets, view_keys) or _default_datasets(datasets)
    if vib_mode is not None:
        return _select_by_vib(datasets, vib_mode)
    if sys.stdin.isatty():
        return _prompt_datasets(datasets)
    return _default_datasets(datasets)


def _run_datasets(datasets, args):
    ret = 0
    for ds in datasets:
        ret = vmol.run(mols=ds.mols, vib=ds.vib, args=args, with_arg0=True)
    return ret


def _handle_orca(path, view_keys, vib_mode, args):
    _parser, data = OrcaPlugin.parse(path)
    datasets = OrcaPlugin.inventory(path, data)
    chosen = _choose_datasets(datasets, view_keys, vib_mode)
    if not chosen:
        msg = f"No datasets selected for {path}."
        raise RuntimeError(msg)
    return _run_datasets(chosen, args)


def _load_cli_files(view_keys, vib_mode):
    """Parse CLI molecule arguments into a viewer payload.

    Returns:
        tuple: ``(to_pop, mols, vib, last_ret, orca_handled)``.
    """
    to_pop = []
    mols = []
    vib = None
    last_ret = 0
    orca_handled = False

    for i, arg in enumerate(sys.argv[1:], start=1):
        if ":" in arg:
            continue
        to_pop.append(i)
        popped = set(to_pop)

        try:
            peek = _peek_parser(arg)
        except (FileNotFoundError, RuntimeError) as e:
            warnings.warn(str(e), RuntimeWarning, stacklevel=2)
            continue

        if plugin_for(peek) is not None:
            remaining = [sys.argv[j] for j in range(len(sys.argv)) if j not in popped]
            try:
                last_ret = _handle_orca(arg, view_keys, vib_mode, remaining)
            except RuntimeError as e:
                warnings.warn(str(e), RuntimeWarning, stacklevel=2)
                continue
            orca_handled = True
            continue

        try:
            parser, data = _parse_file(arg)
        except (FileNotFoundError, RuntimeError) as e:
            warnings.warn(str(e), RuntimeWarning, stacklevel=2)
            continue

        if not (
            hasattr(data, "atomcoords")
            and hasattr(data, "atomnos")
            and len(data.atomcoords)
        ):
            warnings.warn(f"No coordinates found in {arg}.", RuntimeWarning, stacklevel=2)
            continue

        m = _mols_from_data(data, parser)
        v = _vib_from_data(data)
        if not mols and v is not None and vib_mode is not False:
            mols, vib = m, v
            break
        mols.extend(m)

    return to_pop, mols, vib, last_ret, orca_handled


def main():
    """Run the viewer with files parsed by cclib.

    Usage:
        vmol2.py file [options]

    ORCA outputs may offer frequencies, optimization, single point, scan,
    and/or IRC. With several datasets and a TTY, a menu is shown. Otherwise
    use ``view:freq|opt|sp|irc|scan`` or ``vib:0`` / ``vib:1``.

    Returns:
        int: Exit code (0 for success).

    Raises:
        ImportError: If vmol shared library is not found.
        RuntimeError: If no valid molecular data is found in any provided files.
    """
    if vmol is None:
        paths = "\n".join(_paths)
        msg = (
            f"vmol shared library not found in paths. Check your installation.\n"
            f"Searched paths:\n{paths}"
        )
        raise ImportError(msg)

    if len(sys.argv) == 1:
        vmol.run(args=sys.argv, with_arg0=True)
        return 0

    view_keys, vib_mode, view_pops = _collect_view_selectors(sys.argv[1:])
    for i in reversed([j + 1 for j in view_pops]):
        sys.argv.pop(i)

    to_pop, mols, vib, last_ret, orca_handled = _load_cli_files(view_keys, vib_mode)

    if orca_handled and not mols:
        return last_ret

    if not mols:
        msg = "No valid molecular data found in any provided files."
        raise RuntimeError(msg)

    for i in reversed(to_pop):
        sys.argv.pop(i)
    return vmol.run(mols=mols, vib=vib, args=sys.argv, with_arg0=True)


if __name__ == "__main__":
    main()
