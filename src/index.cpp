#include "topkcomp/index1.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace sdsl;
using namespace topkcomp;

int main(int argc, char* argv[]){
    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[1] << " file" << endl;
    }
    ifstream in(argv[1]);
    tVSI entry_priority;
    string entry;
    while ( getline(in, entry, '\t')  ) {
        string s_priority;
        getline(in, s_priority);
        uint64_t priority = stoull(s_priority);
        cout << "(" << entry << ", " << priority << ")" << endl;
        entry_priority.emplace_back(entry, priority);
    }
    cout << "read " << entry_priority.size() << " entries" << endl;
    index1 idx(entry_priority);

    string prefix;
    while ( getline(cin, prefix) ) {
        cout << prefix << endl;
    }
}
