EXECUTABLE = dub-wubber
FRAMEWORKS = -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
OBJS = wubber_test.o dub_wubber.o composer.o
FLAGS = -g -Wno-deprecated
LIBS = libportaudio.a

all: $(OBJS)
	g++ $(FLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBS) $(FRAMEWORKS)

%.o: %.cpp
	gcc -c $(FLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(EXECUTABLE)