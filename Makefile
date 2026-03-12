CC = gcc
CXX = g++

# flags for C and C++ builds
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0`
CXXFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0` -std=c++11

LIBS = `pkg-config --libs gtk+-3.0` -lsqlite3
CPPLIBS = `pkg-config --libs gtk+-3.0 sqlite3`

TARGET = vibe-journal
CPP_TARGET = vj

all: $(TARGET)

# build C++ version if requested (requires gtkmm development packages)
cpp: $(CPP_TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LIBS)

$(CPP_TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(CPP_TARGET) main.cpp $(CPPLIBS)

clean:
	rm -f $(TARGET) $(CPP_TARGET)

.PHONY: all clean cpp