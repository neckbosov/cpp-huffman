#include "huffman.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <string>
#include <utility>
#include <vector>

HuffTree::TreeNode::TreeNode(byte_t symbol, code_t freq) noexcept
{
    letter = symbol;
    frequency = freq;
    left = right = nullptr;
}

HuffTree::TreeNode::TreeNode() noexcept
{
    left = right = nullptr;
    letter = frequency = 0;
}
HuffTree::TreeNode::TreeNode(HuffTree::TreeNodePtr l, HuffTree::TreeNodePtr r) noexcept
{
    left = l;
    right = r;
    frequency = l->frequency + r->frequency;
}
bool HuffTree::TreeNode::is_leaf() const noexcept
{
    return !left && !right;
}

HuffTree::HuffTree() noexcept
{
    root = TreeNodePtr(new TreeNode());
}

HuffTree::HuffTree(std::array<code_t, VALUES_RANGE> frequencies)
{
    struct TreeNodeCmp {
        bool operator()(const TreeNodePtr lhs, const TreeNodePtr rhs)
        {
            return lhs->frequency > rhs->frequency;
        }
    };
    std::priority_queue<TreeNodePtr, std::vector<TreeNodePtr>, TreeNodeCmp> pq;
    for (code_t letter = 0; letter < VALUES_RANGE; ++letter) {
        if (frequencies[letter] == 0)
            continue;
        pq.emplace(new TreeNode(static_cast<byte_t>(letter), frequencies[letter]));
    }
    if (pq.empty()) {
        root = TreeNodePtr(new TreeNode());
    }
    else if (pq.size() == 1) {
        root = TreeNodePtr(new TreeNode());
        root->left = pq.top();
    }
    else {
        while (pq.size() > 1) {
            TreeNodePtr left = pq.top();
            pq.pop();
            TreeNodePtr right = pq.top();
            pq.pop();
            pq.emplace(new TreeNode(left, right));
            root = pq.top();
        }
    }
}

code_t HuffTree::get_total_length_impl(HuffTree::TreeNodePtr node, code_t cur_len) const noexcept
{
    if (!node)
        return 0;
    if (node->is_leaf()) {
        return cur_len * node->frequency;
    }
    else {
        return get_total_length_impl(node->left, cur_len + 1) + get_total_length_impl(node->right, cur_len + 1);
    }
}

code_t HuffTree::get_total_length() const noexcept
{
    return get_total_length_impl(root, 0);
}

void HuffTree::add_letter(code_t code, byte_t len, byte_t letter) noexcept
{
    TreeNodePtr node = root;
    for (byte_t bit = 0; bit < len; ++bit) {
        if ((code >> bit) & 1) {
            if (!node->right) {
                node->right = TreeNodePtr(new TreeNode());
            }
            node = node->right;
        }
        else {
            if (!node->left) {
                node->left = TreeNodePtr(new TreeNode());
            }
            node = node->left;
        }
    }
    node->letter = letter;
}

BitReader::BitReader(const std::string &file)
{
    in = std::ifstream(file, std::ios::binary);
    in.exceptions(std::ifstream::badbit);
    current_byte = 0;
    current_bit = 8;
}

bool BitReader::eof() const noexcept
{
    return (in.eof() || in.fail()) && current_bit == 8;
}

void BitReader::close() noexcept
{
    in.close();
}

bool BitReader::read_bit(bool &bit)
{
    if (current_bit == 8) {
        char c;
        if (!in.get(c)) {
            return false;
        }
        current_byte = static_cast<byte_t>(c);
        current_bit = 0;
    }
    bit = (current_byte >> (current_bit++)) & 1;
    return true;
}

bool BitReader::read_byte(byte_t &byte)
{
    char c;
    if (!in.get(c)) {
        return false;
    }
    byte = (byte_t)c;
    return true;
}

void BitReader::read_bytes(char *bytes, code_t len)
{
    in.read(bytes, len);
}

BitWriter::BitWriter(const std::string &file)
{
    out = std::ofstream(file, std::ios::binary);
    out.exceptions(std::ofstream::badbit);
    current_bit = 0;
    current_byte = 0;
}

BitWriter::~BitWriter()
{
    if (current_bit > 0)
        out.put(current_byte);
    out.close();
}

void BitWriter::close() noexcept
{
    if (current_bit > 0)
        out.put(current_byte);
    current_bit = 0;
    out.close();
}

void BitWriter::write_bit(bool bit)
{
    if (current_bit == 8) {
        out.put(current_byte);
        current_bit = current_byte = 0;
    }
    if (bit) {
        current_byte |= 1 << current_bit;
    }
    ++current_bit;
}

void BitWriter::write_byte(byte_t byte)
{
    out.put(byte);
}

void BitWriter::write_bytes(const char *bytes, code_t len)
{
    out.write(bytes, len);
}

HuffmanArchiever::HuffmanArchiever(const std::string &input, const std::string &output)
{
    input_file = input;
    output_file = output;
}

