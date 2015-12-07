#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "IndirectBuffer.h"

TEST_CASE("IndirectBuffer size constructor") {
  IndirectBuffer buffer(2);
  REQUIRE(buffer.size() == 2);
  REQUIRE(buffer.get(0) == 0);
  REQUIRE(buffer.get(1) == 0);
}

TEST_CASE("IndirectBuffer data constructor") {
  uint8_t data[2] = { 123, 234 };
  IndirectBuffer buffer(data, 2);
  REQUIRE(buffer.size() == 2);
  REQUIRE(buffer.get(0) == 123);
  REQUIRE(buffer.get(1) == 234);
}

TEST_CASE("IndirectBuffer get() and set()") {
  IndirectBuffer buffer;
  buffer.resize(8);

  buffer.set(3, 0x7F);
  buffer.set(4, 0x80);
  buffer.set(5, 0xFF);
  REQUIRE(buffer.get(0) == 0);
  REQUIRE(buffer.get(1) == 0);
  REQUIRE(buffer.get(2) == 0);
  REQUIRE(buffer.get(3) == 0x7F);
  REQUIRE(buffer.get(4) == 0x80);
  REQUIRE(buffer.get(5) == 0xFF);
  REQUIRE(buffer.get(6) == 0);
  REQUIRE(buffer.get(7) == 0);

  float value = M_PI;
  buffer.set(2, (uint8_t *)&value, sizeof(value));
  REQUIRE(buffer.get(0) == 0);
  REQUIRE(buffer.get(1) == 0);
  REQUIRE(buffer.get(2) == ((uint8_t *)&value)[0]);
  REQUIRE(buffer.get(3) == ((uint8_t *)&value)[1]);
  REQUIRE(buffer.get(4) == ((uint8_t *)&value)[2]);
  REQUIRE(buffer.get(5) == ((uint8_t *)&value)[3]);
  REQUIRE(buffer.get(6) == 0);
  REQUIRE(buffer.get(7) == 0);

  value = 123;
  REQUIRE(value == 123);
  buffer.get(2, (uint8_t *)&value, sizeof(value));
  REQUIRE(value == (float)M_PI);

  char data[8] = "testing";
  buffer.set(0, (uint8_t *)data, sizeof(data));
  REQUIRE(buffer.get(0) == 't');
  REQUIRE(buffer.get(1) == 'e');
  REQUIRE(buffer.get(2) == 's');
  REQUIRE(buffer.get(3) == 't');
  REQUIRE(buffer.get(4) == 'i');
  REQUIRE(buffer.get(5) == 'n');
  REQUIRE(buffer.get(6) == 'g');
  REQUIRE(buffer.get(7) == '\0');

  memset(data, -1, sizeof(data));
  buffer.get(0, (uint8_t *)data, sizeof(data));
  REQUIRE(data[0] == 't');
  REQUIRE(data[1] == 'e');
  REQUIRE(data[2] == 's');
  REQUIRE(data[3] == 't');
  REQUIRE(data[4] == 'i');
  REQUIRE(data[5] == 'n');
  REQUIRE(data[6] == 'g');
  REQUIRE(data[7] == '\0');
}

TEST_CASE("IndirectBuffer size() and resize()") {
  IndirectBuffer buffer;
  REQUIRE(buffer.size() == 0);

  buffer.resize(100);
  REQUIRE(buffer.size() == 100);

  buffer.resize(255 * 1024 * 1024);
  REQUIRE(buffer.size() == 255 * 1024 * 1024);

  buffer.resize(300);
  REQUIRE(buffer.size() == 300);

  buffer.resize(0);
  REQUIRE(buffer.size() == 0);

  buffer.resize(200);
  REQUIRE(buffer.size() == 200);
}

TEST_CASE("IndirectBuffer move constructor") {
  IndirectBuffer buffer1;
  buffer1.resize(2);
  buffer1.set(0, 123);
  buffer1.set(1, 234);
  IndirectBuffer buffer2(std::move(buffer1));

  REQUIRE(buffer1.size() == 0);
  REQUIRE(buffer2.size() == 2);
  REQUIRE(buffer2.get(0) == 123);
  REQUIRE(buffer2.get(1) == 234);
}

