#!/usr/bin/env bash

MOLS="$(dirname $(realpath "$0"))/../../mol"
echo '.' | python -c "import os; import vmol; print(vmol.main.run(['${MOLS}/neopentane.xyz', 'symtol:1e-2', 'gui:0'])[1], end='')"
