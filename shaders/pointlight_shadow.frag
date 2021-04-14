#version 330 core

in vec4 diffuse, ambient;
in vec3 normal,lightDir,halfVector;
in float dist;
in vec4 ShadowCoord;
in vec2 uv;

out vec4 fragColor;

//material and lighting information...
struct Material
{
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;
};

struct SpotLight
{
	vec3 pos;

	vec4 diffuse;
	vec4 specular;
	vec4 ambient;

	float att_const;
	float att_linear;
};

uniform Material material;
uniform SpotLight light;

//texture maps
uniform sampler2D color_texture;
uniform sampler2DShadow shadowMap;

vec2 poissonDisk[16] = vec2[]
( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float checkVisibility(vec4 shadowcoord)
{

	float visibility=1.0;

	//determine the shadow uv coordinate (suv)
	vec3 proj = shadowcoord.xyz / shadowcoord.w;
	vec2 uv;
    uv.x = 0.5 * proj.x + 0.5;
    uv.y = 0.5 * proj.y + 0.5;
    float z = 0.5 * proj.z + 0.5;
	vec3 suv=vec3(uv,z);

	//varibles for softer shadow...
	float min_vis=0.5;
	int samplesize=10;
	float decrement=(1.0-min_vis)/samplesize;

	// Fixed bias, or...
	float bias = 0.005;


	// Sample the shadow map 4 times
	for (int i=0;i<samplesize;i++)
	{
		// use either :
		//  - Always the same samples.
		//    Gives a fixed pattern in the shadow, but no noise
		int index = i;
		//  - A random sample, based on the pixel's screen location. 
		//    No banding, but the shadow moves with the camera, which looks weird.
		// int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
		//  - A random sample, based on the pixel's position in world space.
		//    The position is rounded to the millimeter to avoid too much aliasing
		//int index = int(16.0*random(floor(Position_worldspace.xyz*1000.0), i))%16;
		
		suv=vec3(suv.xy + poissonDisk[index]/400.0,  (suv.z-bias));

		// texture returns zero if the comparison between the texturer depth and suv.z fails (i.e., texturer depth>suv.z)
		// when this happens, this pixel is in the shadow...
		// the function returns one if the comparison succeeds
		float sh=texture(shadowMap, suv);

		//reduce visibility
		visibility -= decrement*(1.0-sh);

		if(visibility<min_vis)
		{
			return min_vis;
		}
	}

	return visibility;
}


void main()
{

	float att = 1.0/ (light.att_const + light.att_linear * dist);

	vec4 color = ambient+material.emission; 

	vec3 n = normalize(normal);
	
	vec3 l = normalize(lightDir);

	//compute the dot product between normal and ldir 
	float NdotL = max(dot(n,l),0.0);

	if (NdotL > 0.0) 
	{
		//diffuse
		color += vec4(diffuse.rgb * NdotL,diffuse.a);
		
		//specular
		vec3 halfV = normalize(halfVector);
		float NdotHV = max(dot(n,halfV),0.0);
		vec4 specular_color = (material.specular * light.specular);
		specular_color =  vec4( specular_color.rgb * pow(NdotHV,material.shininess), specular_color.a) ;
		color += vec4( specular_color.rgb * pow(NdotHV,material.shininess), specular_color.a) ;
	}
	

	float visibility=checkVisibility(ShadowCoord);
	color = vec4(color.rgb*att*visibility,color.a);

	//gl_FragColor=color*texture(color_texture,uv);
	//gl_FragColor=color;
    fragColor=color*texture(color_texture,uv);    
}


