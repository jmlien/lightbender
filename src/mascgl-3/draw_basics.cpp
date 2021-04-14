#include "draw_basics.h"

#define DEBUG 0

Camera camera;
Shader shader;

//-----------------------------------------------------------------------------
//
// GL/GUI initialization and setup
//
//-----------------------------------------------------------------------------

GLFWwindow*  initGLcontext(mascgl_workspace& workspace, string title, GLFWerrorfun errfun, GLFWkeyfun keyfun, GLFWmousebuttonfun mousefun, GLFWcursorposfun cursorfun)
{
	glfwSetErrorCallback(errfun);// error_callback);

	// Initialise GLFW
	if (!glfwInit())
	{
		cerr << "! Error: Failed to initialize GLFW" << endl;
		return NULL;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef _WIN32
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//
	// Open a window and create its OpenGL context
	//
	title += ": ";
	title += workspace.env_filename;
	GLFWwindow* window = glfwCreateWindow(workspace.image_w, workspace.image_h, title.c_str(), NULL, NULL);

	if (window == NULL){
		cerr << "! Error: Failed to open a GLFW window" << endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	//
	// Initialize GLEW
	//
	glewExperimental = true; // Needed for core profile
#if ( (defined(__MACH__)) && (defined(__APPLE__)) )
	//nothing
	glewInit();
#else
	if (glewInit() != GLEW_OK)
	{
		cerr << "! Error: Failed to initialize GLEW" << endl;
		return NULL;
	}
#endif

	//
	// set call back functions...
	//
	glfwSetKeyCallback(window, keyfun);
	glfwSetMouseButtonCallback(window, mousefun);
	glfwSetCursorPosCallback(window, cursorfun);

	//set camera position
	camera.setCameraPosX((float)workspace.COM[0]);
	camera.setCameraPosY((float)workspace.COM[1]);
	camera.setCameraPosZ(workspace.R*2.1f);
	camera.init(window);


	//tell workspace to create basic textures...
	workspace.createDefaultTextures();

	//load textures from files
	for (list<object3D*>::iterator i = workspace.models.begin(); i != workspace.models.end(); i++)
	{
		object3D * obj = *i;

		//load textures
		if (!obj->color_texture_filename.empty())
		{
			obj->color_texture = new Texture();
			if (obj->color_texture->loadTexture(obj->color_texture_filename.c_str()) == false)
			{
				cerr << "! Error: Failed to load texture:" << obj->color_texture_filename << endl;
				return nullptr;
			}
		}

		if (!obj->normalmap_texture_filename.empty())
		{
			obj->normalmap_texture = new Texture();
			if (obj->normalmap_texture->loadTexture(obj->normalmap_texture_filename.c_str()) == false)
			{
				cerr << "! Error: Failed to load texture:" << obj->normalmap_texture_filename << endl;
				return nullptr;
			}
		}
	}//end for i

	return window;
}

void setupLight(mascgl_workspace& workspace, Shader& shader)
{
	//Let's have light!
	for (list<light*>::iterator i = workspace.lights.begin(); i != workspace.lights.end(); i++)
	{
		light* li = *i;
		float pos[] = { (float)li->pos[0], (float)li->pos[1], (float)li->pos[2], 1.0f };
		float diffuse[] = { (float)li->mat_color[0], (float)li->mat_color[1], (float)li->mat_color[2], 1.0f };
		float specular[] = { (float)li->mat_specular[0], (float)li->mat_specular[1], (float)li->mat_specular[2], 1.0f };
		float ambient[] = { (float)li->ambient[0], (float)li->ambient[1], (float)li->ambient[2], 1.0f };

		glUniform3fv(shader.value("light.pos"), 1, pos);
		glUniform4fv(shader.value("light.diffuse"), 1, diffuse);
		glUniform4fv(shader.value("light.specular"), 1, specular);
		glUniform4fv(shader.value("light.ambient"), 1, ambient);

		glUniform1f(shader.value("light.att_const"), (float)li->att_const);
		glUniform1f(shader.value("light.att_linear"), (float)li->att_linear);

		//only the first light will be used...
		break;
	}
	//--------------------------------------------------
}

void setupGLflags()
{
	// transparent
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//glEnable(GL_NORMALIZE);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_CULL_FACE);
}

//-----------------------------------------------------------------------------
//
// functions for rendering depth map
//
//-----------------------------------------------------------------------------


//create view/projection matrix from the given light
void createVPfromLight(mascgl_workspace& workspace, light * mylight, glm::mat4& depthProjectionMatrix, glm::mat4& depthViewMatrix)
{
	//setup model, view, projection matrix for light space
	const Point3d& lp = mylight->pos; //light position
	const Point3d& lookat = mylight->lookat; //look at position
	float dist = Vector3d(lp - lookat).norm();
	float dim = workspace.R;

	//mathtool::Point3d lp = mylight->pos; //light position
	//const mathtool::Point3d& lookat = mylight->lookat; //look at position
	//Vector3d viewdir = (mylight->pos - lookat).normalize()*workspace.R;
	//lp = lookat + viewdir;
	//float dist = viewdir.norm();
	//float dim = workspace.R;

	if (mylight->type == light::SPOT_LIGHT)
		depthProjectionMatrix = glm::perspective(45.0, 1.0, mylight->znear, mylight->zfar);
	if (mylight->type == light::POINT_LIGHT)
		depthProjectionMatrix = glm::perspective(45.0, 1.0, mylight->znear, mylight->zfar);
	else
		depthProjectionMatrix = glm::ortho<float>(-dim, dim, -dim, dim, 0, dist + dim * 2);

	depthViewMatrix = glm::lookAt(toglm(lp), toglm(lookat), glm::vec3(0, 1, 0));
}

//create view/projection matrix from the given light
void createVPfromLight(mascgl_workspace& workspace, light * mylight, glm::mat4& depthVP)
{
	glm::mat4 depthProjectionMatrix, depthViewMatrix;
	createVPfromLight(workspace, mylight, depthProjectionMatrix, depthViewMatrix);
	depthVP = depthProjectionMatrix * depthViewMatrix;
}

GLuint renderDepth(mascgl_workspace& workspace, glm::mat4& depthVP, vector<M_buffers>& buffers)
{
	// ---------------------------------------------
	// Render to Texture
	// ---------------------------------------------
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader

	//this is for depth
	GLuint depthTexture = 0;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, workspace.image_w, workspace.image_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);


	//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	//for depth
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
		// No color output in the bound framebuffer, only depth.
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "! Error: glCheckFramebufferStatus failed" << endl;
		exit(-1);
	}

	//------------------------------------------------
	//depth shader
	Shader depth_shader;
	depth_shader.init("shaders/renderDepth.vert", "shaders/renderDepth.frag");
	depth_shader.addvarible("depthMVP");
	//------------------------------------------------

	//-------------------------------------------------------------------------------------
	// Render to our framebuffer
	//-------------------------------------------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, workspace.image_w, workspace.image_h);
	// Render on the whole framebuffer, complete from the lower left corner to the upper right

	// We don't use bias in the shader, but instead we draw back faces, 
	// which are already separated from the front faces by a small distance 
	// (if your geometry is made this way)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
	//-------------------------------------------------------------------------------------

	//
	//
	// render depth...
	//
	//

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	depth_shader.bind();


	//
	//draw meshes
	//
	for (vector<M_buffers>::iterator i = buffers.begin(); i != buffers.end(); i++)
	{
		if (i->m == NULL) continue;

		//bind texture here...
		model & M = *(i->m);

		//compute depthMVP from depthVP
		glm::mat4 depthMVP = depthVP*toglm(M.getCurrentTransform());

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(depth_shader.value("depthMVP"), 1, GL_FALSE, &depthMVP[0][0]);

		//
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, i->vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i->trielementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,         // mode
			M.t_size * 3,         // count
			GL_UNSIGNED_INT,      // type
			(void*)0              // element array buffer offset
			);

		glDisableVertexAttribArray(0);
	}

	//TODO: we should draw spheres here as well...

	//
	depth_shader.unbind();


	//SAVED IMAGE TO FILE
	//this is for depth
