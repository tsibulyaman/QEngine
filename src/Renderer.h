#pragma once
#include <memory>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include "Scene.h"

class Renderer : protected QOpenGLFunctions
{
public:
    void initialize();
    void cleanup();
    void resize(int w, int h);
    void renderScene(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj);
    int pickEntityId(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj, int mouseX, int mouseY, int viewportHeight);

private:
    void initCube();
    void initGrid();
    void renderGrid(const QMatrix4x4& view, const QMatrix4x4& proj);
    void renderEntities(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj, bool pickingPass);
    QMatrix4x4 modelMatrixForEntity(const Entity& entity) const;

    std::unique_ptr<QOpenGLShaderProgram> m_basicProgram;
    std::unique_ptr<QOpenGLShaderProgram> m_pickProgram;
    QOpenGLVertexArrayObject m_cubeVao;
    QOpenGLBuffer m_cubeVbo {QOpenGLBuffer::VertexBuffer};
    int m_cubeVertexCount = 0;

    QOpenGLVertexArrayObject m_gridVao;
    QOpenGLBuffer m_gridVbo {QOpenGLBuffer::VertexBuffer};
    int m_gridVertexCount = 0;

    std::unique_ptr<QOpenGLFramebufferObject> m_pickFbo;
    int m_width = 1;
    int m_height = 1;
};
