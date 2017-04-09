layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_NORMAL_ATTRIB_LOCATION)
in vec3 Normal;

out vec4 position;
out vec3 normal;
out vec2 tex_coord;

uniform mat4 ModelViewProjection;

void main()
{
    vec4 newPosition = Position; 
    if (newPosition.y < -8.0) {
        newPosition.y = -9.0;
    }

    tex_coord = vec2(newPosition.x / 5.0, newPosition.z / 5.0);
    normal = Normal;
    position = newPosition;
    gl_Position = ModelViewProjection * newPosition;
}