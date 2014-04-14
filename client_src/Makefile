# Builds a T-Rex executable


OBJS= Client.o Utils.o 
FLAGS= -g -Wno-deprecated -DGL_GLEXT_PROTOTYPES
EXECUTABLE= ../T-Rex

UNAME= $(shell uname)
ifeq ($(UNAME), Linux)
LIBS= -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi
else
ifeq ($(UNAME), Darwin)
FRAMEWORKS= -framework ApplicationServices -framework GLUT -framework -lglfw3 -framework COCO -framework OpenGL -framework IOKit -framework CoreVideo 
endif
endif


all: client

client: $(OBJS)
	g++ -o $(EXECUTABLE) $(OBJS) $(FLAGS) $(LIBS) $(FRAMEWORKS)

Client.o: Client.cpp 
	g++ -c -o $@ $< $(FLAGS)

Utils.o: Utils.cpp
	g++ -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJS)
