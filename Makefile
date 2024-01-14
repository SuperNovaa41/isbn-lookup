all: src/*.c
	gcc src/main.c src/json.c src/curl.c -lcurl -lcjson -o isbn -Wall
	mv isbn build
clean:
	rm isbn
