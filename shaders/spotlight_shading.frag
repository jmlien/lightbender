//
// TODO: Implement spotlight shading, with diffusion and specular lights
//

varying vec4 diffuse,ambientGlobal, ambient;
varying vec3 normal,lightDir,halfVector;
varying float dist;

void main()
{
	vec3 n,halfV;
	float NdotL,NdotHV;

	float att = 0.1 / (gl_LightSource[0].constantAttenuation +
			           gl_LightSource[0].linearAttenuation * dist +
		               gl_LightSource[0].quadraticAttenuation * dist * dist);		

	vec4 color = att * ambient;

	//vec4 color = vec4(0); //ambientGlobal;
	
	float spotEffect;
	int get_light=0;

	/* a fragment shader can't write a verying variable, hence we need
	   a new variable to store the normalized interpolated normal */

	n = normalize(normal);
	
	/* compute the dot product between normal and ldir */
	NdotL = max(dot(n,normalize(lightDir)),0.0);


	if (NdotL > 0.0) 
	{
		spotEffect = dot(normalize(gl_LightSource[0].spotDirection), normalize(-lightDir));
		
		float threshold = cos(gl_LightSource[0].spotCosCutoff); //spotCosCutoff is in radian

		if (spotEffect > threshold ) 
		{
			spotEffect = pow(spotEffect, gl_LightSource[0].spotExponent);
			att = spotEffect / (gl_LightSource[0].constantAttenuation +
			                    gl_LightSource[0].linearAttenuation * dist +
		                        gl_LightSource[0].quadraticAttenuation * dist * dist);


			color += att * (diffuse * NdotL + ambient);

			halfV = normalize(halfVector);
			NdotHV = max(dot(n,halfV),0.0);
			color += att * gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
		}
	}

	gl_FragColor = color;
}