#include "Boolean_searcher.hpp"

#include <fstream>
#include <set>
#include <sstream>
#include <boost/locale/conversion.hpp>
#include <boost/locale/generator.hpp>
#include <algorithm>

//#include <filesystem>

#include "source/utf8.h"
#include <boost/regex/icu.hpp>
#include <boost/locale.hpp>
#include <numeric>


Node::Node(int type, std::vector<std::size_t> *v, Node *left, Node *right) : _type(type),
                                                                             _left(left),
                                                                             _right(right) {
    doc_id = -1;

    if (_type == 0) {
        _urls = v;
    }
}

Node::Node(int type, std::vector<char> *v, Node *left, Node *right, bool is_compressed) : _type(type),
                                                                                          _left(left),
                                                                                          _right(right) {
    doc_id = -1;

    _not_urls = {};
    if (_type == 0) {
        _compressed_urls = v;
    }
}


std::vector<std::string> Boolean_searcher::parse_query(std::string str) {
    std::vector<std::string> query;

    std::string temp;
    utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
    str = temp;

    boost::u32regex r = boost::make_u32regex(R"(\w+|[\(\)&\|\!])");
    boost::u32regex_token_iterator<std::string::const_iterator>
            i(boost::make_u32regex_token_iterator(str, r, 0)), j;
    boost::locale::generator gen;
    std::locale loc = gen.generate("en_US.UTF-8"); // or "C", "en_US.UTF-8" etc.
    std::locale::global(loc);
    while (i != j) {
        std::string word = *i;
        word = boost::locale::to_lower(word);
        query.push_back(word);
        ++i;
    }
//    for (const auto &c: query) {
//        std::cout << c << " ";
//    }
    return query;
}


bool Boolean_searcher::search(bool is_stream,
                              std::string &query,
                              Index &index) {
    std::vector<std::string> parsed_query = parse_query(query);
    if (parsed_query.empty()) {
        return false;
    }

//    std::cout << index._inverted_index.size() << std::endl;

    Tree tree(create_tree(parsed_query, 0, parsed_query.size() - 1, index));

    Node *node = tree._root;

    std::vector<std::size_t> result;

    if (!is_stream) {
        result = standart_search(index, node);
//        std::string dir = "./output_urls";
//        mkdir(dir);
    } else {
        result = stream_search(index, node);
    }

    std::cout  << "Число документов " << result.size() << std::endl;
/*    for (auto c: result) {
        std::cout << c << " ";
    }*/

//    output_to_file(query, result, index);
    output(query, result, index);

    return true;
}

bool Boolean_searcher::output(std::string query, const std::vector<std::size_t>& result, Index index) {
//    std::cout << query << std::endl;
//    std::cout << result.size() << std::endl;
    for (auto c: result) {
        std::cout << " " << index._doc_url[c] << std::endl;;
    }
//    std::cout << std::endl;
//    std::cout << std::endl;
    return true;
}

bool Boolean_searcher::output_to_file(std::string query, const std::vector<std::size_t>& result, Index index) {
    std::ofstream out("./output_urls/result_for_" + query + ".txt");
    if (! out) {
        return false;
    }
    out << query << std::endl;
    out << result.size() << std::endl;
    for (auto c: result) {
        out << c << " " << index._doc_url[c] << std::endl;;
    }
    out << std::endl;
    out << std::endl;
    out.close();
    return true;
}

