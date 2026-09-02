CXX := g++
CXXFLAGS := -std=gnu++26 -O2

LDFLAGS += -L/usr/local/lib/piper
LDFLAGS += -Wl,-rpath,/usr/local/lib/piper

SRC := \
	main.cpp \
	src/http.cpp \
	src/model.cpp \
	src/database.cpp \
	src/conversation.cpp \
	src/json.cpp \
	src/tool_manager.cpp \
	src/sound.cpp \
	src/client.cpp

OBJ := $(SRC:%.cpp=obj/%.o)

TARGET := a

LIBS := -lcurl -lsimdjson -lmariadb -lyyjson -lb64 -llexbor -lasound

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf obj $(TARGET)
