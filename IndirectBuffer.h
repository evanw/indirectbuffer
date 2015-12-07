#pragma once

#include <string>
#include <vector>

// An indirect buffer is memoery that isn't stored in the emscripten heap when
// compiled to the browser. This lets us get around the limitation of current
// browsers (mostly Chrome) where there aren't enough continuous ranges of
// address space to allocate large typed array objects. This is only an issue
// in 32-bit browsers and is caused by various things including the use of
// tcmalloc and ASLR (Address-Space Layout Randomization). This is just a
// normal chunk of memory on other platforms.
struct IndirectBuffer {
  // Constructors (use "explicit" to avoid cast-like behavior)
  IndirectBuffer();
  explicit IndirectBuffer(size_t count);
  explicit IndirectBuffer(const std::string &text);
  explicit IndirectBuffer(const std::vector<uint8_t> &bytes);
  IndirectBuffer(const uint8_t *bytes, size_t count);
  IndirectBuffer(IndirectBuffer &&buffer) noexcept;
  ~IndirectBuffer();

  // Forbid copying because it's expensive and implicit, use clone() instead
  IndirectBuffer(const IndirectBuffer &) = delete;
  IndirectBuffer &operator = (const IndirectBuffer &) = delete;

  // Moving and cloning
  IndirectBuffer &operator = (IndirectBuffer &&buffer) noexcept;
  IndirectBuffer clone() const;

  // Element accessors
  uint8_t get(size_t index) const;
  void set(size_t index, uint8_t byte);

  // Range accessors
  void get(size_t index, uint8_t *bytes, size_t count) const;
  void set(size_t index, const uint8_t *bytes, size_t count);

  // Size-related functions
  bool empty() const { return size() == 0; }
  size_t size() const;
  void resize(size_t count);

  // Range updates that avoid touching the emscripten heap
  void move(size_t newIndex, size_t oldIndex, size_t count);
  void copyFrom(size_t toIndex, size_t count, const IndirectBuffer &buffer, size_t fromIndex);
  static IndirectBuffer concat(const std::vector<const IndirectBuffer *> &buffers);

  // Convenience function to copy everything into the heap
  std::string toString() const;

  // Pass this to JavaScript to get the Uint8Array from Module._IB_[handleForEmscripten]
  int handleForEmscripten() const;

private:
  #if defined(__EMSCRIPTEN__)
    int _handle = 0;
    size_t _size = 0;
  #else
    std::vector<uint8_t> _data;
  #endif
};
