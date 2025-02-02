#version 460

layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D raytraceTexture;

void main() {
    outColor = vec4(fragColor, 1.0);
    outColor = texture(raytraceTexture, fragTexCoord);
    
}