#if DEBUG
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);

		float * img = new float[workspace.image_w*workspace.image_h];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, img);
		char id[1024];
		sprintf(id, "%0d", (int)time(NULL));
		string filename = "depth_";
		filename = filename + id + ".ppm";
		save2file(filename.c_str(), workspace.image_w, workspace.image_h, img);
		delete[] img;
	}
#endif //DEBUG

	//reset back...
	glDeleteFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, workspace.image_w, workspace.image_h);

	return depthTexture;
}

GLuint renderDepth(mascgl_workspace& workspace, glm::mat4& depthVP, M_buffers& buffer)
{
	if (buffer.m == NULL) return -1;

	// ---------------------------------------------
	// Render to Texture
	// ---------------------------------------------
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader

	//this is for depth
	GLuint depthTexture = 0;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, workspace.image_w, workspace.image_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);


	//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	//for depth
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
		// No color output in the bound framebuffer, only depth.
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "! Error: glCheckFramebufferStatus failed" << endl;
		exit(-1);
	}

	//------------------------------------------------
	//depth shader
	Shader depth_shader;
	depth_shader.init("shaders/renderDepth.vert", "shaders/renderDepth.frag");
	depth_shader.addvarible("depthMVP");
	//------------------------------------------------

	//-------------------------------------------------------------------------------------
	// Render to our framebuffer
	//-------------------------------------------------------------------------------------

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, workspace.image_w, workspace.image_h);
	// Render on the whole framebuffer, complete from the lower left corner to the upper right

	// We don't use bias in the shader, but instead we draw back faces, 
	// which are already separated from the front faces by a small distance 
	// (if your geometry is made this way)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
	//-------------------------------------------------------------------------------------

	//
	//
	// render depth...
	//
	//

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	depth_shader.bind();


	//
	//draw meshes
	//

	//bind texture here...
	model & M = *(buffer.m);


	//compute depthMVP from depthVP
	glm::mat4 depthMVP = depthVP*toglm(M.getCurrentTransform());

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(depth_shader.value("depthMVP"), 1, GL_FALSE, &depthMVP[0][0]);

	//
	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.trielementbuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,         // mode
		M.t_size * 3,         // count
		GL_UNSIGNED_INT,      // type
		(void*)0              // element array buffer offset
		);

	glDisableVertexAttribArray(0);

	//TODO: we should draw spheres here as well...

	//
	depth_shader.unbind();


	//SAVED IMAGE TO FILE
	//this is for depth
