FLAG = -o

compile: main.o build_tree.o backend.o
	gcc main.o build_tree.o backend.o $(FLAG) compile

main.o: main.c build_tree.h
	gcc -c main.c

build_tree.o: build_tree.c build_tree.h
	gcc -c build_tree.c

backend.o: backend.c build_tree.h
	gcc -c backend.c

clean:
	@echo "Delete Files"
	del *.o compile.exe