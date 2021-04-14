
#pragma once
#ifdef _WIN32
#pragma warning( disable : 4244 )
#endif

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ws.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "shader.h"


//model buffer, storing a list of buffer handles
struct M_buffers
{
	M_buffers()
	{
		m = NULL;
		trielementbuffer = vertexbuffer = uvbuffer = tangentbuffer = bitangentbuffer = normalbuffer = -1;
	}

	void destory_buffers()
	{
		glDeleteBuffers(1, &trielementbuffer);
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &tangentbuffer);
		glDeleteBuffers(1, &bitangentbuffer);
		glDeleteBuffers(1, &normalbuffer);
		m = NULL;
	}

	model * m;
	uint trielementbuffer; //triangle buffer (index)
	uint vertexbuffer;     //vertex position buffer
	uint uvbuffer;         //vertex text coord buffer
	uint tangentbuffer;    //vertex tangent buffer
	uint bitangentbuffer;  //vertex bitangent buffer
	uint normalbuffer;     //vertex normal buffer
};


//-----------------------------------------------------------------------------
// Functions for saving texture to file
//-----------------------------------------------------------------------------
GLFWwindow*  initGLcontext(mascgl_workspace& workspace, string title,
	                       GLFWerrorfun errfun, GLFWkeyfun keyfun, GLFWmousebuttonfun mousefun, GLFWcursorposfun cursorfun);

void setupLight(mascgl_workspace& workspace, Shader& shader);
void setupGLflags();

//-----------------------------------------------------------------------------
//
// functions for rendering depth map
//
//-----------------------------------------------------------------------------

//create view/projection matrix from the given light
void createVPfromLight(mascgl_workspace& workspace, light * mylight, glm::mat4& depthProjectionMatrix, glm::mat4& depthViewMatrix);
//create view/projection matrix from the given light
void createVPfromLight(mascgl_workspace& workspace, light * mylight, glm::mat4& depthVP);
GLuint renderDepth(mascgl_workspace& workspace, glm::mat4& depthVP, vector<M_buffers>& buffers);
GLuint renderDepth(mascgl_workspace& workspace, glm::mat4& depthVP, M_buffers& buffer);
GLuint renderShadow(mascgl_workspace& workspace, glm::mat4& depthVP, GLuint depthTexture, vector<M_buffers>& buffers);

//-----------------------------------------------------------------------------
//
// Load and render individual model and shpere
//
//-----------------------------------------------------------------------------

//load the given model M into GL buffers
M_buffers loadModelBuffers(model& M);
void drawMesh(mascgl_workspace& workspace, M_buffers& buffer, Shader& shader, glm::mat4& projection, glm::mat4& view, glm::mat4& depthVP, bool wall_only, bool show_texture);
void drawMesh(mascgl_workspace& workspace, Shader& shader, glm::mat4& projection, glm::mat4& view, glm::mat4& depthVP, vector<M_buffers>& buffers, bool wall_only, bool show_texture);

//-----------------------------------------------------------------------------
// Functions for converting our own math structures to glm structures
//-----------------------------------------------------------------------------
inline glm::vec2 toglm(const Vector2d& v){ return glm::vec2(v[0], v[1]); }
inline glm::vec3 toglm(const Vector3d& v){ return glm::vec3(v[0], v[1], v[2]); }
inline glm::vec2 toglm(const Point2d& v) { return glm::vec2(v[0], v[1]); }
inline glm::vec3 toglm(const Point3d& v) { return glm::vec3(v[0], v[1], v[2]); }


//note: Matrix4x4 is row major and glm::mat4 is column major
inline glm::mat4 toglm(const Matrix4x4& m)
{
	return glm::mat4(glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]),
		glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]),
		glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]),
		glm::vec4(m[0][3], m[1][3], m[2][3], m[3][3]));
}

//note: Matrix3x3 is row major and glm::mat3 is column major
inline glm::mat3 toglm(const Matrix3x3& m)
{
	return glm::mat3(glm::vec3(m[0][0], m[1][0], m[2][0]), glm::vec3(m[0][1], m[1][1], m[2][1]), glm::vec3(m[0][2], m[1][2], m[2][2]));
}

//some helper functions
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }

inline int toInt(double x){ return int(clamp(x) * 255 + .5); }

//-----------------------------------------------------------------------------
// Functions for saving texture to file
//-----------------------------------------------------------------------------
//save depth image to file
void save2file(const std::string& filename, int w, int h, float * img);
//save rendered image to file
void save2file(const std::string& filename, int w, int h, unsigned char * img);




