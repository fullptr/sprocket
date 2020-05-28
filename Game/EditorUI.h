#pragma once
#include <Sprocket.h>

#include "WorldLayer.h"

class EditorUI : public Sprocket::Layer
{
    WorldLayer* d_worldLayer;

    Sprocket::DeveloperUI d_editorUI;

    Sprocket::ModelManager* d_modelManager;

public:
    EditorUI(const Sprocket::CoreSystems& core, WorldLayer* worldLayer);

    void handleEvent(Sprocket::Event& event) override;
    void update(float dt) override;

    friend class EscapeMenu;
};