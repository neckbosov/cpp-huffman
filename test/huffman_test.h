#pragma once
#include "huffman.h"
#include <gtest/gtest.h>
#include <string>

bool file_equals(std::string f1, std::string f2);

class HuffmanTest : public ::testing::Test {
  protected:
    std::string tmp[3];
    void SetUp() override;
    void TearDown() override;
};

class BitIOTest : public ::testing::Test {
  protected:
    std::string tmp_file;
    void SetUp() override;
    void TearDown() override;
};