lexemn:  lexical-analyzer.o lexemn.o utilities.o
	g++ -Wall -Werror -o bin/lexemn lexemn.o lexical-analyzer.o utilities.o -lreadline

lexical-analyzer.o: src/analyzers/lexical-analyzer.cc
	g++ -Wall -Werror -c src/analyzers/lexical-analyzer.cc -I ./include

lexemn.o: src/lexemn.cc
	g++ -Wall -Werror -c -I ./include src/lexemn.cc

utilities.o: src/utilities.cc
	g++ -Wall -Werror -c -I ./include src/utilities.cc

clean:
	rm *.o