#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct TextTextureProgram {
	TextTextureProgram();
	~TextTextureProgram();

	GLuint program = 0;

	GLuint VAO; //an empty VAO

};

extern Load< TextTextureProgram > text_texture_program;

//For convenient scene-graph setup, copy this object:
// NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
// extern Scene::Drawable::Pipeline lit_color_texture_program_pipeline;
