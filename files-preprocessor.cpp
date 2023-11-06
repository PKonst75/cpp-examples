#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

const std::regex REGEX_TYPE1(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
const std::regex REGEX_TYPE2(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories);

// Вспомагательная функция - поиск в директориях
path FindExisted(path local_filenme, const vector<path>& include_directories){
     for(path p : include_directories){
        // Для каждого элемента проверяем существование
        if(filesystem::exists(p / local_filenme)){
            return p / local_filenme;
        }
     }
     return path{};
}
path FindExisted(path local_filenme, path file_location){
     path parent = file_location.parent_path();
    // Ищем в той же директории, что и сам файл
    if(filesystem::exists(parent / local_filenme)){
        return parent / local_filenme;
    }
    return path{};
}

bool IsRegExp(const std::string& line, const path& current_file, const vector<path>& include_directories, path& file_to_find){

    std::smatch m;// Для результатов поиска
    bool search_in_includes = false;
    if(std::regex_match(line, m, REGEX_TYPE1)){
        file_to_find = FindExisted(string(m[1]), current_file);
        if(!file_to_find.empty()){
            return true;
        }
        search_in_includes = true;
    }
    if(search_in_includes || std::regex_match(line, m, REGEX_TYPE2)){
        file_to_find = FindExisted(string(m[1]), include_directories);
        return true;
    }
    return false;
}


// Рекурсивная функция
bool ProcessRegexp(std::ofstream& out, std::ifstream& in, const vector<path>& include_directories, const path& current_file){

    int line_count = 1; // Счетчик линий файла для вывода ошибки
    std::string line;
    // Читаем входной поток построчно
    while(std::getline(in, line)){
        path to_find{};
        if( IsRegExp(line, current_file, include_directories, to_find) ){
            if(to_find.empty()){
                // Не нашли файл - выдаем ошибку
                std::string err = "unknown include file "s + to_find.filename().string() + " at file " + current_file.string() + " at line " + to_string(line_count);
                std::cout << err << std::endl;
                in.close();
                return false;
            }
             // Запустить рекурсию
            std::ifstream fin (to_find, std::ios::in);
            if(!ProcessRegexp(out, fin, include_directories, to_find)){
                in.close(); // Закрываем все файлы по цепочке
                return false;
            }
            // Файл успешно включен - переход к следующей линии
            ++line_count;
            continue;
        }
        // Обычная строчка - просто выводим в файл
        line += "\n";
        // Потенциально опасное место!!! Но как его обойти я не уверен size_t streamsize
        out.write( line.data(), static_cast<streamsize>(line.size()) );
        ++line_count;
    }
    in.close(); // Успешно все прочитали - закрываем входящий файл
    return true;
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories){
    
    // Открываем первый файл для чтения
    if(!filesystem::exists(in_file)){
        return false; // Файл не существует - не делаем ничего
    }
    std::ifstream in_stream(in_file, std::ios::in); // Открываем входной поток для чтения
    if(!in_stream.is_open()){
        return false; // Не смогли отркыть поток для чтения - не дклаем ничего
    }
    std::ofstream out_stream(out_file, std::ios::out); // Открываем выходной поток для записи стирая старый
    if(!out_stream){
        return false; // Не смогли отркыть поток для записи
    }
    // Все файлы успешно открылись - запускаем рекурсию (куда пишем, откуда пишем, директории дял поиска, читаемый файл)
    bool result =  ProcessRegexp(out_stream, in_stream, include_directories, in_file);
    out_stream.close(); // Закрывем файл в который писали
    // Остальное закроет рекурсия
    return result; 
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}