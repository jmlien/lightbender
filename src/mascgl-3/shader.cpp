
//
// Code modified from Swiftless tutorials
// http://www.swiftless.com/glsltuts.html
//

#include "shader.h"
#include <string.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

static char* textFileRead(const char *fileName) 
{
	char* text=NULL;
    
	if (fileName != NULL) 
	{
        FILE *file = fopen(fileName, "rt");
        
		if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);
            
			if (count > 0) 
			{
				text = (char*)malloc(sizeof(char) * (count + 1));
				count = fread(text, sizeof(char), count, file);
				text[count] = '\0';
			}
			fclose(file);
		}
	}

	return text;
}


Shader::Shader() 
{
    this->program_id = 0;
    this->shader_fp = 0;
    this->shader_vp = 0;
}

Shader::Shader(const char *vsFile, const char *fsFile) 
{
    init(vsFile, fsFile);
}

void Shader::init(const char *vsFile, const char *fsFile) 
{
	const char* vsText = textFileRead(vsFile);
	const char* fsText = textFileRead(fsFile);	
   
	if (vsText == NULL) 
	{
		cerr << "! Error: Vertex shader file ("<<vsFile<<") not found" << endl;
		return;
	}

    if (fsText == NULL) 
	{      
		cerr << "! Error: Fragment shader file (" << fsFile << ") not found" << endl;
        return;
    }
    
    //cout<<"[Shader::init]"<<endl;
    //cout<<"vs = "<<vsFile<<endl;
    //cout<<"fs = "<<fsFile<<endl;

    shader_vp = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_vp, 1, &vsText, 0);
	glCompileShader(shader_vp);
	validateShader(shader_vp, vsFile);

	shader_fp = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_fp, 1, &fsText, 0);
	glCompileShader(shader_fp);
	validateShader(shader_fp, fsFile);
    
	program_id = glCreateProgram();
	glAttachShader(program_id, shader_vp);
	glAttachShader(program_id, shader_fp);
	glLinkProgram(program_id);
	validateLinking();
}

Shader::~Shader() 
{
	glDetachShader(program_id, shader_fp);
	glDetachShader(program_id, shader_vp);
    
	glDeleteShader(shader_fp);
	glDeleteShader(shader_vp);
	glDeleteProgram(program_id);
}

unsigned int Shader::id() 
{
	return program_id;
}

void Shader::bind() 
{
	glUseProgram(program_id);
}

void Shader::unbind() 
{
	glUseProgram(0);
}


void Shader::validateShader(GLuint shader, const char* file)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint InfoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> ErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(shader, InfoLogLength, NULL, &ErrorMessage[0]);
		if (InfoLogLength > 0) {
			cerr << "! Error: Shader " << shader << " (" << (file ? file : "") << ") compile error: " << &ErrorMessage[0] << flush;
		}
	}//end 
}

void Shader::validateLinking()
{
	// Check the program
	GLint status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) //something is wrong...
	{
		GLint InfoLogLength;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0){
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program_id, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			cerr << "! Error: Program " << program_id << " link error: " << &ProgramErrorMessage[0] << flush;
		}
	}
}

void Shader::validateProgram()
{
	glValidateProgram(program_id);

	GLint status;
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint InfoLogLength;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0){
			vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program_id, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			cerr << "! Error: Program " << program_id << " failed to validate shader" << &ProgramErrorMessage[0] << endl;
		}
	}
}
