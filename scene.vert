layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

out float height;

uniform mat4 ModelViewProjection;

void main()
{
    height = Position.y;
    gl_Position = ModelViewProjection * Position;
}