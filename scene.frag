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
    
    float offset = random(vec2(position.x, position.z));
    float noisyHeight = position.y + 0.1 * offset;

    float normalizedHeight = (noisyHeight + 9) / 24.0; // [0, 1]

    float scaledHeight = normalizedHeight * 16.0 - 1e-05; // [0, 16)
    
    int heightStep = int(scaledHeight); // {0, 1, 2, 3..15}
    float steepness = dot(normal, vec3(0, 1, 0)) + 1.5;
    
    if (heightStep == 0) {
        // Water
        color = texture(WaterMap, tex_coord).rgba;
    } else if(heightStep == 1) {
        // Water & Sand
        vec4 water = texture(WaterMap, tex_coord).rgba;
        vec4 sand = texture(SandMap, tex_coord).rgba;

        float hfrac = scaledHeight - 1.0;
        color = mix(water, sand, hfrac);
    } else if (heightStep < 3) {
        // Sand
        color = texture(SandMap, tex_coord).rgba;
    } else if (heightStep < 4) {
        // Sand & Grass
        vec4 sand = texture(SandMap, tex_coord).rgba;
        vec4 grass = texture(GrassMap, tex_coord).rgba;

        float hfrac = scaledHeight - 3.0;
        color = mix(sand, grass, hfrac);
    } else if (heightStep < 10) {
        // Grass
        color = texture(GrassMap, tex_coord).rgba;
    } else if (heightStep < 11) {
        // Grass & Rock
        vec4 grass = texture(GrassMap, tex_coord).rgba;
        vec4 rock = texture(RockMap, tex_coord).rgba;

        float hfrac = scaledHeight - 10.0;
        color = mix(grass, rock, hfrac);
    } else if (heightStep < 12) {
        // Rock
        color = texture(RockMap, tex_coord).rgba;
    } else if (heightStep < 13) {
        // Rock & Snow
        vec4 rock = texture(RockMap, tex_coord).rgba;
        vec4 snow = texture(SnowMap, tex_coord).rgba;

        float hfrac = scaledHeight - 12.0;
        color = mix(rock, snow, hfrac);
    } else {
        // Snow
        color = texture(SnowMap, tex_coord).rgba;
    }
    
    FragColor = steepness * color;
}
