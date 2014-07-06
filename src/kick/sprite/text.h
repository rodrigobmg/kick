//
// Created by morten on 30/06/14.
//


#pragma once

#include "kick/scene/component_renderable.h"
#include "kick/scene/game_object.h"
#include "kick/sprite/font.h"
#include "kick/math/bounds2d.h"


namespace kick {
    class Font;
    class Mesh;

    class Text : public ComponentRenderable {
    public:
        Text(GameObject *gameObject);

        Font * getFont() const;

        void setFont(Font *font);

        virtual void render(EngineUniforms *engineUniforms) override;

        std::string const & getText() const;

        void setText(std::string const &text);

        Bounds2D getBounds();
    private:
        Bounds2D bounds;
        void updateVertexBuffer();
        Font *font = nullptr;
        Mesh *mesh = nullptr;
        MeshData *meshData = nullptr;
        std::string text;
    };
}


