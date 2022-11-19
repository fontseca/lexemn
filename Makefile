lexemn:  lexer.o lexemn.o utilities.o book.o
	g++ -std=c++23 -Wall -Werror -o bin/lexemn lexemn.o lexer.o utilities.o book.o -lreadline

lexer.o: src/lexer.cc
	g++ -std=c++23 -Wall -Werror -c src/lexer.cc -I ./include

lexemn.o: src/lexemn.cc
	g++ -std=c++23 -Wall -Werror -c -I ./include src/lexemn.cc

book.o: src/book.cc
	g++ -std=c++23 -Wall -Werror -c -I ./include src/book.cc

utilities.o: src/utilities.cc
	g++ -std=c++23 -Wall -Werror -c -I ./include src/utilities.cc

clean:
	rm *.o