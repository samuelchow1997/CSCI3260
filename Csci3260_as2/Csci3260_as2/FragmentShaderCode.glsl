#version 430

out vec4 FragColor;

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in vec3 Normal;

uniform sampler2D texture12;

uniform vec3 ambientLight;
uniform vec3 diffPos;
uniform vec3 diffColor;
uniform vec3 specPos;
uniform vec3 specColor;

uniform vec3 spotlightPosition;
uniform vec3 spotlightDirection;



void main()
{
	vec3 textureColor = texture(texture12, UV).rgb;


	//diffuse 
	vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(diffPos - vertexPositionWorld);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffColor;

	// specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(specPos - vertexPositionWorld);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0),32);
    vec3 specular = specularStrength * spec * specColor;  



	vec3 colorValue = (vec3(0.1f,0.1f,0.1f) + diffuse + specular);

	vec3 lightDir2 = normalize(spotlightPosition - vertexPositionWorld);

	float theta = dot(lightDir2, normalize(-spotlightDirection));

	if (theta > 0.698131700796    ){
		//diffuse2 
		vec3 norm1 = normalize(Normal);
		vec3 lightDir3 = normalize(spotlightPosition - vertexPositionWorld);
		float diff1 = max(dot(norm1, lightDir3), 0.0);
		vec3 diffuse2 = diff1 * diffColor;

		// specular
		vec3 viewDir1 = normalize(specPos - vertexPositionWorld);
		vec3 reflectDir1 = reflect(-lightDir3, norm1);  
		float spec1 = pow(max(dot(viewDir1, reflectDir1), 0.0), 64);
		vec3 specular2 = specularStrength * spec1 * specColor;  

		colorValue += (vec3(0.1f,0.1f,0.1f) + diffuse2 + specular2);

	}



	//diffuse 
    vec3 lightDir3 = normalize(-spotlightDirection);
    float diff3 = max(dot(norm, lightDir3), 0.0);
    vec3 diffuse3 = diff3 * diffColor;

	// specular
    vec3 viewDir3 = normalize(specPos - vertexPositionWorld);
    vec3 reflectDir3 = reflect(-lightDir3, norm);  
    float spec3 = pow(max(dot(viewDir3, reflectDir3), 0.0),32);
    vec3 specular3 = specularStrength * spec3 * specColor;  

	colorValue += (vec3(0.1f,0.1f,0.1f) + diffuse3 + specular3) * ambientLight;

	vec3 color = colorValue * textureColor;
	FragColor = vec4(color, 1.0);


}
