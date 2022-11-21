#version 450

layout(location = 0) out vec3 fragColor;
layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
} ubo;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(0.67, 0.1, 0.2),
    vec3(0.67, 0.1, 0.2),
    vec3(0.67, 0.1, 0.2)
);

void main() {
    gl_Position = ubo.MVP * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}