EXECUTABLE = dub_wubber
OBJS = wubber_test.o dub_wubber.o composer.o
FLAGS = -g -Wno-deprecated

UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LIBS = -lrt -lasound -ljack -lpthread libportaudio.a
else
ifeq ($(UNAME), Darwin)
FRAMEWORKS = -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
endif
endif

all: $(OBJS)
	g++ $(FLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBS) $(FRAMEWORKS)

composer.o: composer.cpp
	g++ -c $(FLAGS) -o $@ $<
	
dub-wubber.o: dub_wubber.cpp
	g++ -c $(FLAGS) -o $@ $< $(LIBS)
	
wubber_test.o: wubber_test.cpp
	g++ -c $(FLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(EXECUTABLE)
