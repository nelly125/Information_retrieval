#pragma once

#include <iostream>
#include <vector>
#include "Index.hpp"

//priority: () ! & |

enum operations {
    AND = '&', // 1
    OR = '|', // 2
    NOT = '!' //3
};

class Node {
public:
    std::vector<std::size_t> *_urls;
    std::vector<char> *_compressed_urls;
    std::vector<std::size_t> _not_urls = {};
    int doc_id = -1;
    int position = 0;
    bool flag = false;

    Node *_left;
    Node *_right;
    int _type;

    Node(int type, std::vector<std::size_t> *v, Node *left, Node *right);

    Node(int type, std::vector<char> *v, Node *left, Node *right, bool is_compressed);

    ~Node() = default;;

    friend class Tree;

private:
};

class Boolean_searcher {
public:
    Boolean_searcher() { _is_compressed = false; }

    explicit Boolean_searcher(bool is_compressed) : _is_compressed(is_compressed) {
        for (std::size_t i = 0; i <= 9999; ++i) {
            _full_doc_id.push_back(i);
        }
//        std::cout <<_full_doc_id.size();

    };

    static std::vector<std::string> parse_query(std::string);

    bool search(bool is_stream, std::string &query, Index &index);

    std::vector<std::size_t> _empty_urls = {};

    std::vector<char> _empty_compressed_urls = {};


private:

    Node *create_tree(std::vector<std::string> query, int begin, int end, Index &index);

    std::vector<std::size_t> standart_search(Index &index, Node *node);

    std::vector<std::size_t> stream_search(Index &index, Node *node);

    std::size_t process(Node *node, std::size_t doc_id);

    static int operation_type(std::string str);

    bool output_to_file(std::string query, const std::vector<std::size_t>& result, Index index);

    bool output(std::string query, const std::vector<std::size_t>& result, Index index);


    std::size_t fast_search(std::vector<std::size_t> &_urls, std::size_t x);


    bool _is_compressed = false;

    std::size_t _max_doc_id = 0;

    std::vector<std::size_t> _full_doc_id;

//    bool mkdir(const std::string& str);

};


class Tree {
public:
    Tree() : _root(nullptr) {}

    explicit Tree(Node *root) : _root(root) {}

    ~Tree() { delete_nodes(_root); }

    void delete_nodes(Node *node);

    Node *_root;
private:
};
