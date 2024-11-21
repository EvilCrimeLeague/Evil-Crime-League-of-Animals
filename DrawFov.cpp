#include "read_write_chunk.hpp"

#include <glm/glm.hpp>

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <cstddef>


void ChangeFovBuffer(std::vector< Vertex > data, GLuint buffer) {
    this.data = data;

	GLuint total = 0;

	static_assert(sizeof(Vertex) == 3*4+3*4+4*1+2*4, "Vertex is packed.");
	// std::vector< Vertex > data;

	//read + upload data chunk:
    //upload data:
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // GL_DYNAMIC_DRAW
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vertex), data.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // total = GLuint(data.size()); //store total for later checks on index

    //store attrib locations:
    // Position = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Position));
    // Normal = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Normal));
    // Color = Attrib(4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, Color));
    // TexCoord = Attrib(2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, TexCoord));

	/* //DEBUG:
	std::cout << "File '" << filename << "' contained meshes";
	for (auto const &m : meshes) {
		if (&m.second == &meshes.rbegin()->second && meshes.size() > 1) std::cout << " and";
		std::cout << " '" << m.first << "'";
		if (&m.second != &meshes.rbegin()->second) std::cout << ",";
	}
	std::cout << std::endl;
	*/
}

GLuint CreateFovBuffer(std::vector< Vertex > data) {
    GLuint handle;
    glGenBuffer(1, &handle);
    
    ChangeFovBuffer(data, handle);

    return handle;
}