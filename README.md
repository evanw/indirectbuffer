# IndirectBuffer

This library provides a way for [emscripten](http://emscripten.org)-compiled C++ code to store large amounts of data outside of the main heap. Moving large allocations out of the main heap reduces memory fragmentation issues for long-running sessions, allows your code to use more of the limited address space in 32-bit browsers, and allows your code to break past the [31-bit typed array size limitation](https://github.com/WebKit/webkit/blob/f01d2bb66fcde2c3519c4f0c61f790387fd5faee/Source/JavaScriptCore/runtime/ArrayBuffer.h#L255) in 64-bit browsers.

This library isn't exactly what we use internally at [Figma](https://www.figma.com) but it's the same general idea. We're providing this code because we think indirect memory storage is a big win and it's not described in the emscripten documentation. The closest analogy is the emscripten file system API but this IndirectBuffer API is simpler and easier to work with both from C++ and from JavaScript.

To use it, just copy IndirectBuffer.h and IndirectBuffer.cpp into your project. The main.cpp and catch.hpp files included here are for unit testing (run "make test" assuming you have emscripten installed and activated).

Example usage (out-of-heap image decode):

```cpp
// Compile using: emcc example.cpp IndirectBuffer.cpp -std=c++11 -O3 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS="['_main','_DecodeImage_resize','_DecodeImage_finish']"

#include <emscripten.h>
#include <functional>
#include <stdio.h>
#include <string>
#include "IndirectBuffer.h"

struct DecodeImage {
  using Callback = std::function<void (int, int, IndirectBuffer)>;

  DecodeImage(const std::string &url, const Callback &callback) : _callback(callback) {
    EM_ASM_ARGS({
      var image = new Image;
      image.onload = function() {
        var width = image.width;
        var height = image.height;
        var canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;
        var context = canvas.getContext('2d');
        context.drawImage(image, 0, 0);
        var pixels = context.getImageData(0, 0, width, height);
        var handle = Module._DecodeImage_resize($0, pixels.data.length);
        Module._IB_[handle].set(new Uint8Array(pixels.data.buffer));
        Module._DecodeImage_finish($0, width, height);
      };
      image.src = Module.Pointer_stringify($1);
    }, this, url.c_str());
  }

  int resize(size_t size) {
    _buffer.resize(size);
    return _buffer.handleForEmscripten();
  }

  void finish(int width, int height) {
    _callback(width, height, std::move(_buffer));
  }

private:
  Callback _callback;
  IndirectBuffer _buffer;
};

extern "C" int DecodeImage_resize(DecodeImage *self, size_t size) {
  return self->resize(size);
}

extern "C" void DecodeImage_finish(DecodeImage *self, int width, int height) {
  self->finish(width, height);
}

static bool isImageOpaque(const IndirectBuffer &buffer) {
  return EM_ASM_INT({
    var array = Module._IB_[$0];
    for (var i = 3, n = array.length; i < n; i += 4) {
      if (array[i] < 255) {
        return false;
      }
    }
    return true;
  }, buffer.handleForEmscripten());
}

int main() {
  static DecodeImage async("image.png", [](int width, int height, IndirectBuffer buffer) {
    printf("loaded %dx%d image outside main heap\n", width, height);
    printf("image is opaque: %d\n", isImageOpaque(buffer));
  });
  return 0;
}
```
