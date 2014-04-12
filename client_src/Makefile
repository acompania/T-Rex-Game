# Builds a T-Rex executable


OBJS = Client.o Utils.o GLSL_helper.o MStackHelp.o TextureLoader.o mesh.o
FLAGS = -g -Wno-deprecated -DGL_GLEXT_PROTOTYPES
EXECUTABLE = T-Rex

UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LIBS =  -lGL -lGLU -lglut -DGL_GLEXT_PROTOTYPES
else
ifeq ($(UNAME), Darwin)
FRAMEWORKS = -framework ApplicationServices -framework GLUT -framework OpenGL 
endif
endif


all: client

client: $(OBJS)
	g++ -o $(EXECUTABLE) $(OBJS) $(FLAGS) $(LIBS) $(FRAMEWORKS)

Client.o: Client.cpp 
	g++ -c -o $@ $< $(FLAGS)

Utils.o: Utils.cpp
	g++ -c -o $@ $<

# For loading obj files
mesh.o: mesh.cpp
	g++ -c -o $@ $< $(LIBS)

GLSL_helper.o: GLSL_helper.cpp
	g++ -c -o $@ $< $(FLAGS)

# For heirarchical modeling
MStackHelp.o: MStackHelp.cpp
	g++ -c -o $@ $< 

TextureLoader.o: TextureLoader.cpp 
	g++ -c -o $@ $< $(LIBS)

clean:
	rm -f $(EXECUTABLE) $(OBJS)
