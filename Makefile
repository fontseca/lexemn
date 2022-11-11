lexemn:  lexer.o lexemn.o utilities.o
	g++ -Wall -Werror -o bin/lexemn lexemn.o lexer.o utilities.o -lreadline

lexer.o: src/analyzers/lexer.cc
	g++ -Wall -Werror -c src/analyzers/lexer.cc -I ./include

lexemn.o: src/lexemn.cc
	g++ -Wall -Werror -c -I ./include src/lexemn.cc

utilities.o: src/utilities.cc
	g++ -Wall -Werror -c -I ./include src/utilities.cc

clean:
	rm *.o