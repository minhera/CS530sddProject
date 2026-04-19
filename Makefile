############################
# Makefile for Assignment 2
# Student 1 Name:
# Student 1 RED ID:
# Student 2 Name: 
# Student 2 RED ID:
#
############################

CXX = g++
CXXFLAGS = -std=c++11 -g3 -Wall

EXEC = lxe
OBJS = main.o limited_assembler.o symtab.o littab.o optab.o

$(EXEC) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS)

main.o: main.cpp limited_assembler.h optab.h symtab.h littab.h
	$(CXX) $(CXXFLAGS) -c main.cpp

limited_assembler.o: limited_assembler.cpp limited_assembler.h optab.h symtab.h littab.h
	$(CXX) $(CXXFLAGS) -c limited_assembler.cpp

symtab.o: symtab.cpp symtab.h
	$(CXX) $(CXXFLAGS) -c symtab.cpp

littab.o: littab.cpp littab.h
	$(CXX) $(CXXFLAGS) -c littab.cpp

optab.o: optab.cpp optab.h
	$(CXX) $(CXXFLAGS) -c optab.cpp

clean:
	rm -f *.o $(EXEC)