CC := clang
C := cpp

all:
	mkdir -p build
	$(CC) -Wall -Wextra tests.$(C) flexcut_implementation.$(C) -o build/tests

run:
	make
	./build/tests

debug:
	$(CC) tests.$(C) flexcut_implementation.c -o build/tests -g
	seergdb -s build/tests

clean:
	rm -r build
