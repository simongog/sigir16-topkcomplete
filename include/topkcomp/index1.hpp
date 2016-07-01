#pragma once

#include <sdsl/int_vector.hpp>

namespace topkcomp {

class index1 {
    sdsl::int_vector<8> m_text;  // stores the concatenation of all entries
    sdsl::int_vector<>  m_start; // pointers to the start of entries in m_text

    public:
        typedef size_t size_type;

        // Constructor
        index1(const std::vector<std::string, uint64_t>& entry_priority) {
            
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
