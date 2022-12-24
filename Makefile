all:
	mkdir -p build
	gcc -Wall -Wextra tests.c -o build/tests

run:
	make
	./build/tests

debug:
	gcc tests.c -o build/tests -g
	seergdb -s build/tests

clean:
	rm -r build
