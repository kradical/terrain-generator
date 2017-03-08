uniform vec3 CameraPos;

uniform vec3 Ambient;
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float Shininess;

uniform int HasDiffuseMap;
uniform sampler2D DiffuseMap;

in vec4 vertex_position;
in vec2 vertex_texCoord;
in vec3 vertex_normal;

out vec4 FragColor;

void main()
{
    // light at camera
    vec3 lightDir = normalize(CameraPos - vec3(vertex_position));
    float diffuse = clamp(dot(lightDir, vertex_normal), 0, 1);

    float specular;
    if(diffuse > 0) {
        specular = pow(diffuse, Shininess);
    } else {
        specular = 0;
    }

    vec4 diffuseMap;
    if(HasDiffuseMap != 0) {
        diffuseMap = texture(DiffuseMap, vertex_texCoord).rgba;
    } else {
        diffuseMap = vec4(Diffuse, 1.0);
    }

    FragColor = 0.1 * vec4(Ambient, 1.0) 
        + diffuse * diffuseMap
        + specular * vec4(Specular, 1.0);
}