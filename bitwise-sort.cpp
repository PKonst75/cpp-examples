#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <map>



void PrintVector(std::vector<std::string>::iterator begin, std::vector<std::string>::iterator end){
    if(begin == end){
        std::cout << "empty" << std::endl;
        return;
    }
    bool first = true;
    for(std::vector<std::string>::iterator it = begin; it < end; ++it){
        if(!first){
            std::cout << ", ";
        }
        else{
            first = false;
        }
        std::cout << *it;
    }
    std::cout << std::endl;
}
void PrintVector(std::vector<std::string>& v){
    PrintVector(v.begin(), v.end());
}

void PrintReverseVector(std::vector<std::string>::reverse_iterator begin, std::vector<std::string>::reverse_iterator end){
    if(begin == end){
        std::cout << "empty" << std::endl;
        return;
    }
    bool first = true;
    for(std::vector<std::string>::reverse_iterator it = begin; it < end; ++it){
        if(!first){
            std::cout << ", ";
        }
        else{
            first = false;
        }
        std::cout << *it;
    }
    std::cout << std::endl;
}

void PrintBuckets(std::map<char, std::vector<std::string>>& buckets){
    for(char c = '0'; c <= '9'; ++c){
        std::cout << "Bucket " << c << ": ";
        PrintVector(buckets[c]);
    }
    std::cout << "**********" << std::endl;
}

void VectorToBuckets(std::map<char, std::vector<std::string>>& buckets, std::vector<std::string>& strs, int pos){
    for(std::string str : strs){
        buckets[str[pos]].push_back(str); 
    }
    strs.clear();
}

    void BucketsToVector(std::map<char, std::vector<std::string>>& buckets, std::vector<std::string>& res){
    for(char c = '0'; c <= '9'; ++c){
        for(std::string str : buckets[c]){
           res.push_back(str);
        }
        buckets[c].clear();
    }
}


void CategorySort(std::vector<std::string>& v){

    std::cout << "Initial array:" << std::endl;
    PrintVector(v);
    std::cout << "**********" << std::endl;

    int pos = -1;
    int len = 0;
    if(!v.empty()){
        len = v[0].length();
        pos = len - 1;
    }

    std::map<char, std::vector<std::string>> buckets;

    while(pos != -1){
        VectorToBuckets(buckets, v, pos);
        std::cout << "Phase " << len - pos << std::endl;
        PrintBuckets(buckets);
        BucketsToVector(buckets, v);
        --pos;
    }
    // Инdертируем результирующий вектор
    std::cout << "Sorted array:" << std::endl;
    PrintVector(v);
}


   
int main(){
    int n;

    std::cin >> n;
    std::vector<std::string> values;
    values.reserve(n);
    for(int i = 0; i < n; ++i){
       std::string value;
        std::cin >> value;
        values.push_back(value);
    }

    CategorySort(values);

}
