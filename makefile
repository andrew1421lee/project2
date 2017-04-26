FLAGS = 

all: cache-sim

cache-sim: Driver.o Utils.o
	g++ $(FLAGS) Driver.o Utils.o -o cache-sim

Driver.o: Driver.cpp
	g++ $(FLAGS) -c Driver.cpp

Utils.o: Utils.cpp
	g++ $(FLAGS) -c Utils.cpp

clean:
	rm -f *.o cache-sim


