#pragma once

#include "index_common.hpp"
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/bp_support.hpp>
#include <sdsl/rmq_support.hpp>
#include <algorithm>
#include <queue>
#include <locale> // required by std::tolower

namespace topkcomp {

template<typename t_csa = sdsl::csa_wt<>,
         typename t_bv = sdsl::sd_vector<>,
         typename t_rnk= typename t_bv::rank_1_type,
         typename t_sel= typename t_bv::select_1_type,
         typename t_rmq = sdsl::rmq_succinct_sct<0>>
class index5 {
    typedef sdsl::int_vector<8> t_label;
    typedef edge_rac<t_label>   t_edge_label;

    t_csa              m_csa;   // compressed suffix array CSA
    t_bv               m_start; // marks the starts of the entries in CSA
    t_rnk              m_start_rnk; // rank support structure for m_start
    t_sel              m_start_sel; // select support structure for m_start
    sdsl::int_vector<> m_priority;   //
    t_rmq              m_rmq;        // range maximum query support for m_priority


    public:
        typedef size_t size_type;

        // Constructor
        index5(const tVSI& entry_priority=tVSI()) {
            {
                std::string concat;
                for (auto ep : entry_priority) {
                    concat.append(ep.first.begin(), ep.first.end());
                }
                construct_im(m_csa, concat.c_str(), 1);
            }
            {
                sdsl::bit_vector bv(m_csa.size(), 0);
                size_t sa_pos = 0;
                for (size_t i = 0; i < entry_priority.size(); ++i){
                    for (size_t j = 0; j < entry_priority[entry_priority.size()-i-1].first.size(); ++j) {
                        sa_pos = m_csa.lf[sa_pos];
                    }
                    bv[sa_pos] = 1;
                }
                m_start = t_bv(bv);
                m_start_rnk = t_rnk(&m_start);
                m_start_sel = t_sel(&m_start);
            }

            // get maximum of priorities
            auto max_priority = std::max_element(entry_priority.begin(), entry_priority.end(),
                                    [] (const tSI& a, const tSI& b){
                                        return a.second < b.second;
                                    })->second;
            size_t N   = entry_priority.size();
            // initialize m_priority
            m_priority = sdsl::int_vector<>(entry_priority.size(), 0, sdsl::bits::hi(max_priority)+1);
            for (size_t i=0; i < N; ++i) {
                m_priority[i] = entry_priority[i].second;
            }
            // initialize range maximum structure
            m_rmq = t_rmq(&m_priority);
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
            written_bytes += m_csa.serialize(out, child, "csa");
            written_bytes += m_start.serialize(out, child, "start");
            written_bytes += m_start_rnk.serialize(out, child, "start_rnk");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_priority.serialize(out, child, "priority");
            written_bytes += m_rmq.serialize(out, child, "rmq");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method
        void load(std::istream& in) {
            m_csa.load(in);
            m_start.load(in);
            m_start_rnk.load(in);
            m_start_rnk.set_vector(&m_start);
            m_start_sel.load(in);
            m_start_sel.set_vector(&m_start);
            m_priority.load(in);
            m_rmq.load(in);
        }

    private:

        // Return range [lb, rb) of matching entries
        std::array<size_t,2> prefix_range(const std::string& prefix) const {
            auto sa_range = lex_interval(m_csa, prefix.begin(), prefix.end());
            return {{m_start_rnk(sa_range[0]), m_start_rnk(sa_range[1])}};
        }

        std::string label(size_t idx) const {
            size_t sa_pos = m_start_sel(idx+1);
            std::string res;
            do {
                res.append(1, first_row_symbol(sa_pos, m_csa));
                sa_pos = m_csa.psi[sa_pos];
            } while ( !m_start[sa_pos] );
            return res; 
        }
};

} // end namespace topkcomp
