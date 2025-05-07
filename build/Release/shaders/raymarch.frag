#version 450 core
layout(location = 0) out vec4 FragColor;

// Uniforms
uniform vec2  u_resolution;
uniform int   u_maxSteps;
uniform float u_epsilon;
uniform mat4  u_objInvTransform;
uniform vec3  u_camPos, u_camForward, u_camRight, u_camUp;
uniform uint  u_spawnCount;

// SSBOs
layout(std430, binding = 0) readonly buffer PosMinors {
    vec4 posMinors[];  // xyz = center, w = radius
};
layout(std430, binding = 1) readonly buffer IDs {
    uint ids[];
};
layout(std430, binding = 2) readonly buffer Orients {
    vec4 quats[];      // x,y,z,w
};

// Rotate v by the inverse of quaternion q
vec3 rotateInv(vec4 q, vec3 v) {
    vec3 t =  2.0 * cross(q.xyz, v);
    vec3 r =  v + q.w * t + cross(q.xyz, t);
    // Inverse rotation: conjugate quaternion = (–xyz, w)
    return v - q.w * t + cross(q.xyz, t);
}

float torusSDF(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz)-t.x, p.y);
    return length(q) - t.y;
}

float map(vec3 p) {
    // Base fractal: rotating unit-sphere
    vec3 op = (u_objInvTransform * vec4(p,1)).xyz;
    float d = length(op) - 1.0;

    // Blend in each torus, rotated by its quaternion
    for(uint i=0u; i<u_spawnCount; ++i){
        vec4 pm = posMinors[i];
        vec4 q  = quats[i];
        vec3 sc = (u_objInvTransform * vec4(pm.xyz,1)).xyz;
        vec3 rel = op - sc;
        rel = rotateInv(q, rel);
        float td = torusSDF(rel, vec2(pm.w, pm.w*0.4));
        float k = 0.3;
        float h = max(k - abs(d - td), 0.0) / k;
        d = min(d, td) - h*h*k*0.25;
    }
    return d;
}

vec3 estimateNormal(vec3 p) {
    const float e = 1e-4;
    return normalize(vec3(
        map(p+vec3( e,0,0)) - map(p-vec3( e,0,0)),
        map(p+vec3(0, e,0)) - map(p-vec3(0, e,0)),
        map(p+vec3(0,0, e)) - map(p-vec3(0,0, e))
    ));
}

vec3 shade(vec3 p, vec3 rd) {
    vec3 n = estimateNormal(p);
    vec3 L = normalize(vec3(1,1,1));
    float diff = max(dot(n,L),0.0);
    return vec3(0.2) + diff * vec3(0.8);
}

void main(){
    vec2 uv = (gl_FragCoord.xy / u_resolution)*2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;
    vec3 rd = normalize(uv.x*u_camRight + uv.y*u_camUp + u_camForward);
    vec3 ro = u_camPos;

    // Into fractal local space
    vec3 rlo = (u_objInvTransform * vec4(ro,1)).xyz;
    vec3 rld = normalize((u_objInvTransform * vec4(rd,0)).xyz);

    float t = 0.0;
    for(int i=0;i<u_maxSteps;++i){
        vec3 pos = rlo + rld*t;
        float dist = map(pos);
        if(dist < u_epsilon){
            vec3 worldPos = ro + rd*t;
            FragColor = vec4(shade(worldPos,rd),1.0);
            return;
        }
        t += dist;
        if(t > 100.0) break;
    }
    FragColor = vec4(1,0,1,1);
}
