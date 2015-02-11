serialoptim:
	g++ -std=c++11 -Wall serialoptim.cpp -o serialoptim

serialneoptim:
	g++ -std=c++11 -Wall serialneoptim.cpp -o serialneoptim

paraleloptim:
	g++ -std=c++11 -Wall -fopenmp paraleloptim.cpp -o paraleloptim

paralelneoptim:
	g++ -std=c++11 -Wall -fopenmp paralelneoptim.cpp -o paralelneoptim

all: serialoptim serialneoptim paraleloptim paralelneoptim

clean:
	rm -rf serialoptim serialneoptim paraleloptim paralelneoptim