std::vector<std::size_t> Boolean_searcher::standart_search(Index &index,
                                                           Node *node) {
    if (node->_type == 0) {      /* word */
//        std::cout << "GET URLS >" << std::endl;
//        for (const auto &c: *(node->_urls)) {
//            std::cout << c << " ";
//        }
//        std::cout << std::endl;
        if (!_is_compressed) {
            return *(node->_urls);
        } else {
            std::vector<char> temp = *(node->_compressed_urls);
//            std::cout << "temp " << temp.size();

            std::vector<std::size_t> decode = index.decode_vector(temp);
//            std::cout << "decode" << decode.size() << std::endl;
            return decode;
        }
    } else if (node->_type == 2) {      /* | */
//        std::cout << "OR" << std::endl;

        auto left = standart_search(index, node->_left);
        auto right = standart_search(index, node->_right);
        std::vector<std::size_t> temp;
        std::set_union(left.begin(), left.end(), right.begin(), right.end(),
                       std::back_inserter(temp));
//        std::cout << "После объединения" << std::endl;
/*        for (const auto &c: temp) {
            std::cout << c << " ";
        }*/
        return temp;
    } else if (node->_type == 1) {      /* & */
//        std::cout << "AND" << std::endl;
        auto left = standart_search(index, node->_left);
        auto right = standart_search(index, node->_right);
        std::vector<std::size_t> temp = {};
        if (!left.empty() && !right.empty()) {
//            std::cout << "here _not empty" << std::endl;
            std::set_intersection(left.begin(), left.end(), right.begin(), right.end(),
                                  std::back_inserter(temp));
        }
//        std::cout << "После пересечения" << std::endl;
        for (const auto &c: temp) {
//            std::cout << c << " ";
        }
        return temp;
    } else if (node->_type == 3) {      /* ! */
        std::vector<std::size_t> temp;
        std::vector<std::size_t> next = standart_search(index, node->_right);
//        std::cout << std::endl;
        std::set_difference(_full_doc_id.begin(), _full_doc_id.end(), next.begin(), next.end(),
                            std::back_inserter(temp));
        return temp;
    }
    return _empty_urls;
}

std::size_t Boolean_searcher::fast_search(std::vector<std::size_t> & _urls, std::size_t x) {
    if (x > _urls.back()) {
        return 1000001;
    }
    std::size_t median;
    std::size_t right_index = _urls.size();
    std::size_t left_index = 0;
    while(left_index + 1 != right_index) {
//        std::cout << left_index << " " << right_index << std::endl;
        median = (left_index + right_index ) / 2;
        if(x > _urls[median]) {
            left_index = median;
            continue;
        } else if (_urls[median] > x) {
            right_index = median;
            continue;
        }
            return _urls[median];
    }
    if (_urls[left_index] < x) {
        return _urls[right_index];
    } else {
        return _urls[left_index];
    }
}


std::size_t Boolean_searcher::process( Node *node, std::size_t doc_id) {
    if (node->_type == 0) {
        /* word */
        std::vector<std::size_t> urls;

        if (! _is_compressed) {
            urls = *(node->_urls);;
        } else {
            std::vector<char> temp = *(node->_compressed_urls);
//            std::cout << "temp " << temp.size();
            urls= Index::decode_vector(temp);
//            std::cout << "decode" << urls.size() << std::endl;
        }
        if (!urls.empty()) {
            if (doc_id > urls.back()) {
                return 1000001;
            }
            if (urls.back() > _max_doc_id) { _max_doc_id = urls.back(); }
            return fast_search(urls, doc_id);
        }
        return 1000001;
    }
//    std::cout << "qqqqqqqqqq" << std::endl;
    std::size_t left, right;
    if (node->_left) {
        left = process(node->_left, doc_id);
    }
    if (node->_right) {
        right = process(node->_right, doc_id);
    }
    if (node->_type == 2) { // |
//        std::cout << left << " " << right;
        return std::min(left, right);
    }
    if (node->_type == 1) { //&
        while (left != right && left != 1000001 && right != 1000001) {
            if (left < right) {
                left = process(node->_left, right);
            } else {
                right = process(node->_right, left);
            }
        }
        if (left == 1000001 || right == 1000001) {
            return 1000001;
        }
        return left;
    }
    if (node->_type == 3) { // !
        std::vector urls = *(node->_right->_urls);

        if ((*(node->_right))._not_urls.empty()) {
            std::set_difference(_full_doc_id.begin(), _full_doc_id.end(), urls.begin(), urls.end(),
                                std::back_inserter((*(node->_right))._not_urls));
            if ((*(node->_right))._not_urls.back() > _max_doc_id) { _max_doc_id = (*(node->_right))._not_urls.back(); }
//            std::cout << urls.size() << " " << (*(node->_right))._not_urls.size() << std::endl;
        }
        urls = (*(node->_right))._not_urls;

        if (!urls.empty()) {
            if (doc_id > urls.back()) {
                return 1000001;
            }
            if (std::binary_search(urls.begin(), urls.end(), doc_id)) {
                return doc_id;
            }
        }
    }
    return 1000001;
}


