all: src/*.c
	gcc src/main.c src/json.c src/curl.c -lcurl -lcjson -o isbn -Wall
	mkdir -p build
	mv isbn build
clean:
	rm -rf build
