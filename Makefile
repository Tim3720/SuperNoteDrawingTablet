CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23 -O3

# Add Eigen include path to compiler flags
CXXFLAGS += $(EIGEN_CFLAGS)

# Source files
SRCDIR = src
BUILDDIR := build
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRCS))

# Executable name
TARGET = SuperNoteDriver

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: all clean
