#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;


out vec4 diffuse,ambientGlobal, ambient;
out vec3 normal,lightDir,halfVector;
out float dist;
out vec3 fragmentcolor;

//Model, view, projection matrices
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat3 MV3x3;

void main()
{	
	// first transform the normal into camera space and normalize the result
	normal = normalize(MV3x3 * vertexNormal_modelspace);
	
	// now normalize the light's direction. Note that according to the
	// OpenGL specification, the light is stored in eye space.
	gl_Position = MVP * vec4(vertexPosition_modelspace,1);
	vec3 vertexPosition_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;

	//light
    vec3 light0_camerapace = (V* vec4(gl_LightSource[0].position.xyz,1) ).xyz;
	vec3 L_cameraspace= light0_camerapace-vertexPosition_cameraspace;
	lightDir = normalize(L_cameraspace);

	
	// compute the distance to the light source to a varying variable
	dist = length(L_cameraspace);

	// Normalize the halfVector to pass it to the fragment shader 
	{
			
		// compute eye vector and normalize it 
		vec3 eye = normalize(-vertexPosition_cameraspace);

		// compute the half vector
		halfVector = normalize(lightDir + eye);
	}

	// Compute the diffuse, ambient and globalAmbient terms
	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	ambientGlobal = gl_LightModel.ambient * gl_FrontMaterial.ambient;
} 
