#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Renderer/SimpleRenderer.hpp>

#include <Engine/Core/Runtime/StorageSystem.hpp>
#include <Engine/Core/Runtime/IWindow.hpp>
#include <Engine/Core/Runtime/Graphics/ICamera.hpp>

using namespace engine::core::runtime;

namespace engine::renderer {
    bool SimpleRenderer::Initialize(core::runtime::graphics::IGraphicsContext *gfxContext) {
        gfxContext->Bind();

        auto backendName = gfxContext->GetBackend()->GetIdentifier();
        std::string fShaderFile, vShaderFile, vShaderFile2D;

        if (backendName == "opengl") {
            printf("SimpleRenderer: Identified OpenGL backend!\n");

            vShaderFile = "Engine/Shaders/OpenGL/Vertex3D.glsl";
            vShaderFile2D = "Engine/Shaders/OpenGL/Vertex2D.glsl";
            fShaderFile = "Engine/Shaders/OpenGL/Fragment3D.glsl";

            b_SetTexUniform = true;
        } else if (backendName == "dx9") {
            printf("SimpleRenderer: Identified DirectX 9 backend!\n");

            vShaderFile = "Engine/Shaders/DirectX/Vertex3D.hlsl";
            vShaderFile2D = "Engine/Shaders/DirectX/Vertex2D.hlsl";
            fShaderFile = "Engine/Shaders/DirectX/Pixel3D.hlsl";
        } else {
            printf("SimpleRenderer: Unknown '%s' backend! Support must be added!\n", backendName.c_str());
            return false;
        }

        auto vShader = gfxContext->GetBackend()->CreateShader();

        if (!vShader) {
            printf("SimpleRenderer: Could not create vertex shader!\n");
            return false;
        }

        auto vShader2D = gfxContext->GetBackend()->CreateShader();

        if (!vShader2D) {
            printf("SimpleRenderer: Could not create 2D vertex shader!\n");
            return false;
        }

        auto fShader = gfxContext->GetBackend()->CreateShader();

        if (!fShader) {
            printf("SimpleRenderer: Could not create fragment shader!\n");
            return false;
        }

        auto fShader2D = gfxContext->GetBackend()->CreateShader();

        if (!fShader2D) {
            printf("SimpleRenderer: Could not create 2D fragment shader!\n");
            return false;
        }

        vShader->SetSource(StorageSystem::ReadFileString(vShaderFile), core::runtime::graphics::SHADER_TYPE_VERTEX);
        vShader2D->SetSource(StorageSystem::ReadFileString(vShaderFile2D), core::runtime::graphics::SHADER_TYPE_VERTEX);
        fShader->SetSource(StorageSystem::ReadFileString(fShaderFile), core::runtime::graphics::SHADER_TYPE_FRAGMENT);
        fShader2D->SetSource(StorageSystem::ReadFileString(fShaderFile), core::runtime::graphics::SHADER_TYPE_FRAGMENT);

        if (!vShader->Compile() || !vShader2D->Compile() || !fShader->Compile() || !fShader2D->Compile()) {
            printf("SimpleRenderer: Failed to compile one or more of the shaders!\n");

            vShader->Destroy();
            vShader2D->Destroy();
            fShader->Destroy();
            fShader2D->Destroy();

            return false;
        }

        m_RendererShader = std::move(gfxContext->GetBackend()->CreateShaderProgram());

        if (!m_RendererShader) {
            printf("SimpleRenderer: Could not create shader program!\n");
            return false;
        }

        m_UIRendererShader = std::move(gfxContext->GetBackend()->CreateShaderProgram());

        if (!m_UIRendererShader) {
            printf("SimpleRenderer: Could not create UI shader program!\n");
            return false;
        }

        m_RendererShader->AddShader(std::move(vShader));
        m_RendererShader->AddShader(std::move(fShader));

        m_UIRendererShader->AddShader(std::move(vShader2D));
        m_UIRendererShader->AddShader(std::move(fShader2D));

        if (!m_RendererShader->Link()) {
            printf("SimpleRenderer: Failed to compile/link renderer shader!\n");
            goto COMPILE_FAIL_CLEANUP;
        }

        if (!m_UIRendererShader->Link()) {
            printf("SimpleRenderer: Failed to compile/link UI renderer shader!\n");
            goto COMPILE_FAIL_CLEANUP;
        }

        m_UIVertexBuffer = std::move(gfxContext->GetBackend()->CreateVertexBuffer());

        if (!m_UIVertexBuffer) {
            printf("SimpleRenderer: Could not create UI vertex buffer!\n");
            goto COMPILE_FAIL_CLEANUP;
        }

        m_WhitePixel = gfxContext->GetBackend()->CreateTexture();

        if (!m_WhitePixel) {
            printf("SimpleRenderer: Could not create white pixel texture!\n");
            goto COMPILE_FAIL_CLEANUP;
        }

        if (!m_WhitePixel->Create(
                {
                    {
                        core::runtime::graphics::colors::white,
                        core::runtime::graphics::colors::white,
                        core::runtime::graphics::colors::white,
                        core::runtime::graphics::colors::white
                    },
                    {1.f, 1.f}
                }
        )) {
            printf("SimpleRenderer: Could not upload white pixel texture!\n");
COMPILE_FAIL_CLEANUP:
            if(m_RendererShader) {
                m_RendererShader->Destroy();
            }

            if(m_UIRendererShader) {
                m_UIRendererShader->Destroy();
            }

            if(m_WhitePixel) {
                m_WhitePixel->Destroy();
            }

            return false;
        }

        m_GraphicsContext = gfxContext;

        return true;
    }

