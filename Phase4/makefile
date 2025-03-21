build:
	rm -f ./a.out
	gcc -std=c11 -g -Wall -pedantic -Werror main.c execute.c parser.o programgraph.o ram.o scanner.o tokenqueue.o -lm -Wno-unused-variable -Wno-unused-function 

run:
	./a.out

valgrind:
	rm -f ./a.out
	gcc -std=c11 -g -Wall -pedantic -Werror main.c execute.c parser.o programgraph.o ram.o scanner.o tokenqueue.o -lm -Wno-unused-variable -Wno-unused-function
	valgrind --tool=memcheck --leak-check=no --track-origins=yes ./a.out "$(file)"

submit:
	/home/cs211/w2025/tools/project06  submit  main.c  execute.c README.md