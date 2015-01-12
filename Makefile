CC=gcc
CFLAGS=-g -O2 -Wall -ISource $(OPTFLAGS)

SOURCES=$(wildcard Source/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
GUISOURCES=Source/Main.cpp
GUIEXE=bin/AGMSAVLTest

TARGET=build/libavlduptree.so

all: $(TARGET) $(SO_TARGET)

$(TARGET): build $(OBJECTS)
	$(CC) -shared -o $(TARGET) $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

clean:
	@rm -rf build $(OBJECTS) $(TESTS)

install: all $(GUIEXE)
	install $(TARGET) $(PREFIX)

$(GUIEXE): CFLAGS += -Wno-multichar
$(GUIEXE): CFLAGS+= -lbe
$(GUIEXE): $(SOURCES) $(GUISOURCES)
	$(CC) $(CFLAGS) $^ -o $@
	install $(GUIEXE) $(BINDIR)/

.PHONY: all clean install
