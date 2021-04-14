#ifndef __SHADER_H
#define __SHADER_H

#include <string>
#include <map>
#include <climits>
using namespace std;

#if ( (defined(__MACH__)) && (defined(__APPLE__)) )   
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#endif

class Shader 
{
public:
	Shader();
	Shader(const char *vsFile, const char *fsFile);
	~Shader();
	
    void init(const char *vsFile, const char *fsFile);
    
	void bind();
	void unbind();
	
	unsigned int id();
	
	//shader varibles
	unsigned int addvarible(const string& name) 
	{ 
		unsigned int value = glGetUniformLocation(program_id, name.c_str());;
		shader_variables[name] = value; 
		return value;
	}

	void setvarible(const string& name, unsigned int value) { shader_variables[name]=value;  }

	//find the value associated with the name
	//when no given name is found, return UINT_MAX
	unsigned int value(const string& name)
	{ 
		if (shader_variables.find(name) == shader_variables.end()) return UINT_MAX;
		return shader_variables[name]; 
	}

	//check shader/program status
	void validateShader(GLuint shader, const char* file=NULL);
	void validateLinking();
	void validateProgram();

private:

	unsigned int program_id; //program id
	unsigned int shader_vp;
	unsigned int shader_fp;
	map<string, unsigned int> shader_variables;
};

#endif
