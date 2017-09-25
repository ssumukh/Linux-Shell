makefil: parser.c builtinFuncs.c shell.c
	gcc -o shell builtinFuncs.c parser.c shell.c -I. 