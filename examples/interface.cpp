#include <stdio.h>

#include <chrono>
#include <functional>

#include <nve.h>
#include <vulkan/vulkan_handles.h>

int main(int argc, char** argv)
{
      RenderConfig renderConfig;
      renderConfig.title = "";
      Renderer renderer;
      renderer.init(renderConfig);
      const auto cube = renderer.create_default_model(DefaultModel::Cube);
      renderer.m_ecs.get_component<DynamicModel>(cube).set_fragment_shader("fragments/lamb_wmat.frag.spv");
      renderer.active_camera().m_position = Vector3{ -2.f, 0.f, 1.f };
      renderer.active_camera().m_rotation = Vector3{ 0.f, 20.f, 0.f };

      const float speed = 0.005f;

      float total = 0.f;
      while (true)
      {
            auto res = renderer.render();
            if (res == NVE_RENDER_EXIT_SUCCESS || total >= 10.f)
                  break;

            renderer.active_camera().m_position.x += speed * (renderer.get_key(GLFW_KEY_W) - renderer.get_key(GLFW_KEY_S));
            renderer.active_camera().m_position.y += speed * (renderer.get_key(GLFW_KEY_D) - renderer.get_key(GLFW_KEY_A));
            renderer.active_camera().m_position.z += speed * (renderer.get_key(GLFW_KEY_E) - renderer.get_key(GLFW_KEY_Q));
      }
      std::cout << "\n";
      renderer.clean_up();

      return 0;
}