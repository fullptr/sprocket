#include <Sprocket.h>
#include "WorldLayer.h"
#include "EditorUI.h"
#include "EscapeMenu.h"

int main()
{
    using namespace Sprocket;
    Log::Init();

    Window window("Workshop");
    ModelManager modelManager;
    TextureManager textureManager;

    App app;

    window.SetCallback([&app](Event& event) {
        app.OnEvent(event);
    });

    CoreSystems core;
    core.window = &window;
    core.modelManager = &modelManager;
    core.textureManager = &textureManager;

    auto worldLayer = app.Add<WorldLayer>(core);
    auto editorUi = app.Add<EditorUI>(core, worldLayer.get());
    app.Add<EscapeMenu>(core, worldLayer.get(), editorUi.get());

    Stopwatch watch;
    watch.PrintFramerate(false);

    while (window.Running()) {
        window.Clear();
        
        watch.OnUpdate();
        app.OnUpdate(watch.DeltaTime());

        window.OnUpdate();
    }

    return 0;
}