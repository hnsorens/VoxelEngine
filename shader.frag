#version 460

layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D raytraceTexture;

void main() {
    vec2 texelSize = 1.0 / textureSize(raytraceTexture, 0);
    
    vec4 samples[9];
    int index = 0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            samples[index++] = texture(raytraceTexture, fragTexCoord + offset);
        }
    }

    // Sort the samples by brightness and pick the middle one
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 9; j++) {
            if (dot(samples[i].rgb, vec3(0.299, 0.587, 0.114)) > dot(samples[j].rgb, vec3(0.299, 0.587, 0.114))) {
                vec4 temp = samples[i];
                samples[i] = samples[j];
                samples[j] = temp;
            }
        }
    }

    // outColor = samples[4]; // Middle value
    outColor = texture(raytraceTexture, fragTexCoord);

    // if (fragTexCoord.x > 0.4975 && fragTexCoord.x < 0.5025 && fragTexCoord.y > 0.495 && fragTexCoord.y < 0.505)
    // outColor = vec4(0.8, 0.8, 0.8, 1.0);
}