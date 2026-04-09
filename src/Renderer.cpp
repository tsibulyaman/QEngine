#include "Renderer.h"
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector3D>

#ifndef SHADER_DIR
#define SHADER_DIR "./shaders"
#endif

static QVector3D idToColor(int id)
{
    return QVector3D(((id >> 16) & 0xFF) / 255.0f,
                     ((id >> 8) & 0xFF) / 255.0f,
                     (id & 0xFF) / 255.0f);
}

static bool buildProgram(QOpenGLShaderProgram& program,
                         const QString& vertexPath,
                         const QString& fragmentPath,
                         const char* debugName)
{
    if (!QFileInfo::exists(vertexPath)) {
        qWarning() << debugName << "missing vertex shader:" << vertexPath;
        return false;
    }
    if (!QFileInfo::exists(fragmentPath)) {
        qWarning() << debugName << "missing fragment shader:" << fragmentPath;
        return false;
    }

    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath)) {
        qWarning() << debugName << "vertex compile failed:" << vertexPath << program.log();
        return false;
    }
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath)) {
        qWarning() << debugName << "fragment compile failed:" << fragmentPath << program.log();
        return false;
    }
    if (!program.link()) {
        qWarning() << debugName << "link failed:" << program.log();
        return false;
    }

    qDebug() << debugName << "linked OK from" << vertexPath << "and" << fragmentPath;
    return true;
}

void Renderer::initialize()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const QString shaderDir = QString::fromUtf8(SHADER_DIR);
    const QString basicVert = QDir(shaderDir).filePath("basic.vert");
    const QString basicFrag = QDir(shaderDir).filePath("basic.frag");
    const QString pickVert = QDir(shaderDir).filePath("pick.vert");
    const QString pickFrag = QDir(shaderDir).filePath("pick.frag");

    m_basicProgram = std::make_unique<QOpenGLShaderProgram>();
    m_pickProgram = std::make_unique<QOpenGLShaderProgram>();

    const bool basicOk = buildProgram(*m_basicProgram, basicVert, basicFrag, "basicProgram");
    const bool pickOk = buildProgram(*m_pickProgram, pickVert, pickFrag, "pickProgram");

    if (!basicOk || !pickOk) {
        qWarning() << "Shader initialization failed. SHADER_DIR =" << shaderDir;
        qWarning() << "Current working directory =" << QDir::currentPath();
    }

    initCube();
    initGrid();
    resize(m_width, m_height);
}

void Renderer::cleanup()
{
    m_cubeVbo.destroy();
    m_cubeVao.destroy();
    m_gridVbo.destroy();
    m_gridVao.destroy();
    m_pickFbo.reset();
    m_basicProgram.reset();
    m_pickProgram.reset();
}

void Renderer::resize(int w, int h)
{
    m_width = qMax(1, w);
    m_height = qMax(1, h);
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::Depth);
    fmt.setInternalTextureFormat(GL_RGBA8);
    m_pickFbo = std::make_unique<QOpenGLFramebufferObject>(m_width, m_height, fmt);
}

void Renderer::initCube()
{
    const float v[] = {
        -0.5f,-0.5f, 0.5f, 0,0,1,  0.5f,-0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1,  0.5f, 0.5f, 0.5f, 0,0,1, -0.5f, 0.5f, 0.5f, 0,0,1,
        -0.5f,-0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, 0.5f, 0.5f,-0.5f, 0,0,-1,
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0.5f, 0.5f,-0.5f, 0,0,-1, 0.5f,-0.5f,-0.5f, 0,0,-1,
        -0.5f, 0.5f,-0.5f, 0,1,0, -0.5f, 0.5f, 0.5f, 0,1,0, 0.5f, 0.5f, 0.5f, 0,1,0,
        -0.5f, 0.5f,-0.5f, 0,1,0, 0.5f, 0.5f, 0.5f, 0,1,0, 0.5f, 0.5f,-0.5f, 0,1,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f, 0.5f, 0,-1,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0,
         0.5f,-0.5f,-0.5f, 1,0,0,  0.5f, 0.5f,-0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,
         0.5f,-0.5f,-0.5f, 1,0,0,  0.5f, 0.5f, 0.5f, 1,0,0,  0.5f,-0.5f, 0.5f, 1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f,-0.5f, 0.5f,-1,0,0, -0.5f, 0.5f, 0.5f,-1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0, -0.5f, 0.5f, 0.5f,-1,0,0, -0.5f, 0.5f,-0.5f,-1,0,0
    };

    m_cubeVertexCount = 36;
    m_cubeVao.create();
    m_cubeVao.bind();
    m_cubeVbo.create();
    m_cubeVbo.bind();
    m_cubeVbo.allocate(v, sizeof(v));

    if (m_basicProgram && m_basicProgram->isLinked()) {
        m_basicProgram->bind();
        m_basicProgram->enableAttributeArray(0);
        m_basicProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
        m_basicProgram->enableAttributeArray(1);
        m_basicProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));
        m_basicProgram->release();
    }

    if (m_pickProgram && m_pickProgram->isLinked()) {
        m_pickProgram->bind();
        m_pickProgram->enableAttributeArray(0);
        m_pickProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
        m_pickProgram->release();
    }

    m_cubeVao.release();
    m_cubeVbo.release();
}