    void SimpleRenderer::Destroy() {
        if (m_RendererShader) {
            m_RendererShader->Destroy();
            m_RendererShader = nullptr;
        }

        if (m_UIRendererShader) {
            m_UIRendererShader->Destroy();
            m_UIRendererShader = nullptr;
        }

        if (m_WhitePixel) {
            m_WhitePixel->Destroy();
            m_WhitePixel = nullptr;
        }
    }

    void SimpleRenderer::BeginFrame() {
        if (b_InFrame) {
            printf("SimpleRenderer: Cannot call BeginFrame when you are already in a frame!\n");
            return;
        }

        b_InFrame = true;
    }

    void SimpleRenderer::UseCamera(core::runtime::graphics::ICamera* camera) {
        if (!b_InFrame) {
            printf("SimpleRenderer: Cannot call UseCamera when you didn't even begin a frame!\n");
            return;
        }

        m_CurrentCamera = camera;
        m_CurrentCamera->Update(m_GraphicsContext->GetOwnerWindow()->GetSize());
    }

    void SimpleRenderer::EndFrame() {
        if (!b_InFrame) {
            printf("SimpleRenderer: Cannot call EndFrame when you didn't even begin a frame!\n");
            return;
        }

        if (!m_RendererShader) {
            printf("SimpleRenderer: Renderer not initialized!\n");
            return;
        }

        if (!m_UIRendererShader) {
            printf("SimpleRenderer: UI Renderer not initialized!\n");
            return;
        }

        // if we have a 3D camera set for this scene, we render stuff
        if(m_CurrentCamera) {
            m_RendererShader->Bind();

            // set uniform for texture sampler ID (to be investigated, only applies to GL for now)
            if (b_SetTexUniform) {
                m_RendererShader->SetUniformI("sTexture", 0);
            }

            auto idMatrix = glm::identity<glm::mat4x4>();

            m_RendererShader->SetUniformMat4("ufProjMatrix", m_CurrentCamera->GetProjectionMtx());
            m_RendererShader->SetUniformMat4("ufViewMatrix", m_CurrentCamera->GetViewMtx());

            // draw 3D meshes
            for (auto &buf: m_MeshQueue) {
                m_WhitePixel->Bind(0);

                auto model = glm::translate(glm::mat4(1.0f), buf.m_Position.ToGlmVec());
                model = glm::rotate(model, glm::radians(buf.m_Rotation.x), glm::vec3(1.0f, 0.0, 0.0));
                model = glm::rotate(model, glm::radians(buf.m_Rotation.y), glm::vec3(0.0, 1.0f, 0.0));
                model = glm::rotate(model, glm::radians(buf.m_Rotation.z), glm::vec3(0.0, 0.0, 1.0f));
                model = glm::scale(model, buf.m_Scale.ToGlmVec());

                m_RendererShader->SetUniformMat4("ufModelMatrix", model);

                buf.m_Buffer->Bind();
                buf.m_Buffer->Draw();
                buf.m_Buffer->Unbind();

                m_WhitePixel->Unbind();
            }

            m_RendererShader->Unbind();
        } else {
            printf("SimpleRenderer: No camera assigned to this frame; 3D Rendering is skipped!\n");
        }

        // if we have UI items in the queue, we render them
        if(!m_UIQueue.empty()) {
            // set viewport
            auto size = m_GraphicsContext->GetOwnerWindow()->GetSize();
            auto ortho = glm::orthoZO(0.f, (float) size.x, (float) size.y, 0.f, -1.f, 1.f);
            m_UIRendererShader->SetUniformMat4("ufProjMatrix", ortho);

            // set uniform for texture sampler ID (to be investigated, only applies to GL for now)
            if (b_SetTexUniform) {
                m_UIRendererShader->SetUniformI("sTexture", 0);
            }

            m_UIRendererShader->Bind();

            // enable alpha blending for UI elements
            m_GraphicsContext->GetBackend()->EnableFeatures(core::runtime::graphics::BACKEND_FEATURE_ALPHA_BLENDING);
            m_GraphicsContext->GetBackend()->EnableFeatures(core::runtime::graphics::BACKEND_FEATURE_SCISSOR_TEST);

            // draw UI meshes
            for (auto &bufUI: m_UIQueue) {
                // set scissoring
                m_GraphicsContext->GetBackend()->SetScissor(bufUI.m_Scissor, bufUI.m_Size);

                // render buffer
                m_UIVertexBuffer->Upload(bufUI.m_Vertices, bufUI.m_PrimType,
                                         core::runtime::graphics::BUFFER_USAGE_HINT_STREAM);

                m_UIVertexBuffer->Bind();

                if (bufUI.m_Texture) {
                    bufUI.m_Texture->Bind(0);
                } else {
                    printf("SimpleRenderer: UI element has no texture! Defaulting to White Pixel!\n");
                    m_WhitePixel->Bind(0);
                }

                m_UIVertexBuffer->Draw();

                if (bufUI.m_Texture) {
                    bufUI.m_Texture->Unbind();
                } else {
                    m_WhitePixel->Unbind();
                }
            }

            m_UIVertexBuffer->Unbind();
            m_UIRendererShader->Unbind();

            m_GraphicsContext->GetBackend()->DisableFeatures(core::runtime::graphics::BACKEND_FEATURE_ALPHA_BLENDING);
            m_GraphicsContext->GetBackend()->DisableFeatures(core::runtime::graphics::BACKEND_FEATURE_SCISSOR_TEST);
        }

        m_MeshQueue.clear();
        m_UIQueue.clear();

        b_InFrame = false;
        m_CurrentCamera = nullptr;
    }

    void SimpleRenderer::SubmitMesh(const core::runtime::graphics::MeshRenderItem &meshItem) {
        if (!b_InFrame) {
            printf("SimpleRenderer: Cannot submit buffer when the frame did not begin!\n");
            return;
        }

        m_MeshQueue.emplace_back(meshItem);
    }

    void SimpleRenderer::SubmitUI(const core::runtime::graphics::UIRenderItem &uiItem) {
        if (!b_InFrame) {
            printf("SimpleRenderer: Cannot submit UI buffer when the frame did not begin!\n");
            return;
        }

        m_UIQueue.emplace_back(uiItem);
    }
}