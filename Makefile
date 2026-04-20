CXX = clang++
CXXFLAGS += -std=c++17 -Wall -Wextra -g -O3 -Wno-deprecated-declarations 
CPPFLAGS +=	-Iexternal_depn -Iexternal_depn/glm -Iheaders -Iexternal_depn/rapidjson-1.1.0/include \
			-Iexternal_depn/lua-5.4.6  -Iexternal_depn/LuaBridge-2.7 -Iexternal_depn/box2d-2.4.1/include \
			-Iexternal_depn/box2d-2.4.1/src \
# 			-Iexternal_depn/SDL2 -Iexternal_depn/SDL_image -Iexternal_depn/SDL_mixer \
# 			-Iexternal_depn/SDL_ttf

# LDFLAGS += -Lexternal_depn/box2d-2.4.1/build/bin -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2_image -llua5.4 -lbox2d
		
# BOX2D_LIB = external_depn/box2d-2.4.1/build/bin/libbox2d.a

LDFLAGS +=  -Fexternal_depn/SDL2/lib -framework SDL2 -Wl,-rpath,external_depn/SDL2/lib \
			-Fexternal_depn/SDL_image/lib -framework SDL2_image -Wl,-rpath,external_depn/SDL_image/lib \
			-Fexternal_depn/SDL_mixer/lib -framework SDL2_mixer -Wl,-rpath,external_depn/SDL_mixer/lib \
			-Fexternal_depn/SDL_ttf/lib -framework SDL2_ttf -Wl,-rpath,external_depn/SDL_ttf/lib \
			-Lexternal_depn/lua-5.4.6 -llua -Lexternal_depn/box2d-2.4.1/build/bin -lbox2d
SRC_DIR = src
BUILD_DIR = build

TARGET := waschbaer_engine
# Source files (automatically finds all .cpp files in SRC_DIR)
SOURCES := $(wildcard $(SRC_DIR)/*.cpp) 
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)


# $(BOX2D_LIB):
# 	cd external_depn/box2d-2.4.1 && mkdir -p build && cd build && cmake .. -DBOX2D_BUILD_TESTBED=OFF -DBOX2D_BUILD_UNIT_TESTS=OFF && make

$(TARGET): $(OBJECTS) # $(BOX2D_LIB)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# $(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp | $(BUILD_DIR)
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)