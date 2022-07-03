export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
g++ -O3 -std=c++20 disarm.cpp `llvm-config --cxxflags --ldflags --libs armdesc armdisassembler aarch64desc aarch64disassembler` -o disarm
