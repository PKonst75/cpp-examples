#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
:  search_server_ (search_server),
   empty_results_count_(0)
{
      
}
// сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddRequestResult(search_server_.FindTopDocuments(raw_query, status));
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return AddRequestResult(search_server_.FindTopDocuments(raw_query));
}
int RequestQueue::GetNoResultRequests() const {
    return empty_results_count_;
}
     
std::vector<Document> RequestQueue::AddRequestResult(std::vector<Document> result){
    requests_.push_back(QueryResult(result));
    if(result.empty()){
        ++empty_results_count_;
    }
    if( requests_.size() > min_in_day_ ){
        if(requests_.front().IsEmpty()){
            --empty_results_count_;
        }
        requests_.pop_front();
    }
    return result;
}

RequestQueue::QueryResult::QueryResult(std::vector<Document> result)
: result_count(result.size())
{
}
bool RequestQueue::QueryResult::IsEmpty(){
    return !(static_cast<bool>(result_count));
}
