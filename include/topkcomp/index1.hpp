#pragma once

#include "index_common.hpp"
#include <sdsl/int_vector.hpp>
#include <algorithm>
#include <array>

namespace topkcomp {

class index1 {
    sdsl::int_vector<8> m_text;   // stores the concatenation of all strings
    sdsl::int_vector<>  m_start;  // pointers to the start of strings in m_text
    sdsl::int_vector<>  m_weight; // weights of strings

    public:
        typedef size_t size_type;

        // Constructor takes a sorted list of (string,weight)-pairs
        index1(const tVPSU& string_weight=tVPSU()) {
            using namespace sdsl;
            if ( !string_weight.empty() ) {
                uint64_t N, n, max_weight;
                std::tie(N, n, max_weight) = input_stats(string_weight);

                // initialize m_text
                m_text = int_vector<8>(n);
                // initialize m_start
                m_start = int_vector<>(N+1, 0, bits::hi(n)+1);
                // initialize m_weight
                m_weight = int_vector<>(N, 0, bits::hi(max_weight)+1);

                size_t idx = 0;
                for (size_t i=0; i < N; ++i) {
                    m_start[i] = idx;
                    m_weight[i] = string_weight[i].second;
                    for (auto c : string_weight[i].first) {
                        m_text[idx++] = c;
                    }
                }
                m_start[string_weight.size()] = idx;
            }
        }

        // Return range [lb, rb) of matching entries
        t_range prefix_range(const std::string& prefix) const {
            t_range res = {{0, m_weight.size()}};
            for (size_t i=0; i<prefix.size(); ++i) {
                // use binary search at each step to narrow the interval
                res[0] = std::lower_bound(m_start.begin()+res[0], m_start.begin()+res[1],
                        prefix[i],  [&](uint64_t idx, uint8_t c){
                                        return m_text[idx+i] < c;
                                    }) - m_start.begin();
                res[1] = std::upper_bound(m_start.begin()+res[0], m_start.begin()+res[1],
                        prefix[i],  [&](uint8_t c, uint64_t idx){
                                        return c < m_text[idx+i];
                                    }) - m_start.begin();
            }
            return res;
        }

        // k > 0
        tVPSU top_k(const std::string& prefix, size_t k) const {
            auto range   = prefix_range(prefix);
            auto top_idx = heaviest_indexes_in_range(k, range, m_weight);
            tVPSU result_list(top_idx.size());
            for (size_t i=0; i < top_idx.size(); ++i){
                auto idx = top_idx[i];
                auto s = std::string(m_text.begin()+m_start[idx], 
                                     m_text.begin()+m_start[idx+1]); 
                result_list[i] = tPSU(s, m_weight[idx]);
            }
            return result_list; 
        }

        // Serialize method (calls serialize method of each member)
        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr,
                  std::string name="") const {
            using namespace sdsl;
            auto child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_text.serialize(out, child, "text");
            written_bytes += m_start.serialize(out, child, "start");
            written_bytes += m_weight.serialize(out, child, "weight");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method (calls load method of each member)
        void load(std::istream& in) {
            m_text.load(in);
            m_start.load(in);
            m_weight.load(in);
        }
};

} // end namespace topkcomp
