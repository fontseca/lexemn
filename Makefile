lexemn:  lexer.o lexemn.o utility.o book.o
	g++ -std=c++23 -Wall -Werror -pedantic -o bin/lexemn lexemn.o lexer.o utility.o book.o -lreadline

lexer.o: src/lexer.cc
	g++ -std=c++23 -Wall -Werror -pedantic -c src/lexer.cc -I ./include

lexemn.o: src/lexemn.cc
	g++ -std=c++23 -Wall -Werror -pedantic -c -I ./include src/lexemn.cc

book.o: src/book.cc
	g++ -std=c++23 -Wall -Werror -pedantic -c -I ./include src/book.cc

utility.o: src/utility.cc
	g++ -std=c++23 -Wall -Werror -pedantic -c -I ./include src/utility.cc

clean:
	rm *.o