#pragma once

#include <vector>

#include <Engine/Core/Runtime/Graphics/IRenderer.hpp>

namespace engine::renderer {
    struct SimpleRenderer : public core::runtime::graphics::IRenderer {
         bool Initialize(core::runtime::graphics::IGraphicsContext* gfxCtx) override;
         void Destroy() override;

         void BeginFrame() override;
         void EndFrame() override;

         void UseCamera(core::runtime::graphics::ICamera* camera) override;
         void SubmitMesh(const core::runtime::graphics::MeshRenderItem& mesh) override;
         void SubmitUI(const core::runtime::graphics::UIRenderItem& ui) override;
    protected:
        core::runtime::graphics::IGraphicsContext* m_GraphicsContext;
        core::runtime::graphics::ICamera* m_CurrentCamera;

        std::unique_ptr<core::runtime::graphics::ITexture> m_WhitePixel;
        std::unique_ptr<core::runtime::graphics::IShaderProgram> m_RendererShader;
        std::unique_ptr<core::runtime::graphics::IShaderProgram> m_UIRendererShader;
        std::unique_ptr<core::runtime::graphics::IVertexBuffer> m_UIVertexBuffer;

        std::vector<core::runtime::graphics::MeshRenderItem> m_MeshQueue;
        std::vector<core::runtime::graphics::UIRenderItem> m_UIQueue;

        bool b_InFrame;
        bool b_SetTexUniform;
    };
}