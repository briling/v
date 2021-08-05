if executable('ctags')
  set tags=.tags
  set tags+=~/.vim/systags

  nnoremap <C-]> <Esc>:exe "ptjump " . expand("<cword>")<Esc>

  function! MakeTypes()
    silent !./.maketypes.bash
  endfunction

  function! MakeTags()
    silent !ctags -R -f .tags
  endfunction

  function! CheckFiles()
    if ! filereadable('.types.vim') | call MakeTypes() | else  | exe 'source .types.vim'  | endif
    if ! filereadable('.tags')      | call MakeTags()  | endif
  endfunction

  autocmd BufRead,BufnewFile *.[ch] call CheckFiles()
  autocmd BufWritePre *.[ch] call MakeTags()
  autocmd BufWritePre *.[ch] call MakeTypes()
endif
