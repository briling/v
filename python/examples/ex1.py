#!/usr/bin/env python3

import os
import vmol

# vmol.SO = 'v.cpython-313-x86_64-linux-gnu.so'  # set if want to specify the .so path
mols = f'{os.path.dirname(__file__)}/../../mol/'
args = [f'{mols}/MOL_3525.xyz']
r, o = vmol.main.run(args)

print("Return value:", r)
print("Captured output:")
print(o)
