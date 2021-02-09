#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using byte_t = unsigned char;
using code_t = std::uint32_t;

const code_t VALUES_RANGE = (code_t)(1) << (sizeof(byte_t) * 8);
const byte_t bits_in_byte = 8 * sizeof(byte_t);
class HuffTree {

    class TreeNode {
      public:
        std::shared_ptr<TreeNode> left;
        std::shared_ptr<TreeNode> right;
        byte_t letter;
        code_t frequency;
        TreeNode(byte_t symbol, code_t freq) noexcept;
        TreeNode() noexcept;
        TreeNode(std::shared_ptr<TreeNode> l, std::shared_ptr<TreeNode> r) noexcept;
        bool is_leaf() const noexcept;
    };
    friend class HuffmanArchiever;
    using TreeNodePtr = std::shared_ptr<TreeNode>;
    TreeNodePtr root;
    code_t get_total_length_impl(TreeNodePtr node, code_t cur_len) const noexcept;

  public:
    HuffTree() noexcept;
    HuffTree(std::array<code_t, VALUES_RANGE> frequencies);
    code_t get_total_length() const noexcept;
    void add_letter(code_t code, byte_t len, byte_t letter) noexcept;
};

class BitReader {
    byte_t current_bit, current_byte;
    std::ifstream in;

  public:
    BitReader(const std::string &file);
    bool read_bit(bool &bit);
    bool read_byte(byte_t &byte);
    void read_bytes(char *bytes, code_t len);
    bool eof() const noexcept;
    void close() noexcept;
};

class BitWriter {
    byte_t current_bit, current_byte;

    std::ofstream out;

  public:
    BitWriter(const std::string &file);
    ~BitWriter();
    void write_bit(bool bit);
    void write_byte(byte_t byte);
    void write_bytes(const char *bytes, code_t len);
    void close() noexcept;
};

using BitIOException = std::ios::failure;

class HuffmanArchiever {
    std::string input_file, output_file;
    std::array<std::pair<code_t, byte_t>, VALUES_RANGE> table;
    HuffTree tree;
    std::array<code_t, VALUES_RANGE> get_frequencies() const noexcept;
    void make_table_impl(HuffTree::TreeNodePtr node, code_t code, byte_t len);
    void make_table();
    void write_encoded(code_t code_len, code_t table_size) const noexcept;

  public:
    HuffmanArchiever(const std::string &input, const std::string &output);

    void encode();
    void decode();
};