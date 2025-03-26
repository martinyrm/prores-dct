#ifndef COMPUTE_H
#define COMPUTE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

class Compute {
    public:
    unsigned int id;
    unsigned int out_tex;

    Compute( const char* path, glm::uvec2 size ) {
        work_size = size;

        // read in shader code
        std::string compute_code;
        std::ifstream file;

        file.exceptions( std::ifstream::failbit | std::ifstream::badbit );

        try {
            file.open( path );
            std::stringstream file_stream;
            file_stream << file.rdbuf();
            file.close();

            compute_code = file_stream.str();
        } catch ( std::ifstream::failure e ) {
            std::cerr << "failed to read compute shader file" << std::endl;
        }

        const char* c_shader_code = compute_code.c_str();

        // compile shader
        unsigned int shader;

        shader = glCreateShader( GL_COMPUTE_SHADER );
        glShaderSource( shader, 1, &c_shader_code, NULL );
        glCompileShader( shader );
	
    	int success;
    	char infoLog[512];

	    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	
	    if (!success) {
		    glGetShaderInfoLog(shader, 512, NULL, infoLog);
		    std::cerr << "ERROR\n" << infoLog << std::endl;
	    }


        // create program
        id = glCreateProgram();
        glAttachShader( id, shader );
        glLinkProgram( id );
	
    	int success2;
	    glGetProgramiv(id, GL_LINK_STATUS, &success2);

	
	    char infoLog2[512];
	    if (!success2) {
		    glGetProgramInfoLog(id, 512, NULL, infoLog2);
		    std::cerr << "Error Linking problem \n" << infoLog << std::endl;
	    }
        // cleanup
        glDeleteShader( shader );

            }

    ~Compute() {
        glDeleteProgram( id );
    }

    void use() {
        glUseProgram( id );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, out_tex );
    }

    void dispatch() {
        glDispatchCompute( work_size.x, work_size.y, 1 );
    }

    void wait() {
        glMemoryBarrier( GL_ALL_BARRIER_BITS );
    }

    GLuint generate_and_store_ssbo(int* data) {
        GLuint ssbo;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(int), data, GL_DYNAMIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return ssbo;
    }


    void update_ssbo(int* data, GLuint ssbo) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 64 * sizeof(int), data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }


    std::vector<int> copy_ssbo_to_host(GLuint ssbo, int block_len) { 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        std::vector<int> ssbo_data(block_len);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 64 * sizeof(int), ssbo_data.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return ssbo_data;
    }

    void set_values( int16_t* values ) {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RED_INTEGER, work_size.x, work_size.y, 0, GL_RED_INTEGER, GL_INT, values );
    }

    std::vector<int> get_values() {
        unsigned int collection_size = work_size.x * work_size.y;
        std::vector<int> compute_data( collection_size );
        glGetTexImage( GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_INT, compute_data.data() );

        return compute_data;
    }

private:
    glm::uvec2 work_size;
};

#endif
