#include <iostream>
#include <cstring>
#include "tgp.h"

int main() {
    std::cout << "Testing TGP allocation..." << std::endl;

    // Test 1: Allocation on stack (this might cause stack overflow)
    std::cout << "Testing stack allocation..." << std::endl;
    try {
        TGP tgp_stack;
        std::cout << "Stack allocation successful!" << std::endl;
    } catch (...) {
        std::cout << "Stack allocation failed!" << std::endl;
    }

    // Test 2: Allocation on heap (this should work)
    std::cout << "Testing heap allocation..." << std::endl;
    try {
        TGP* tgp_heap = new TGP();
        std::cout << "Heap allocation successful!" << std::endl;
        delete tgp_heap;
    } catch (...) {
        std::cout << "Heap allocation failed!" << std::endl;
    }

    return 0;
}