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
service: $(SERVICEBUILDDIR)/main.o
	g++ $(COMPILE_FLAGS) -o neighbourSearchingService $(SERVICEBUILDDIR)/main.o

$(SERVICEBUILDDIR)/main.o: service/main.cpp
	g++ $(COMPILE_FLAGS) -c service/main.cpp -o $(SERVICEBUILDDIR)/main.o

