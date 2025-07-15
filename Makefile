main: main.c textColors.c
	gcc -o main main.c textColors.c -lcurl -lcjson

dev: main.c textColors.c
	gcc -o main main.c textColors.c -lcurl -lcjson -Wall -Wextra -Wpedantic

lmain: main
	sudo cp main /usr/bin/weather
