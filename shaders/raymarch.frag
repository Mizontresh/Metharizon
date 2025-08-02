#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec3 cameraPos;
    float time;
    mat4 viewMatrix;
    float aspectRatio;
} pc;

// Basic raymarching setup
struct Ray {
    vec3 origin;
    vec3 direction;
};

// Mandelbulb distance field
float mandelbulbSDF(vec3 p) {
    vec3 z = p;
    float dr = 1.0;
    float r = 0.0;
    
    for (int i = 0; i < 15; i++) {
        r = length(z);
        if (r > 2.0) break;
        
        // Convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        
        // Power-8 Mandelbulb
        float zr = pow(r, 8.0);
        theta = theta * 8.0;
        phi = phi * 8.0;
        
        // Convert back to cartesian coordinates
        z = zr * vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        z += p;
        
        dr = pow(r, 7.0) * 8.0 * dr + 1.0;
    }
    
    return 0.5 * log(r) * r / dr;
}

vec3 estimateNormal(vec3 p) {
    const float epsilon = 0.001;
    return normalize(vec3(
        mandelbulbSDF(vec3(p.x + epsilon, p.y, p.z)) - mandelbulbSDF(vec3(p.x - epsilon, p.y, p.z)),
        mandelbulbSDF(vec3(p.x, p.y + epsilon, p.z)) - mandelbulbSDF(vec3(p.x, p.y - epsilon, p.z)),
        mandelbulbSDF(vec3(p.x, p.y, p.z + epsilon)) - mandelbulbSDF(vec3(p.x, p.y, p.z - epsilon))
    ));
}

vec3 raymarch(Ray ray) {
    float maxDist = 100.0;
    float dist = 0.0;
    vec3 pos = ray.origin;
    
    for (int i = 0; i < 100; i++) {
        float sceneDist = mandelbulbSDF(pos);
        if (sceneDist < 0.001) {
            vec3 normal = estimateNormal(pos);
            
            // Simple, solid color for Mandelbulb
            vec3 color = vec3(0.2, 0.6, 0.8); // Solid blue
            color += normal * 0.3; // Simple lighting
            
            return color;
        }
        if (dist > maxDist) break;
        dist += sceneDist;
        pos = ray.origin + ray.direction * dist;
    }
    
    // Simple background
    return vec3(0.1, 0.2, 0.4);
}

void main() {
    // Create ray from camera
    vec2 uv = inTexCoord * 2.0 - 1.0;
    uv.x *= pc.aspectRatio;
    
    Ray ray;
    ray.origin = pc.cameraPos;
    ray.direction = normalize(vec3(uv, 1.0));
    ray.direction = normalize((pc.viewMatrix * vec4(ray.direction, 0.0)).xyz);
    
    outColor = vec4(raymarch(ray), 1.0);
} 