main: main.c
	gcc -o main main.c -lcurl -lcjson

dev: main.c
	gcc -o main main.c -lcurl -lcjson -Wall -Wextra -Wpedantic

install: main
	sudo cp main /usr/bin/weather
