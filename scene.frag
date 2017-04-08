uniform sampler2D WaterMap;

in float height;
in vec2 tex_coord;

out vec4 FragColor;


void main() {
    vec4 color;

    vec4 water = vec4(167.0/255.0, 219.0/255.0, 216.0/255.0, 1.0);
    vec4 dirt = vec4(120.0/255.0, 72/255.0, 0.0, 1.0);
    vec4 grass = vec4(141.0/255.0, 189.0/255.0, 12.0/255.0, 1.0);
    vec4 rock = vec4(102.0/255.0, 130.0/255.0, 132.0/255.0, 1.0);
    vec4 snow = vec4(1.0);

    float normalizedHeight = (height + 4.9) / 20.0;

    if (normalizedHeight < 0.0) {
        color = texture(WaterMap, tex_coord).rgba;
    } else {
        float hscaled = normalizedHeight * 2.0 - 1e-05; // hscaled should range in [0,2)
        
        int hi = int(hscaled); // hi should range in [0,1]
        float hfrac = hscaled-float(hi); // hfrac should range in [0,1]
        
        if (hi == 0) {
            color = mix(dirt, grass, hfrac);
        } else {
            color = mix(grass, rock, hfrac);
        }
    }
    
    FragColor = color;
}