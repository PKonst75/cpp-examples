#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class Hash{
public:
    Hash(std::string& s, unsigned int p, unsigned int x)
    : x_(x), p_(p), hash_(s.size() + 1, 0), pow_(s.size() + 1, 1)
    {
        MakeHash(s);
        MakePowers();
    }

    bool IsEqual(size_t len, size_t a, size_t b){
        unsigned long long h1 = (hash_[a + len] + (hash_[b] * pow_[len]) % p_) % p_;
        unsigned long long h2 = (hash_[b + len] + (hash_[a] * pow_[len]) % p_) % p_;
        return h1 == h2;
    }

private:
    unsigned int x_;
    unsigned int p_;
    std::vector<unsigned long long> hash_;
    std::vector<unsigned long long> pow_;
    

    unsigned int MakeCoefficient(char c){
        // Получить коэффициент полинома по символу
        // a = 97 z =122 => a = 1
        return static_cast<unsigned int>(c - 'a' + 1);
    }

    void MakeHash(std::string& s){
        // i - hash соответсвует i - 1 букве
        for(size_t i = 1; i < hash_.size(); ++i){
            hash_[i] = ( (hash_[i-1] * x_) % p_ + MakeCoefficient(s[i - 1]) ) % p_;
        }
    }

    void MakePowers(){
        // res[i] = x ** i 
        for(size_t i = 1; i < pow_.size(); ++i){
            pow_[i] = (pow_[i-1] * x_) % p_;
        }
    }

};

int main(){

    std::string s;
    std::cin >> s;

    std::vector<std::tuple<size_t, size_t, size_t>> query;
    int q;
    std::cin >> q;

    for(int i = 0; i < q; ++i){
        int l, a, b;
        std::cin >> l >> a >> b;
        query.push_back(std::tuple<size_t, size_t, size_t> {l, a, b});
    }

    Hash h1(s, std::pow(10, 9) + 7, 257);
    Hash h2(s, std::pow(10, 9) + 17, 337);
    Hash h3(s, std::pow(10, 9) + 37, 511);

    for(auto [l, a, b] : query){
        if(h1.IsEqual(l, a, b) && h3.IsEqual(l, a, b)){
            std::cout << "yes" << std::endl;
            continue;
        } 
        std::cout << "no" << std::endl;
    }
}