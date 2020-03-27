// Copyright: 2020 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/utils/any.hpp"

#include <vector>

#include <quick/debug.hpp>
#include "gtest/gtest.h"


using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using std::vector;

using aparse::utils::any;

TEST(ANY, Basic) {
  any x = 44;
  EXPECT_EQ(44, x.cast_to<int>());
  any y;
  y = std::move(x);
  EXPECT_EQ(44, y.cast_to<int>());
  x = std::move(y);
  EXPECT_EQ(44, x.cast_to<int>());
  y = x;
  EXPECT_EQ(44, y.cast_to<int>());
  x = string("Mohit Saini");
  EXPECT_EQ("Mohit Saini", x.cast_to<string>());
  y = std::move(x);
  auto z = y;
  EXPECT_EQ("Mohit Saini", z.cast_to<string>());
  x = y;
  EXPECT_EQ("Mohit Saini", x.cast_to<string>());
  const auto& xx = x.cast_to<string>();
  auto& yy = x.cast_to<string>();
  EXPECT_EQ("Mohit Saini", xx);
  yy = "Saini Mohit";
  EXPECT_EQ("Saini Mohit", x.cast_to<string>());
  x.create_in_place<int>(10);
  EXPECT_EQ(10, x.cast_to<int>());
  x.create_in_place<vector<int>>((vector<int>{10, 20, 30}));
  x.create_in_place<vector<int>>(5, 111);
  EXPECT_EQ(5, x.cast_to<vector<int>>().size());
  y = x;
  EXPECT_EQ(5, y.cast_to<vector<int>>().size());
  string e_str;
  try {
    y.cast_to<int>();
  } catch (const std::exception& e) {
    e_str = e.what();
  }
  EXPECT_EQ("[aparse::utility::any]: Bad casting", e_str);
}
