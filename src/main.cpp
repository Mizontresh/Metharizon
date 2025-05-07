#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_query.hpp> // for glm::reflect

#include "Window.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "Raymarcher.hpp"

// Stub SDF; replace with your real map() logic
float cpuSDF(const glm::vec3& p) {
    return 1e6f;
}

int main(){
    // — init window & subsystems —
    Window window;
    if(!window.init(1280,720,"Metharizon")) return -1;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    Time time; time.init();
    Input input;
    Raymarcher rm;
    if(!rm.init()) return -1;

    // — raymarch config —
    RaymarchConfig cfg{};
    cfg.maxSteps = 64;
    cfg.epsilon  = 0.001f;
    cfg.pass     = 0;

    // — camera & fractal transform —
    glm::vec3 camPos(0,0,3);
    glm::quat viewOri(1,0,0,0);
    const glm::vec3 worldUp(0,1,0);
    const float speed = 20.0f, sens = 0.0025f;
    int mode = 2;
    glm::mat4 fractalXform(1.0f);

    // — dynamic bodies data —
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<float>     radii;
    std::vector<float>     masses;
    std::vector<float>     inertias;
    std::vector<glm::quat> orientations;
    std::vector<glm::vec3> angVel;
    std::vector<unsigned>  ids;
    unsigned nextID = 1;

    // — physics params —
    const float spawnDist   = 2.0f;
    const float bodyR       = 0.2f;
    const float density     = 1.0f;
    const float G           = 200.0f;
    const float softening   = 0.1f;
    const float restitution = 1.0f;
    const float mu          = 0.2f;
    const int   substeps    = 4;
    auto computeMass    =[&](float r){ return density*(4.0f/3.0f)*3.14159265f*r*r*r; };
    auto computeInertia =[&](float m,float r){ return 0.4f * m * r*r; };

    while(window.isOpen()){
        window.pollEvents();
        input.update();

        float dt   = time.deltaTime();
        float dt_s = dt / float(substeps);

        // — spawn on 'P' —
        if(input.wasKeyPressed(SDL_SCANCODE_P)) {
            glm::vec3 p = camPos + (viewOri * glm::vec3(0,0,-1)) * spawnDist;
            positions  .push_back(p);
            velocities .push_back(glm::vec3(0.0f));
            radii      .push_back(bodyR);
            masses     .push_back(computeMass(bodyR));
            inertias   .push_back(computeInertia(masses.back(), bodyR));
            orientations.push_back(glm::quat(1,0,0,0));
            angVel     .push_back(glm::vec3(0.0f));
            ids        .push_back(nextID++);
        }

        // — camera control —
        int mx,my; input.getMouseDelta(mx,my);
        viewOri = glm::normalize(glm::angleAxis(-mx*sens, viewOri*worldUp)*viewOri);
        viewOri = glm::normalize(glm::angleAxis(-my*sens, viewOri*glm::vec3(1,0,0))*viewOri);
        glm::vec3 forward = viewOri*glm::vec3(0,0,-1);
        glm::vec3 right   = viewOri*glm::vec3(1,0,0);
        glm::vec3 upVec   = viewOri*glm::vec3(0,1,0);
        if(input.isKeyDown(SDL_SCANCODE_W))      camPos += forward * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_S))      camPos -= forward * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_A))      camPos -= right   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_D))      camPos += right   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_SPACE))  camPos += upVec   * speed * dt;
        if(input.isKeyDown(SDL_SCANCODE_LSHIFT)) camPos -= upVec   * speed * dt;
        if(input.wasKeyPressed(SDL_SCANCODE_ESCAPE)) break;

        size_t n = positions.size();
        for(int step=0; step<substeps; ++step){
            // 1) Gravity
            std::vector<glm::vec3> forces(n, glm::vec3(0.0f));
            for(size_t i=0;i<n;++i){
                for(size_t j=i+1;j<n;++j){
                    glm::vec3 d = positions[j]-positions[i];
                    float dist2 = glm::dot(d,d)+softening*softening;
                    float invD  = 1.0f/std::sqrt(dist2);
                    glm::vec3 dir = d * invD;
                    float mag = G * masses[i] * masses[j] * (invD*invD);
                    forces[i] +=  mag * dir;
                    forces[j] -=  mag * dir;
                }
            }
            // 2) Integrate linear
            for(size_t i=0;i<n;++i){
                velocities[i] += (forces[i]/masses[i]) * dt_s;
                positions [i] += velocities[i] * dt_s;
            }
            // 3) Sphere–sphere collisions
            for(size_t i=0;i<n;++i){
                for(size_t j=i+1;j<n;++j){
                    glm::vec3 d = positions[j]-positions[i];
                    float dist2 = glm::dot(d,d);
                    float Rsum  = radii[i]+radii[j];
                    if(dist2 < Rsum*Rsum){
                        float dist = std::sqrt(dist2);
                        glm::vec3 N = dist>0? d/dist: glm::vec3(1,0,0);
                        // unstuck
                        float pen = Rsum - dist;
                        positions[i] -= 0.5f * pen * N;
                        positions[j] += 0.5f * pen * N;

                        // relative velocity at contact
                        glm::vec3 rA =  N * radii[i];
                        glm::vec3 rB = -N * radii[j];
                        glm::vec3 vA = velocities[i] + glm::cross(angVel[i],rA);
                        glm::vec3 vB = velocities[j] + glm::cross(angVel[j],rB);
                        glm::vec3 relV = vB - vA;

                        // normal impulse
                        float vn = glm::dot(relV, N);
                        if(vn < 0.0f){
                            float invM = 1.0f/masses[i] + 1.0f/masses[j];
                            float Jn = -(1.0f+restitution)*vn / invM;
                            glm::vec3 J = Jn * N;

                            velocities[i] -= J * (1.0f/masses[i]);
                            velocities[j] += J * (1.0f/masses[j]);

                            // contact point on sphere i:
                            glm::vec3 contact = positions[i] + rA;
                            glm::vec3 torque = glm::cross(contact - positions[i], -J);
                            angVel[i] += torque / inertias[i];

                            contact = positions[j] + rB;
                            torque = glm::cross(contact - positions[j], +J);
                            angVel[j] += torque / inertias[j];

                            // friction (Coulomb)
                            glm::vec3 vt = relV - vn*N;
                            float vt_len = glm::length(vt);
                            if(vt_len > 1e-4f){
                                glm::vec3 tdir = vt / vt_len;
                                float Jt = glm::min(mu * Jn, vt_len * masses[i]);
                                glm::vec3 Jf = -Jt * tdir;
                                velocities[i] -= Jf * (1.0f/masses[i]);
                                velocities[j] += Jf * (1.0f/masses[j]);
                                // torque from friction
                                contact = positions[i] + rA;
                                angVel[i] += glm::cross(contact - positions[i], -Jf) / inertias[i];
                                contact = positions[j] + rB;
                                angVel[j] += glm::cross(contact - positions[j], +Jf) / inertias[j];
                            }
                        }
                    }
                }
            }
            // 4) Sphere–SDF collisions (similar torque logic)
            glm::mat4 invX = glm::inverse(fractalXform);
            for(size_t i=0;i<n;++i){
                glm::vec3 lp = glm::vec3(invX * glm::vec4(positions[i],1));
                float d = cpuSDF(lp);
                if(d < radii[i]){
                    const float e=1e-3f;
                    glm::vec3 N = glm::normalize(glm::vec3(
                        cpuSDF(lp+glm::vec3(e,0,0)) - cpuSDF(lp-glm::vec3(e,0,0)),
                        cpuSDF(lp+glm::vec3(0,e,0)) - cpuSDF(lp-glm::vec3(0,e,0)),
                        cpuSDF(lp+glm::vec3(0,0,e)) - cpuSDF(lp-glm::vec3(0,0,e))
                    ));
                    float pen = radii[i] - d;
                    positions[i] += N * pen;

                    glm::vec3 v0 = velocities[i];
                    glm::vec3 J = masses[i] * (glm::reflect(v0,N) * restitution - v0);
                    velocities[i] = v0 + J / masses[i];

                    // contact point:
                    glm::vec3 contact = positions[i] - N * radii[i];
                    glm::vec3 torque = glm::cross(contact - positions[i], J);
                    angVel[i] += torque / inertias[i];
                }
            }
            // 5) Integrate spin
            for(size_t i=0;i<n;++i){
                glm::quat wq(0, angVel[i]);
                glm::quat dq = wq * orientations[i] * (0.5f * dt_s);
                orientations[i] = glm::normalize(orientations[i] + dq);
            }
        }

        // — upload & render —
        rm.updateSpawns(positions, radii, ids, orientations);

        int W,H; window.getSize(W,H);
        cfg.resolution = {float(W),float(H)};
        cfg.time       = time.totalTime();
        cfg.camPos     = camPos;
        cfg.camForward = forward;
        cfg.camRight   = right;
        cfg.camUp      = upVec;

        char title[128];
        float fps = dt>0?1.0f/dt:0.0f, ms=dt*1000.0f;
        std::snprintf(title,128,"Metharizon | Mode %d | %.1f FPS | %.2f ms | %u objs",
                      mode,fps,ms,unsigned(n));
        window.setTitle(title);

        window.clear();
        rm.render(cfg, mode, glm::inverse(fractalXform));
        window.swapBuffers();
    }

    time.shutdown();
    return 0;
}