void Renderer::initGrid()
{
    QVector<float> verts;
    const int half = 20;
    for (int i = -half; i <= half; ++i) {
        float f = float(i);
        verts << -half << 0.f << f <<  half << 0.f << f;
        verts << f << 0.f << -half <<  f << 0.f << half;
    }
    m_gridVertexCount = verts.size() / 3;

    m_gridVao.create();
    m_gridVao.bind();
    m_gridVbo.create();
    m_gridVbo.bind();
    m_gridVbo.allocate(verts.constData(), verts.size() * int(sizeof(float)));

    if (m_basicProgram && m_basicProgram->isLinked()) {
        m_basicProgram->bind();
        m_basicProgram->enableAttributeArray(0);
        m_basicProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
        m_basicProgram->release();
    }

    m_gridVao.release();
    m_gridVbo.release();
}

QMatrix4x4 Renderer::modelMatrixForEntity(const Entity& entity) const
{
    QMatrix4x4 model;
    model.translate(entity.transform.position);
    model.rotate(entity.transform.rotationEuler.x(), 1.f, 0.f, 0.f);
    model.rotate(entity.transform.rotationEuler.y(), 0.f, 1.f, 0.f);
    model.rotate(entity.transform.rotationEuler.z(), 0.f, 0.f, 1.f);
    model.scale(entity.transform.scale);
    return model;
}

void Renderer::renderGrid(const QMatrix4x4& view, const QMatrix4x4& proj)
{
    if (!m_basicProgram || !m_basicProgram->isLinked())
        return;

    m_basicProgram->bind();
    QMatrix4x4 model;
    m_basicProgram->setUniformValue("uModel", model);
    m_basicProgram->setUniformValue("uView", view);
    m_basicProgram->setUniformValue("uProj", proj);
    m_basicProgram->setUniformValue("uColor", QVector3D(0.32f, 0.34f, 0.38f));
    m_basicProgram->setUniformValue("uLightDir", QVector3D(0.4f, 1.0f, 0.25f));
    m_basicProgram->setUniformValue("uUseLighting", 0);
    glDisable(GL_CULL_FACE);
    m_gridVao.bind();
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    m_gridVao.release();
    glEnable(GL_CULL_FACE);
    m_basicProgram->release();
}

void Renderer::renderEntities(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj, bool pickingPass)
{
    auto* program = pickingPass ? m_pickProgram.get() : m_basicProgram.get();
    if (!program || !program->isLinked())
        return;

    program->bind();
    m_cubeVao.bind();

    for (const auto& e : scene.entities()) {
        QMatrix4x4 model = modelMatrixForEntity(e);
        program->setUniformValue("uModel", model);
        program->setUniformValue("uView", view);
        program->setUniformValue("uProj", proj);

        if (pickingPass) {
            program->setUniformValue("uIdColor", idToColor(e.id));
        } else {
            QVector3D color = e.selected ? QVector3D(1.0f, 0.72f, 0.22f) : QVector3D(0.72f, 0.76f, 0.85f);
            program->setUniformValue("uColor", color);
            program->setUniformValue("uLightDir", QVector3D(0.4f, 1.0f, 0.25f));
            program->setUniformValue("uUseLighting", 1);
        }

        glDrawArrays(GL_TRIANGLES, 0, m_cubeVertexCount);
    }

    m_cubeVao.release();
    program->release();
}

void Renderer::renderScene(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj)
{
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderGrid(view, proj);
    renderEntities(scene, view, proj, false);
}

int Renderer::pickEntityId(const Scene& scene, const QMatrix4x4& view, const QMatrix4x4& proj, int mouseX, int mouseY, int viewportHeight)
{
    if (!m_pickFbo || !m_pickProgram || !m_pickProgram->isLinked())
        return -1;

    GLfloat oldClearColor[4] = {0.f, 0.f, 0.f, 1.f};
    glGetFloatv(GL_COLOR_CLEAR_VALUE, oldClearColor);

    m_pickFbo->bind();
    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderEntities(scene, view, proj, true);

    unsigned char pixel[4] = {0, 0, 0, 0};
    const int readY = qBound(0, viewportHeight - mouseY - 1, viewportHeight - 1);
    glReadPixels(qBound(0, mouseX, m_width - 1), readY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    m_pickFbo->release();

    glClearColor(oldClearColor[0], oldClearColor[1], oldClearColor[2], oldClearColor[3]);

    return (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
}
