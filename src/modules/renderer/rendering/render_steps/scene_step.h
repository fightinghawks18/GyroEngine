//
// Created by lepag on 6/10/2025.
//

#pragma once

#include "types.h"
#include "../render_step.h"
#include "resources/buffer.h"
#include "resources/pipeline.h"


class SceneStep : public IRenderStep {
public:
    ~SceneStep() override;

    void setPipeline(Pipeline* pipeline);
    void setVertexBuffer(Buffer* vertexBuffer);
    void setIndexBuffer(Buffer* indexBuffer);
    void setVertices(const std::vector<types::Vertex>& vertices);
    void setIndices(const std::vector<uint32_t>& indices);

    void execute(Renderer& renderer) override;
private:
    Pipeline* m_pipeline;
    Buffer* m_vertexBuffer;
    Buffer* m_indexBuffer;
    std::vector<types::Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

};
