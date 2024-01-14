all: main.c
	gcc main.c json.c curl.c -lcurl -lcjson -o isbn -Wall
clean:
	rm isbn
