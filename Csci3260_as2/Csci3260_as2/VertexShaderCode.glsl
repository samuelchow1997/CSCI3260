#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normal;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * position;
	UV = vertexUV;

	vec4 normal_temp = model * vec4(normal, 0);
	normalWorld = normal_temp.xyz;

	vertexPositionWorld = vec3(model * position);
	//Normal = normal;
	Normal = mat3(transpose(inverse(model))) * normal;

}
