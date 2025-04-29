#version 450 core
in vec2 vUV;
out vec4 FragColor;

uniform int   u_mode;
uniform vec2  u_resolution;
uniform float u_time;
uniform int   u_maxSteps;
uniform float u_epsilon;
uniform int   u_pass;

uniform mat4  u_objInvTransform;  // <— new

uniform vec3  u_camPos;
uniform vec3  u_camForward;
uniform vec3  u_camRight;
uniform vec3  u_camUp;

float sdSphere(vec3 p, float r) {
    return length(p) - r;
}
float sdBox(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return length(max(d,0.0)) + min(max(d.x,max(d.y,d.z)),0.0);
}
float smoothUnion(float d1, float d2, float k){
    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k*h*(1.0 - h);
}

float mapScene(vec3 p) {
    // bring into object space
    p = (u_objInvTransform * vec4(p,1)).xyz;
    // same composite object
    float s = sdSphere(p, 0.8);
    float b = sdBox(p - vec3(1.2,0,0), vec3(0.5));
    return smoothUnion(s,b,0.4);
}

vec3 calcNormal(vec3 p) {
    float e = u_epsilon;
    return normalize(vec3(
      mapScene(p + vec3(e,0,0)) - mapScene(p - vec3(e,0,0)),
      mapScene(p + vec3(0,e,0)) - mapScene(p - vec3(0,e,0)),
      mapScene(p + vec3(0,0,e)) - mapScene(p - vec3(0,0,e))
    ));
}

vec3 raymarch(vec3 ro, vec3 rd){
    float t=0.0, ri1=0.0, ri=mapScene(ro+rd*t), di;
    for(int i=0;i<u_maxSteps;++i){
        if(u_mode==0){
            di = ri;
        } else if(u_mode==1){
            float relaxed=ri+0.5*(ri-ri1);
            di = max(relaxed, ri);
        } else {
            float inf=(ri*(ri+ri1)-(ri-ri1)*ri)/(ri1+ri-ri1);
            float test=mapScene(ro+rd*(t+inf));
            di = (inf>ri+test)?ri:inf;
        }
        t+=di; ri1=ri; ri=mapScene(ro+rd*t);
        if(ri<u_epsilon){
            vec3 p=ro+rd*t, n=calcNormal(p);
            float diff=max(dot(n,normalize(vec3(1,1,0.5))),0.0);
            return vec3(0.2,0.8,1.0)*diff;
        }
        if(t>50.0) break;
    }
    return vec3(1.0,0.7137,0.7569);
}

void main(){
    vec2 uv=(vUV*u_resolution-0.5*u_resolution)/u_resolution.y;
    vec3 rd=normalize(uv.x*u_camRight+uv.y*u_camUp+u_camForward);
    vec3 col=raymarch(u_camPos,rd);
    FragColor=vec4(col,1.0);
}
