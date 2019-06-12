HDRS = include/elfloader.h
CXX_SRC = src/elfloader.cpp src/codeify.cpp
OUTPUT = qx86-codeify

CXX_OBJS = $(subst .cpp,.o,$(CXX_SRC))

INCLUDES=-Iinclude
CXXFLAGS?=-O3 -Wall


default: $(OUTPUT)

$(OUTPUT): $(CXX_OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(CXX_OBJS)

$(CXX_OBJS): $(HDRS) $(CXX_SRC)
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $*.cpp -o $@

clean:
	rm -f $(CXX_OBJS) $(OUTPUT)

