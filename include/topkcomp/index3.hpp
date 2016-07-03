#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <algorithm>
#include <queue>
#include <locale> // required by std::tolower

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type>
class index3 {
    sdsl::int_vector<8> m_labels;  // labels of the tree
    sdsl::bit_vector    m_bp; // balanced parantheses sequence of tree
    t_bv                m_start_bv;  // bitvector which represents the start of labels in m_text
    t_sel               m_start_sel; // select structure for m_start_bv
    sdsl::int_vector<>  m_priority;  //

    public:
        typedef size_t size_type;

        void build_tree(const tVSI& entry_priority) {
            build_tree(entry_priority, 0, entry_priority.size(), 0);
        }

        void build_tree(const tVSI& entry_priority, size_t lb, size_t rb, size_t depth) {
            if ( lb >= rb )
                return;
            std::cout << "(";
//            std::cout<<depth<<"-["<<lb<<","<<rb-1<<"]"<<std::endl;
            size_t d = depth;
            const char* lb_entry = entry_priority[lb].first.c_str();
            const char* rb_entry = entry_priority[rb-1].first.c_str();
            while ( lb_entry[d] !=0 and lb_entry[d] == rb_entry[d] ) {
                std::cout << lb_entry[d];
                ++d;
            }
            if ( lb_entry[d] == 0 ) {
                ++lb;
            }
            while ( lb < rb ) {
                char c = entry_priority[lb].first[d];
//                std::cout<<"fc="<<fc<<" d="<<d<<std::endl;
/*              // TODO: why does binary search not work here ???
                id_rac id(rb);
                size_t mid = std::upper_bound(id.begin()+lb+1, id.begin()+rb, fc,
                                [&](char c, size_t idx) {
                                std::cout<<"comp c = "<<c<<" < " << (char)entry_priority[idx].first[d]<<" at index "<<idx<<std::endl;
                                    return c < entry_priority[idx].first[d];
                                }) - id.begin();
*/
                size_t mid = lb+1;
                while ( mid < rb and entry_priority[mid].first[d] == c ) {
                    ++mid;
                }
//                std::cout<<"["<<lb<<","<<mid-1<<"_"<<rb<<"] d="<<d<<std::endl;
                build_tree(entry_priority, lb, mid, d);
                lb = mid;
            }
            std::cout << ")";
        }

        // Constructor
        index3(const tVSI& entry_priority=tVSI()) {
            std::stack<size_t> depth;

            build_tree(entry_priority);
        }

        // k > 0
        tVSI top_k(std::string prefix, size_t k){
            tVSI result_list;
            size_t lb = 0;              // inclusive left bound
            size_t rb = m_priority.size(); // exclusive right bound
            // min-priority queue holds (priority, index)-pairs
            std::priority_queue<tII, std::vector<tII>, std::greater<tII>> pq;
            for (size_t i=lb; i<rb; ++i){
                if ( pq.size() < k ) {
                    pq.emplace(m_priority[i], i);
                } else if ( m_priority[i] > pq.top().first ) {
                    pq.pop();
                    pq.emplace(m_priority[i], i);
                }
            }
            while ( !pq.empty() ) {
                auto idx = pq.top().second;
                auto entry = std::string(m_labels.begin()+m_start_sel(idx+1), m_labels.begin()+m_start_sel(idx+2));
                result_list.emplace_back(entry, m_priority[idx]);
                pq.pop();
            }
            std::reverse(result_list.begin(), result_list.end());
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
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_priority.serialize(out, child, "priority");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method
        void load(std::istream& in) {
            m_labels.load(in);
            m_bp.load(in);
            m_start_bv.load(in);
            m_start_sel.load(in);
            m_start_sel.set_vector(&m_start_bv);
            m_priority.load(in);
        }
};

} // end namespace topkcomp
