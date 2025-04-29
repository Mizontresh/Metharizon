#version 450 core

// uniforms
uniform float u_config[6];    // [0]=resX, [1]=resY, [2]=time, [3]=maxSteps, [4]=epsilon, [5]=pass
uniform vec3  u_camPos;
uniform vec3  u_camForward;
uniform vec3  u_camRight;
uniform vec3  u_camUp;

out vec4 FragColor;

// simple sphere SDF
float sdSphere(vec3 p, float r) {
    return length(p) - r;
}

// animated scene
float sceneSDF(vec3 p) {
    float r = 0.75 + 0.25 * sin(u_config[2]);
    return sdSphere(p, r);
}

// basic sphere-tracing
float sphereTrace(vec3 ro, vec3 rd) {
    float t = 0.0;
    for (int i = 0; i < int(u_config[3]); ++i) {
        vec3 p = ro + rd * t;
        float d = sceneSDF(p);
        if (d < u_config[4]) return t;
        t += d;
        if (t > 100.0) break;
    }
    return -1.0;
}

void main() {
    vec2 res = vec2(u_config[0], u_config[1]);
    vec2 uv  = (gl_FragCoord.xy / res) * 2.0 - 1.0;
    uv.x *= res.x / res.y;

    // ray origin & direction
    vec3 ro = u_camPos;
    vec3 rd = normalize(uv.x * u_camRight +
                        uv.y * u_camUp +
                        1.5 * u_camForward);

    float tHit = sphereTrace(ro, rd);
    vec3 col = vec3(0.0);
    if (tHit > 0.0) {
        vec3 p = ro + rd * tHit;
        // estimate normal
        float h = u_config[4];
        vec3 n = normalize(vec3(
            sceneSDF(p + vec3(h,0,0)) - sceneSDF(p - vec3(h,0,0)),
            sceneSDF(p + vec3(0,h,0)) - sceneSDF(p - vec3(0,h,0)),
            sceneSDF(p + vec3(0,0,h)) - sceneSDF(p - vec3(0,0,h))
        ));
        float diff = clamp(dot(n, normalize(vec3(1,1,1))), 0.0, 1.0);
        col = vec3(0.2) + diff * vec3(0.8);
    }

    FragColor = vec4(col, 1.0);
}
