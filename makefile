debug:
	g++ src/*.cpp -I include -I test -std=c++11
test:
	g++ test/*.cpp -I include -I test -std=c++11
