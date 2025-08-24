# GameManager/Makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic -O2 -g -fPIC -I../common -I../UserCommon

# All cpp files in this folder, except main or registration impls
GM_SRCS := $(filter-out %main.cpp %Registration.cpp,$(wildcard *.cpp))

# Pull in only the UserCommon sources you actually need
# If you are unsure, start with all and trim later
UC_SRCS := $(filter-out %main.cpp %Registration.cpp,$(wildcard ../UserCommon/*.cpp))

SRCS := $(GM_SRCS) $(UC_SRCS)
OBJS := $(SRCS:.cpp=.o)

TARGET := Algorithm__208000547_208000547.so

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -shared -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

print:
	@echo SRCS=$(SRCS)
