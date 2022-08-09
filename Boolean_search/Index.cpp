#include <fstream>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/locale.hpp>
#include <boost/regex/icu.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <iostream>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cassert>

#include "Index.hpp"
#include "source/utf8.h"
#include "./ext_lib/struct.h"


//VarByte

std::unordered_set<std::string> Index::parse_article(std::string text) {
    std::unordered_set<std::string> parsed_article;

    std::string temp;
    utf8::replace_invalid(text.begin(), text.end(), back_inserter(temp));
    text = temp;

    boost::u32regex r = boost::make_u32regex("\\w+");
    boost::u32regex_token_iterator<std::string::const_iterator>
            i(boost::make_u32regex_token_iterator(text, r, 0)), j;
    while (i != j) {
        std::string word = *i;;
        ++i;
        parsed_article.insert(word);
    }
    return parsed_article;
}

int Index::create_forwarded_index() {
    std::cout << "Process:: Reading data" << std::endl;


    std::map<int, std::string> doc_names = {{0, "lenta.ru_4deb864d-3c46-45e6-85f4-a7ff7544a3fb_01.gz"},
                                            {1, "lenta.ru_80e74243-83da-4367-8ae3-fe38d333f283_01.gz"},
                                            {2, "lenta.ru_159b9f4b-972b-48b1-8ec3-44fbd6be33c4_01.gz"},
                                            {3, "lenta.ru_6398c7e2-16da-40d2-8923-95f65aaaeb07_01.gz"},
                                            {4, "lenta.ru_aa5a1ef9-6ca4-4dc7-890f-308d4d62db59_01.gz"},
                                            {5, "lenta.ru_b81aa623-ba55-43dc-b3c5-47ae2253ad27_01.gz"},
                                            {6, "lenta.ru_b6838708-1aa9-496f-bf88-e277374f93a8_01.gz"},
                                            {7, "lenta.ru_d1f7e910-b5f1-4719-b724-090093e143fe_01.gz"}};
//    std::vector<std::string> doc_url;

    char *buffer = new char[4];
    char *big_buffer = new char[1000000];

    for (std::size_t i = 0; i < FILES_COUNT; ++i) {
        std::string file_name = "./data/" + doc_names[i];
        std::ifstream file;
        file.open(file_name, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error in open file './in.bin'" << std::endl;
            return -1;
        }
        boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
        inbuf.push(boost::iostreams::gzip_decompressor());
        inbuf.push(file);
        std::istream instream(&inbuf);
        instream.read(buffer, 4);

        while (instream) {
            int32_t words_count;
            struct_unpack(buffer, "i", &words_count);
            instream.read(big_buffer, words_count);

            int index;
            for (index = 0; index < words_count && (big_buffer[index] != '\x1a' || index < 4); index++) {}
            std::string url_dirty(big_buffer, index);
            std::size_t pos = url_dirty.find_first_of("http");
            std::string url = url_dirty.substr(pos);
            _doc_url.push_back(url);

            std::string text(&big_buffer[index + 2], words_count - index - 2);

            boost::locale::generator gen;
            std::locale loc = gen.generate("en_US.UTF-8"); // or "C", "en_US.UTF-8" etc.
            std::locale::global(loc);
            text = boost::locale::to_lower(text);

            std::unordered_set<std::string> words;
            words = parse_article(text);
            _forward_index.insert({_doc_id_counter, words});

            _doc_id_counter++;
            instream.read(buffer, 4);
        }
        file.close();
    }
//    std::cout << " " << _forward_index.size() << "\n";

    delete[] buffer;
    delete[] big_buffer;
    return 0;
}

void Index::create_inverted_index() {
    std::cout << "Process:: Create inverted index" << std::endl;

    for (std::size_t i = 0; i < _doc_id_counter; ++i) {
        for (std::string word: _forward_index[i]) {
            if (_inverted_index.find(word) != _inverted_index.end()) {
                _inverted_index[word].push_back(i);
            } else {
                std::vector<std::size_t> temp = {i};
                _inverted_index.insert({word, temp});
            }
        }
    }
}

void Index::save_inverted_index() {
    std::cout << "Process:: Save inverted index" << std::endl;
    std::ofstream output("inverted_index.txt", std::ios::binary);
    for (const auto &it: _inverted_index) {
        std::string word = it.first;
        output << word << ": ";
        for (auto url_ind: _inverted_index[word]) {
            output << url_ind << " ";
        }
        output << "\n";
    }
    output.close();
}


[[maybe_unused]] void Index::save_compressed_index() {
    std::cout << "Process:: Save compressed inverted index" << std::endl;
    std::ofstream output("compressed_index.txt", std::ios::binary);
    for (const auto &it: _compressed_inverted_index) {
        std::string word = it.first;
        output << word << ": ";
        for (auto url_ind: _compressed_inverted_index[word]) {
            output << url_ind;
        }
        output << "\n";
    }
    output.close();
}

