//
// TODO: Implement normal mapping. This should add on to the texture mapping shader: texturing.vert/frag
//

varying vec4 diffuse, ambient;
varying vec3 normal,lightDir,halfVector, tangent, bitangent;
varying float dist;
uniform sampler2D color_texture;  //color texture
uniform sampler2D norml_map; //normal map

vec3 newnormal(in vec2 uv)
{
	vec3 texel = texture2D(norml_map,uv).rgb;
	vec3 tsn =  normalize(texel*2.0-vec3(1)); //tangent space normal

	//tangent space matrix
	vec3 mytangent=normalize(tangent);
	vec3 mybitangent=normalize(bitangent);
	vec3 mynormal=normalize(normal);
	mat3 tsm = mat3(mytangent,mybitangent,mynormal); 
	return normalize(tsm*tsn);
}

vec4 computeLightColor(vec3 n)
{
	/* compute the dot product between normal and ldir */
	vec3 halfV;
	float NdotL,NdotHV;

	NdotL = max(dot(n,normalize(lightDir)),0.0);
	vec4 color = vec4(0);
	float att,spotEffect;
	int get_light=0;


	if (NdotL > 0.0) 
	{
		spotEffect = dot(normalize(gl_LightSource[0].spotDirection), normalize(-lightDir));
		
		float threshold = cos(gl_LightSource[0].spotCosCutoff); //spotCosCutoff is in radian

		if (spotEffect >threshold ) 
		{
			spotEffect = pow(spotEffect, gl_LightSource[0].spotExponent);
			att = spotEffect / (gl_LightSource[0].constantAttenuation +
			                    gl_LightSource[0].linearAttenuation * dist +
		                        gl_LightSource[0].quadraticAttenuation * dist * dist);


			color += att * (diffuse * NdotL + ambient);

			halfV = normalize(halfVector);
			NdotHV = max(dot(n,halfV),0.0);
			color += att * gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV,gl_FrontMaterial.shininess);
			get_light=1;
		}
	}

	if(get_light==0)
	{		
		att = 1.0 / (gl_LightSource[0].constantAttenuation +
			         gl_LightSource[0].linearAttenuation * dist +
		             gl_LightSource[0].quadraticAttenuation * dist * dist);		
		color = att * ambient;
	}

	//----------------------------------------------------------------------------

	return color;
}

void main()
{
	/* compute the normal from normal map*/
	vec3 n = newnormal(gl_TexCoord[0].st);

	/* compute color from light*/
	vec4 color = computeLightColor(n);
	
	/* compute color from texture*/
	vec3 ct;
	vec4 texel;
	float at,af;
	af = gl_FrontMaterial.diffuse.a;
		
	texel = texture2D(color_texture,gl_TexCoord[0].st);
	ct = texel.rgb;
	at = texel.a;
	
	//done
	gl_FragColor = vec4(color.rgb * ct, at * af);	
}