#if DEBUG
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);

		float * img = new float[workspace.image_w*workspace.image_h];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, img);
		char id[1024];
		sprintf(id, "%0d", (int)time(NULL));
		string filename = "depth_";
		filename = filename + id + ".ppm";
		save2file(filename.c_str(), workspace.image_w, workspace.image_h, img);

		filename = filename + id + ".dds";
		int save_result = SOIL_save_image(filename.c_str(), SOIL_SAVE_TYPE_DDS, workspace.image_w, workspace.image_h, 3, img);

		delete[] img;
	}
#endif

	//reset back...
	glDeleteFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, workspace.image_w, workspace.image_h);

	return depthTexture;
}






GLuint renderShadow(mascgl_workspace& workspace, glm::mat4& depthVP, GLuint depthTexture, vector<M_buffers>& buffers)
{
	//
	//prepare framebuffer for shadow rendering...
	// ---------------------------------------------
	// Render to Texture
	// ---------------------------------------------
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	//
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// shadow texture.
	// this is for color
	GLuint colorTexture;
	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, workspace.image_w, workspace.image_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//for color
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0);
	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// The depth buffer (this block of code is necessary...)
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, workspace.image_w, workspace.image_h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr << "! Error: glCheckFramebufferStatus failed" << endl;
		exit(-1);
	}

	//bind this frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, workspace.image_w, workspace.image_h);

	//------------------------------------------------
	//load shadow only shader
	Shader shadow_shader;
	shadow_shader.init("shaders/pointlight_shadow_only.vert", "shaders/pointlight_shadow_only.frag");
	//get uniform variables
	shadow_shader.addvarible("MVP");
	shadow_shader.addvarible("V");
	shadow_shader.addvarible("M");
	shadow_shader.addvarible("MV3x3");
	shadow_shader.addvarible("DepthBiasMVP");
	shadow_shader.addvarible("shadowMap");
	//------------------------------------------------

	//draw shadow
	//-----------------------------------------------------------------------
	//
	// draw to FramebufferName...
	// setup model, view, projection matrix
	//
	//-----------------------------------------------------------------------

	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shadow_shader.bind();

	//let's look at the scene from the light (but near the bbox)
	light * mylight = workspace.lights.front();
	mathtool::Point3d lp = mylight->pos; //light position
	const mathtool::Point3d& lookat = mylight->lookat; //look at position
	Vector3d viewdir = (mylight->pos - lookat).normalize()*workspace.R;
	lp = lookat + viewdir;
	float dist = viewdir.norm();
	float dim = workspace.R;
	glm::mat4 ProjectionMatrix = glm::ortho<float>(-dim, dim, -dim, dim, 0, dist + workspace.R * 2);
	glm::mat4 ViewMatrix = glm::lookAt(toglm(lp), toglm(lookat), glm::vec3(0, 1, 0));
	//glm::mat4 ProjectionMatrix, ViewMatrix;
	//createVPfromLight(mylight, ProjectionMatrix, ViewMatrix);

	//bind shadow texture...
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glUniform1i(shadow_shader.value("shadowMap"), 0);

	//here we draw walls only
	bool wallonly = true;
	bool texture = false;

	drawMesh(workspace, shadow_shader, ProjectionMatrix, ViewMatrix, depthVP, buffers, wallonly, texture);

	//done....
	shadow_shader.unbind();


	//unbind this frame buffer
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteRenderbuffers(1, &depthrenderbuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, workspace.image_w, workspace.image_h);

	return colorTexture;
}



