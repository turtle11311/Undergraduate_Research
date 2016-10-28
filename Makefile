CC = gcc
CXX = g++
CXXFLAGS += -g -std=c++11 -Wall -Isrc/
LEX = flex
YACC = bison
LDFLAGS += -lfl
VPATH = src:build
BUILDDIR = build
TARGET = threshold_logic_merger

.PHONY: all clean

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c -o $@

$(TARGET): $(BUILDDIR)/main.o $(BUILDDIR)/tln_scanner.o $(BUILDDIR)/tln_parser.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(BUILDDIR)/tln_scanner.cpp: tln_token.l tln_parser.cpp
	$(LEX) -t $< > $@

$(BUILDDIR)/tln_parser.cpp: tln_parser.y
	$(YACC) -d $< -o $@

$(BUILDDIR):
		mkdir $@
clean:
	$(RM) -rf $(BUILDDIR)