TEST_CASE("IndirectBuffer move assignment operator") {
  IndirectBuffer buffer1;
  buffer1.resize(2);
  buffer1.set(0, 123);
  buffer1.set(1, 234);

  IndirectBuffer buffer2;
  buffer2.resize(3);
  buffer2.set(0, 1);
  buffer2.set(1, 2);
  buffer2.set(2, 3);

  buffer1 = std::move(buffer2);

  REQUIRE(buffer1.size() == 3);
  REQUIRE(buffer1.get(0) == 1);
  REQUIRE(buffer1.get(1) == 2);
  REQUIRE(buffer1.get(2) == 3);

  REQUIRE(buffer2.size() == 2);
  REQUIRE(buffer2.get(0) == 123);
  REQUIRE(buffer2.get(1) == 234);
}

TEST_CASE("IndirectBuffer clone()") {
  IndirectBuffer buffer1;
  buffer1.resize(2);
  buffer1.set(0, 123);
  buffer1.set(1, 234);

  IndirectBuffer buffer2 = buffer1.clone();

  REQUIRE(buffer1.size() == 2);
  REQUIRE(buffer1.get(0) == 123);
  REQUIRE(buffer1.get(1) == 234);

  REQUIRE(buffer2.size() == 2);
  REQUIRE(buffer2.get(0) == 123);
  REQUIRE(buffer2.get(1) == 234);

  buffer1.set(0, 1);
  buffer1.set(1, 2);

  REQUIRE(buffer1.size() == 2);
  REQUIRE(buffer1.get(0) == 1);
  REQUIRE(buffer1.get(1) == 2);

  REQUIRE(buffer2.size() == 2);
  REQUIRE(buffer2.get(0) == 123);
  REQUIRE(buffer2.get(1) == 234);
}

TEST_CASE("IndirectBuffer move()") {
  IndirectBuffer buffer1(std::vector<uint8_t> { 'a', 'b', 'c', 'd', 'e' });
  buffer1.move(1, 2, 2);
  REQUIRE(buffer1.toString() == "acdde");

  IndirectBuffer buffer2(std::vector<uint8_t> { 'a', 'b', 'c', 'd', 'e' });
  buffer2.move(2, 1, 2);
  REQUIRE(buffer2.toString() == "abbce");
}

TEST_CASE("IndirectBuffer copyFrom()") {
  IndirectBuffer buffer1(std::vector<uint8_t> { 'a', 'b', 'c', 'd', 'e' });
  IndirectBuffer buffer2(std::vector<uint8_t> { 'f', 'g', 'h', 'i', 'j', 'k' });
  buffer2.copyFrom(3, 2, buffer1, 1);
  REQUIRE(buffer1.toString() == "abcde");
  REQUIRE(buffer2.toString() == "fghbck");
}

TEST_CASE("IndirectBuffer copyFrom() with self") {
  IndirectBuffer buffer1(std::vector<uint8_t> { 'a', 'b', 'c', 'd', 'e' });
  buffer1.copyFrom(1, 2, buffer1, 2);
  REQUIRE(buffer1.toString() == "acdde");

  IndirectBuffer buffer2(std::vector<uint8_t> { 'a', 'b', 'c', 'd', 'e' });
  buffer2.copyFrom(2, 2, buffer2, 1);
  REQUIRE(buffer2.toString() == "abbce");
}

TEST_CASE("IndirectBuffer concat()") {
  IndirectBuffer buffer1(std::vector<uint8_t> { 'a', 'b', 'c' });
  IndirectBuffer buffer2(std::vector<uint8_t> { 'd', 'e' });
  IndirectBuffer buffer3(std::vector<uint8_t> {});
  IndirectBuffer buffer4(std::vector<uint8_t> { 'f' });
  IndirectBuffer all = IndirectBuffer::concat(std::vector<const IndirectBuffer *> { &buffer1, &buffer2, &buffer3, &buffer4 });
  REQUIRE(buffer1.toString() == "abc");
  REQUIRE(buffer2.toString() == "de");
  REQUIRE(buffer3.toString() == "");
  REQUIRE(buffer4.toString() == "f");
  REQUIRE(all.toString() == "abcdef");
}
