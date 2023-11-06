#pragma once
#include "document.h"
#include "search_server.h"
#include <deque>
#include <string>
#include <vector>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate); 
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status); 
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        // определите, что должно быть в структуре
        QueryResult(std::vector<Document> result);
        bool IsEmpty();
        size_t result_count;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    int empty_results_count_;

    std::vector<Document> AddRequestResult(std::vector<Document> result);
}; 

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    return AddRequestResult( search_server_.FindTopDocuments(raw_query, document_predicate) );
}
