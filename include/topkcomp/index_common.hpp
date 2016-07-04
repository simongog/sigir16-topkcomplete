#pragma once

#include <string>
#include <utility>
#include <sdsl/int_vector.hpp>

namespace topkcomp{
    typedef std::pair<uint64_t, uint64_t>    tII;
    typedef std::pair<std::string, uint64_t> tSI;
    typedef std::vector<tSI>                 tVSI;

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

    template<typename t_label>
    struct edge_rac{
        typedef typename t_label::value_type value_type;
//        typedef size_t value_type;
        typedef size_t  size_type;
        typedef std::vector<size_t>::difference_type  difference_type;
        typedef sdsl::random_access_const_iterator<edge_rac> iterator_type;
        const t_label* m_label;
        size_t m_begin;
        size_t m_end;

        edge_rac(const t_label *p=nullptr, size_t begin=0, size_t end=0) : m_label(p), m_begin(begin), m_end(end) {};

        value_type operator[](size_type i) const{ return (*m_label)[i+m_begin]; }

        size_type size() const{ return m_end-m_begin; }

        iterator_type begin() const{
            return iterator_type(this, 0);
        }

        iterator_type end() const{
            return iterator_type(this, size());
        }
    };

} // end namespace topkcomp
