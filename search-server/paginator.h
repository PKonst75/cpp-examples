#pragma once
#include <iostream>
#include <vector>

template <typename Iterator>
class Page{
    public:
        Page(Iterator page_start, Iterator page_end)
        : page_start_(page_start), page_end_(page_end), size_(static_cast<size_t>(std::distance(page_end_, page_start_)))
        {

        }
        Iterator begin()const {
            return page_start_;
        }
        Iterator end()const {
            return page_end_;
        }
        size_t size() const {
            return size_;
        }
    private:
        Iterator page_start_;
        Iterator page_end_;
        size_t size_;
};

template <typename Iterator>
std::ostream& operator << (std::ostream& out, const Page<Iterator>& p){
    for(Iterator it = p.begin(); it != p.end(); ++it){
        out << *it;
    }
    return out;
 }

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator container_begin, Iterator container_end, size_t page_size)
    :  container_begin_(container_begin), container_end_(container_end), page_size_(page_size)
    {
        Iterator it = container_begin_;
        size_t in_page = 0;
        Iterator page_begin = it;
        while(it != container_end_){
            while(in_page < page_size && it != container_end_){
                ++it;
                ++in_page;
            }
            pages_.push_back(Page<Iterator>(page_begin, it));
            in_page = 0;
            page_begin = it;
        }
    }
    auto begin()const {
        return pages_.begin();
    }
    auto end()const {
        return pages_.end();
    }
    size_t size() const {
        return pages_.size();
    }

private:
    std::vector< Page<Iterator> > pages_;
    Iterator container_begin_;
    Iterator container_end_;
    size_t page_size_;

}; 

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}