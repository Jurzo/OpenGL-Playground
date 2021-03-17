#version 460 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 direction;
    float cutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

in vec3 LightPos;
in vec3 lightDirection;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

void main()
{
    vec3 lightDir = normalize(LightPos - FragPos);

    float theta = dot(lightDir, normalize(-lightDirection));

    if (theta > light.cutOff){

        float dist = length(LightPos - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * dist + 
                    light.quadratic * (dist * dist));

        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

        // diffuse
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

        // specular
        vec3 viewDir = normalize(-FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

        //ambient  *= attenuation; 
        diffuse  *= attenuation;
        specular *= attenuation;  

        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
    } else {
        FragColor = vec4(light.ambient * texture(material.diffuse, TexCoords).rgb, 1.0);
    }
}