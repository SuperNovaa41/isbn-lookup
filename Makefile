all: main.c
	gcc main.c -lcurl -lcjson -o isbn -Wall
clean:
	rm isbn
