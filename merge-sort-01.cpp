#include <iostream>
#include <queue>

void Print(int v[], int begin, int end){
     bool first = true;
    for(int i = begin; i < end; ++i){
        if (!first){
            std::cout << " ";
        }
        else{
            first = false;
        }
        std::cout << v[i];
    }
    std::cout << std::endl;
}

void MyMergeVector(int v[], int begin, int end, int dev){
    std::queue<int> s1;
    std::queue<int> s2;

    for(int i = dev; i < end; ++i)
    {
        s2.push(v[i]);
    }
    for(int i = begin; i < dev; ++i)
    {
        s1.push(v[i]);
    }

    int index = begin;
    while(!s1.empty() && !s2.empty()){
        // Основной цикл
        int data_v1 = s1.front();
        int data_v2 = s2.front();
        if(data_v1 <= data_v2){ // Перввый вектор идет в приоритете
            v[index] = data_v1;
            s1.pop();
            ++index;
            continue;
        }
        v[index] = data_v2;
        ++index;
        s2.pop();
    }

    while(!s1.empty()){
        v[index] = s1.front();
        ++index;
        s1.pop();
    }
    while(!s2.empty()){
        v[index] = s2.front();
        ++index;
        s2.pop();
    }
    return;
}


void MergeSort(int v[], int begin, int end){
    // Делим ветор на две части
    int size = static_cast<int>(end - begin);
    if(size == 0){
        return;
    }
    if(size == 1){
        // Вектор уже отсортирован!
        return;
    }
    if(size == 2){
        // Просто поменяем местами, чтобы не делать лишних вызовов
        if(v[begin] > v[end - 1]){
            std::swap(v[begin], v[end -1]);
        }
        return;
    }
    int it = ( begin + (size / 2) );
    MergeSort(v, begin, it);
    MergeSort(v, it, end);
    MyMergeVector(v,  begin, end, it);
}

   
int main(){
    int n;

    std::cin >> n;
    int values[n];
    for(int i = 0; i < n; ++i){
        std::cin >> values[i];
    }

    MergeSort(values, 0, n);

    bool first = true;
    for(int i = 0; i < n; ++i){
        if (!first){
            std::cout << " ";
        }
        else{
            first = false;
        }
        std::cout << values[i];
    }
    std::cout << std::endl;
}
