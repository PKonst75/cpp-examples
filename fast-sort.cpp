#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

long SelectX(std::vector<long>::iterator begin, std::vector<long>::iterator end){
    // Выбор опорной точки
    int size = static_cast<int>(end - begin);
    if(size < 100){
        // Просто возьмем середину
        return *(begin + size / 2);
    }

    // Тспользуем алгоритм
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(1, size / 10);
    int first = dist(gen);  // Сгенерили первый элемент
    std::vector<long> x(9, 0);
    for (int i = 0; i < 9; i++)
    {
        x.at(i) = *(begin + (first + size / 10 * i));
    }
    std::sort(x.begin(), x.end());
    if(x.at(0) == x.at(8)){
        // Потом использовать для ускорения!
        return x.at(0);
    }
    return x.at(4);
}

void PartitionIterator(std::vector<long>::iterator begin, std::vector<long>::iterator end, long preset_x = 0, bool preset = false){
    if(begin == end){
        return; // Пустой массив всегда отстортирован
    }
    // Для ускорения
    if(begin + 1 == end){
        return; // Один элемент тоже всегда сортирован
    }
    if(begin + 2 == end){
        if (*(end - 1) < *begin){
            std::swap(*(end-1), *begin);
        }
        return; // Два элемента - просто меняем местами по необходимости
    }
    // Алгоритм поиска точки разбиения
    long x;
    if(preset){
        x = preset_x;
    }
    else{
        x = SelectX(begin, end); // Гарантированно, что есть хоть один элемент равный X!!!
    }
    auto l = begin;
    auto r = end;
    bool swap_l = false;
    bool swap_r = false;
    long local_max = x;

    while(l != r){
        // Двигаем итератор l
        if(*l < x ){
            // Ок он удовлетворяет предикату, сдвигаем!
            ++l;
            continue; // Чтобы не пропустить проверку
        }
        else{
            swap_l = true;
        }
        // Теперь двигаем итератор r
        if(r == end){
            --r; // Предварительная сдвижка, это может вобще не понадобится
            continue; // Чтобы не пропустить проверку
        }
         if(*r >= x ){
            // Ок он удовлетворяет предикату, сдвигаем!
            if (*r > local_max){
                local_max = *r; // Позволит проверить равный отрезок сразу
            }
            --r;
            continue; // Чтобы не пропустить проверку
        }
        else{
            swap_r = true;
        }

        if(swap_r && swap_l){
            // Нужно поменять местами неправильные элементы
            std::swap(*l, *r);
            swap_r = false;
            swap_l = false;
            continue; // Чтобы продолжить проверки
        }
    }

    // Нашли точку разбиения
    // Запускаем рекурсию

    // Разбираем случаи конца
    if(l == end){
        // Просто дошли до конца массива не встерив элементов больше или равного х!
        // То есть это значит, ошибка алгоритма
        // Но если все же просто закрываем рекурсию
        return;
    }
    if(r == begin){
        // Просто дошли до конца массива не встертив элементво меньше x!
        // X - это минимум - не повезло или все элементы одинаковы
        if(local_max == x){
            // Все элементы одинаковы - участок отсортирован
            // Закрываем рекурсию
            if(local_max == *r){
                return;
            }
            local_max = *r;
        }
        // Неповезло! Запускаем новую рекурсию уже с максимумом
        PartitionIterator(begin, end, local_max, true);
        return;
    }
    // В общем случае
    //  l Указатель покажет на первый элеиент >= X! вот они и два интервала!
    PartitionIterator(begin, l);
    PartitionIterator(l, end);
}

int CountFromIterator(const std::vector<long>& v, std::vector<long>::iterator it){

    if(it == v.end()){
        // Дошли до конца не встретив больше X
        return static_cast<int>(v.size());
    }
    if(it == v.begin()){
        // Дошли до начала не встретив  меньше x
        return 0;
    }
    // В середине
    return static_cast<int>(it - v.begin());
}

int main(){

    int n;
    std::vector<long> values;
    std::cin >> n;
    values.reserve(n);
    for(int i = 0; i < n; ++i){
        long value;
        std::cin >> value;
        values.push_back(value);
    }

    PartitionIterator(values.begin(), values.end());

    bool first = true;
    for(long v : values){
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