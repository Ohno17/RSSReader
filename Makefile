# Variables
CXX = g++
K_CXX = arm-linux-gnueabi-g++
LIBS = gtk+-2.0 libsoup-2.4 libxml-2.0
SRC = src/main.cpp src/save.cpp
OUT_DIR = build

# Flags
CXXFLAGS = `pkg-config --cflags $(LIBS)` -Wno-deprecated-declarations -static-libstdc++
LDFLAGS = `pkg-config --libs $(LIBS)`

# Targets
.PHONY: all desktop run clean

all: clean desktop run

desktop:
	mkdir -p $(OUT_DIR)
	mkdir -p downloads
	$(CXX) $(SRC) -o $(OUT_DIR)/rssreader_desktop $(CXXFLAGS) $(LDFLAGS)

kindle:
	mkdir -p $(OUT_DIR)
	$(K_CXX) -DKINDLE $(SRC) -o $(OUT_DIR)/rssreader_kindle $(CXXFLAGS) $(LDFLAGS)

run:
	$(OUT_DIR)/rssreader_desktop

clean:
	rm -rf $(OUT_DIR)