build: example.exe

clean:
	rm -rf *.o
	rm -rf *.exe
	rm -rf test/*.o
	rm -rf example/*.exe

all: clean build

test: test.exe
	./test.exe
	
test.exe: vt100.o vt100-tgetc.o history.o test.o 
	gcc -std=c99 -Wall -o test.exe vt100.o vt100-tgetc.o history.o test.o 
	
example.exe: vt100.o vt100-tgetc.o clarg.o history.o main.o server.o
	gcc -std=c99 -Wall -o example.exe vt100.o vt100-tgetc.o clarg.o history.o main.o server.o  -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -lwsock32 -lws2_32 -lsetupapi
    
vt100.o: vt100.c vt100.h history.h
	gcc -O3 -std=c99 -Wall -c vt100.c
    
vt100-tgetc.o: vt100-tgetc.c vt100-tgetc.h vt100.h history.h
	gcc -O3 -std=c99 -Wall -c vt100-tgetc.c

history.o: history.c history.h
	gcc -O3 -std=c99 -Wall -c history.c
	
clarg.o: clarg.h clarg.c 
	gcc -O3 -std=c99 -Wall -c clarg.c
    
test.o: test/test.c history.h vt100-tgetc.h vt100.h
	gcc -O3 -std=c99 -Wall -c ./test/test.c
    
server.o: ./example/server.c ./example/server.h
	gcc -O3 -std=c99 -Wall -c ./example/server.c

main.o: ./example/main.c ./example/server.h vt100-tgetc.h vt100.h history.h clarg.h
	gcc -O3 -std=c99 -Wall -c ./example/main.c
    
  