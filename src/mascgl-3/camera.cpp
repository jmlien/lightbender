#include "camera.h"



// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <math.h>

using namespace std;
//const float PI = 3.1415926f;

void Camera::init(GLFWwindow * window)
{
	m_DisalbeMouseControl = false;
	m_currentAzim = 0;
	m_deltaAzim = 0;
	m_currentElev = 0;
	m_deltaElev = 0;
	pressed = false;
	m_StartX=m_StartY=0;
	setFOV(window, 45.0f);
	update();
}

void Camera::update()
{
	//mouse control is disabled
	if (m_DisalbeMouseControl) return;

	glm::vec3 pos = m_CameraPos + m_deltaDis;
	float elev = m_currentElev + m_deltaElev;
	float azim = m_currentAzim + m_deltaAzim;

	// Camera matrix
	glm::mat4 AzimM, ElevM, TransM;
	TransM=glm::translate(TransM, -pos);
	AzimM=glm::rotate(AzimM, azim, glm::vec3(0.0, 1.0, 0.0));
	ElevM= glm::rotate(ElevM, elev, glm::vec3(1.0, 0.0, 0.0));
	ViewMatrix = TransM*ElevM*AzimM;

	////compute window x, y dir
	//float m_WindowX[3], m_WindowY[3];
	//m_WindowX[0] = 1; m_WindowX[1] = 0; m_WindowX[2] = 0;
	//m_WindowY[0] = 0; m_WindowY[1] = 1; m_WindowY[2] = 0;
	//cameraRotateX(m_WindowX, m_currentElev + m_deltaElev);
	//cameraRotateY(m_WindowX, m_currentAzim + m_deltaAzim);
	//m_WindowX[2] = -m_WindowX[2];
	//cameraRotateX(m_WindowY, m_currentElev + m_deltaElev);
	//cameraRotateY(m_WindowY, m_currentAzim + m_deltaAzim);
	//m_WindowY[2] = -m_WindowY[2];
}

//hand cursor moving event
void Camera::cursor_callback(GLFWwindow * window, double x, double y)
{
	if (!m_DisalbeMouseControl)
	{

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) //right button pressed
		{
			m_deltaDis[2] = (GLfloat)((m_CameraPos[2]>10) ? m_CameraPos[2] : 10) * ((GLfloat)(y - m_StartY)) / 100.0f;
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) //middle button pressed
		{
			m_deltaDis[0] = -(GLfloat)((m_CameraPos[0]>5) ? m_CameraPos[0] : 5) * ((GLfloat)(x - m_StartX)) / 20.0f;
			m_deltaDis[1] =  (GLfloat)((m_CameraPos[1]>5) ? m_CameraPos[1] : 5) * ((GLfloat)(y - m_StartY)) / 20.0f;
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) //left button pressed
		{
			m_deltaAzim = ((GLfloat)(x - m_StartX)) / 500.0f;
			m_deltaElev = ((GLfloat)(y - m_StartY)) / 500.0f;
		}
	}

	update();
}

//handles mouse click
void Camera::mouse_callback(GLFWwindow * window, int button, int action, int mode)
{

	//if (mode != GLFW_MOD_CONTROL)
	{
		if (action == GLFW_RELEASE) //reset every thing
		{
			for (int iD = 0; iD<3; iD++){
				m_CameraPos[iD] += m_deltaDis[iD];
				m_deltaDis[iD] = 0;
			}

			m_currentElev += m_deltaElev;
			m_currentAzim += m_deltaAzim;

			m_deltaElev = 0.0;
			m_deltaAzim = 0.0;
		}
		else { //press down
			// Get mouse position
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			m_StartY = (float)ypos;
			m_StartX = (float)xpos;
		}
	}
}

void Camera::setFOV(GLFWwindow * window, float fov)
{
	FoV = fov;
	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	ProjectionMatrix = glm::perspective(FoV, width*1.0f/height, 0.1f, 10000.0f);
}


void Camera::cameraRotateX(float v[3], float degree)
{
	float c = (float)cos(PI*degree / 180);
	float s = (float)sin(PI*degree / 180);
	float v1 = v[1] * c - v[2] * s;
	float v2 = v[1] * s + v[2] * c;
	v[1] = v1; v[2] = v2;
}

void Camera::cameraRotateY(float v[3], float degree)
{
	float c = (float)cos(PI*degree / 180);
	float s = (float)sin(PI*degree / 180);
	float v0 = v[0] * c + v[2] * s;
	float v2 = -v[0] * s + v[2] * c;
	v[0] = v0; v[2] = v2;
}
