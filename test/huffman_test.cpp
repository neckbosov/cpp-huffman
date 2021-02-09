#include "huffman_test.h"
#include "huffman.h"
#include <array>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <random>
#include <string>

bool file_equals(std::string f1, std::string f2)
{
    std::ifstream in1(f1, std::ios::binary), in2(f2, std::ios::binary);
    while (true) {
        char c1, c2;
        in1.get(c1);
        in2.get(c2);
        if (in1.eof() || in2.eof()) {
            break;
        }
        if (c1 != c2) {
            return false;
        }
    }
    return in1.eof() && in2.eof();
}

void HuffmanTest::SetUp()
{
    for (int i = 0; i < 3; i++) {
        tmp[i] = "kektmp" + std::to_string(i);
        std::ofstream fs(tmp[i], std::ios::binary);
        fs.close();
    }
}

void HuffmanTest::TearDown()
{
    for (int i = 0; i < 3; i++) {
        remove(tmp[i].c_str());
    }
}

void BitIOTest::SetUp()
{
    tmp_file = "kektmp";
    std::ofstream fs(tmp_file, std::ios::binary);
    fs.close();
}

void BitIOTest::TearDown()
{
    remove(tmp_file.c_str());
}

TEST_F(HuffmanTest, EmptyFile)
{
    HuffmanArchiever archiever(tmp[0], tmp[1]);
    archiever.encode();
    archiever = HuffmanArchiever(tmp[1], tmp[2]);
    archiever.decode();
    EXPECT_TRUE(file_equals(tmp[0], tmp[2]));
    std::ifstream in2(tmp[1], std::ios::binary);
    code_t code_len, table_size;
    char c;
    in2.read(reinterpret_cast<char *>(&code_len), sizeof(code_t));
    in2.read(reinterpret_cast<char *>(&table_size), sizeof(code_t));
    EXPECT_EQ(code_len, 0);
    EXPECT_EQ(table_size, 0);
    in2.get(c);
    EXPECT_TRUE(in2.eof());
}

TEST_F(HuffmanTest, OneSymbol)
{
    std::ofstream out(tmp[0], std::ios::binary);
    out.put(0);
    out.close();
    HuffmanArchiever archiever(tmp[0], tmp[1]);
    archiever.encode();
    archiever = HuffmanArchiever(tmp[1], tmp[2]);
    archiever.decode();
    EXPECT_TRUE(file_equals(tmp[0], tmp[2]));
}

TEST_F(HuffmanTest, TextFile)
{
    std::ofstream out(tmp[0]);
    out << "hello world";
    out.close();
    HuffmanArchiever archiever(tmp[0], tmp[1]);
    archiever.encode();
    archiever = HuffmanArchiever(tmp[1], tmp[2]);
    archiever.decode();
    EXPECT_TRUE(file_equals(tmp[0], tmp[2]));
}

TEST_F(HuffmanTest, RandomBinaryFile)
{
    std::mt19937 gen(228);
    std::ofstream out(tmp[0], std::ios::binary);
    const int number_of_bytes = 5 * (1 << 20);
    for (int i = 0; i < number_of_bytes; ++i) {
        out.put(static_cast<char>(gen()));
    }
    out.close();
    HuffmanArchiever archiever(tmp[0], tmp[1]);
    archiever.encode();
    archiever = HuffmanArchiever(tmp[1], tmp[2]);
    archiever.decode();
    EXPECT_TRUE(file_equals(tmp[0], tmp[2]));
}

TEST(HuffTreeTest, Empty)
{
    HuffTree tree;
    EXPECT_EQ(tree.get_total_length(), 0);
}

TEST(HuffTreeTest, OneSymbol)
{
    std::array<code_t, VALUES_RANGE> frequencies;
    frequencies.fill(0);
    frequencies[0] = 1;
    HuffTree tree(frequencies);
    EXPECT_EQ(tree.get_total_length(), 1);
}

TEST(HuffTreeTest, BambooHuffmanTree)
{
    std::array<code_t, VALUES_RANGE> frequencies;
    frequencies.fill(0);
    frequencies[1] = frequencies[2] = 6;
    frequencies[4] = frequencies[3] = 2;
    frequencies[5] = frequencies[6] = 1;
    HuffTree tree(frequencies);
    EXPECT_EQ(tree.get_total_length(), 42);
}

TEST_F(BitIOTest, ReadEmptyFile)
{
    BitReader reader(tmp_file);
    bool bit;
    EXPECT_FALSE(reader.read_bit(bit));
    EXPECT_TRUE(reader.eof());
}

TEST_F(BitIOTest, ReadBits)
{
    byte_t val = 228;
    std::ofstream out(tmp_file, std::ios::binary);
    out.put(val);
    out.close();
    BitReader reader(tmp_file);
    byte_t byte = 0;
    bool bit;
    for (int i = 0; i < bits_in_byte; i++) {
        EXPECT_TRUE(reader.read_bit(bit));
        if (bit) {
            byte |= 1 << i;
        }
    }
    EXPECT_FALSE(reader.read_bit(bit));
    EXPECT_TRUE(reader.eof());
    EXPECT_EQ(byte, val);
}

TEST_F(BitIOTest, WriteNothing)
{
    BitWriter writer(tmp_file);
    writer.close();
    std::ifstream in(tmp_file);
    char val;
    EXPECT_FALSE(in.get(val));
    EXPECT_TRUE(in.eof());
}

TEST_F(BitIOTest, WriteBits)
{
    BitWriter writer(tmp_file);
    char val = 47;
    for (int i = 0; i < 6; i++) {
        writer.write_bit((val >> i) & 1);
    }
    writer.close();
    std::ifstream in(tmp_file);
    char byte;
    EXPECT_TRUE(in.get(byte));
    EXPECT_EQ(byte, val);
    EXPECT_FALSE(in.get(val));
    EXPECT_TRUE(in.eof());
}