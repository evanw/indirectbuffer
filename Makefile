test: test-cpp test-js

test-cpp:
	c++ main.cpp IndirectBuffer.cpp -std=c++11
	./a.out
	rm a.out

test-js:
	emcc main.cpp IndirectBuffer.cpp -std=c++11
	node a.out.js
	rm a.out.js
