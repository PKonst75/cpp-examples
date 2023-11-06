#pragma once

#include <algorithm>
#include <random>
#include <vector>

std::vector<int> GenerateRandomVector(int size, int max_value = 0){
      // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine {rnd_device()};  // Generates random integers
    std::uniform_int_distribution<int> dist {1, std::max(max_value, size) };
    
    auto gen = [&dist, &mersenne_engine](){
                   return dist(mersenne_engine);
               };

    std::vector<int> vec(size);
    std::generate(begin(vec), end(vec), gen);
    return vec;
}

std::vector<int> GenerateSortRandomVector(int size, int max_value = 0){
    std::vector<int> v = GenerateRandomVector(size, max_value);
    std::sort(v.begin(), v.end());
    return v;
}

std::vector<int> GenerateUniqueRandomVector(int size, int max_value){
    std::vector<int> v(std::max(size, max_value));
    for(int i = 0; i< v.size(); ++i){
        v[i] = i + 1;
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);
    v.resize(size);
    return v;
}

std::vector<int> GenerateSortUniqueRandomVector(int size, int max_value){
    std::vector<int> v = GenerateUniqueRandomVector(size, max_value);
     std::sort(v.begin(), v.end());
    return v;
}