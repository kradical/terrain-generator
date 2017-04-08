uniform sampler2D WaterMap;
uniform sampler2D SandMap;
uniform sampler2D GrassMap;
uniform sampler2D RockMap;
uniform sampler2D SnowMap;

in float height;
in vec2 tex_coord;

out vec4 FragColor;


void main() {
    vec4 color;

    float normalizedHeight = (height + 9) / 24.0; // [0, 1]

    float scaledHeight = normalizedHeight * 16.0 - 1e-05; // [0, 16)
    
    int heightStep = int(scaledHeight); // {0, 1, 2, 3..15}
    float hfrac = scaledHeight - float(heightStep); // [0, 15]
    
    if (heightStep == 0) {
        // Water
        color = texture(WaterMap, tex_coord).rgba;
    } else if(heightStep == 1) {
        // Water & Sand
        vec4 water = texture(WaterMap, tex_coord).rgba;
        vec4 sand = texture(SandMap, tex_coord).rgba;
        color = mix(water, sand, hfrac);
    } else if (heightStep < 3) {
        // Sand
        color = texture(SandMap, tex_coord).rgba;
    } else if (heightStep < 5) {
        // Sand & Grass
        vec4 sand = texture(SandMap, tex_coord).rgba;
        vec4 grass = texture(GrassMap, tex_coord).rgba;
        color = mix(sand, grass, hfrac);
    } else if (heightStep < 8) {
        // Grass
        color = texture(GrassMap, tex_coord).rgba;
    } else if (heightStep < 12) {
        // Grass & Rock
        vec4 grass = texture(GrassMap, tex_coord).rgba;
        vec4 rock = texture(RockMap, tex_coord).rgba;
        color = mix(grass, rock, hfrac);
    } else if (heightStep < 14) {
        // Rock
        color = texture(RockMap, tex_coord).rgba;
    } else if (heightStep < 15) {
        // Rock & Snow
        vec4 rock = texture(RockMap, tex_coord).rgba;
        vec4 snow = texture(SnowMap, tex_coord).rgba;
        color = mix(rock, snow, hfrac);
    } else {
        // Snow
        color = texture(SnowMap, tex_coord).rgba;
    }
    
    FragColor = color;
}
