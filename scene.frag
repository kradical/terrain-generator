out vec4 FragColor;

in float height;

void main()
{
    float normalizedHeight = (height + 15.0) / 30.0;
    FragColor = vec4(normalizedHeight, normalizedHeight, normalizedHeight, 1);
}