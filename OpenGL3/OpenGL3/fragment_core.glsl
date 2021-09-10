#version 460

in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;

out vec4 fs_color;
uniform vec3 lightPos0;
uniform vec3 CamPosition;

void main()
{
	//fs_color=vec4(vs_color,1.0f); 

	// Ambient Light
	vec3 ambientLight = vec3(0.1f,0.1f,0.4f);

	// Diffuse Light.
	vec3 posToLightDirVec = normalize(lightPos0 - vs_position);
	vec3 diffuseColor = vec3(1.f,1.f,1.f);
	float diffuse = clamp(dot(posToLightDirVec, vs_normal),0,1);
	vec3 diffuseFinal = diffuseColor * diffuse;

	// Specular Light.
	vec3 lightToPosDirVec = normalize(vs_position - lightPos0);
	vec3 reflectDirVec = normalize(reflect(lightToPosDirVec,normalize(vs_normal)));
	vec3 postToViewDirVec = normalize(CamPosition - vs_position);
	float specularConsntant = pow(max(dot(postToViewDirVec,reflectDirVec),0),0.5);
	vec3 specularFinal = vec3(1.f,1.f,1.f) * specularConsntant;

	fs_color =vec4(vs_color,1.f) + vec4(ambientLight,1.f) + vec4(diffuseFinal,1.f) + vec4(specularFinal,1.f);
}
	
