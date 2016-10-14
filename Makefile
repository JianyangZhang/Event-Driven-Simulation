pa1:mm1.o rng.o
	g++ -lrt mm1.o rng.o -o pa1

mm1.o:mm1.cpp
	g++ -c mm1.cpp

rng.o:rng.cpp
	g++ -c rng.cpp
	
clean:
	rm -rf *o pa1
