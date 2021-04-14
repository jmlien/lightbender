//
// TODO: Implement relief mapping. This should add on to the normal mapping shader: normalmapping.vert/frag
//

//------------------------------------------------------------------------------------
//
// this implements parallax offset mapping; not relief mapping
//
//------------------------------------------------------------------------------------
#version 330

#define PARALLAX_SCALE 0.01

varying vec4 diffuse, ambient;
varying vec3 normal, eyeDir, lightDir, halfVector, tangent, bitangent;
varying float dist;

uniform sampler2D color_texture;  //color texture
uniform sampler2D norml_map;      //normal map
uniform sampler2D relief_map;     //relief map

//tangent space matrix
mat3 tsm;

//cmopute uv offset
vec2 offset(in vec2 uv)
{
	float height = texture(relief_map,uv).r; //height map
	
	//contruct tangent space system
	mat3 tsm_inv = transpose(tsm);
	vec3 eye = normalize(eyeDir);
	vec3 eye_ts=normalize(tsm_inv*eye);

	// calculate amount of offset for Parallax Mapping With Offset Limiting
	vec2 coordoffset = PARALLAX_SCALE * eye_ts.xy * height;

   // return new texture coordinates
   return uv + coordoffset;
}

//
vec3 newnormal(vec2 uv)
{
	vec3 texel = texture(norml_map,uv).rgb;
	vec3 tsn =  normalize(texel*2.0-vec3(1)); //tangent space normal
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
	//compute tangent space matrix
	vec3 mytangent=normalize(tangent);
	vec3 mybitangent=normalize(bitangent);
	vec3 mynormal=normalize(normal);
	tsm = mat3(mytangent,mybitangent,mynormal); 

	//
	vec2 uv=offset(gl_TexCoord[0].st);

	/* compute the normal from normal map*/
	vec3 n = newnormal(uv);

	/* compute color from light*/
	vec4 color = computeLightColor(n);
	
	/* compute color from texture*/
	vec3 ct;
	vec4 texel;
	float at,af;
	af = gl_FrontMaterial.diffuse.a;
		
	texel = texture(color_texture,uv);
	ct = texel.rgb;
	at = texel.a;
	
	//done
	gl_FragColor = vec4(color.rgb * ct, at * af);	
}
