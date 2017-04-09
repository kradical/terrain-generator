in vec4 position;

out vec4 FragColor;

uniform samplerCube Skybox;

void main() {
    FragColor = texture(Skybox, vec3(position));
}
