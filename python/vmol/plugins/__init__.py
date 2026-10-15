"""Format-specific plugins for reading quantum-chemical outputs."""

from vmol.plugins.gaussian import GaussianPlugin
from vmol.plugins.orca import OrcaPlugin

__all__ = ["GaussianPlugin", "OrcaPlugin", "plugin_for"]

_PLUGINS = {
    "ORCA": OrcaPlugin,
    "Gaussian": GaussianPlugin,
}


def plugin_for(parser):
    """Return a plugin class for *parser*, or None if none is registered."""
    name = getattr(parser, "logname", None)
    return _PLUGINS.get(name)
