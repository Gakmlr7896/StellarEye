CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
LIBS = -lcfitsio

TARGETS = dataloader filter_galaxies

all: $(TARGETS)

dataloader: src/DataControl/dataloader.cpp
	$(CXX) $(CXXFLAGS) src/DataControl/dataloader.cpp -o Executable/dataloader $(LIBS)

filter_galaxies: src/DataControl/filter_galaxies.cpp
	$(CXX) $(CXXFLAGS) src/DataControl/filter_galaxies.cpp -o Executable/filter_galaxies $(LIBS)

GalaxyDataLoader: src/DataControl/GalaxyDataLoader.cpp 
	$(CXX) $(CXXFLAGS) src/DataControl/GalaxyDataLoader.cpp -o Executable/GalaxyDataLoader $(LIBS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean

run_dataloader: dataloader
	./Executable/dataloader

run_filter: filter_galaxies
	./Executable/filter_galaxies

run_galaxyloader: GalaxyDataLoader
	./Executable/GalaxyDataLoader
