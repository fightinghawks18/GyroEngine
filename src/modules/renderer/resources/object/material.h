//
// Created by lepag on 6/19/2025.
//

#pragma once
#include "../pipeline/pipeline.h"
#include "../texture/texture.h"

namespace GyroEngine::Resources
{
    class Material
    {
    private:
        Pipeline* m_pipeline;
        TextureHandle m_albedo;
        TextureHandle m_normal;
        TextureHandle m_metallic;
        TextureHandle m_roughness;
        TextureHandle m_ao;
    };

    using MaterialHandle = std::shared_ptr<Material>;
}
