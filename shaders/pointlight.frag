#version 330 core

in vec4 diffuse,ambientGlobal, ambient;
in vec3 normal,lightDir,halfVector;
in float dist;

void main()
{

	float att = 1.0/ (gl_LightSource[0].constantAttenuation +
			           gl_LightSource[0].linearAttenuation * dist +
		               gl_LightSource[0].quadraticAttenuation * dist * dist);

	vec4 color = ambient; 

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
		vec4 specular_color = (gl_FrontMaterial.specular * gl_LightSource[0].specular);
		specular_color =  vec4( specular_color.rgb * pow(NdotHV,gl_FrontMaterial.shininess), specular_color.a) ;
		color += vec4( specular_color.rgb * pow(NdotHV,gl_FrontMaterial.shininess), specular_color.a) ;

		color = vec4(color.rgb*att,color.a);
	}
	

	gl_FragColor = color;
}