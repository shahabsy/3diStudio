#version 450

// Input from vertex buffer (we will create this later)
layout(location = 0) in vec2 inPosition;

// Output fragment shader
layout(location = 0) out vec3 fragColor;

// Push constants for transformation (fast, small data)
layout(push_constant) uniform PushConstants {
    mat4 modelViewProjection;
} pushConstants;

void main()
{
	// Transform vertex positoin
    gl_Position = pushConstants.modelViewProjection * vec4(inPosition, 0.0, 1.0);

    // Pass different colors to create gradient
    if(gl_VertexIndex == 0) {
        fragColor = vec3(1.0, 0.0, 0.0); // Red
    } else if (gl_VertexIndex == 1) {
        fragColor = vec3(0.0, 1.0, 0.0); // Green
    } else {
        fragColor = vec3(0.0, 0.0, 1.0); // Blue
    }
}