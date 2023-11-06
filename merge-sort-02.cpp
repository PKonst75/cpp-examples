#include <iostream>
#include <queue>
#include <stack>
#include <vector>


void MyMergeVector(std::vector<int>& v1, std::vector<int>& v2){
    std::stack<int> s1;
    std::stack<int> s2;

    while (!v2.empty())
    {
        s2.push(v2.back());
        v2.pop_back();
    }
    while (!v1.empty())
    {
        s1.push(v1.back());
        v1.pop_back();
    }

    while(!s1.empty() && !s2.empty()){
        // Основной цикл
        int data_v1 = s1.top();
        int data_v2 = s2.top();
        if(data_v1 <= data_v2){ // Перввый вектор идет в приоритете
            v1.push_back(data_v1);
            s1.pop();
            continue;
        }
        v1.push_back(data_v2);
        s2.pop();
    }

    while(!s1.empty()){
        v1.push_back(s1.top());
        s1.pop();
    }
    while(!s2.empty()){
        v1.push_back(s2.top());
        s2.pop();
    }
    return;
}


void MergeSort(std::vector<int>& v){
    // Делим ветор на две части
    int size = static_cast<int>(v.end() - v.begin());
    if(size == 0){
        // Вектор пуст!
        return;
    }
    if(size == 1){
        // Вектор уже отсортирован!
        return;
    }
    if(size == 2){
        // Просто поменяем местами, чтобы не делать лишних вызовов
        if(*(v.begin()) > v.back()){
            std::swap(*(v.begin()), v.back());
        }
        return;
    }
    std::vector<int>::iterator it = (v.begin() + (size / 2) );
    std::vector<int> v2(it, v.end());
    v.resize(it - v.begin());
    v.shrink_to_fit();
    MergeSort(v);
    MergeSort(v2);
    MyMergeVector(v,  v2);
    v2.clear();
    v2.reserve(0);
}

   
int main(){
    int n;

    std::cin >> n;
    std::vector<int> values;
    values.reserve(n);
    for(int i = 0; i < n; ++i){
       int value;
        std::cin >> value;
        values.push_back(value);
    }

    MergeSort(values);

    bool first = true;
    for(int v : values){
        if (!first){
            std::cout << " ";
        }
        else{
            first = false;
        }
        std::cout << v;
    }
    std::cout << std::endl;
}
