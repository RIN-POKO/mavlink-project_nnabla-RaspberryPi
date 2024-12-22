# Makefile for unified build

# Detect architecture
ARCH := $(shell uname -m)

ifeq ($(ARCH), aarch64)
    CXX = g++
    CXXFLAGS = -I$(shell pwd)/mavlink/include/mavlink/v2.0 -I$(shell pwd)/nnabla-cpplib-1.38.0-Linux_aarch64/include -I/usr/include/libcamera -I$(shell pwd)/LCCV/include -std=c++17 `pkg-config --cflags opencv4`
    LDFLAGS = -L$(shell pwd)/nnabla-cpplib-1.38.0-Linux_aarch64/lib -L/usr/lib/aarch64-linux-gnu \
              -Wl,-rpath,$(shell pwd)/nnabla-cpplib-1.38.0-Linux_aarch64/lib -Wl,-rpath,/usr/lib/aarch64-linux-gnu
    LDLIBS = -lnnabla -lnnabla_utils -lnnabla_cli -lcamera -lcamera-base -lcamera_app `pkg-config --libs opencv4`
    export LD_LIBRARY_PATH := $(shell pwd)/nnabla-cpplib-1.38.0-Linux_aarch64/lib:/usr/lib/aarch64-linux-gnu:$(LD_LIBRARY_PATH)

else ifeq ($(ARCH), armv7l)
    CXX = g++
    CXXFLAGS = -I$(shell pwd)/mavlink/include/mavlink/v2.0 -I$(shell pwd)/nnabla-cpplib-1.40.0.-Linux_armv7l/include -I/usr/include/libcamera -I$(shell pwd)/LCCV/include -std=c++17 `pkg-config --cflags opencv4`
    LDFLAGS = -L$(shell pwd)/nnabla-cpplib-1.40.0.-Linux_armv7l/lib -L/usr/lib/arm-linux-gnueabihf \
              -Wl,-rpath,$(shell pwd)/nnabla-cpplib-1.40.0.-Linux_armv7l/lib -Wl,-rpath,/usr/lib/arm-linux-gnueabihf
    LDLIBS = -lnnabla -lnnabla_utils -lnnabla_cli -lcamera -lcamera-base -lcamera_app `pkg-config --libs opencv4`
    export LD_LIBRARY_PATH := $(shell pwd)/nnabla-cpplib-1.40.0.-Linux_armv7l/lib:/usr/lib/arm-linux-gnueabihf:$(LD_LIBRARY_PATH)

else ifeq ($(ARCH), x86_64)
    CXX = g++
    CXXFLAGS = -I$(shell pwd)/mavlink/include/mavlink/v2.0 -I$(shell pwd)/nnabla-cpplib-1.39.0-Rocky8.9_x86_64/include -I/usr/include/libcamera -I$(shell pwd)/LCCV/include -std=c++17 `pkg-config --cflags opencv4`
    LDFLAGS = -L$(shell pwd)/nnabla-cpplib-1.39.0-Rocky8.9_x86_64/lib -L/usr/lib/x86_64-linux-gnu \
              -Wl,-rpath,$(shell pwd)/nnabla-cpplib-1.39.0-Rocky8.9_x86_64/lib -Wl,-rpath,/usr/lib/x86_64-linux-gnu
    LDLIBS = -lnnabla -lnnabla_utils -lnnabla_cli -lcamera -lcamera-base -lcamera_app `pkg-config --libs opencv4`
    export LD_LIBRARY_PATH := $(shell pwd)/nnabla-cpplib-1.39.0-Rocky8.9_x86_64/lib:/usr/lib/x86_64-linux-gnu:$(LD_LIBRARY_PATH)

else
    $(error Unsupported architecture: $(ARCH))
endif

# Targets
all: mavlink_control

mavlink_control: mavlink_control.o serial_port.o udp_port.o autopilot_interface.o nnabla_runtime.o lccv.o libcamera_app.o libcamera_app_options.o
	$(CXX) -g -Wall $(LDFLAGS) $^ -o $@ $(LDLIBS)

git_submodule:
	git submodule update --init --recursive

mavlink_control.o: mavlink_control.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

serial_port.o: serial_port.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

udp_port.o: udp_port.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

autopilot_interface.o: autopilot_interface.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

nnabla_runtime.o: nnabla_runtime.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

lccv.o: LCCV/src/lccv.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

libcamera_app.o: LCCV/src/libcamera_app.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

libcamera_app_options.o: LCCV/src/libcamera_app_options.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o mavlink_control
