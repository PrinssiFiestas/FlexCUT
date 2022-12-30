CC := clang

all:
	mkdir -p build
	$(CC) -Wall -Wextra tests.c flexcut_implementation.c -o build/tests

run:
	make
	./build/tests

debug:
	$(CC) tests.c flexcut_implementation.c -o build/tests -g
	seergdb -s build/tests

clean:
	rm -r build
