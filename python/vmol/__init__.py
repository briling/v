import os
from sysconfig import get_path, get_config_var
from vmol import stdout, main

_v = 'v'
_suffix = get_config_var('EXT_SUFFIX')
_paths = [
        f'{__path__[0]}/{_v}.so',
        f'{__path__[0]}/{_v}{_suffix}',
        f"{get_path('purelib')}/{__package__}/{_v}{_suffix}",
        f"{get_path('platlib')}/{__package__}/{_v}{_suffix}",
        f"{get_path('purelib')}/{_v}{_suffix}",
        f"{get_path('platlib')}/{_v}{_suffix}",
        ]

_exists = [os.path.isfile(p) for p in _paths]

SO = _paths[_exists.index(True)] if sum(_exists) else None
