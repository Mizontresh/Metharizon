#version 450 core

// vertex attribute: a full‐screen “triangle” in NDC
layout(location = 0) in vec2 aPos;

// pass this UV to the fragment stage
out vec2 vUV;

void main() {
    // remap from [-1,1] to [0,1]
    vUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
