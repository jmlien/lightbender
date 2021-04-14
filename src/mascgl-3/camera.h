//------------------------------------------------------------------------------
//  Copyright 2007-2014 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#pragma once

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

//
#include "model.h"

class Camera {

public:

    ////////////////////////////////////////////////////////////////////////////////
    //
    //  Initializer
    //
    ////////////////////////////////////////////////////////////////////////////////

  	void init(GLFWwindow * window);

    ////////////////////////////////////////////////////////////////////////////////
    //
    //  Access Methods
    //
    ////////////////////////////////////////////////////////////////////////////////

  	void setFOV(GLFWwindow * window, float fov);
	  float getFOV() const { return FoV; }

	  glm::mat4 getViewMatrix(){ return ViewMatrix; }
	  glm::mat4 getProjectionMatrix(){ return ProjectionMatrix; }

    void gliDisableMouse(){ m_DisalbeMouseControl=true; }
    void gliEndMouse(){ m_DisalbeMouseControl=true; }

	  const glm::vec3 getCameraPos() { return m_CameraPos; }
    void setCameraPosX(float x) { m_CameraPos.x=x; }
    void setCameraPosY(float y) { m_CameraPos.y=y; }
    void setCameraPosZ(float z) { m_CameraPos.z=z; }

    //angle_in_radians
    void setAzim(float angle) { m_currentAzim=angle; }
    void setElev(float angle) { m_currentElev=angle; }
    float getAzim() { return m_currentAzim; }
    float getElev() { return m_currentElev; }


  	//hand cursor moving event
  	void cursor_callback(GLFWwindow * window, double x, double y);

  	//handles mouse click
  	void mouse_callback(GLFWwindow * window, int button, int action, int mode);


private:

  	//update the camera view matrix
  	void update();

  	void cameraRotateX(float v[3], float degree);
  	void cameraRotateY(float v[3], float degree);

  	float FoV;

  	glm::mat4 ViewMatrix;

  	glm::mat4 ProjectionMatrix;

    bool m_DisalbeMouseControl;

  	glm::vec3 m_CameraPos;
  	glm::vec3 m_deltaDis;

  	float m_currentAzim, m_deltaAzim; //angle_in_radians
  	float m_currentElev, m_deltaElev;

  	bool  pressed;

  	float m_StartX, m_StartY;
};
