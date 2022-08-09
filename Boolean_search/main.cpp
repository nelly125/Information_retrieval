#include <iostream>
#include <unordered_set>
#include <fstream>
#include <algorithm>

#include "Index.hpp"
#include "Boolean_searcher.hpp"
#include "Tests.hpp"


int main() {

    bool read_from_file = false;
    bool is_compressed = false;
    bool is_stream = true;

    //INVERTED_INDEX
    Index index;

    if( ! read_from_file) {
        index.create_forwarded_index();
        index.create_inverted_index();
        index.save_inverted_index();
        index.save_doc_id();
    } else {
        index.get_doc_id_from_file();
        index.get_index_from_file("inverted_index.txt");
//        index.get_compressed_index_from_file();
    }

    //DECODE
    if (is_compressed) {
        index.encode_varbyte();
        index.save_compressed_index();
        index.decode_varbyte();
    }

    //INPUT TASKS TO SEARCH AND SEARCH
    Boolean_searcher searcher(is_compressed);
    std::string query = "()";

    while (std::cin) {
        std::cout << "Введите запрос: (Ctrl-D - завершение ввода): " << std::endl;
        getline(std::cin, query);
        if (!std::cin.eof()) {
            searcher.search(is_stream, query, index);
        }
    }

//    time_test();

}
