a4:	main.o init.o graphics.o gameLogic.o
	gcc -o a4 main.o init.o graphics.o gameLogic.o -lpthread -lncurses

main.o:	main.c defs.h
	gcc -c main.c

init.o:	init.c defs.h
	gcc -c init.c

graphics.o:	graphics.c defs.h
	gcc -c graphics.c

gameLogic.o:	gameLogic.c defs.h
	gcc -c gameLogic.c
