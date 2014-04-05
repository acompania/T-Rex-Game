# Builds a T-Rex executable

EXECUTABLE = ../T-Rex
OBJS = Client.o Utils.o GLSL_helper.o MStackHelp.o TextureLoader.o mesh.o
# Frameworks are a Mac thing
FRAMEWORKS = -framework ApplicationServices -framework GLUT -framework OpenGL 
FLAGS = -g -Wno-deprecated -DGL_GLEXT_PROTOTYPES

UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LIBS =  -lGL -lGLU -lglut -DGL_GLEXT_PROTOTYPES
endif


all: client

client: $(OBJS)
ifeq ($(UNAME), Darwin)
	g++ -o $(EXECUTABLE) $(OBJS) $(FLAGS) $(FRAMEWORKS)
else ifeq ($(UNAME), Linux)
	g++ -o $(EXECUTABLE) $(OBJS) $(FLAGS) $(LIBS)
endif

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