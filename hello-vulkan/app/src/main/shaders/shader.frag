#version 450

// Input colour coming from the vertex shader
layout(location = 0) in vec3 fragColor;

// Output colour for the fragment
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}