#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <chrono>
#include <numeric>
#include <sstream>

#include <nve.h>

#include "pbd.h"

float turningSpeed = 0.5f;
float moveSpeed = 2.f;
float camDistance = 5;
Vector3 camPos = { 0,0,0 };
Vector3 camRot = { 0,0,0 };

void str_camera_movement(Renderer& renderer, Camera& camera)
{
    Vector3 forward = { cos(glm::radians(camRot.z)), sin(glm::radians(camRot.z)), 0 };
    Vector3 right = { -sin(glm::radians(camRot.z)), cos(glm::radians(camRot.z)), 0 };

    if (renderer.get_key(GLFW_KEY_W) == GLFW_PRESS)
        camPos += forward * moveSpeed;
    if (renderer.get_key(GLFW_KEY_S) == GLFW_PRESS)
        camPos -= forward * moveSpeed;

    if (renderer.get_key(GLFW_KEY_A) == GLFW_PRESS)
        camPos -= right * moveSpeed;
    if (renderer.get_key(GLFW_KEY_D) == GLFW_PRESS)
        camPos += right * moveSpeed;

    if (renderer.get_key(GLFW_KEY_E) == GLFW_PRESS)
        camPos.z += moveSpeed;
    if (renderer.get_key(GLFW_KEY_Q) == GLFW_PRESS)
        camPos.z -= moveSpeed;

    if (renderer.get_key(GLFW_KEY_UP) == GLFW_PRESS)
        camRot.y -= turningSpeed;
    if (renderer.get_key(GLFW_KEY_DOWN) == GLFW_PRESS)
        camRot.y += turningSpeed;

    if (renderer.get_key(GLFW_KEY_LEFT) == GLFW_PRESS)
        camRot.z -= turningSpeed;
    if (renderer.get_key(GLFW_KEY_RIGHT) == GLFW_PRESS)
        camRot.z += turningSpeed;

    camRot.y = math::clamp(camRot.y, 0, 90);

    camera.m_position = camPos + Vector3 { -cos(glm::radians(camRot.z)), -sin(glm::radians(camRot.z)), sin(glm::radians(camRot.y))} * camDistance;
    camera.m_rotation = { camRot.x, camRot.y, camRot.z };
}
void fps_camera_movement(Renderer& renderer, Camera& camera)
{
    Vector3 forward = { cos(glm::radians(camera.m_rotation.z)), sin(glm::radians(camera.m_rotation.z)), 0 };
    Vector3 right = { -sin(glm::radians(camera.m_rotation.z)), cos(glm::radians(camera.m_rotation.z)), 0 };

    if (renderer.get_key(GLFW_KEY_W) == GLFW_PRESS)
        camera.m_position += forward * moveSpeed;
    if (renderer.get_key(GLFW_KEY_S) == GLFW_PRESS)
        camera.m_position -= forward * moveSpeed;

    if (renderer.get_key(GLFW_KEY_A) == GLFW_PRESS)
        camera.m_position -= right * moveSpeed;
    if (renderer.get_key(GLFW_KEY_D) == GLFW_PRESS)
        camera.m_position += right * moveSpeed;

    if (renderer.get_key(GLFW_KEY_E) == GLFW_PRESS)
        camera.m_position.z += moveSpeed * 10;
    if (renderer.get_key(GLFW_KEY_Q) == GLFW_PRESS)
        camera.m_position.z -= moveSpeed * 10;

    if (renderer.get_key(GLFW_KEY_UP) == GLFW_PRESS)
        camera.m_rotation.y -= turningSpeed;
    if (renderer.get_key(GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.m_rotation.y += turningSpeed;

    if (renderer.get_key(GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.m_rotation.z -= turningSpeed;
    if (renderer.get_key(GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.m_rotation.z += turningSpeed;

    camera.m_rotation.y = math::clamp(camera.m_rotation.y, -50, 50);
}
void cart_camera_movement(Renderer& renderer, Camera& camera)
{
    if (renderer.get_key(GLFW_KEY_W) == GLFW_PRESS)
        camera.m_position += VECTOR_RIGHT * moveSpeed;
    if (renderer.get_key(GLFW_KEY_S) == GLFW_PRESS)
        camera.m_position -= VECTOR_RIGHT * moveSpeed;

    if (renderer.get_key(GLFW_KEY_A) == GLFW_PRESS)
        camera.m_position -= VECTOR_FORWARD * moveSpeed;
    if (renderer.get_key(GLFW_KEY_D) == GLFW_PRESS)
        camera.m_position += VECTOR_FORWARD * moveSpeed;

    if (renderer.get_key(GLFW_KEY_E) == GLFW_PRESS)
        camera.m_position += VECTOR_UP * moveSpeed * 10.f;
    if (renderer.get_key(GLFW_KEY_Q) == GLFW_PRESS)
        camera.m_position -= VECTOR_UP * moveSpeed * 10.f;
}

int main(int argc, char** argv)
{
    Renderer renderer;
    RenderConfig renderConfig;
    renderConfig.width = 2000;
    renderConfig.height = 1200;
    renderConfig.title = "Vulkan";
    renderConfig.dataMode = RenderConfig::Indexed;
    renderConfig.enableValidationLayers = true;
    renderConfig.clearColor = Vector3(0, 187, 233);
    renderConfig.cameraEnabled = true;
    renderConfig.autoECSUpdate = false;

    renderer.init(renderConfig);

    // Camera

    Camera camera;
    camera.m_position = Vector3(0, 0, 52.f);
    camera.m_orthographic = true;
    renderer.set_active_camera(&camera);

    int fps = 0;
    int avgFps = 0;
    std::vector<int> fpsQueue(30);
    std::string fpsText;
    std::string avgFpsText;
    float deltaTime = 0.f;
    auto lastTime = std::chrono::high_resolution_clock::now();
    uint32_t frame = 0;

    Profiler profiler;

    profiler.start_measure("complete model loading");

    DynamicModel ball;
    ball.load_mesh("/default_models/circle/quad.obj");

    DynamicModel emptycircle;
    emptycircle.load_mesh("/default_models/circle/emptycirclemesh.obj");

    // Position Based Dynamics
    PBDSystem pbd;
    renderer.m_ecs.register_system<PBDSystem>(&pbd);

    auto boundary = renderer.m_ecs.create_entity();

    float mouseParticleRadius = 5.f;
    float particleRadius = 1.f;
    float boundingParticleRadius = 50.f;

    renderer.m_ecs.add_component<Transform>(boundary).scale = Vector3(boundingParticleRadius + 2.f);
    renderer.m_ecs.add_component<DynamicModel>(boundary) = emptycircle;
    auto& bo = renderer.m_ecs.add_component<PBDParticle>(boundary);
    bo.invmass = 0.f;
    bo.radius = 0.f;

    std::vector<EntityId> particles;
    const float PARTICLE_DIST = 2.5f;

    int particleCount = 50;
    auto genParticles = [&](int count, Vec position) {
          const int sqrtPC = (int) std::sqrt(count);
          for (int i = 0; i < count; i++)
          {
                auto particle = renderer.m_ecs.create_entity();
                auto& pbdParticle = renderer.m_ecs.add_component<PBDParticle>(particle);
                pbdParticle.position = Vec(
                      (float)(i % sqrtPC) * PARTICLE_DIST - sqrtPC / 2.f + 0.01f + position.x,
                      (float)(i / sqrtPC) * PARTICLE_DIST - sqrtPC / 2.f + position.y
                );
                pbdParticle.radius = particleRadius;
                auto& transform = renderer.m_ecs.add_component<Transform>(particle);
                transform.scale = Vector3(particleRadius / 2.f);

                auto constraint = pbd.add_constraint<CollisionConstraint>({ boundary, particle }, InverseInequality);
                constraint->m_distance = boundingParticleRadius;
                constraint->m_stiffness = 1.f;
                renderer.m_ecs.add_component<DynamicModel>(particle) = ball;

                particles.push_back(particle);
          }
    };
    CollisionConstraintGenerator colConstGen;
    pbd.register_self_generating_constraint(&colConstGen);

    float profilerTime = 0.f;

    int particleCounter = 1;
    
    bool updateECS = false;
    bool singleUpdateECS = false;
    bool running = true;
    while (running)
    {
        if (deltaTime != 0)
            moveSpeed *= deltaTime;
        if (camera.m_orthographic)
            cart_camera_movement(renderer, camera);
        else
            fps_camera_movement(renderer, camera);
        if (deltaTime != 0)
            moveSpeed /= deltaTime;

        renderer.gui_begin();
        
        profiler.start_measure("total time");

        //profiler.start_measure("gui");

        // mouse interaction
        //auto mousePos = renderer.mouse_to_screen(renderer.get_mouse_pos());
        //if (renderer.get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        //{
        //      float zoom = 1.f / math::max(0.001f, camera.m_position.z + 1);
        //      float ratio = (float) renderConfig.height / renderConfig.width;
        //      Vec worldPos = Vec(
        //            mousePos.x / zoom / ratio,
        //            mousePos.y / zoom
        //      );
        //      renderer.m_ecs.get_component<PBDParticle>(particles.back()).position = worldPos;
        //      renderer.m_ecs.get_component<PBDParticle>(particles.back()).oldPosition
        //            = renderer.m_ecs.get_component<PBDParticle>(particles.back()).position;
        //}

        // GUI
        renderer.draw_engine_gui([&]() {

              ImGui::Begin("General");

              if (ImGui::Button("Generate Particles"))
              {
                    genParticles(particleCount, { 0.f, 0.f });
              }
              ImGui::DragInt("New Particle Count", &particleCount, 1.f, 0);

              if (frame >= fps / 2)
              {
                    fpsText = std::to_string(fps) + " fps";
                    avgFpsText = std::to_string(avgFps) + " fps (avg)";
                    frame = 0;

              }
              ImGui::Text(fpsText.c_str());
              ImGui::Text(avgFpsText.c_str());
              frame++;

              ImGui::SliderFloat("speed", &moveSpeed, 0.f, 4.f);
              ImGui::SliderFloat("sensitivity", &turningSpeed, 0.f, 0.5f);

              ImGui::SliderFloat3("cam pos", (float*)&camera.m_position, -10, 10);

              singleUpdateECS = ImGui::Button("step ecs");
              if (ImGui::Button("update ecs"))
                    updateECS = !updateECS;

              if (updateECS)
                    ImGui::Text("ECS activated");
              else
                    ImGui::Text("ECS deactivated");

              if (ImGui::Button("Ortho / Persp"))
              {
                    camera.m_orthographic = !camera.m_orthographic;
                    camera.m_position = Vector3(0, 0, 10.f);
              }

              ImGui::End();

              // profiler.end_measure("gui", true);
        });

        profiler.start_measure("ecs");
        if (updateECS || singleUpdateECS)
            // renderer.m_ecs.update_systems(1.f / 120.f);
            renderer.m_ecs.update_systems(deltaTime);
        //std::cout << deltaTime << '\n';

        profiler.end_measure("ecs", true);

        profiler.start_measure("render");
        NVE_RESULT res = renderer.render();
        if (res == NVE_RENDER_EXIT_SUCCESS)
            running = false;
        profiler.end_measure("total time", false);

        // time
        auto now = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastTime).count() / 1000000.f;

        fps = 1.f / deltaTime;
        fpsQueue.erase(fpsQueue.begin());
        fpsQueue.push_back(deltaTime);
        avgFps = 1.f / (std::accumulate(fpsQueue.begin(), fpsQueue.end(), 0.0) / 30.f);
        lastTime = now;

        profilerTime += deltaTime;
        if (profilerTime > 1.f)
            Profiler::print_buf();
        profiler.out_buf() << "\nprint\n";
    }

    return 0;
}