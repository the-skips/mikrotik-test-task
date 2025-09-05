COMPILE_FLAGS = -std=c++11 -Wall

CLIBUILDDIR = build/cli
SERVICEBUILDDIR = build/service

all: cli service

# CLI
cli: $(CLIBUILDDIR)/main.o
	g++ $(COMPILE_FLAGS) -o neighbourCli $(CLIBUILDDIR)/main.o

$(CLIBUILDDIR)/main.o: cli/main.cpp
	g++ $(COMPILE_FLAGS) -c cli/main.cpp -o $(CLIBUILDDIR)/main.o

# SERVICE
service: $(SERVICEBUILDDIR)/main.o $(SERVICEBUILDDIR)/serviceNode.o
	g++ $(COMPILE_FLAGS) -o neighbourSearchingService $(SERVICEBUILDDIR)/main.o $(SERVICEBUILDDIR)/serviceNode.o

$(SERVICEBUILDDIR)/main.o: service/main.cpp service/serviceNode.h
	g++ $(COMPILE_FLAGS) -c service/main.cpp -o $(SERVICEBUILDDIR)/main.o

$(SERVICEBUILDDIR)/serviceNode.o: service/serviceNode.cpp service/serviceNode.h
	g++ $(COMPILE_FLAGS) -c service/serviceNode.cpp -o $(SERVICEBUILDDIR)/serviceNode.o


