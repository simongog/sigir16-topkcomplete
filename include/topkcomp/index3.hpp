#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type,
         typename t_rac_weight = sdsl::int_vector<>,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>>
class index3 {
    typedef sdsl::int_vector<8> t_label;
    typedef edge_rac<t_label>   t_edge_label;

    t_label             m_labels;     // concatenation of tree labels
    sdsl::bit_vector    m_bp;         // balanced parentheses sequence of tree
    t_bp_support        m_bp_support; // support structure for m_bp
    t_bp_rnk10          m_bp_rnk10;   // rank for leaf nodes in m_bp
    t_bp_sel10          m_bp_sel10;   // select for leaf nodes in m_bp
    t_bv                m_start_bv;   // marks start of labels in m_labels
    t_sel               m_start_sel;  // select structure for m_start_bv
    t_rac_weight        m_weight;     // weights of strings 


    public:
        typedef size_t size_type;
        constexpr static bool case_sensitive = true;

        // Constructor takes a sorted list of (string,weight)-pairs
        index3(const tVPSU& string_weight=tVPSU()) {
            using namespace sdsl;
            if ( !string_weight.empty() ) {
                uint64_t N, n, max_weight;
                std::tie(N, n, max_weight) = input_stats(string_weight);
                // initialize m_weight
                {
                    int_vector<> weight(N, 0, bits::hi(max_weight)+1);
                    for (size_t i=0; i < N; ++i) {
                        weight[i] = string_weight[i].second;
                    }
                    m_weight = t_rac_weight(weight);
                }
                // build the succinct tree
                build_tree(string_weight, N, n);
                // initialize the support structures
                m_start_sel  = t_sel(&m_start_bv);
                m_bp_support = t_bp_support(&m_bp);
                m_bp_rnk10   = t_bp_rnk10(&m_bp);
                m_bp_sel10   = t_bp_sel10(&m_bp);
            }
        }
 