std::array<code_t, VALUES_RANGE> HuffmanArchiever::get_frequencies() const noexcept
{
    BitReader reader(input_file);
    std::array<code_t, VALUES_RANGE> frequencies;
    frequencies.fill(0);

    while (!reader.eof()) {
        try {
            byte_t byte;
            if (!reader.read_byte(byte)) {
                break;
            }
            ++frequencies[byte];
        }
        catch (const BitIOException &e) {
            std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                      << " - something went wrong with input file" << std::endl;
            exit(-1);
        }
    }

    return frequencies;
}

void HuffmanArchiever::make_table_impl(HuffTree::TreeNodePtr node, code_t code, byte_t len)
{
    if (node->is_leaf()) {
        table[node->letter] = std::make_pair(code, len);
    }
    else {
        if (node->left) {
            make_table_impl(node->left, code, len + 1);
        }
        if (node->right) {
            make_table_impl(node->right, code | (1 << len), len + 1);
        }
    }
}

void HuffmanArchiever::make_table()
{
    table.fill(std::make_pair(0, 0));
    make_table_impl(tree.root, 0, 0);
}

void HuffmanArchiever::write_encoded(code_t code_len, code_t table_size) const noexcept
{
    BitWriter writer(output_file);
    BitReader reader(input_file);
    try {
        writer.write_bytes(reinterpret_cast<const char *>(&code_len), sizeof(code_t));
        writer.write_bytes(reinterpret_cast<const char *>(&table_size), sizeof(code_t));
        for (code_t byte = 0; byte < VALUES_RANGE; byte++) {
            if (table[byte].second == 0)
                continue;
            writer.write_bytes(reinterpret_cast<const char *>(&table[byte].first), sizeof(code_t));
            writer.write_byte(table[byte].second);
            writer.write_byte(byte);
        }
    }
    catch (const BitIOException &) {
        std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                  << " - something went wrong with output file" << std::endl;
        exit(-1);
    }
    while (!reader.eof()) {
        byte_t byte;
        try {
            if (!reader.read_byte(byte)) {
                break;
            }
        }
        catch (const BitIOException &) {
            std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                      << " - something went wrong with input file" << std::endl;
            exit(-1);
        }
        code_t code = table[byte].first;
        byte_t len = table[byte].second;
        try {
            for (byte_t bit = 0; bit < len; bit++) {
                writer.write_bit((code >> bit) & 1);
            }
        }
        catch (const BitIOException &) {
            std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                      << " - something went wrong with output file" << std::endl;
            exit(-1);
        }
    }
}

void HuffmanArchiever::encode()
{
    std::array<code_t, VALUES_RANGE> frequencies = get_frequencies();
    code_t file_size = std::accumulate(frequencies.begin(), frequencies.end(), static_cast<code_t>(0));
    tree = HuffTree(frequencies);
    make_table();
    code_t code_len = 0;
    for (code_t byte = 0; byte < VALUES_RANGE; ++byte) {
        code_len += frequencies[byte] * table[byte].second;
    }
    code_t table_size = std::count_if(table.begin(), table.end(), [](std::pair<code_t, byte_t> val) { return val.second > 0; });
    std::cout << file_size << std::endl;
    std::cout << (code_len + bits_in_byte - 1) / (bits_in_byte) << std::endl;
    std::cout << table_size * (sizeof(code_t) + sizeof(byte_t)) + 2 * sizeof(code_t) << std::endl;
    write_encoded(code_len, table_size);
}

void HuffmanArchiever::decode()
{
    BitReader reader(input_file);
    code_t table_size, code_len;
    try {
        reader.read_bytes(reinterpret_cast<char *>(&code_len), sizeof(code_t));
        reader.read_bytes(reinterpret_cast<char *>(&table_size), sizeof(code_t));
    }
    catch (const BitIOException &) {
        std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                  << " - something went wrong with input file" << std::endl;
        exit(-1);
    }
    tree = HuffTree();
    for (code_t i = 0; i < table_size; i++) {
        code_t code;
        byte_t len = 0, byte = 0;
        try {
            reader.read_bytes(reinterpret_cast<char *>(&code), sizeof(code_t));
            reader.read_byte(len);
            reader.read_byte(byte);
            tree.add_letter(code, len, byte);
        }
        catch (const BitIOException &) {
            std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                      << " - something went wrong with input file" << std::endl;
            exit(-1);
        }
    }
    BitWriter writer(output_file);
    HuffTree::TreeNodePtr node = tree.root;
    code_t file_size = 0;
    for (code_t i = 0; i < code_len; i++) {
        bool bit;
        reader.read_bit(bit);
        if (bit) {
            node = node->right;
        }
        else {
            node = node->left;
        }
        if (!node) {
            std::cerr << "Incorrect input file" << std::endl;
            exit(-1);
        }
        if (node->is_leaf()) {
            try {
                writer.write_byte(node->letter);
            }
            catch (const BitIOException &) {
                std::cerr << "Generated exception in " << __FILE__ << ":" << __LINE__
                          << " - something went wrong with output file" << std::endl;
                exit(-1);
            }
            node = tree.root;
            ++file_size;
        }
    }
    std::cout << file_size << std::endl;
    std::cout << (code_len + bits_in_byte - 1) / (bits_in_byte) << std::endl;
    std::cout << table_size * (sizeof(code_t) + sizeof(byte_t)) + 2 * sizeof(code_t) << std::endl;
}