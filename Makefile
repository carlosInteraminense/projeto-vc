CPP_FLAGS=
OPENCV_LIBS= -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_tracking
LD_FLAGS= $(OPENCV_LIBS)

sources=$(wildcard src/*.cpp)
objects=$(patsubst src/%.cpp, release/%.o, $(sources))

release/%.o :src/%.cpp
	g++ $(CPP_FLAGS) -c $^ -o $@
	
all: $(objects)
	g++ $(CPP_FLAGS) -o detectPeople $(objects) $(LD_FLAGS)

clean:
	rm -rf release/* detectPeople
