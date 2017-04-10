layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

out vec4 position;

uniform mat4 ModelViewProjection;

void main() {
    position = Position;
    gl_Position = ModelViewProjection * Position;
}