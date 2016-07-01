#include "topkcomp/index1.hpp"
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;
using namespace sdsl;

int main(int argc, char* argv[]){
    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[1] << " file" << endl;
    }
    ifstream in(argv[1]);
    string entry;
    while ( getline(in, entry, '\t')  ) {
        string s_priority;
        getline(in, s_priority);
        uint64_t priority = stoull(s_priority);
        cout << "(" << entry << ", " << priority << ")" << endl;
    }
}
