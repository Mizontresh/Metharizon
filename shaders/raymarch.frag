#version 450 core
layout(location=0) out vec4 FragColor;

uniform vec2  u_resolution;
uniform float u_time;
uniform int   u_maxSteps;
uniform float u_epsilon;
uniform int   u_pass;
uniform int   u_mode;
uniform mat4  u_objInvTransform;
uniform vec3  u_camPos;
uniform vec3  u_camForward;
uniform vec3  u_camRight;
uniform vec3  u_camUp;

uniform uint  u_spawnCount;

// SSBO binding points 0 and 1:
layout(std430, binding = 0) readonly buffer PosMinors {
    vec4 posMinors[];  // .xyz=pos, .w=minor radius
};
layout(std430, binding = 1) readonly buffer IDs {
    uint ids[];
};

// torus SDF (donut)
float torusSDF(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz)-t.x, p.y);
    return length(q) - t.y;
}

float map(vec3 p) {
    // rotating unit-sphere in object space
    vec3 op = (u_objInvTransform * vec4(p,1.0)).xyz;
    float d   = length(op) - 1.0;

    // blend in each spawned torus via smooth-min
    for (uint i = 0u; i < u_spawnCount; ++i) {
        vec4 pm = posMinors[i];
        float td = torusSDF(p - pm.xyz, vec2(pm.w, pm.w*0.4));
        float k  = 0.3;
        float h  = max(k - abs(d - td), 0.0)/k;
        d = min(d,td) - h*h*k*0.25;
    }
    return d;
}

vec3 estimateNormal(vec3 p) {
    float h = 1e-4;
    return normalize(vec3(
        map(p+vec3(h,0,0)) - map(p-vec3(h,0,0)),
        map(p+vec3(0,h,0)) - map(p-vec3(0,h,0)),
        map(p+vec3(0,0,h)) - map(p-vec3(0,0,h))
    ));
}

vec3 shade(vec3 p, vec3 rd) {
    vec3 n = estimateNormal(p);
    vec3 L = normalize(vec3(1,1,1));
    float diff = max(dot(n,L),0.0);
    return vec3(0.2) + diff*vec3(0.8);
}

void main(){
    vec2 uv = (gl_FragCoord.xy/u_resolution)*2.0-1.0;
    uv.x *= u_resolution.x/u_resolution.y;
    vec3 rd = normalize(uv.x*u_camRight + uv.y*u_camUp + u_camForward);
    vec3 ro = u_camPos;
    float t=0.0;
    for(int i=0;i<u_maxSteps;++i){
        vec3 pos=ro + rd*t;
        float dist=map(pos);
        if(dist< u_epsilon){
            FragColor = vec4(shade(pos,rd),1.0);
            return;
        }
        t+=dist;
        if(t>100.0) break;
    }
    FragColor = vec4(1.0,0.0,1.0,1.0);
}
