#include "IndirectBuffer.h"

#include <assert.h>

#if defined(__EMSCRIPTEN__)
  #include <emscripten.h>
#endif

IndirectBuffer::IndirectBuffer() {
  #if defined(__EMSCRIPTEN__)
    static int nextHandle = 1;
    _handle = ++nextHandle;

    EM_ASM_ARGS({
      (Module._IB_ || (Module._IB_ = {}))[$0] = new Uint8Array();
    }, _handle);
  #endif
}

IndirectBuffer::IndirectBuffer(size_t count) : IndirectBuffer() {
  resize(count);
}

IndirectBuffer::IndirectBuffer(const std::string &text) : IndirectBuffer((const uint8_t *)text.data(), text.size()) {
}

IndirectBuffer::IndirectBuffer(const std::vector<uint8_t> &bytes) : IndirectBuffer(bytes.data(), bytes.size()) {
}

IndirectBuffer::IndirectBuffer(const uint8_t *bytes, size_t count) : IndirectBuffer(count) {
  set(0, bytes, count);
}

IndirectBuffer::IndirectBuffer(IndirectBuffer &&buffer) noexcept : IndirectBuffer() {
  *this = std::move(buffer);
}

IndirectBuffer::~IndirectBuffer() {
  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      delete Module._IB_[$0];
    }, _handle);
  #endif
}

IndirectBuffer &IndirectBuffer::operator = (IndirectBuffer &&buffer) noexcept {
  #if defined(__EMSCRIPTEN__)
    std::swap(_handle, buffer._handle);
    std::swap(_size, buffer._size);
  #else
    _data.swap(buffer._data);
  #endif
  return *this;
}

IndirectBuffer IndirectBuffer::clone() const {
  IndirectBuffer result(size());
  result.copyFrom(0, size(), *this, 0);
  return result;
}

uint8_t IndirectBuffer::get(size_t index) const {
  assert(index < size());

  #if defined(__EMSCRIPTEN__)
    return EM_ASM_INT({
      return Module._IB_[$0][$1];
    }, _handle, index);
  #else
    return _data[index];
  #endif
}

void IndirectBuffer::set(size_t index, uint8_t byte) {
  assert(index < size());

  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      Module._IB_[$0][$1] = $2;
    }, _handle, index, byte);
  #else
    _data[index] = byte;
  #endif
}

void IndirectBuffer::get(size_t index, uint8_t *bytes, size_t count) const {
  assert(index + count <= size());

  if (!bytes || !count) {
    return;
  }

  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      Module.HEAP8.set(Module._IB_[$0].subarray($1, $1 + $3), $2);
    }, _handle, index, bytes, count);
  #else
    memcpy(bytes, _data.data() + index, count);
  #endif
}

void IndirectBuffer::set(size_t index, const uint8_t *bytes, size_t count) {
  assert(index + count <= size());

  if (!bytes || !count) {
    return;
  }

  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      Module._IB_[$0].set(Module.HEAP8.subarray($2, $2 + $3), $1);
    }, _handle, index, bytes, count);
  #else
    memcpy(_data.data() + index, bytes, count);
  #endif
}

size_t IndirectBuffer::size() const {
  #if defined(__EMSCRIPTEN__)
    return _size;
  #else
    return _data.size();
  #endif
}

void IndirectBuffer::resize(size_t count) {
  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      var old = Module._IB_[$0];
      (Module._IB_[$0] = new Uint8Array($1)).set(old.length < $1 ? old : old.subarray(0, $1));
    }, _handle, count);
    _size = count;
  #else
    _data.resize(count);
  #endif
}

void IndirectBuffer::move(size_t newIndex, size_t oldIndex, size_t count) {
  assert(oldIndex + count <= size());
  assert(newIndex + count <= size());

  if (oldIndex == newIndex) {
    return;
  }

  #if defined(__EMSCRIPTEN__)
    EM_ASM_ARGS({
      var array = Module._IB_[$0];
      array.set(array.subarray($2, $2 + $3), $1);
    }, _handle, newIndex, oldIndex, count);
  #else
    memmove(_data.data() + newIndex, _data.data() + oldIndex, count);
  #endif
}

void IndirectBuffer::copyFrom(size_t toIndex, size_t count, const IndirectBuffer &buffer, size_t fromIndex) {
  assert(fromIndex + count <= buffer.size());
  assert(toIndex + count <= size());

  if (this == &buffer) {
    move(toIndex, fromIndex, count);
  } else {
    #if defined(__EMSCRIPTEN__)
      EM_ASM_ARGS({
        var fromArray = Module._IB_[$3];
        var toArray = Module._IB_[$0];
        toArray.set(fromArray.subarray($4, $4 + $2), $1);
      }, _handle, toIndex, count, buffer._handle, fromIndex);
    #else
      set(toIndex, buffer._data.data() + fromIndex, count);
    #endif
  }
}

IndirectBuffer IndirectBuffer::concat(const std::vector<const IndirectBuffer *> &buffers) {
  size_t totalSize = 0;
  for (const auto &buffer : buffers) {
    totalSize += buffer->size();
  }
  IndirectBuffer result(totalSize);
  size_t index = 0;
  for (const auto &buffer : buffers) {
    result.copyFrom(index, buffer->size(), *buffer, 0);
    index += buffer->size();
  }
  return std::move(result);
}

std::string IndirectBuffer::toString() const {
  std::string result;
  result.resize(size());
  get(0, (uint8_t *)&result[0], size());
  return result;
}

int IndirectBuffer::handleForEmscripten() const {
  #if defined(__EMSCRIPTEN__)
    return _handle;
  #else
    return 0;
  #endif
}
