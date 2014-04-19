CC=g++
EXECUTABLE=bin/main.out
OBJS=bin/Main.o bin/CMeshLoaderSimple.o bin/GLSL_helper.o bin/MStackHelp.o
CFLAGS= -std=c++0x -g -DGL_GLEXT_PROTOTYPES
INCS=lib

UNAME=$(shell uname)
ifeq ($(UNAME), Linux)
LIBS=-lGL -lGLU -lglfw3 -lXxf86vm -lXrandr -lpthread -lXi -lX11 -lXcursor -lftgl -lrt
endif

all: bin $(OBJS)
	$(CC) $(CFLAGS) -I$(INCS) -L$(INCS) $(OBJS) -o $(EXECUTABLE) $(LIBS)

bin: 
	mkdir -p bin

bin/%.o: src/%.cpp 
	$(CC) -I$(INCS) -L$(INCS) $(CFLAGS) -c -o $@ $<

clean: bin
	rm -r bin