std::vector<std::size_t> Boolean_searcher::stream_search(Index &index, Node *node) {
    _max_doc_id = 0;
    std::vector<std::size_t> result = {};
    std::size_t doc_id = 0;
    std::size_t process_doc_id;
    int i = 0;

    while (i == 0 || doc_id <= _max_doc_id) {
//        std::cout << doc_id << std::endl;
        process_doc_id = process(node, doc_id);
        if (doc_id == process_doc_id) {
//            std::cout << "found" << std::endl;
            result.push_back(process_doc_id);
            doc_id = process_doc_id;
            doc_id ++;
        } else  if (process_doc_id != 1000001) {
//            std::cout << "upper if bourser" << std::endl;
            doc_id = process_doc_id;
        } else{
            doc_id++;
        }
        ++i;
    }
//    std::cout << doc_id  << std::endl;
//    std::cout << result.size() << std::endl;
//    for (auto c: result) {
//        std::cout << c << " " << std::endl;
//    }
    return result;
}

int Boolean_searcher::operation_type(std::string str) {
    int type = 0;
    if (str == "!" || str == "&" || str == "|") {
        char c = str[0];
//            std::cout << c << std::endl;
        if (c == '&') {
            type = 1;
//            std::cout << "and";
        }
        if (c == '|') {
            type = 2;
//            std::cout << "or";
        }
        if (c == '!') {
            type = 3;
//            std::cout << "not";
        }
    }
    return type;
}

Node *Boolean_searcher::create_tree(std::vector<std::string> query, int begin, int end, Index &index) {
//    std::cout << "create" << " " << begin << " " << end << std::endl;
    if (end >= query.size()) {
        return nullptr;
    }
    if (begin == end) {
//        std::cout << query[begin] << std::endl;
        std::string word_to_find = query[begin];

        if (!_is_compressed) {
            auto it = index._inverted_index.find(word_to_find);
            if (it == index._inverted_index.end()) {
                std::vector<std::size_t> urls = {};
                Node *node = new Node(0, &_empty_urls, nullptr, nullptr);
                return node;
            }
            Node *node = new Node(0, &it->second, nullptr, nullptr);
            return node;
        } else {
//            std::cout << "everything is OK before" << std::endl;

            auto it = index._compressed_inverted_index.find(word_to_find);
            if (it == index._compressed_inverted_index.end()) {
                std::vector<std::size_t> urls = {};
                Node *node = new Node(0, &_empty_compressed_urls, nullptr, nullptr, true);
//                std::cout << "everything is OK" << std::endl;
                return node;
            }
//            std::cout << "everything is OK" << std::endl;
            Node *node = new Node(0, &it->second, nullptr, nullptr, true);
//            std::cout << "everything is OK ***" << std::endl;
            return node;
        }
    }
    int i;
    for (i = begin; i < end; ++i) {
        std::string c = query[i];
        if (c == "(" || c == ")" || c == "!" || c == "&" || c == "|")
            break;
    }
    if (query[i] == "(") {
        int brackets_diff = 1;
        int end_brackets = i + 1;
        while (brackets_diff > 0) {
            if (query[end_brackets] == ")") {
                --brackets_diff;
            } else if (query[end_brackets] == "(")
                ++brackets_diff;
            end_brackets++;
        }
        end_brackets--;
//        std::cout << "( " << i << " " << end_brackets - 2 << std::endl;
        if (end_brackets == end) {
            return create_tree(query, i + 1, end_brackets - 1, index);
        } else {
            std::size_t type = operation_type(query[end_brackets + 1]);
            return new Node(type, nullptr, create_tree(query, i + 1, end_brackets - 1, index),
                            create_tree(query, end_brackets + 2, end, index));
        }
    } else {
        if (query[i] == "!" || query[i] == "&" || query[i] == "|") {
            int type = operation_type(query[i]);
            if (type == 3) {
                return new Node(type, nullptr, nullptr,
                                create_tree(query, i + 1, end, index));
            } else {
                return new Node(type, nullptr, create_tree(query, begin, i - 1, index),
                                create_tree(query, i + 1, end, index));
            }
        } else {
//            std::cout << "word" << std::endl;
            return new Node(0, nullptr, create_tree(query, begin, i - 1, index),
                            create_tree(query, i + 1, end, index));
        }
    }

}

void Tree::delete_nodes(Node *node) {
    if (!node) {
        return;
    }
    delete_nodes(node->_left);
    delete_nodes(node->_right);
    delete node;
}

/*
bool Boolean_searcher::mkdir(const std::string& dir) {
    std::error_code ec;
    if (std::filesystem::exists(dir)) {
        return std::filesystem::is_directory(std::filesystem::status(dir));
    } else {
        return std::filesystem::create_directories(dir, ec);
    }
}
*/

