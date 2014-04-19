#include <string>
// #include "freetype-gl/freetype-gl.h"
// #include "freetype-gl/demo-font.c"
#include <FTGL/ftgl.h>

class Text {
public:
	Text() {}
	void draw(int pts, int bcount, int fps) {
     // ::vec2 pen = {10,10};
     // ::vec4 color = {0,0,0,1};

     // add_text(buffer, font, _text, &color, &pen);

	  FTGLPixmapFont font("/usr/share/fonts/truetype/freefont/FreeMono.ttf");
	  font.FaceSize(72);
	  char buff[100];
	  sprintf(buff, "Pts: %d. Bunnies: %d FPS: %d", pts, bcount, fps);
     font.Render(buff);
  	}	
	void setText(char * text) { _text = text; }

private:
	unsigned int _sentence; // I removed = 0 RAYMOND
	char *_text;

	static bool _init;
	// static texture_atlas_t *atlas;
	// static texture_font_t *font;
	// static vertex_buffer_t *buffer; 

	static void init() {
    // atlas = texture_atlas_new(512, 512, 1);
    // font = texture_font_new_from_file(atlas, 16, "resources/VeraMono.ttf");

    // buffer= vertex_buffer_new("v3i:t2f:c4f");
    // texture_font_load_glyphs(font, L" !\"#$%&'()*+,-./0123456789:;<=>?"
    //                                  L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    //                                  L"`abcdefghijklmnopqrstuvwxyz{|}~");
    // _init = true;

	  _init = true;
	}
};

bool Text::_init = false;

