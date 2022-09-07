CC      = gcc
INCLUDE = -I/usr/include/glib-2.0 \
          -I/usr/lib/$(shell uname -i)-linux-gnu/glib-2.0/include
CFLAGS  = $(FLAGS_GLOBAL)
CFLAGS  += -g -Werror -fno-strict-aliasing
LIB     = -lpthread \
          -lglib-2.0

OBJECTS = flow.o \
          sm3.o \
          timer.o \
          ageing.o \
          hashtable.o \
          test.o

TARGET = flowageing.bin

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(INCLUDE) $(CFLAGS) $(LIB)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf *.o $(TARGET)
