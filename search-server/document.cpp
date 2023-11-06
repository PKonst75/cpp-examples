#include "document.h"
#include <string>

using namespace std::literals;

Document::Document(int _id, double _relevance, int _rating)
    :   id(_id),
        relevance(_relevance),
        rating(_rating)
{
}
  
std::ostream& operator << (std::ostream& out, const Document& doc){
    out << "{ "s;
    out << "document_id = "s << doc.id;
    out << ", relevance = "s << doc.relevance;
    out << ", rating = "s << doc.rating;
    out << " }"s;
    return out;
}