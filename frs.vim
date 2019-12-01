" Vim syntax file
" Language: Koala
" Maintainer: Daniel Schloegl
" Latest Revision: 30 November 2019

if exists("b:current_syntax")
  finish
endif

syn keyword frsCmd var val O I push pop

" Integer with - + or nothing in front
syn match frsNumber '\d\+'
syn match frsNumber '[-+]\d\+'

" Floating point number with decimal no E or e 
syn match frsNumber '[-+]\d\+\.\d*'

" Floating point like number with E and no decimal point (+,-)
syn match frsNumber '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+'
syn match frsNumber '\d[[:digit:]]*[eE][\-+]\=\d\+'

" Floating point like number with E and decimal point (+,-)
syn match frsNumber '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'
syn match frsNumber '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'

syn region frsString start='"' end='"' contained

let b:current_syntax = "frs"

hi def link frsCmd      Statement
hi def link frsNumber   Constant
hi def link frsString   Constant
