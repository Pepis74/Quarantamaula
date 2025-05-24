#version 460 core
out vec4 fragColor;
in vec3 position;
uniform samplerCube envMap;

const float PI = 3.14159265359;
const float SAMPLING_DELTA = 0.00125;
const float MAX_HDR_VALUE = 1e5; // Adjust based on your HDR range

void main() {
    vec3 N = normalize(position);
    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    float delta = SAMPLING_DELTA;

    for (float phi = 0.0; phi < 2.0 * PI; phi += delta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += delta) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            vec3 sampleColor = texture(envMap, sampleVec).rgb;
            sampleColor = clamp(sampleColor, 0.0, MAX_HDR_VALUE);
            float cosTheta = cos(theta);
            irradiance += sampleColor * cosTheta * sin(theta) * cosTheta;
        }
    }

    irradiance *= delta * delta;
    fragColor = vec4(irradiance, 1.0);
}