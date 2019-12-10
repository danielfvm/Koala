build:
	mkdir -p bin obj
	gcc -c src/main.c -o obj/main.o
	gcc -c src/gnumber.c -o obj/gnumber.o
	gcc -c src/multisearcher.c -o obj/multisearcher.o
	gcc -c src/interpreter.c -o obj/interpreter.o
	gcc -c src/compiler.c -o obj/compiler.o
	gcc -c src/util.c -o obj/util.o
	gcc obj/main.o obj/gnumber.o obj/multisearcher.o obj/compiler.o obj/interpreter.o obj/util.o -o bin/koala -lm

run:
	./bin/koala examples/example.frs

clean:
	rm -rf obj

install:
	sudo cp bin/koala /usr/bin/koala
