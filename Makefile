CXX = g++
CXXFLAGS = -std=c++20 -Isrc -Wall
LDFLAGS = -lws2_32
TARGET = client.exe

SRCS = $(shell find src -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
