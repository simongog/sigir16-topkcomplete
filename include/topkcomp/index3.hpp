#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <sdsl/bp_support.hpp>
#include <algorithm>
#include <queue>
#include <locale> // required by std::tolower

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type,
         typename t_bp_support = sdsl::bp_support_sada<>,
         typename t_bp_rnk10 = sdsl::rank_support_v<10,2>,
         typename t_bp_sel10 = sdsl::select_support_mcl<10,2>>
class index3 {
    sdsl::int_vector<8> m_labels;  // labels of the tree
    sdsl::bit_vector    m_bp; // balanced parantheses sequence of tree
    t_bp_support        m_bp_support; // support structure for m_bp
    t_bp_rnk10          m_bp_rnk10;  // rank for leaves in bp
    t_bp_sel10          m_bp_sel10;  // select for leaves in bp
    t_bv                m_start_bv;  // bitvector which represents the start of labels in m_text
    t_sel               m_start_sel; // select structure for m_start_bv
    sdsl::int_vector<>  m_priority;  //

    public:
        typedef size_t size_type;

        void build_tree(const tVSI& entry_priority, size_t& bp_idx, size_t &start_idx, size_t &label_idx) {
            build_tree(entry_priority, 0, entry_priority.size(), 0, bp_idx, start_idx, label_idx);
        }

        void build_tree(const tVSI& entry_priority, size_t lb, size_t rb, size_t depth, size_t& bp_idx, size_t& start_idx, size_t& label_idx) {
            if ( lb >= rb )
                return;
            m_bp[bp_idx++] = 1; // append ,,(''
            std::cout << "(";
//            std::cout<<depth<<"-["<<lb<<","<<rb-1<<"]"<<std::endl;
            size_t d = depth;
            const char* lb_entry = entry_priority[lb].first.c_str();
            const char* rb_entry = entry_priority[rb-1].first.c_str();
            while ( lb_entry[d] !=0 and lb_entry[d] == rb_entry[d] ) {
                std::cout << lb_entry[d];
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
                build_tree(entry_priority, lb, mid, d, bp_idx, start_idx, label_idx);
                lb = mid;
            }
            std::cout << ")";
            m_bp[bp_idx++] = 0; // append ,,)''
        }

        // Constructor
        index3(const tVSI& entry_priority=tVSI()) {
            // get the length of the concatenation of all strings
            uint64_t n = std::accumulate(entry_priority.begin(), entry_priority.end(),
                            0, [](uint64_t a, std::pair<std::string, uint64_t> ep){
                                    return a + ep.first.size();
                               });
            // m_bp size is at most 2*2*N
            // 
            size_t N   = entry_priority.size(), bp_idx=0, start_idx = 0, label_idx=0;
            m_labels   = sdsl::int_vector<8>(n);
            m_start_bv = sdsl::bit_vector(N+n+2, 0);
            m_bp       = sdsl::bit_vector(2*2*N, 0);
            m_start_bv[start_idx++] = 1;
            build_tree(entry_priority, bp_idx, start_idx, label_idx);
            m_bp.resize(bp_idx);
            m_labels.resize(label_idx);
            m_start_bv.resize(start_idx);
            std::cout<<"m_bp="<<m_bp<<std::endl;
            std::cout<<"m_labels=";
            for(char c : m_labels ) std::cout << c;
            std::cout<<std::endl;
            std::cout<<"m_start_bv="<<m_start_bv<<std::endl;
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
