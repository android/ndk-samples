#version 450

// Input colour coming from the vertex shader
layout(location = 0) in vec3 fragColor;

// Output colour for the fragment
layout(location = 0) out vec4 outColor1;
layout(location = 1) out vec4 outColor2;
layout(location = 2) out vec4 outColor3;
layout(location = 3) out vec4 outColor4;

void main() {
    outColor1 = vec4(fragColor, 1.0);
    outColor2 = vec4(fragColor, 1.0);
    outColor3 = vec4(fragColor, 1.0);
    outColor4 = vec4(fragColor, 1.0);
}
