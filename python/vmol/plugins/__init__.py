"""Format-specific plugins for reading quantum-chemical outputs."""

from vmol.plugins.orca import OrcaPlugin

__all__ = ["OrcaPlugin", "plugin_for"]


def plugin_for(parser):
    """Return a plugin for *parser*, or None if no dedicated plugin exists."""
    if getattr(parser, "logname", None) == "ORCA":
        return OrcaPlugin()
    return None
