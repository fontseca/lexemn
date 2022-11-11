lexemn:  lexical-analyzer.o lexemn.o
	g++ -Wall -Werror -o bin/lexemn lexemn.o lexical-analyzer.o -lreadline

lexical-analyzer.o: src/analyzers/lexical-analyzer.cc
	g++ -Wall -Werror -c src/analyzers/lexical-analyzer.cc -I ./include

lexemn.o: src/lexemn.cc
	g++ -Wall -Werror -c -I ./include src/lexemn.cc

clean:
	rm *.o