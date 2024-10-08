# Detect OS
ifeq ($(OS),Windows_NT)
    # Windows settings
    EXECUTABLE = bifrost.exe
    INCLUDES = -IC:/SFML/include -IE:/Projects/fuzzylite/install/include
    LIBS = -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system -ldwmapi -LE:/Projects/fuzzylite/install/bin -llibfuzzylite
    RM = del /Q
else
    # Linux settings
    EXECUTABLE = bifrost
    INCLUDES = -I/usr/include/SFML -I/usr/local/include/fuzzylite
    LIBS = -L/usr/lib -lsfml-graphics -lsfml-window -lsfml-system -L/usr/local/lib -lfuzzylite
    RM = rm -f
endif

# Common settings
CXX = g++
CXXFLAGS = -g
SRCDIR = src
SRCEXT = cpp
SOURCES = $(wildcard *.cpp) $(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS = $(SOURCES:.cpp=.o)

# Build target
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)

.PHONY: clean
