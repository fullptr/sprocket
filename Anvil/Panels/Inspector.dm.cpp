#include "Inspector.h"
#include "Anvil.h"
#include "ImGuiXtra.h"
#include "Maths.h"
#include "Components.h"
#include "DevUI.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Sprocket {
namespace {

void ShowGuizmo(
    Anvil& editor,
    TransformComponent& c,
    ImGuizmo::OPERATION mode,
    ImGuizmo::MODE coords,
    glm::vec3* snap = nullptr)
{
    if (!editor.IsGameRunning()) {
        auto& camera = editor.GetEditorCamera();
        auto tr = Maths::Transform(c.position, c.orientation, c.scale);
        ImGuizmo::Manipulate(
            glm::value_ptr(camera.View()),
            glm::value_ptr(camera.Proj()),
            mode,
            coords,
            glm::value_ptr(tr),
            nullptr,
            &snap->x
        );
        Maths::Decompose(tr, &c.position, &c.orientation, &c.scale);
    }
}

}

void Inspector::Show(Anvil& editor)
{
    ECS::Entity entity = editor.Selected();

    if (!editor.Selected().Valid()) {
        if (ImGui::Button("New Entity")) {
            auto e = editor.GetScene()->Entities().New();
            editor.SetSelected(e);
        }
        return;
    }
    int count = 0;

#ifdef DATAMATIC_BLOCK
    if (entity.Has<{{Comp.Name}}>()) {
        auto& c = entity.Get<{{Comp.Name}}>();
        if (ImGui::CollapsingHeader("{{Comp.DisplayName}}")) {
            ImGui::PushID(count++);
            {{Attr.Inspector.Display}};
            {{Comp.Inspector.GuizmoSettings}}
            if (ImGui::Button("Delete")) { entity.Remove<{{Comp.Name}}>(); }
            ImGui::PopID();
        }
        {{Comp.Inspector.Guizmo}}
    }

#endif
    ImGui::Separator();

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("missing_components_list");
    }

    if (ImGui::BeginPopup("missing_components_list")) {
#ifdef DATAMATIC_BLOCK
        if (!entity.Has<{{Comp.Name}}>() && ImGui::Selectable("{{Comp.DisplayName}}")) {
            {{Comp.Name}} c;
            entity.Add<{{Comp.Name}}>(c);
        }
#endif
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::Button("Duplicate")) {
        ECS::Entity copy = Loader::Copy(&editor.GetScene()->Entities(), entity);
        editor.SetSelected(copy);
    }
    if (ImGui::Button("Delete Entity")) {
        entity.Delete();
        editor.ClearSelected();
    }
}
    
}
