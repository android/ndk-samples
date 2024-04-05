#version 450

// Colour passed to the fragment shader
layout(location = 0) out vec3 fragColor;

// Uniform buffer containing an MVP matrix.
// Currently the vulkan backend only sets the rotation matix
// required to handle device rotation.
layout(binding = 0) uniform UniformBufferObject {
    mat4 MVP;
} ubo;

vec2 positions[9] = vec2[](
    vec2(-0.984375, -0.99609375),
    vec2(-0.99999, -0.98828125),
    vec2(-0.96875, -0.98828125),

    vec2(-0.96875, -0.984375),
    vec2(1., -1.),
    vec2(1., 1.),

    vec2(-0.96875, -0.984375),
    vec2(-1., 1.),
    vec2(1., 1.)
);

void main() {
    gl_Position = ubo.MVP * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = vec3(0.67, 0.1, 0.2);
}
