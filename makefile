CXX      = g++
CXXFLAGS = -std=c++17 -I. -I./imgui -I./include -I./minizip
LDFLAGS  = -lglfw -lGL -ldl -ljsoncpp -lz -fpermissive  -Wwrite-strings

SRCS = main.cpp \
       src/glad.c \
	src/ylfile.cpp\
	src/yltool.cpp\
       imgui/imgui.cpp \
       imgui/imgui_demo.cpp \
       imgui/imgui_draw.cpp \
       imgui/imgui_tables.cpp \
       imgui/imgui_widgets.cpp \
       imgui/backends/imgui_impl_glfw.cpp \
       imgui/backends/imgui_impl_opengl3.cpp\
			 src/g4d.cpp\
       minizip/ioapi.c \
       minizip/mztools.c \
       minizip/unzip.c \
       minizip/zip.c

TARGET = g4d

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)
