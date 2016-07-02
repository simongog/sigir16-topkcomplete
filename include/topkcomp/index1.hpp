#pragma once

#include "index_common.hpp"
#include <sdsl/int_vector.hpp>
#include <algorithm>

namespace topkcomp {

class index1 {
    sdsl::int_vector<8> m_text;  // stores the concatenation of all entries
    sdsl::int_vector<>  m_start; // pointers to the start of entries in m_text
    sdsl::int_vector<>  m_priority; // priorities of entries

    public:
        typedef size_t size_type;

        // Constructor
        index1(const tVSI& entry_priority) {
            // get the length of the concatenation of all strings
            uint64_t n = std::accumulate(entry_priority.begin(), entry_priority.end(),
                            0, [](uint64_t a, std::pair<std::string, uint64_t> ep){
                                    return a + ep.first.size();
                               });
            // get maximum of priorities
            auto max_priority = std::max_element(entry_priority.begin(), entry_priority.end(),
                                    [] (const tSI& a, const tSI& b){
                                        return a.second < b.second;
                                    })->second;
            // initialize m_text
            m_text = sdsl::int_vector<8>(n);
            // initialize m_start
            m_start = sdsl::int_vector<>(n, 0, sdsl::bits::hi(n)+1);
            // initialize m_priority
            m_priority = sdsl::int_vector<>(n, 0, sdsl::bits::hi(max_priority)+1);

            size_t idx = 0;
            for (size_t i=0; i < entry_priority.size(); ++i) {
                m_start[i] = idx;
                m_priority[i] = entry_priority[i].second;
                for (auto c : entry_priority[i].first) {
                    m_text[idx++] = c;
                }
            }
        }

        tVSI top_k(std::string prefix, size_t k){
            // TODO: elegant solution using lower and upper bound
            return tVSI();    
        }

        // Serialize method
        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr,
                  std::string name="") const {
            using namespace sdsl;
            structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_text.serialize(out, child, "text");
            written_bytes += m_start.serialize(out, child, "start");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method
        void load(std::istream& in) {
            m_text.load(in);
            m_start.load(in);
        }
        
};

} // end namespace topkcomp