//-----------------------------------------------------------------------------
//
// Load and render individual model and shpere
//
//-----------------------------------------------------------------------------



//load the given model M into GL buffers
M_buffers loadModelBuffers(model& M)
{
	M_buffers buffers;
	buffers.m = &M;

	//get positions of each vertex
	{
		std::vector<glm::vec3> indexed_vertices;
		indexed_vertices.reserve(M.v_size);
		for (unsigned int i = 0; i < M.v_size; i++)
		{
			vertex& v = M.vertices[i];
			glm::vec3 pos = toglm(v.p);
			indexed_vertices.push_back(pos);
		}

		glGenBuffers(1, &buffers.vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
	}

	//get normals of each vertex
	{
		std::vector<glm::vec3> indexed_normals;
		indexed_normals.reserve(M.v_size);
		for (unsigned int i = 0; i < M.v_size; i++)
		{
			vertex& v = M.vertices[i];
			glm::vec3 normal = toglm(v.n);
			indexed_normals.push_back(normal);
		}

		glGenBuffers(1, &buffers.normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
	}

	//get tangent of each vertex
	{
		std::vector<glm::vec3> tangents;
		tangents.reserve(M.v_size);
		for (unsigned int i = 0; i < M.v_size; i++)
		{
			vertex& v = M.vertices[i];
			glm::vec3 tangent = toglm(v.t);
			tangents.push_back(tangent);
		}

		glGenBuffers(1, &buffers.tangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.tangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);
	}

	//get bitangent of each vertex
	{
		std::vector<glm::vec3> bitangents;
		bitangents.reserve(M.v_size);
		for (unsigned int i = 0; i < M.v_size; i++)
		{
			vertex& v = M.vertices[i];
			glm::vec3 bitangent = toglm(v.b);
			bitangents.push_back(bitangent);
		}

		glGenBuffers(1, &buffers.bitangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.bitangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);
	}

	//get UV of each vertex
	{
		std::vector<glm::vec2> UVs;
		for (unsigned int i = 0; i < M.v_size; i++)
		{
			vertex& v = M.vertices[i];
			glm::vec2 uv(v.uv[0], v.uv[1]);
			UVs.push_back(uv);
		}

		glGenBuffers(1, &buffers.uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffers.uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
	}

	//
	// find indices for each triangle vertex
	//
	{
		//std::vector<uint> indices;
		std::vector<unsigned int> indices;
		int vindex = 0;
		for (unsigned int i = 0; i < M.t_size; i++)
		{
			triangle & tri = M.tris[i];

			for (short d = 0; d < 3; d++)
			{
				indices.push_back((unsigned int)tri.v[d]);
			}

		}//end for i

		// Generate a buffer for the indices as well
		glGenBuffers(1, &buffers.trielementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.trielementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	}

	return buffers;
}

void drawMesh(mascgl_workspace& workspace, M_buffers& buffer, Shader& shader, glm::mat4& projection, glm::mat4& view, glm::mat4& depthVP, bool wall_only, bool show_texture)
{
	if (buffer.m == NULL) return;

	unsigned int texture_location = shader.value("color_texture");
	if (texture_location != UINT_MAX)
	{
		glActiveTexture(GL_TEXTURE1);
		glUniform1i(texture_location, 1);
	}

	//draw meshes

	//bind texture here...
	model & M = *(buffer.m);

	//setup transforms
	glm::mat4 ModelMatrix = toglm(M.getCurrentTransform());
	glm::mat4 ModelViewMatrix = view * ModelMatrix;
	glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
	glm::mat4 MVP = projection * view * ModelMatrix;
	glm::mat4 depthMVP = depthVP * ModelMatrix;

	//matrix to map points into light space

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(shader.value("MVP"), 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(shader.value("M"), 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(shader.value("V"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix3fv(shader.value("MV3x3"), 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
	glUniformMatrix4fv(shader.value("DepthBiasMVP"), 1, GL_FALSE, &depthMVP[0][0]);

	//

	if (show_texture && texture_location != UINT_MAX)
	{
		if (M.color_texture != NULL)
		{
			//M.color_texture->bind(shader.id(), "color_texture", GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, M.color_texture->getTextureID());
		}
		else
		{
			//workspace.texture_white.bind(shader.id(), "color_texture", GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, workspace.texture_white.getTextureID());
		}
	}

	//check if matrial is defined....
	if (shader.value("material.diffuse") != UINT_MAX)
	{
		float diffuse[] = { (float)M.mat_color[0], (float)M.mat_color[1], (float)M.mat_color[2], 1.0f };
		float specular[] = { (float)M.mat_specular[0], (float)M.mat_specular[1], (float)M.mat_specular[2], 1.0f };
		float emission[] = { (float)M.mat_emission[0], (float)M.mat_emission[1], (float)M.mat_emission[2], 1.0f };
		glUniform4fv(shader.value("material.diffuse"), 1, diffuse);
		glUniform4fv(shader.value("material.specular"), 1, specular);
		glUniform4fv(shader.value("material.emission"), 1, emission);
		glUniform1f(shader.value("material.shininess"), M.mat_shininess);
	}

	shader.validateProgram();

	//
	// 1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 4th attribute buffer : tangents
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.tangentbuffer);
	glVertexAttribPointer(
		3,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 5th attribute buffer : bitangents
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.bitangentbuffer);
	glVertexAttribPointer(
		4,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);


	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.trielementbuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,             // mode
		M.t_size * 3,             // count
		GL_UNSIGNED_INT,          // type
		(void*)0                  // element array buffer offset
		);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);

}

void drawMesh(mascgl_workspace& workspace, Shader& shader, glm::mat4& projection, glm::mat4& view, glm::mat4& depthVP, vector<M_buffers>& buffers, bool wall_only, bool show_texture)
{
	unsigned int texture_location = shader.value("color_texture");
	if (texture_location != UINT_MAX)
	{
		glActiveTexture(GL_TEXTURE1);
		glUniform1i(texture_location, 1);
	}

	//draw meshes
	for (vector<M_buffers>::iterator i = buffers.begin(); i != buffers.end(); i++)
	{
		if (wall_only && workspace.is_wall(i->m) == false) continue; //not a wall and only wall should be rendered...
		//if (shadow_caster_only && i->m->cast_shadow == false && workspace.is_wall(i->m) == false) continue;//this model does not cast shadow, so ignore

		drawMesh(workspace, *i, shader, projection, view, depthVP, wall_only, show_texture);
	}
}

//-----------------------------------------------------------------------------
//
// Save texture or image to file
//
//-----------------------------------------------------------------------------

//save rendered image to file
void save2file(const std::string& filename, int w, int h, unsigned char * img)
{
	FILE *f = fopen(filename.c_str(), "w");         // Write image to PPM file.

	fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			int id = ((h - i - 1)*w + j) * 3;
			fprintf(f, "%d %d %d ", (int)img[id], (int)img[id + 1], (int)img[id + 2]);
		}

	}
	fclose(f);
}

//save depth image to file
void save2file(const std::string& filename, int w, int h, float * img)
{
	FILE *f = fopen(filename.c_str(), "w");         // Write image to PPM file.

	fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			int id = ((h - i - 1)*w + j);
			int depth = toInt(img[id]);
			fprintf(f, "%d %d %d ", depth, depth, depth);
		}
	}
	fclose(f);
}