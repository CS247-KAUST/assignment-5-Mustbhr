#version 410

in vec2 texCoord;

uniform sampler2D txtr;
uniform vec4 vertexColor;

uniform int colormapMode;   // 0 = off (gray), 1 = rainbow, 2 = cool-warm
uniform float blendFactor;  // 0 = gray, 1 = full colormap

out vec4 fragColor;

vec3 rainbow(float t) {
    // blue -> cyan -> green -> yellow -> red
    vec3 c;
    if (t < 0.25) {
        c = mix(vec3(0,0,1), vec3(0,1,1), t/0.25);
    } else if (t < 0.5) {
        c = mix(vec3(0,1,1), vec3(0,1,0), (t-0.25)/0.25);
    } else if (t < 0.75) {
        c = mix(vec3(0,1,0), vec3(1,1,0), (t-0.5)/0.25);
    } else {
        c = mix(vec3(1,1,0), vec3(1,0,0), (t-0.75)/0.25);
    }
    return c;
}

vec3 coolWarm(float t) {
    // blue -> white -> red
    if (t < 0.5)
        return mix(vec3(0,0,1), vec3(1,1,1), t/0.5);
    else
        return mix(vec3(1,1,1), vec3(1,0,0), (t-0.5)/0.5);
}

void main() {
    if (colormapMode == 3) {
        // solid color (used for line/glyph overlays)
        fragColor = vertexColor;
    } else if (colormapMode == 0) {
        // grayscale background (default). texture is single-channel (GL_RED),
        // so splat .r into rgb.
        float s = texture(txtr, texCoord).r;
        fragColor = vertexColor + vec4(s, s, s, 1.0);
    } else {
        float s = texture(txtr, texCoord).r;
        vec3 gray = vec3(s);
        vec3 mapped;
        if (colormapMode == 1) mapped = rainbow(s);
        else                   mapped = coolWarm(s);
        vec3 col = mix(gray, mapped, blendFactor);
        fragColor = vec4(col, 1.0);
    }
}
