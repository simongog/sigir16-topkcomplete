#include "topkcomp/index1.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace sdsl;
using namespace topkcomp;


int main(int argc, char* argv[]){
    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[1] << " file" << endl;
    }
    using clock = std::chrono::high_resolution_clock;
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
    sort(entry_priority.begin(), entry_priority.end());
    index1 idx(entry_priority);

    string prefix;
    while ( getline(cin, prefix) ) {
        auto query_start = clock::now();
        auto result_list = idx.top_k(prefix, 5);
        auto query_time  = clock::now() - query_start;
        auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
        
        cout << "-- top results:" << endl;
        for (size_t i=0; i<result_list.size(); ++i) {
            cout << result_list[i].first << "  " << result_list[i].second << endl;
        }
        cout << "-- (" << std::setprecision(3) << query_us / 1000.0;
        cout << " ms)" << endl;
    }
}
