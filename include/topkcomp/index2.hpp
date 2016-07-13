#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <algorithm>

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type>
class index2 {
    sdsl::int_vector<8> m_text;      // stores the concatenation of all strings
    sdsl::int_vector<>  m_weight;    // weights of strings
    t_bv                m_start_bv;  // marks start of strings in m_text
    t_sel               m_start_sel; // select structure for m_start_bv

    public:
        typedef size_t size_type;
        constexpr static bool case_sensitive = true;

        // Constructor takes a sorted list of (string,weight)-pairs
        index2(const tVPSU& string_weight=tVPSU()) {
            using namespace sdsl;
            if ( !string_weight.empty() ) {
                uint64_t N, n, max_weight;
                std::tie(N, n, max_weight) = input_stats(string_weight);

                // initialize m_text
                m_text = int_vector<8>(n);
                // initialize bitvector for starting positions
                bit_vector start(n+1, 0);
                // initialize m_weight
                m_weight = int_vector<>(N, 0, bits::hi(max_weight)+1);

                size_t idx = 0;
                for (size_t i=0; i < string_weight.size(); ++i) {
                    start[idx] = 1;
                    m_weight[i] = string_weight[i].second;
                    for (auto c : string_weight[i].first) {
                        m_text[idx++] = c;
                    }
                }
                start[idx] = 1;
                // initialize member bit vector
                m_start_bv = t_bv(std::move(start));
                // initialize select structure for member bit vector
                m_start_sel = t_sel(&m_start_bv);
            }
        }

        // Return range [lb, rb) of matching strings
        t_range prefix_range(const std::string& prefix) const {
            t_range res = {{0, m_weight.size()}};
            id_rac id(m_weight.size());
            for (size_t i=0; i<prefix.size(); ++i) {
                // use binary search at each step to narrow the interval
                res[0] = std::lower_bound(id.begin()+res[0], id.begin()+res[1],
                            prefix[i],  [&](size_t idx, char c){
                                return m_text[m_start_sel(idx+1)+i] < c;
                            }) - id.begin();
                res[1] = std::upper_bound(id.begin()+res[0], id.begin()+res[1],
                            prefix[i],  [&](char c, size_t idx){
                                return c < m_text[m_start_sel(idx+1)+i];
                            }) - id.begin();
            }
            return res;
        }


        // k > 0
        tVPSU top_k(const std::string& prefix, size_t k) const {
            auto range = prefix_range(prefix);
            auto top_idx = heaviest_indexes_in_range(k, range, m_weight);
            tVPSU result_list(top_idx.size());
            for (size_t i=0; i < top_idx.size(); ++i){
                auto idx = top_idx[i];
                auto s = std::string(m_text.begin()+m_start_sel(idx+1), 
                                     m_text.begin()+m_start_sel(idx+2));
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
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_weight.serialize(out, child, "weight");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method (calls load method of each member)
        void load(std::istream& in) {
            m_text.load(in);
            m_start_bv.load(in);
            m_start_sel.load(in);
            // next: set pointer to structure which will be supported
            m_start_sel.set_vector(&m_start_bv); 
            m_weight.load(in);
        }
};

} // end namespace topkcomp
