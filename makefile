
CFLAGS = -O3 -std=c99 -Wall

ifeq ($(OS),WIN)
BUILD = app.exe
SERVER = server-win.c
else
BUILD = app
SERVER = server-gnu.c
endif

build: $(BUILD)

clean:
	rm -rf *.o
	rm -rf *.exe
	rm -rf test/*.o
	rm -rf app

all: clean build

test: test.exe
	./test.exe
	
test.exe: vt100.o vt100-tgetc.o history.o test.o clarg.o 
	gcc -o $@ $^
	
app.exe: vt100.o vt100-tgetc.o clarg.o history.o main.o server.o
	gcc -o $@ $^ -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread.dll -Wl,-Bdynamic -lwsock32 -lws2_32

app: vt100.o vt100-tgetc.o clarg.o history.o main.o server.o
	gcc -o $@ $^ -lpthread
    
vt100.o: vt100.c vt100.h terminal-io.h history.h
	gcc $(CFLAGS) -c vt100.c
    
vt100-tgetc.o: vt100-tgetc.c terminal-io.h vt100.h history.h
	gcc $(CFLAGS) -c vt100-tgetc.c

history.o: history.c history.h
	gcc $(CFLAGS) -c history.c
	
clarg.o: clarg.h clarg.c 
	gcc $(CFLAGS) -c clarg.c
    
test.o: test/test.c history.h terminal-io.h vt100.h
	gcc $(CFLAGS) -c ./test/test.c
    
server.o: ./example/$(SERVER) ./example/server.h
	gcc $(CFLAGS) -c -o server.o ./example/$(SERVER)

main.o: ./example/main.c ./example/server.h terminal-io.h vt100.h history.h clarg.h
	gcc $(CFLAGS) -c ./example/main.c
    
  
