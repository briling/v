#!/usr/bin/env python3

import sys
import vmol

args = sys.argv[1:]
_, o = vmol.main.run(args)
print(o)
