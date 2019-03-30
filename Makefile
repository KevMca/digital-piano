CC=gcc

noteListentestmake: noteListen.c
	$(CC) -o noteListen noteListen.c noteFunc.c -lpigpio -Wl,-rpath=/lib/PIGPIO -lrt -lncurses -lasound
