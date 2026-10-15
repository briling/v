#!/usr/bin/env python
"""Run the vmol viewer with molecular data parsed by cclib."""

import sys
import os
import warnings

from vmol import vmol, _paths
from vmol.plugins import plugin_for
from vmol.plugins.common import Dataset, has_coords, parse_cclib, vib_from_data
from vmol.plugins.vibtraj import (
    DEFAULT_NPOINTS,
    DEFAULT_SCALE,
    mode_labels,
    mode_trajectory,
)

try:
    import cclib
except ModuleNotFoundError:
    msg = "cclib not found. Install cclib to use this script."
    raise ModuleNotFoundError(msg) from None


VIEW_KEYS = ("freq", "opt", "sp", "irc", "scan", "geom")


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

    Raises:
        FileNotFoundError: If *path* does not exist.
    """
    if not os.path.isfile(path):
        msg = f"File {path} not found."
        raise FileNotFoundError(msg)
    return parse_cclib(path)


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
    if not has_coords(data):
        msg = f"No coordinates found in {path}."
        raise RuntimeError(msg)
    return _mols_from_data(data, parser), vib_from_data(data)


def _collect_cli_selectors(argv):
    """Parse ``view:`` / ``vib:`` / ``mode:`` / ``scale:`` / ``npoints:`` from *argv*.

    Returns:
        tuple: ``(view_keys, vib_mode, mode, scale, npoints, indices_to_pop)``.

    Raises:
        ValueError: If a ``view:`` key is unknown or a selector value is invalid.
    """
    view_keys = []
    vib_mode = None
    mode = None
    scale = None
    npoints = None
    to_pop = []
    for i, arg in enumerate(argv):
        if arg.startswith("view:"):
            key = arg.split(":", 1)[1].strip().lower()
            if key not in VIEW_KEYS:
                msg = f"Unknown view selector '{arg}'. Expected one of: " + ", ".join(
                    f"view:{k}" for k in VIEW_KEYS
                )
                raise ValueError(msg)
            view_keys.append(key)
            to_pop.append(i)
        elif arg.startswith("vib:"):
            try:
                vib_mode = bool(int(arg.split(":", 1)[1]))
            except ValueError:
                continue
        elif arg.startswith("mode:"):
            mode = int(arg.split(":", 1)[1])
            to_pop.append(i)
        elif arg.startswith("scale:"):
            scale = float(arg.split(":", 1)[1])
            to_pop.append(i)
        elif arg.startswith("npoints:"):
            npoints = int(arg.split(":", 1)[1])
            to_pop.append(i)
    return view_keys, vib_mode, mode, scale, npoints, to_pop


def _default_datasets(datasets):
    for ds in datasets:
        if ds.key == "freq":
            return [ds]
    return [datasets[0]]


def _prompt_datasets(datasets):
    print(f"Found {len(datasets)} dataset(s) in this file:")
    for i, ds in enumerate(datasets, start=1):
        print(f"  {i}) [{ds.key}] {ds.label}")
    choice = _read_prompt_line(
        "Enter number(s), comma-separated list, or 'all': ",
    ).strip().lower()
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
            warnings.warn(
                f"Ignoring invalid choice '{part}'.", RuntimeWarning, stacklevel=2,
            )
            continue
        if not 1 <= idx <= len(datasets):
            warnings.warn(
                f"Ignoring out-of-range choice {idx}.", RuntimeWarning, stacklevel=2,
            )
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
        warnings.warn(
            "vib:1 requested but no frequencies found.", RuntimeWarning, stacklevel=2,
        )
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


def _read_prompt_line(prompt: str) -> str:
    print(prompt, end="", flush=True)
    try:
        return sys.stdin.readline()
    except EOFError:
        return ""


def _prompt_or_default(prompt: str, default, cast):
    if not sys.stdin.isatty():
        return default
    raw = _read_prompt_line(prompt).strip()
    return cast(raw) if raw else default


def _resolve_freq_params(vib, mode, scale, npoints):
    """Resolve mode (1-based), scale, and npoints via CLI values and/or TTY prompts.

    Returns:
        tuple: ``(mode_0based, scale, npoints)``.

    Raises:
        ValueError: If the mode index is out of range.
    """
    nmodes = len(vib["freq"])
    if mode is None:
        if sys.stdin.isatty():
            print(f"Found {nmodes} vibrational mode(s):")
            for line in mode_labels(vib):
                print(f"  {line}")
            mode = _prompt_or_default(
                f"Enter mode number (1-{nmodes}): ", 1, int,
            )
        else:
            warnings.warn(
                "No mode:N given for frequencies; using mode 1.",
                RuntimeWarning,
                stacklevel=2,
            )
            mode = 1

    if not 1 <= mode <= nmodes:
        msg = f"mode:{mode} out of range (1-{nmodes})"
        raise ValueError(msg)

    if scale is None:
        scale = _prompt_or_default(
            f"Displacement magnitude in Angstrom [{DEFAULT_SCALE}]: ",
            DEFAULT_SCALE,
            float,
        )
    if npoints is None:
        npoints = _prompt_or_default(
            f"Number of frames [{DEFAULT_NPOINTS}]: ",
            DEFAULT_NPOINTS,
            int,
        )
    return mode - 1, float(scale), int(npoints)


def _freq_to_trajectory(ds, freq_opts) -> Dataset:
    if ds.vib is None or not ds.mols:
        msg = "Frequency dataset is missing vibration or geometry data."
        raise RuntimeError(msg)
    mode0, scale, npoints = _resolve_freq_params(ds.vib, *freq_opts)
    mols = mode_trajectory(
        ds.mols[0], ds.vib, mode0, scale=scale, npoints=npoints,
    )
    fq = float(ds.vib["freq"][mode0])
    return Dataset(
        key="freq",
        label=f"Mode {mode0 + 1} ({fq:.2f} cm-1, A={scale:g} A, {npoints} frames)",
        mols=mols,
        vib=None,
    )


def _run_datasets(datasets, args, freq_opts):
    ret = 0
    for ds in datasets:
        if ds.key == "freq":
            ds = _freq_to_trajectory(ds, freq_opts)
        ret = vmol.run(mols=ds.mols, vib=ds.vib, args=args, with_arg0=True)
    return ret


def _handle_plugin(plugin, path, select, args, freq_opts):
    view_keys, vib_mode = select
    _parser, data = plugin.parse(path)
    datasets = plugin.inventory(path, data)
    chosen = _choose_datasets(datasets, view_keys, vib_mode)
    if not chosen:
        msg = f"No datasets selected for {path}."
        raise RuntimeError(msg)
    return _run_datasets(chosen, args, freq_opts)


def _try_generic_file(arg, vib_mode, freq_opts, mols, vib):
    """Load a non-plugin cclib file into the accumulating mol list.

    Returns:
        tuple: ``(mols, vib, done)`` where *done* means stop loading files.
    """
    try:
        parser, data = _parse_file(arg)
    except (FileNotFoundError, RuntimeError) as e:
        warnings.warn(str(e), RuntimeWarning, stacklevel=3)
        return mols, vib, False

    if not has_coords(data):
        warnings.warn(
            f"No coordinates found in {arg}.", RuntimeWarning, stacklevel=3,
        )
        return mols, vib, False

    m = _mols_from_data(data, parser)
    v = vib_from_data(data)
    if not mols and v is not None and vib_mode is not False:
        fake = Dataset(key="freq", label="frequencies", mols=m[:1], vib=v)
        traj = _freq_to_trajectory(fake, freq_opts)
        return traj.mols, None, True
    mols.extend(m)
    return mols, vib, False


def _load_cli_files(select, freq_opts):
    """Parse CLI molecule arguments into a viewer payload.

    Returns:
        tuple: ``(to_pop, mols, vib, last_ret, plugin_handled)``.
    """
    vib_mode = select[1]
    to_pop = []
    mols = []
    vib = None
    last_ret = 0
    plugin_handled = False

    for i, arg in enumerate(sys.argv[1:], start=1):
        if ":" in arg:
            continue
        to_pop.append(i)

        try:
            peek = _peek_parser(arg)
        except (FileNotFoundError, RuntimeError) as e:
            warnings.warn(str(e), RuntimeWarning, stacklevel=2)
            continue

        plugin = plugin_for(peek)
        if plugin is not None:
            remaining = [
                sys.argv[j] for j in range(len(sys.argv)) if j not in set(to_pop)
            ]
            try:
                last_ret = _handle_plugin(
                    plugin, arg, select, remaining, freq_opts,
                )
            except (RuntimeError, ValueError) as e:
                warnings.warn(str(e), RuntimeWarning, stacklevel=2)
                continue
            plugin_handled = True
            continue

        mols, vib, done = _try_generic_file(arg, vib_mode, freq_opts, mols, vib)
        if done:
            break

    return to_pop, mols, vib, last_ret, plugin_handled


def main():
    """Run the viewer with files parsed by cclib.

    Usage:
        vmol2.py file [options]

    ORCA and Gaussian outputs may offer frequencies, optimization, single
    point, scan, and/or IRC. With several datasets and a TTY, a menu is shown.
    For frequencies, pick a mode and displacement (or pass ``mode:`` /
    ``scale:`` / ``npoints:``). Otherwise use ``view:freq|opt|sp|irc|scan``
    or ``vib:0`` / ``vib:1``.

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

    view_keys, vib_mode, mode, scale, npoints, view_pops = _collect_cli_selectors(
        sys.argv[1:],
    )
    for i in reversed([j + 1 for j in view_pops]):
        sys.argv.pop(i)

    select = (view_keys, vib_mode)
    freq_opts = (mode, scale, npoints)
    to_pop, mols, vib, last_ret, plugin_handled = _load_cli_files(select, freq_opts)

    if plugin_handled and not mols:
        return last_ret

    if not mols:
        msg = "No valid molecular data found in any provided files."
        raise RuntimeError(msg)

    for i in reversed(to_pop):
        sys.argv.pop(i)
    return vmol.run(mols=mols, vib=vib, args=sys.argv, with_arg0=True)


if __name__ == "__main__":
    main()
