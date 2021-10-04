Compiler = "g++.exe"
Options = "-DUNICODE -std=c++14"
ASIOpath = "R:\asio-1.18.2\include"

.PHONY: clear

./src/bin/main.exe: ./src/bin/main.o ./src/bin/wincon.o
	$(Compiler) $^ -o $@ -lpthread -lws2_32 -lwsock32

./src/bin/main.o: ./src/expchess.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)

./src/bin/wincon.o: ./src/wincontrol.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)
