#!/bin/bash
ctags --c-kinds=gstu -o- src/v.c src/*/*.{c,h} | \
awk 'BEGIN{printf("syntax keyword Type\t")}{printf("%s ", $1)}END{print ""}' > .types.vim
