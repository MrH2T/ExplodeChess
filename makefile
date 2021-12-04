Compiler = "D:\TDM-GCC-64\bin\g++.exe"
Options = "-DUNICODE -std=c++14"
ASIOpath = "D:\asio\asio-1.20.0\include"
Linker = "-static"

.PHONY: clear

./src/bin/main.exe: ./src/bin/main.o ./src/bin/wincon.o
	$(Compiler) $^ -o $@ -lpthread -lws2_32 -lwsock32 $(Linker)

./src/bin/main.o: ./src/expchess.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)

./src/bin/wincon.o: ./src/wincontrol.cpp
	$(Compiler) $^ -c -o $@ $(Options) -I$(ASIOpath)
