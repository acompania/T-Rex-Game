EXECUTABLE = dub-wubber
FRAMEWORKS = -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices -framework Carbon -framework OpenGL -framework GLUT
FLAGS = -g -Wno-deprecated
OBJS = brisk.o briskhelper.o GLSL_helper.o composer.o

all: $(OBJS)
	g++ $(FLAGS) -o $(EXECUTABLE) $(OBJS) libportaudio.a $(FRAMEWORKS) -lfftw3f -lm

%.o: %.cpp
	gcc -c $(FLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(EXECUTABLE)