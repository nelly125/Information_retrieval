#pragma once


#include <unordered_set>
#include <vector>
#include <map>


#define FILES_COUNT 8


class Index {
public:
    int create_forwarded_index();

    void create_inverted_index();

    void save_inverted_index();

    void save_doc_id();

    [[maybe_unused]] void save_compressed_index();

    void encode_varbyte();

    void decode_varbyte();

    static std::vector<char> encode_vector(const std::vector<std::size_t> &index_doc_id);

    static std::vector<std::size_t> decode_vector(std::vector<char> compressed_index);

    [[maybe_unused]] void get_index_from_file(std::string in);

    [[maybe_unused]] void get_compressed_index_from_file();

    [[maybe_unused]] void get_doc_id_from_file();

    [[maybe_unused]] std::vector<std::size_t> get_full_doc_id() const;

    std::map<std::string, std::vector<std::size_t>> _inverted_index;
    std::map<std::string, std::vector<char>> _compressed_inverted_index;
    std::vector<std::string> _doc_url;


private:
    static std::unordered_set<std::string> parse_article(std::string text);

    [[maybe_unused]] static void print_byte(char val);

    std::map<std::size_t, std::unordered_set<std::string>> _forward_index;

    std::size_t _doc_id_counter = 0;
};


