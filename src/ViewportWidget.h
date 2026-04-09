#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPoint>
#include "Renderer.h"
#include "Scene.h"

class QUndoStack;
class QRubberBand;

class ViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum class ToolMode { Move, Rotate, Scale };
    enum class SpaceMode { World, Local };

    explicit ViewportWidget(QWidget* parent = nullptr);
    ~ViewportWidget() override;

    Scene& scene();
    void selectEntity(int id);
    void frameSelected();
    void setUndoStack(QUndoStack* stack);
    void setSnapEnabled(bool enabled);
    bool snapEnabled() const;
    void setWireframeEnabled(bool enabled);
    bool wireframeEnabled() const;
    void setToolMove();
    void setToolRotate();
    void setToolScale();
    ToolMode toolMode() const;
    void setSpaceWorld();
    void setSpaceLocal();
    SpaceMode spaceMode() const;

signals:
    void selectionChanged();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void showEvent(QShowEvent* e) override;

private:
    void updateCamera();
    bool hitHandle(const QPoint& pos, int& axisOut) const;
    QPoint projectWorldToScreen(const QVector3D& p) const;
    float applySnap(float v) const;
    QString toolName() const;
    QString spaceName() const;
    QVector3D axisVector(int axis, const Transform& t) const;

    Renderer m_renderer;
    Scene m_scene;
    QPoint m_lastMousePos;
    float m_yaw = -25.f;
    float m_pitch = -20.f;
    float m_distance = 8.f;
    QVector3D m_target {0.f, 0.f, 0.f};
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;

    bool m_draggingGizmo = false;
    int m_dragAxis = -1;
    QPoint m_dragStartMouse;
    QVector3D m_dragStartPosition;
    Transform m_dragStartTransform;
    QUndoStack* m_undoStack = nullptr;
    bool m_snapEnabled = false;
    bool m_wireframeEnabled = false;
    ToolMode m_toolMode = ToolMode::Move;
    SpaceMode m_spaceMode = SpaceMode::World;

    QRubberBand* m_rubberBand = nullptr;
    QPoint m_selectionOrigin;
    bool m_boxSelecting = false;
};
