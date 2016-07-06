#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/rmq_support.hpp>
#include <algorithm>
#include <queue>
#include <locale> // required by std::tolower

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v5<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>,
         typename t_rmq = sdsl::rmq_succinct_sct<0>>
class index4 {
    typedef sdsl::int_vector<8> t_label;
    typedef edge_rac<t_label>   t_edge_label;

    t_label             m_labels;     // labels of the tree
    sdsl::bit_vector    m_bp;         // balanced parantheses sequence of tree
    t_bp_support        m_bp_support; // support structure for m_bp
    t_bp_rnk10          m_bp_rnk10;   // rank for leaves in bp
    t_bp_sel10          m_bp_sel10;   // select for leaves in bp
    t_bv                m_start_bv;   // bitvector which represents the start of labels in m_labels
    t_sel               m_start_sel;  // select structure for m_start_bv
    sdsl::int_vector<>  m_priority;   //
    t_rmq               m_rmq;        // range maximum query support for m_priority


    public:
        typedef size_t size_type;

        // Constructor
        index4(const tVSI& entry_priority=tVSI()) {
            if ( entry_priority.size() == 0 ) {
                return;
            }
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
            size_t N   = entry_priority.size(), bp_idx=0, start_idx = 0, label_idx=0;
            // initialize m_priority
            m_priority = sdsl::int_vector<>(entry_priority.size(), 0, sdsl::bits::hi(max_priority)+1);
            for (size_t i=0; i < N; ++i) {
                m_priority[i] = entry_priority[i].second;
            }
            // initialize range maximum structure
            m_rmq = t_rmq(&m_priority);
            // m_bp size is at most 2*2*N
            m_labels   = sdsl::int_vector<8>(n);
            m_start_bv = sdsl::bit_vector(N+n+2, 0);
            m_bp       = sdsl::bit_vector(2*2*N, 0);
            m_start_bv[start_idx++] = 1;
            build_tree(entry_priority, bp_idx, start_idx, label_idx);
            m_bp.resize(bp_idx);
            m_labels.resize(label_idx);
            m_start_bv.resize(start_idx);
            m_start_sel  = t_sel(&m_start_bv);
            m_bp_support = t_bp_support(&m_bp);
            m_bp_rnk10   = t_bp_rnk10(&m_bp);
            m_bp_sel10   = t_bp_sel10(&m_bp);
        }

        struct priority_interval{
            uint64_t p;
            size_t idx, lb, rb;
            priority_interval(uint64_t f_p, size_t f_idx, size_t f_lb, size_t f_rb) : 
                p(f_p), idx(f_idx), lb(f_lb), rb(f_rb) {}

            bool operator<(const priority_interval& pi) const {
                if ( p != pi.p ) return p < pi.p;
                if ( idx != pi.idx ) return idx < pi.idx;
                if ( lb != pi.lb ) return lb < pi.lb;
                return rb < pi.rb;
            }
        };

        // k > 0
        tVSI top_k(const std::string& prefix, size_t k) const{
            auto range = prefix_range(prefix);
            tVSI result_list;
            std::priority_queue<priority_interval> pq;

            auto push_interval = [&](size_t f_lb, size_t f_rb) {
                if ( f_rb > f_lb ) {
                    size_t max_idx = m_rmq(f_lb, f_rb-1);
                    pq.push(priority_interval(m_priority[max_idx], max_idx, f_lb, f_rb));
                }
            };

            push_interval(range[0], range[1]);
            
            while ( result_list.size() < k and !pq.empty() ) {
                auto iv = pq.top();
                pq.pop();
                auto idx = iv.idx;
                result_list.emplace_back(label(idx), m_priority[idx]);
                push_interval(iv.lb, idx);
                push_interval(idx+1, iv.rb);
            }
            return result_list; 
        }

        // Serialize method
        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr,
                  std::string name="") const {
            using namespace sdsl;
            structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_labels.serialize(out, child, "labels");
            written_bytes += m_bp.serialize(out, child, "bp");
            written_bytes += m_bp_support.serialize(out, child, "bp_support");
            written_bytes += m_bp_rnk10.serialize(out, child, "bp_rnk10");
            written_bytes += m_bp_sel10.serialize(out, child, "bp_sel10");
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_priority.serialize(out, child, "priority");
            written_bytes += m_rmq.serialize(out, child, "rmq");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method
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
            m_priority.load(in);
            m_rmq.load(in);
        }

    private:

        void build_tree(const tVSI& entry_priority, size_t& bp_idx, size_t &start_idx, size_t &label_idx) {
            build_tree(entry_priority, 0, entry_priority.size(), 0, bp_idx, start_idx, label_idx);
        }

        void build_tree(const tVSI& entry_priority, size_t lb, size_t rb, size_t depth, size_t& bp_idx, size_t& start_idx, size_t& label_idx) {
            if ( lb >= rb )
                return;
            m_bp[bp_idx++] = 1; // append ,,(''
            size_t d = depth;
            const char* lb_entry = entry_priority[lb].first.c_str();
            const char* rb_entry = entry_priority[rb-1].first.c_str();
            while ( lb_entry[d] !=0 and lb_entry[d] == rb_entry[d] ) {
                m_labels[label_idx++] = lb_entry[d];
                start_idx++;
                ++d;
            }
            m_start_bv[start_idx++] = 1; 
            if ( lb_entry[d] == 0 ) {
                ++lb;
            }
            while ( lb < rb ) {
                char c = entry_priority[lb].first[d];
                size_t mid = lb+1;
                while ( mid < rb and entry_priority[mid].first[d] == c ) {
                    ++mid;
                }
                build_tree(entry_priority, lb, mid, d, bp_idx, start_idx, label_idx);
                lb = mid;
            }
            m_bp[bp_idx++] = 0; // append ,,)''
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
                while ( ++i < cv.size() and w_edge[0] < prefix[m] ) {
                    w_edge = edge(node_id(cv[i]));
                }
                if ( prefix[m] != w_edge[0] ) { // no matching child found
                    return {{0,0}};
                } else {
                    w = cv[i-1];
                    size_t mm = m+1;
                    while ( mm < prefix.size() and mm-m < w_edge.size() and  prefix[mm] == w_edge[mm-m] ) {
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
            return {{m_bp_rnk10(v), m_bp_rnk10(m_bp_support.find_close(v)+1)}};
        }


        // v -> [1..N]
        size_t node_id(size_t v) const{
            return m_bp_support.rank(v);
        }

        // node_id -> edge of node
        t_edge_label edge(size_t v_id) const{
            size_t begin = m_start_sel(v_id) + 1 - v_id;
            size_t end   = m_start_sel(v_id+1) + 1 - (v_id+1);
            return t_edge_label(&m_labels, begin, end);
        }

        // check if v is a leaf
        size_t is_leaf(size_t v) const {
            return m_bp[v+1] == 0;
        }

        // check if v is the root node
        size_type is_root(size_t v) const {
            return v == 0;
        }

        // parent of v
        size_type parent(size_t v) const {
            return m_bp_support.enclose(v);
        }

        // reconstruct label at position idx of original sequence
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
