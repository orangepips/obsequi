#ifndef OBSEQUI_TEST
#define OBSEQUI_TEST

#include <iostream>
#include <string>
#include <vector>

// Basic Test Infrastructure.
typedef void(*TestFunc)();
struct TestData { std::string test_name; TestFunc test_func; };
std::vector<TestData> tests;
bool add_test(std::string test_name, TestFunc test_func) {
  TestData data = {test_name, test_func};
  tests.push_back(data);
  return true;
}
#define TEST(x) void x(); bool init_##x = add_test(#x, x); void x()
#define NO_TEST(x) void x()

// Run each test.
int main() {
  for (const TestData& data : tests) {
    std::cout << "START TEST: " << data.test_name << std::endl;
    data.test_func();
    std::cout << "TEST PASSED: " << data.test_name << std::endl;
    std::cout << std::endl;
  }
}

#endif  // OBSEQUI_TEST
