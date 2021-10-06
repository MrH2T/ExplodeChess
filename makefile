Compiler = "R:\winlibs-x86_64-posix-seh-gcc-11.2.0-mingw-w64-9.0.0-r1\mingw64\bin\g++.exe"
Options = "-DUNICODE -std=c++14"
ASIOpath = "R:\asio-1.18.2\include"
Linker = "-static"

.PHONY: clear

./src/bin/main.exe: ./src/bin/main.o ./src/bin/wincon.o
	$(Compiler) $^ -o $@ -lpthread -lws2_32 -lwsock32 $(Linker)

./src/bin/main.o: ./src/expchess.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)

./src/bin/wincon.o: ./src/wincontrol.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)
