lexemn:  lexical-analyzer.o lexemn.o
	g++ -Wall -Werror -o bin/lexemn lexemn.o lexical-analyzer.o -lreadline

lexical-analyzer.o: src/analyzers/lexical-analyzer.cpp
	g++ -Wall -Werror -c src/analyzers/lexical-analyzer.cpp -I ./include

lexemn.o: src/lexemn.cpp
	g++ -Wall -Werror -c -I ./include src/lexemn.cpp

clean:
	rm *.o