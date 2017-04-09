uniform sampler2D WaterMap;
uniform sampler2D SandMap;
uniform sampler2D GrassMap;
uniform sampler2D RockMap;
uniform sampler2D SnowMap;

in vec4 position;
in vec3 normal;
in vec2 tex_coord;

out vec4 FragColor;

float random(vec2 p){return fract(cos(dot(p,vec2(23.14069263277926,2.665144142690225)))*123456.);}


void main() {
    vec4 color;

    float normalizedHeight = clamp(0.0, 1.0, (position.y + 7.5) / 24.0); // [0, 1]
    float steepness = dot(normal, vec3(0, 1, 0));

    vec4 water = texture(WaterMap, tex_coord).rgba;
    vec4 sand = texture(SandMap, tex_coord).rgba;
    vec4 grass = texture(GrassMap, tex_coord).rgba;
    vec4 rock = texture(RockMap, tex_coord).rgba;
    vec4 snow = texture(SnowMap, tex_coord).rgba;

    if (normalizedHeight > 0) {
        float fraction = clamp(0.0, 1.0, normalizedHeight * 10.0);
        color = mix(sand, grass, 0.8 * fraction + 0.2 * steepness);
        
        float fraction2 = clamp(0.0, 1.0, normalizedHeight * 1.5);
        color = mix(color, rock, 0.8 * fraction2 + 0.2 * steepness);  
        
        if (normalizedHeight > 0.75) {
            float fraction3 = 4 * (normalizedHeight - 0.75);
            color = mix(color, snow, fraction3);
        }
    } else {
        color = texture(WaterMap, tex_coord).rgba;
    }
    
    FragColor = color;
}
