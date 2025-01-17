CXX = g++
CXXFLAGS = -std=c++11 -Wall
INCLUDE_DIR = /usr/local/include
LIB_DIR = /usr/local/lib
BOOST_LIBS = -lboost_system -lboost_thread 



# Debug and Release flags
CXXFLAGS_DEBUG = -O0 -g2 -ggdb
CXXFLAGS_RELEASE = -O3

# Source files
SRCS = main.cpp Utils/Config.cpp Server/BoostServer.cpp Server/HttpServerConnection.cpp Scheduler.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = L4-httpflvServer

# Libraries to link
LIBS = -ljsoncpp -lpthread $(BOOST_LIBS)

# Default build type
BUILD_TYPE = DEBUG

# Include and library directories
INCLUDES = -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

# Debug build
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: clean all

# Release build
release: CXXFLAGS += $(CXXFLAGS_RELEASE)
release: clean all