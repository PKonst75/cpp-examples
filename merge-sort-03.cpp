#include <iostream>
#include <queue>
#include <vector>

void MyMergeVector(std::vector<int>::iterator& begin, std::vector<int>::iterator& end, std::vector<int>::iterator& dev){

    auto it1 = begin;
    auto it2 = dev;
    auto w_it = begin; // Итератор позиции записи для еще дбольшей экономии памяти
    std::queue<int> res;

    while(it1 < dev && it2 < end){

        while(w_it < it1){
            // Есть что и куда записывать!!
            *w_it = res.front();
            res.pop(); // Уменьшили потребляемую память!!!
            ++w_it;
        }
        // Основной цикл
        int data_v1 = *it1;
        int data_v2 = *it2;
        if(data_v1 <= data_v2){ // Перввый вектор идет в приоритете
            res.push(data_v1);
            ++it1;
            continue;
        }
        res.push(data_v2);
        ++it2;
    }

    if(it1 == dev){
        // Делать вобще ничего не нужно, остаток и так отсортирован
        // вписываем оставшиеся элементы из res
        while (!res.empty())
        {
            *w_it = res.front();
            ++w_it;
            res.pop();
        }
        return;
    }
    // Добавляем остаток от v1 в конец
    //std::copy(it1, dev, end - (dev-it1));
    std::move(it1, dev, end - (dev-it1));
    // Теперь записываем  оставшиеся данные в основной массив
    while (!res.empty())
    {
        *w_it = res.front();
        ++w_it;
        res.pop();
    }
    return;
}


void MergeSort(std::vector<int>::iterator begin, std::vector<int>::iterator end){
    // Делим ветор на две части
    int size = static_cast<int>(end - begin);
    if(size == 0){
        // Пустой вектор
        return;
    }
    if(size == 1){
        // Вектор уже отсортирован!
        return;
    }
    if(size == 2){
        // Просто поменяем местами, чтобы не делать лишних вызовов
        if(*begin > *(end - 1)){
            std::swap(*begin, *(end-1));
        }
        return;
    }
    std::vector<int>::iterator it = (begin + (size / 2) );
    MergeSort(begin, it);
    MergeSort(it, end);
    MyMergeVector(begin,  end, it);
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

    MergeSort(values.begin(), values.end());

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
