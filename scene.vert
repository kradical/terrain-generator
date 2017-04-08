layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

out float height;
out vec2 tex_coord;

uniform mat4 ModelViewProjection;

void main()
{
    vec4 newPosition = Position; 
    if (newPosition.y < -4.0) {
        newPosition.y = -5.0;
    }

    tex_coord = vec2(newPosition.x, newPosition.z);
    height = newPosition.y;
    gl_Position = ModelViewProjection * newPosition;
}