au BufWritePre * silent !ctags -R -f .tags
set tags=.tags
set tags+=~/.vim/systags

nnoremap <C-]> <Esc>:exe "ptjump " . expand("<cword>")<Esc>

au BufWritePre * silent !./.maketypes.bash
autocmd BufRead,BufnewFile *.[ch] exe 'so ' . '.types.vim'
