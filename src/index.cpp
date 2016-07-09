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
    const string index_name = INDEX_NAME;
    const string index_file = std::string(argv[1])+"."+INDEX_NAME+".sdsl";
    if ( argc < 2 ) {
        cout << "Usage: ./" << argv[0] << " file" << endl;
        cout << "  Constructs a top-k completion index." << endl;
        cout << "  The index will be stored in file.";
        cout << index_name << ".sdsl" << endl;
        return 1;
    }
    using clock = std::chrono::high_resolution_clock;
    ifstream in(argv[1]);
    tVPSU string_weight;
    string entry;
    while ( getline(in, entry, '\t')  ) {
        string s_weight;
        getline(in, s_weight);
        uint64_t weight = stoull(s_weight);
        string_weight.emplace_back(entry, weight);
    }
    sort(string_weight.begin(), string_weight.end());
    cout << "read and sorted " << string_weight.size() << " entries" << endl;

    auto construction_start = clock::now();
    t_index topk_index(string_weight);
    auto construction_time = clock::now() - construction_start;
    auto construction_ms    = chrono::duration_cast<chrono::milliseconds>(construction_time).count();
    cout << "(construction took "<< std::setprecision(3) << construction_ms / 1000.0;
    cout << " s)" << endl;
    sdsl::store_to_file(topk_index, index_file);
    write_structure<HTML_FORMAT>(topk_index, string(argv[1])+"."+index_name+".html");
    cout << "(index size is " << size_in_mega_bytes(topk_index) << " MiB)" << endl;
    sdsl::load_from_file(topk_index, index_file);

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
