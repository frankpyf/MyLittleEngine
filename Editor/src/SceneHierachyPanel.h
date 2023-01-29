#include "MLE.h"

namespace editor{
    class SceneHierachyPanel
    {
    public:
        SceneHierachyPanel() = default;
        SceneHierachyPanel(const std::shared_ptr<engine::Scene>& in_scene);

        void SetScene(const std::shared_ptr<engine::Scene>& in_scene);

        void OnUIRender();
    private:

    private:
        std::shared_ptr<engine::Scene> current_scene_;
        engine::Entity selected_entity_;
    };
}