void Index::save_doc_id() {
    std::cout << "Process:: Save doc_id urls" << std::endl;

    std::ofstream output("doc_id.txt", std::ios::binary);
    for (std::size_t doc_id = 0; doc_id < _doc_id_counter; ++doc_id) {
//        std::cout << _doc_url[doc_id] << std::endl;
        output << doc_id << ": " << _doc_url[doc_id] << std::endl;
    }
    output.close();
}

[[maybe_unused]] void Index::print_byte(char val) {
    for (int i = 7; 0 <= i; i--) {
        std::cout << ((val & (1 << i)) ? '1' : '0');
    }
    std::cout << " ";
}

void Index::encode_varbyte() {
    std::cout << "Process:: Encode" << std::endl;
    for (const auto &it: _inverted_index) {
        std::string word = it.first;
        std::vector<char> compressed = encode_vector(_inverted_index[word]);
        _compressed_inverted_index.insert({word, compressed});
    }
}

void Index::decode_varbyte() {
    std::cout << "Process:: Decode" << std::endl;
    std::map<std::string, std::vector<size_t>> decode_index;
    for (const auto &it: _compressed_inverted_index) {
        std::string word = it.first;
        std::vector<size_t> result = decode_vector(_compressed_inverted_index[word]);
        decode_index.insert({word, result});
    }

    std::cout << "Process:: Assert (decode == inverted index)" << std::endl;
    assert(decode_index == _inverted_index);
    std::cout << "Result: Everything is OK. Encode and decode are working" << std::endl;
}

std::vector<char> Index::encode_vector(const std::vector<std::size_t> &index_doc_id) {
    std::size_t x;
    char res;
    std::vector<char> compressed_index;
    for (std::size_t i: index_doc_id) {
        x = i;
        while (x > 127) {
            res = x & 127 | 128;
            compressed_index.push_back(res);
//            print_byte(res);
            x >>= 7;
        }
        res = x;
        compressed_index.push_back(res);
//        print_byte(res);
    }
    return compressed_index;
}

std::vector<std::size_t> Index::decode_vector(std::vector<char> compressed_index) {
    std::vector<std::size_t> result;
    std::size_t value, temp;
    int shift;
    uint32_t remaining_buffer = compressed_index.size();
    char cur_buf;

    for (int i = 0; i < compressed_index.size(); i++) {
        cur_buf = compressed_index[i];
        value = 0;
        shift = 0;
        while ((temp = cur_buf) > 127) {
            value |= (temp & 127) << shift;
            shift += 7;
            ++i;
            --remaining_buffer;
            cur_buf = compressed_index[i];
        }
        result.push_back(value |= (temp << shift));
    }
    return result;
}

[[maybe_unused]] void Index::get_index_from_file(std::string str) {

    std::cout << "Process:: Reading index from file" << std::endl;

    std::ifstream in(str);
    if (!in.is_open()) {
        std::cerr << "Error in open file" << std::endl;
    }
    std::string line;
    int count = 0;
    while (std::getline(in, line)) {
        std::size_t index = line.find(':');
        std::string word = line.substr(0, index);
//        std::cout << word << std::endl;
        std::string temp = line.substr(index + 2, line.size());

        std::vector<std::string> doc_id;
        std::vector<std::size_t> doc_id_size_t;

        boost::split(doc_id, temp, boost::is_any_of(" "), boost::token_compress_on);
        for (const auto &c: doc_id) {
            std::stringstream sstream(c);
            std::size_t result;
            sstream >> result;
            doc_id_size_t.push_back(result);
        }
        count++;
        _inverted_index[word] = doc_id_size_t;
    }

}

[[maybe_unused]] [[maybe_unused]] void Index::get_compressed_index_from_file() {
    std::ifstream in("compressed_index.txt");
    if (!in.is_open()) {
        std::cerr << "Error in open file" << std::endl;
    }
    std::string line;
    std::size_t doc_id = 0;
    std::vector<char> temp;
    while (std::getline(in, line)) {
        temp = {};
        std::size_t index = line.find(':');
        std::cout << "OK" << std::endl;
        std::string word = line.substr(0, index);
        std::string compressed_index = line.substr(index + 2, line.size());
        for (char c: compressed_index) {
            temp.push_back(c);
        }
        std::cout << "OK2" << std::endl;

        _compressed_inverted_index.insert({word, temp});
    }
    std::cout << _compressed_inverted_index.size();
}

[[maybe_unused]] void Index::get_doc_id_from_file() {
    std::ifstream in("doc_id.txt");
    if (!in.is_open()) {
        std::cerr << "Error in open file" << std::endl;
    }
    std::string line;
    std::size_t doc_id = 0;
    while (std::getline(in, line)) {
        std::size_t index = line.find(':');
        std::string std_doc_id = line.substr(0, index);
        std::string url = line.substr(index + 2, line.size());
        std::stringstream sstream(std_doc_id);
        std::size_t result;
        sstream >> result;
        _doc_url.push_back(url);
    }
}

[[maybe_unused]] std::vector<std::size_t> Index::get_full_doc_id() const {
    std::vector<std::size_t> temp;
    for (std::size_t i = 0; i < _doc_id_counter; ++i) {
        temp.push_back(i);
    }
    return temp;
}
