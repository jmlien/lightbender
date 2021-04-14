#pragma once

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


#include <string>
using namespace std;


#include "soil/SOIL.h"

inline bool checkglError()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR) return true;

	switch (error)
	{
	case GL_INVALID_ENUM: cerr << "! Error: GL_INVALID_ENUM " << endl; break;
	case GL_INVALID_VALUE: cerr << "! Error: GL_INVALID_VALUE " << endl; break;
	case GL_INVALID_OPERATION: cerr << "! Error: GL_INVALID_OPERATION " << endl; break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: cerr << "! Error: GL_INVALID_FRAMEBUFFER_OPERATION " << endl; break;
	case GL_OUT_OF_MEMORY: cerr << "! Error: GL_OUT_OF_MEMORY " << endl; break;
	case GL_STACK_UNDERFLOW: cerr << "! Error: GL_STACK_UNDERFLOW " << endl; break;
	case GL_STACK_OVERFLOW: cerr << "! Error: GL_STACK_OVERFLOW " << endl; break;
	}

	return false;
}

class Texture
{
public:

	Texture()
	{
		texture_id=-1;
		current_unit = 0;
		width = 0;
		height = 0;
		format = GL_RGB;
	}

	bool loadTexture(const char * filename)
	{
		unsigned int flag=SOIL_FLAG_POWER_OF_TWO
			| SOIL_FLAG_MIPMAPS
			//| SOIL_FLAG_COMPRESS_TO_DXT
			| SOIL_FLAG_TEXTURE_REPEATS
			//| SOIL_FLAG_INVERT_Y
			| SOIL_FLAG_DDS_LOAD_DIRECT;

		texture_id=SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, flag);

		if (texture_id == 0) return false;

		return checkglError();
	}

	bool loadTexture(const char * filename, int width, int height)
	{
		unsigned char * data;
		FILE * file;

		//The following code will read in our RAW file
		file = fopen(filename, "rb");

		if (file == NULL) return false;
		data = (unsigned char *)malloc(width * height * 3);
		assert(data);

		fread(data, width * height * 3, 1, file);

		fclose(file);

		glGenTextures(1, &texture_id); //generate the texture with the loaded data
		glBindTexture(GL_TEXTURE_2D, texture_id); //bind the texture to it��s array

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //set texture environment parameters

		//And if you go and use extensions, you can use Anisotropic filtering textures which are of an
		//even better quality, but this will do for now.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Here we are setting the parameter to repeat the texture instead of clamping the texture
		//to the edge of our shape.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Generate the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		free(data); //free the texture

		//
		this->width = width;
		this->height = height;
		this->format = GL_RGB;
		//

		return checkglError();
	}

	bool createTexture(GLenum format, int width, int height, unsigned char * data)
	{
		glGenTextures(1, &texture_id); //generate the texture with the loaded data
		glBindTexture(GL_TEXTURE_2D, texture_id); //bind the texture to it��s array

		//And if you go and use extensions, you can use Anisotropic filtering textures which are of an
		//even better quality, but this will do for now.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Here we are setting the parameter to repeat the texture instead of clamping the texture
		//to the edge of our shape.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Generate the texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		//
		this->width = width;
		this->height = height;
		this->format = format;
		//

		return checkglError();
	}

	bool createDepthTexture(int width, int height, float * data)
	{
		glGenTextures(1, &texture_id); //generate the texture with the loaded data
		glBindTexture(GL_TEXTURE_2D, texture_id); //bind the texture to it��s array

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

		//Generate the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);

		//
		this->width = width;
		this->height = height;
		this->format = GL_DEPTH_COMPONENT;
		//

		return checkglError();
	}


	void freeTexture(GLuint texture)
	{
		glDeleteTextures(1, &texture);
	}

	void bind(int program, const string& sampler_name, GLenum unit)
	{
		glActiveTexture(unit);
		texture_location = glGetUniformLocation(program, sampler_name.c_str());
		glUniform1i(texture_location, unit - GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		current_unit = unit;
	}

	void unbind()
	{
		glActiveTexture(current_unit);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(texture_location, 0);
	}

	GLuint getTextureID() const { return texture_id; }

	void save2file(const string& filename)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		if (format == GL_DEPTH_COMPONENT) //depth...
		{
			float * img = new float[width*height];
			assert(img);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, img);
			save2file(filename,img);
		}
		else //color... (only works for RGB...)
		{
			unsigned char * img = new unsigned char[width*height*3];
			assert(img);
			glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, img);
			save2file(filename, img);
		}
	}

private:

	//some helper functions
	inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }

	inline int toInt(double x){ return int(clamp(x) * 255 + .5); }

	//save rendered image to file
	void save2file(const std::string& filename, unsigned char * img)
	{
		FILE *f = fopen(filename.c_str(), "w");         // Write image to PPM file.

		fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int id = ((height - i - 1)*width + j) * 3;
				fprintf(f, "%d %d %d ", (int)img[id], (int)img[id + 1], (int)img[id + 2]);
			}

		}
		fclose(f);
	}

	//save depth image to file
	void save2file(const std::string& filename, float * img)
	{
		FILE *f = fopen(filename.c_str(), "w");         // Write image to PPM file.

		fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int id = ((height - i - 1)*width + j);
				int depth = toInt(img[id]);
				fprintf(f, "%d %d %d ", depth, depth, depth);
			}

		}
		fclose(f);
	}
	//

	int current_unit;
	int texture_location;
	GLuint texture_id;

	int width, height;
	GLenum format;
};
