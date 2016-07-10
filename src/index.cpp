#include "topkcomp/index.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace sdsl;
using namespace topkcomp;

typedef INDEX_TYPE t_index;

int main(int argc, char* argv[]){
    using clock = chrono::high_resolution_clock;
    const string index_name = INDEX_NAME;
    const string index_file = std::string(argv[1])+"."+INDEX_NAME+".sdsl";
    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[0] << " file" << endl;
        cout << "  Constructs a top-k completion index." << endl;
        cout << "  The index will be stored in file.";
        cout << index_name << ".sdsl" << endl;
        return 1;
    }
    t_index topk_index;
    generate_index_from_file(topk_index, argv[1], index_file, index_name);

    cout << "Please enter queries line by line." << endl;
    cout << "Pressing Crtl-D will quit the program." << endl;
    string prefix;
    while ( getline(cin, prefix) ) {
        auto query_start = clock::now();
        auto result_list = topk_index.top_k(prefix, 5);
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
