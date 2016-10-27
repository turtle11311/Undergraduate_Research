CC = gcc
CXX = g++
CXXFLAGS += -g -std=c++11 -Wall
LEX = flex
YACC = bison
LDFLAGS += -lfl
VPATH = src/
TARGET = threshold_logic_merger

.PHONY: all clean

all: $(TARGET)

$(TARGET): tln_scanner.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

tln_scanner.o: tln_scanner.cpp

tln_scanner.cpp: tln_token.l
	$(LEX) -t $^ > $@

clean:
	$(RM) *.cpp *.hpp *.o
