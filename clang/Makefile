all:
	@echo this is all target

main: main.o copy.o
	@echo main
	gcc -o main main.o copy.o

main.o: main.c
	@echo main.o
	gcc -c main.c

copy.o: copy.c copy.h
	@echo copy.o
	gcc -c copy.c

say:
	@echo ${MSG}

clean:
	@echo this is clean target
	rm *.o
	rm long

