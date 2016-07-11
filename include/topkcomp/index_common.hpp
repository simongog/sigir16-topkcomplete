#pragma once

#include <string>
#include <utility>
#include <queue>
#include <array>
#include <sdsl/int_vector.hpp>

namespace topkcomp{

    // ---
    // Guru of the week case-insensitive string class
    // http://www.gotw.ca/gotw/029.htm
    struct ci_char_traits : public std::char_traits<char> {
        static bool eq(char c1, char c2) { return (uint8_t)toupper(c1) == (uint8_t)toupper(c2); }
        static bool ne(char c1, char c2) { return (uint8_t)toupper(c1) != (uint8_t)toupper(c2); }
        static bool lt(char c1, char c2) { return (uint8_t)toupper(c1) <  (uint8_t)toupper(c2); }
        static int compare(const char* s1, const char* s2, size_t n) {
            while( n-- != 0 ) {
                if( (uint8_t)toupper(*s1) < (uint8_t)toupper(*s2) ) return -1;
                if( (uint8_t)toupper(*s1) > (uint8_t)toupper(*s2) ) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char* find(const char* s, int n, char a) {
            while( n-- > 0 && (uint8_t)toupper(*s) != (uint8_t)toupper(a) ) {
                ++s;
            }
            return s;
        }
    };

    typedef std::basic_string<char, ci_char_traits> ci_string;

    std::basic_ostream<char, std::char_traits<char>>&
    operator<<(std::basic_ostream<char, std::char_traits<char>>& os, const ci_string& str) {
        os << str.c_str();
        return os;
    }
    // ---


    // helpful typedefs
    typedef std::pair<uint64_t, uint64_t>            tPUU;
    typedef std::tuple<uint64_t, uint64_t, uint64_t> tTUUU;
    typedef std::pair<ci_string, uint64_t>           tPSU;
    typedef std::vector<uint64_t>                    tVU;
    typedef std::vector<tPSU>                        tVPSU;
    typedef std::array<size_t,2>                     t_range;

    // Get input statistics of (string, weight)-list
    // \returns A tuple consisting of
    //        * the length of the (string, weight)-list
    //        * the total length of all strings
    //        * the maximal weight of a string
    tTUUU input_stats(const tVPSU& string_weight) {
         // get the length of the concatenation of all strings
        uint64_t n = std::accumulate(string_weight.begin(), string_weight.end(),
                        0, [](uint64_t a, const tPSU& ep){
                                return a + ep.first.size();
                           });
        // get maximum of priorities
        uint64_t max_weight = std::max_element(string_weight.begin(), string_weight.end(),
                                [] (const tPSU& a, const tPSU& b){
                                    return a.second < b.second;
                                })->second;   
        return tTUUU(string_weight.size(), n, max_weight);
    }

    // Get k heaviest indexes in range r
    template<typename t_rac_weight>
    tVU heaviest_indexes_in_range(size_t k, t_range r, const t_rac_weight& w){
         // min-priority queue holds (weight, index)-pairs
        std::priority_queue<tPUU, std::vector<tPUU>, std::greater<tPUU>> pq;
        for (size_t i=r[0]; i<r[1]; ++i){
            if ( pq.size() < k ) {
                pq.emplace(w[i], i);
            } else if ( w[i] > pq.top().first ) {
                pq.pop();
                pq.emplace(w[i], i);
            }
        }
        tVU res(pq.size());
        // insert k heaviest indexes into result vector
        while ( !pq.empty() ) {
            res[pq.size()-1] = pq.top().second;
            pq.pop();
        }
        return res; 
    }

    // helper struct for top-k calculation with rmq
    struct weight_interval{
        uint64_t w;
        size_t idx, lb, rb;
        weight_interval(uint64_t f_w, size_t f_idx, size_t f_lb, size_t f_rb) : 
            w(f_w), idx(f_idx), lb(f_lb), rb(f_rb) {}

        bool operator<(const weight_interval& wi) const {
            return std::tie(w, idx, lb, rb) < std::tie(wi.w, wi.idx, wi.lb, wi.rb);
        }
    };

    // Get k heaviest indexes in range r using a rmq structure
    template<typename t_rac_weight, typename t_rmq>
    tVU heaviest_indexes_in_range(size_t k, t_range r, const t_rac_weight& w, const t_rmq& rmq){
        std::priority_queue<weight_interval> pq;
        auto push_interval = [&](size_t f_lb, size_t f_rb) {
            if ( f_rb > f_lb ) {
                size_t max_idx = rmq(f_lb, f_rb-1);
                pq.push(weight_interval(w[max_idx], max_idx, f_lb, f_rb));
            }
        };
        tVU res;
        push_interval(r[0], r[1]);
        while ( res.size() < k and !pq.empty() ) {
            auto iv = pq.top(); pq.pop();
            res.push_back(iv.idx);
            push_interval(iv.lb, iv.idx);
            push_interval(iv.idx+1, iv.rb);
        }
        return res; 
    }

    // helper struct for edge label
    template<typename t_label>
    struct edge_rac{
        typedef typename t_label::value_type value_type;
        typedef size_t  size_type;
        typedef std::vector<size_t>::difference_type  difference_type;
        typedef sdsl::random_access_const_iterator<edge_rac> iterator_type;
        const t_label* m_label;
        size_t m_begin;
        size_t m_end;

        edge_rac(const t_label *p=nullptr, size_t begin=0, size_t end=0) : m_label(p), m_begin(begin), m_end(end) {};

        value_type operator[](size_type i) const{
            if ( i < size() ) {
                return (*m_label)[i+m_begin]; 
            } else { // return C-string sentinel character
                return 0;
            }
        }

        size_type size() const{ return m_end-m_begin; }

        iterator_type begin() const{
            return iterator_type(this, 0);
        }

        iterator_type end() const{
            return iterator_type(this, size());
        }
    };

    // constant space random access container for id function
    struct id_rac{
        typedef size_t value_type;
        typedef size_t  size_type;
        typedef std::vector<size_t>::difference_type  difference_type;
        typedef sdsl::random_access_const_iterator<id_rac> iterator_type;
        size_t m_size;

        id_rac(size_t size=0) : m_size(size) {};

        value_type operator[](size_type i) const{ return i; }

        size_type size() const{ return m_size; }

        iterator_type begin() const{
            return iterator_type(this, 0);
        }

        iterator_type end() const{
            return iterator_type(this, size());
        }
    };

} // end namespace topkcomp
