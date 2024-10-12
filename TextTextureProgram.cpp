#include "TextTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"

Load< TextTextureProgram > text_texture_program(LoadTagEarly, []() -> TextTextureProgram const * {
	TextTextureProgram *ret = new TextTextureProgram();

	//----- build the pipeline template -----

	//make a 1-pixel white texture to bind by default:
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1,
        1,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        tex_data.data()
    );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	GL_ERRORS();     

	return ret;
});

// Reference: https://stackoverflow.com/questions/2588875/whats-the-best-way-to-draw-a-fullscreen-quad-in-opengl-3-2
TextTextureProgram::TextTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
R"(#version 330
out vec2 TexCoords; 
void main() {
	// vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
	// gl_Position = vec4(vertices[gl_VertexID],0,1);
	// TexCoords = 0.5 * gl_Position.xy + vec2(0.5);
	float x,y;

	x=-1.0+float(((gl_VertexID&1)<<2));
	y=-1.0+float(((gl_VertexID&2)<<1));
	
	TexCoords.x=(x+1.0)*0.5;
	TexCoords.y=(y+1.0)*0.5;
	gl_Position=vec4(x,y,0.0,1.0);
})"
	,
		//fragment shader:
R"(#version 330
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;

void main()
{    
    color = texture(text, TexCoords);
})"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Disable byte-alignment restriction 

	GLuint TEX_sampler2D = glGetUniformLocation(program, "text");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(TEX_sampler2D, 0); //set TEX to sample from GL_TEXTURE0

	glGenVertexArrays(1, &VAO); 
	glBindVertexArray(VAO); 

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

TextTextureProgram::~TextTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