        // k > 0
        tVPSU top_k(const std::string& prefix, size_t k) const {
            auto range = prefix_range(prefix);
            auto top_idx = heaviest_indexes_in_range(k, range, m_weight);
            tVPSU result_list;
            for (auto idx : top_idx){
                result_list.push_back(tPSU(label(idx), m_weight[idx]));
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
            written_bytes += m_labels.serialize(out, child, "labels");
            written_bytes += m_bp.serialize(out, child, "bp");
            written_bytes += m_bp_support.serialize(out, child, "bp_support");
            written_bytes += m_bp_rnk10.serialize(out, child, "bp_rnk10");
            written_bytes += m_bp_sel10.serialize(out, child, "bp_sel10");
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_weight.serialize(out, child, "weight");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method (calls load method of each member)
        void load(std::istream& in) {
            m_labels.load(in);
            m_bp.load(in);
            m_bp_support.load(in, &m_bp);
            m_bp_support.set_vector(&m_bp);
            m_bp_rnk10.load(in);
            m_bp_rnk10.set_vector(&m_bp);
            m_bp_sel10.load(in);
            m_bp_sel10.set_vector(&m_bp);
            m_start_bv.load(in);
            m_start_sel.load(in);
            m_start_sel.set_vector(&m_start_bv);
            m_weight.load(in);
        }

    private:

        // Build balanced parentheses sequence of the trie of the strings
        void build_tree(const tVPSU& string_weight, size_t N, size_t n) {
            using namespace sdsl;
            bit_vector start_bv(2*N+n+2, 0);   // initialize to worst case size
            m_labels   = int_vector<8>(n);     // initialize to worst case size
            m_bp       = bit_vector(2*2*N, 0); // initialize to worst case size


            auto bp_it    = m_bp.begin();
            auto start_it = start_bv.begin();
            auto label_it = m_labels.begin();
            *(start_it++) = 1;                // mark start of first label 
            build_tree(string_weight, 0, N, 0, bp_it, start_it, label_it);
            m_bp.resize(bp_it-m_bp.begin());            // resize to actual size
            m_labels.resize(label_it-m_labels.begin()); // resize to actual size
            start_bv.resize(start_it-start_bv.begin()); // resize to actual size

            m_start_bv = t_bv(start_bv);     // copy to member bitvector
        }

        // Recursive tree construction
        void
        build_tree(const tVPSU& string_weight, size_t lb, size_t rb, size_t depth,
                   sdsl::bit_vector::iterator& bp_it,
                   sdsl::bit_vector::iterator& start_it,
                   sdsl::int_vector<8>::iterator& label_it) 
        {
            if ( lb >= rb )
                return;
            *(bp_it++) = 1; // append ,,(''
            size_t d = depth;
            const uint8_t* lb_entry = (const uint8_t*)(string_weight[lb].first.c_str());
            const uint8_t* rb_entry = (const uint8_t*)(string_weight[rb-1].first.c_str());
            // extend common prefix
            while ( lb_entry[d] !=0 and lb_entry[d] == rb_entry[d] ) {
                *(label_it++) = lb_entry[d]; // store common char
                ++start_it; ++d;
            }
            *(start_it++) = 1; // mark end of edge label
            // if node is not a leaf
            if ( lb+1 < rb) {
                // handle children
                while ( lb < rb ) {
                    uint8_t c = string_weight[lb].first.c_str()[d];
                    size_t mid = lb+1;
                    while ( mid < rb and ((uint8_t)string_weight[mid].first.c_str()[d]) == c ) {
                        ++mid;
                    }
                    build_tree(string_weight, lb, mid, d, bp_it, start_it, label_it);
                    lb = mid;
                }
            }
            bp_it++; // move iterator to right; e.g. append ,,)''
        }

        // Return range [lb, rb) of matching entries
        std::array<size_t,2> prefix_range(const std::string& prefix) const {
            size_t v = 0; // node is represented by position of opening parenthesis in bp
            size_t m = 0; // length of common prefix
            while ( m < prefix.size() ) {
                auto cv = children(v);
                if ( cv.size() == 0 ) { // v is already a leaf, prefix is longer than leaf
                    return {{0,0}};
                }
                auto w = v;
                auto w_edge = edge(node_id(cv[0]));
                size_t i = 0;
                while ( ++i < cv.size() and w_edge[0] < ((uint8_t)prefix[m]) ) {
                    w_edge = edge(node_id(cv[i]));
                }
                if ( ((uint8_t)prefix[m]) != w_edge[0] ) { // no matching child found
                    return {{0,0}};
                } else {
                    w = cv[i-1];
                    size_t mm = m+1;
                    while ( mm < prefix.size() and mm-m < w_edge.size() and  (uint8_t)prefix[mm] == w_edge[mm-m] ) {
                        ++mm;
                    }
                    // edge search exhausted 
                    if ( mm-m == w_edge.size() ){
                        v = w;
                        m = mm;
                    } else { // edge search not exhausted
                        if ( mm == prefix.size() ) { // pattern exhausted
                            v = w;
                            m = mm;
                        } else { // pattern not exhausted -> mismatch
                            return {{0,0}};
                        }
                    }
                }
            }
            // Map from sub tree rooted at v to entries in the original array
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)}};
        }
        
       // Map node v to its unique identifier. node_id : v -> [1..N]
        size_t node_id(size_t v) const{
            return m_bp_support.rank(v);
        }

        // Get edge label leading to node v with node_id(v) = v_id
        t_edge_label edge(size_t v_id) const{
            size_t begin = m_start_sel(v_id) + 1 - v_id;
            size_t end   = m_start_sel(v_id+1) + 1 - (v_id+1);
            return t_edge_label(&m_labels, begin, end);
        }

        // Check if v is a leaf
        size_t is_leaf(size_t v) const {
            return m_bp[v+1] == 0;
        }

        // Check if v is the root node
        size_type is_root(size_t v) const {
            return v == 0;
        }

        // Return parent of v
        size_type parent(size_t v) const {
            return m_bp_support.enclose(v);
        }

        // Reconstruct label at position idx of original sequence
        std::string label(size_t idx) const {
            std::stack<size_t> node_stack;
            node_stack.push(m_bp_sel10(idx+1)-1);
            while ( !is_root(node_stack.top()) ) {
                size_t p = parent(node_stack.top());
                node_stack.push(p);
            }
            std::string res;
            while ( !node_stack.empty() ){
                auto e = edge(node_id(node_stack.top()));
                res.append(e.begin(), e.end());
                node_stack.pop();
            }
            return res;
        }

        // Return all children of v
        std::vector<size_t> children(size_t v) const {
            std::vector<size_t> res;
            size_t cv = v+1;
            while ( m_bp[cv] ) {
                res.push_back(cv);
                cv = m_bp_support.find_close(cv) + 1;
            }
            return res;
        }

 };

} // end namespace topkcomp
