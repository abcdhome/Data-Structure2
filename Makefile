CC = gcc
CFLAGS = -Wall -g
TARGET = viva

OBJS = main.o stbar.o msgbar.o text.o terminal.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c stbar.h msgbar.h text.h terminal.h
	$(CC) $(CFLAGS) -c main.c

stbar.o: stbar.c stbar.h terminal.h
	$(CC) $(CFLAGS) -c stbar.c

msgbar.o: msgbar.c msgbar.h terminal.h
	$(CC) $(CFLAGS) -c msgbar.c

text.o: text.c text.h
	$(CC) $(CFLAGS) -c text.c

terminal.o: terminal.c terminal.h
	$(CC) $(CFLAGS) -c terminal.c

clean:
	rm -f $(OBJS) $(TARGET)

