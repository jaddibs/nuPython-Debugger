build:
	rm -f ./a.out
	g++ -std=c++17 -g -Wall main.o debugger.cpp nupython.o -lm -Wno-unused-variable -Wno-unused-function

run:
	./a.out

valgrind:
	rm -f ./a.out
	g++ -std=c++17 -g -Wall main.o debugger.cpp nupython.o -lm -Wno-unused-variable -Wno-unused-function
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./a.out "$(file)"

clean:
	rm -f ./a.out
  
submit:
	/home/cs211/w2025/tools/project04 submit debugger.cpp debugger.h README.md