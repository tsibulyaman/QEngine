#include "ViewportWidget.h"
#include "Commands.h"
#include <QUndoStack>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QRubberBand>
#include <QOpenGLContext>
#include <QTimer>
#include <QShowEvent>

ViewportWidget::ViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

}

ViewportWidget::~ViewportWidget()
{
    makeCurrent();
    m_renderer.cleanup();
    doneCurrent();
}

Scene& ViewportWidget::scene() { return m_scene; }
void ViewportWidget::setUndoStack(QUndoStack* stack) { m_undoStack = stack; }
void ViewportWidget::setSnapEnabled(bool enabled) { m_snapEnabled = enabled; update(); }
bool ViewportWidget::snapEnabled() const { return m_snapEnabled; }
void ViewportWidget::setWireframeEnabled(bool enabled) { m_wireframeEnabled = enabled; update(); }
bool ViewportWidget::wireframeEnabled() const { return m_wireframeEnabled; }
void ViewportWidget::setToolMove() { m_toolMode = ToolMode::Move; update(); }
void ViewportWidget::setToolRotate() { m_toolMode = ToolMode::Rotate; update(); }
void ViewportWidget::setToolScale() { m_toolMode = ToolMode::Scale; update(); }
ViewportWidget::ToolMode ViewportWidget::toolMode() const { return m_toolMode; }
void ViewportWidget::setSpaceWorld() { m_spaceMode = SpaceMode::World; update(); }
void ViewportWidget::setSpaceLocal() { m_spaceMode = SpaceMode::Local; update(); }
ViewportWidget::SpaceMode ViewportWidget::spaceMode() const { return m_spaceMode; }

QString ViewportWidget::toolName() const
{
    switch (m_toolMode) {
        case ToolMode::Move: return "Move";
        case ToolMode::Rotate: return "Rotate";
        case ToolMode::Scale: return "Scale";
    }
    return "Move";
}

QString ViewportWidget::spaceName() const
{
    return m_spaceMode == SpaceMode::World ? "World" : "Local";
}

QVector3D ViewportWidget::axisVector(int axis, const Transform& t) const
{
    QVector3D base = axis == 0 ? QVector3D(1,0,0) : axis == 1 ? QVector3D(0,1,0) : QVector3D(0,0,1);
    if (m_spaceMode == SpaceMode::World) return base;
    QMatrix4x4 m;
    m.rotate(t.rotationEuler.x(), 1.f, 0.f, 0.f);
    m.rotate(t.rotationEuler.y(), 0.f, 1.f, 0.f);
    m.rotate(t.rotationEuler.z(), 0.f, 0.f, 1.f);
    return (m * QVector4D(base, 0.0f)).toVector3D().normalized();
}

void ViewportWidget::selectEntity(int id)
{
    m_scene.selectById(id);
    emit selectionChanged();
    update();
}

void ViewportWidget::frameSelected()
{
    if (auto* e = m_scene.selectedEntity()) {
        m_target = e->transform.position;
        float maxScale = qMax(e->transform.scale.x(), qMax(e->transform.scale.y(), e->transform.scale.z()));
        m_distance = qMax(2.5f, maxScale * 4.0f);
        update();
    }
}

void ViewportWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepthf(1.0f);
    m_renderer.initialize();
    updateCamera();
}

void ViewportWidget::resizeGL(int w, int h)
{
    const int fbW = qMax(1, int(w * devicePixelRatioF()));
    const int fbH = qMax(1, int(h * devicePixelRatioF()));
    m_proj.setToIdentity();
    m_proj.perspective(60.f, float(w) / float(h > 0 ? h : 1), 0.1f, 1000.f);
    m_renderer.resize(fbW, fbH);
}

void ViewportWidget::paintGL()
{
    updateCamera();
    glViewport(0, 0, int(width() * devicePixelRatioF()), int(height() * devicePixelRatioF()));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_wireframeEnabled) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_renderer.renderScene(m_scene, m_view, m_proj);
    if (m_wireframeEnabled) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void ViewportWidget::showEvent(QShowEvent* e)
{
    QOpenGLWidget::showEvent(e);
    QTimer::singleShot(0, this, [this]() {
        update();
    });
}

void ViewportWidget::paintEvent(QPaintEvent* e)
{
    QOpenGLWidget::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QColor(220, 220, 230));
    QString flags = QString("Tool:%1  Space:%2  Snap:%3  Wire:%4")
        .arg(toolName()).arg(spaceName())
        .arg(m_snapEnabled ? "On" : "Off")
        .arg(m_wireframeEnabled ? "On" : "Off");
    p.drawText(12, 24, "Viewport: LMB select/tool, drag empty area for box select, RMB orbit, wheel zoom, W/E/R tools, G/L space");
    p.drawText(12, 44, flags);

    if (auto* entity = m_scene.selectedEntity()) {
        const QVector3D origin = entity->transform.position;
        const float gizmoLen = qMax(0.9f, m_distance * 0.12f);
        QPoint c = projectWorldToScreen(origin);
        QPoint xh = projectWorldToScreen(origin + axisVector(0, entity->transform) * gizmoLen);
        QPoint yh = projectWorldToScreen(origin + axisVector(1, entity->transform) * gizmoLen);
        QPoint zh = projectWorldToScreen(origin + axisVector(2, entity->transform) * gizmoLen);

        if (m_toolMode == ToolMode::Move || m_toolMode == ToolMode::Scale) {
            p.setPen(QPen(QColor(230, 70, 70), 3)); p.setBrush(QColor(230, 70, 70)); p.drawLine(c, xh); p.drawEllipse(xh, 6, 6);
            p.setPen(QPen(QColor(80, 220, 120), 3)); p.setBrush(QColor(80, 220, 120)); p.drawLine(c, yh); p.drawEllipse(yh, 6, 6);
            p.setPen(QPen(QColor(90, 150, 255), 3)); p.setBrush(QColor(90, 150, 255)); p.drawLine(c, zh); p.drawEllipse(zh, 6, 6);
        } else {
            int r1 = 28, r2 = 40, r3 = 52;
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(230, 70, 70), 3)); p.drawEllipse(c, r1, r1);
            p.setPen(QPen(QColor(80, 220, 120), 3)); p.drawEllipse(c, r2, r2);
            p.setPen(QPen(QColor(90, 150, 255), 3)); p.drawEllipse(c, r3, r3);
        }
    }
}

void ViewportWidget::updateCamera()
{
    QMatrix4x4 rot;
    rot.rotate(m_yaw, 0.f, 1.f, 0.f);
    rot.rotate(m_pitch, 1.f, 0.f, 0.f);
    QVector3D dir = rot.map(QVector3D(0.f, 0.f, -1.f));
    QVector3D eye = m_target - dir * m_distance;
    m_view.setToIdentity();
    m_view.lookAt(eye, m_target, QVector3D(0.f, 1.f, 0.f));
}

QPoint ViewportWidget::projectWorldToScreen(const QVector3D& p) const
{
    QVector4D clip = m_proj * m_view * QVector4D(p, 1.0f);
    if (qFuzzyIsNull(clip.w())) return QPoint(-10000, -10000);

    QVector3D ndc = clip.toVector3DAffine();

    const float viewAspect = 16.0f / 9.0f;
    const float widgetAspect = height() > 0 ? float(width()) / float(height()) : viewAspect;

    QRect vp;
    if (widgetAspect > viewAspect) {
        const int vpW = int(height() * viewAspect);
        const int x = (width() - vpW) / 2;
        vp = QRect(x, 0, vpW, height());
    } else {
        const int vpH = int(width() / viewAspect);
        const int y = (height() - vpH) / 2;
        vp = QRect(0, y, width(), vpH);
    }

    const int sx = vp.left() + int((ndc.x() * 0.5f + 0.5f) * vp.width());
    const int sy = vp.top() + int((1.0f - (ndc.y() * 0.5f + 0.5f)) * vp.height());
    return QPoint(sx, sy);
}

bool ViewportWidget::hitHandle(const QPoint& pos, int& axisOut) const
{
    auto* entity = m_scene.selectedEntity();
    if (!entity) return false;

    const QVector3D origin = entity->transform.position;
    const float gizmoLen = qMax(0.9f, m_distance * 0.12f);
    QPoint c = projectWorldToScreen(origin);

    if (m_toolMode == ToolMode::Rotate) {
        int d = (c - pos).manhattanLength();
        if (d < 34 && d > 18) { axisOut = 0; return true; }
        if (d < 46 && d > 26) { axisOut = 1; return true; }
        if (d < 58 && d > 38) { axisOut = 2; return true; }
        return false;
    }

    QPoint xh = projectWorldToScreen(origin + axisVector(0, entity->transform) * gizmoLen);
    QPoint yh = projectWorldToScreen(origin + axisVector(1, entity->transform) * gizmoLen);
    QPoint zh = projectWorldToScreen(origin + axisVector(2, entity->transform) * gizmoLen);
    auto nearPt = [&](const QPoint& a) { return (a - pos).manhattanLength() < 16; };
    if (nearPt(xh)) { axisOut = 0; return true; }
    if (nearPt(yh)) { axisOut = 1; return true; }
    if (nearPt(zh)) { axisOut = 2; return true; }
    return false;
}

float ViewportWidget::applySnap(float v) const
{
    if (!m_snapEnabled) return v;
    const float step = (m_toolMode == ToolMode::Rotate) ? 15.0f : 0.5f;
    return std::round(v / step) * step;
}

void ViewportWidget::mousePressEvent(QMouseEvent* e)
{
    m_lastMousePos = e->pos();
    if (e->button() == Qt::LeftButton) {
        int axis = -1;
        if (hitHandle(e->pos(), axis)) {
            if (auto* entity = m_scene.selectedEntity()) {
                m_draggingGizmo = true;
                m_dragAxis = axis;
                m_dragStartMouse = e->pos();
                m_dragStartPosition = entity->transform.position;
                m_dragStartTransform = entity->transform;
                return;
            }
        }

        makeCurrent();
        const int px = int(e->position().x() * devicePixelRatioF());
        const int py = int(e->position().y() * devicePixelRatioF());
        const int fbH = int(height() * devicePixelRatioF());
        int pickedId = m_renderer.pickEntityId(m_scene, m_view, m_proj, px, py, fbH);
        if (pickedId > 0) {
            if (!(e->modifiers() & Qt::ControlModifier))
                m_scene.clearSelection();
            if (auto* pe = m_scene.findById(pickedId))
                pe->selected = true;
            emit selectionChanged();
            update();
        } else {
            m_boxSelecting = true;
            m_selectionOrigin = e->pos();
            m_rubberBand->setGeometry(QRect(m_selectionOrigin, QSize()));
            m_rubberBand->show();
        }
    }
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* e)
{
    QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();

    if (m_draggingGizmo) {
        auto* entity = m_scene.selectedEntity();
        if (!entity) return;
        QPoint drag = e->pos() - m_dragStartMouse;
        float factor = (e->modifiers() & Qt::ShiftModifier) ? 0.003f : 0.01f;
        float amount = float(drag.x() + (-drag.y())) * factor;

        if (m_toolMode == ToolMode::Move) {
            QVector3D dir = axisVector(m_dragAxis, m_dragStartTransform);
            entity->transform.position = m_dragStartPosition + dir * applySnap(amount);
        } else if (m_toolMode == ToolMode::Rotate) {
            QVector3D r = m_dragStartTransform.rotationEuler;
            float deg = applySnap(amount * 180.0f);
            if (m_dragAxis == 0) r.setX(deg + m_dragStartTransform.rotationEuler.x());
            if (m_dragAxis == 1) r.setY(deg + m_dragStartTransform.rotationEuler.y());
            if (m_dragAxis == 2) r.setZ(deg + m_dragStartTransform.rotationEuler.z());
            entity->transform.rotationEuler = r;
        } else if (m_toolMode == ToolMode::Scale) {
            QVector3D s = m_dragStartTransform.scale;
            float scaleDelta = qMax(0.05f, m_dragStartTransform.scale[m_dragAxis] + amount);
            if (m_dragAxis == 0) s.setX(scaleDelta);
            if (m_dragAxis == 1) s.setY(scaleDelta);
            if (m_dragAxis == 2) s.setZ(scaleDelta);
            entity->transform.scale = s;
        }

        emit selectionChanged();
        update();
        return;
    }

    if (m_boxSelecting) {
        m_rubberBand->setGeometry(QRect(m_selectionOrigin, e->pos()).normalized());
        return;
    }

    if (e->buttons() & Qt::RightButton) {
        m_yaw += delta.x() * 0.4f;
        m_pitch += delta.y() * 0.4f;
        m_pitch = qBound(-89.f, m_pitch, 89.f);
        update();
    }
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && m_draggingGizmo) {
        m_draggingGizmo = false;
        m_dragAxis = -1;
        auto* entity = m_scene.selectedEntity();
        if (entity && m_undoStack && entity->transform.position != m_dragStartTransform.position) {
            auto* cmd = new BeginGizmoMoveCommand(&m_scene, entity->id, m_dragStartTransform, this, nullptr);
            cmd->setAfter(entity->transform);
            m_undoStack->push(cmd);
        }
        update();
    }

    if (e->button() == Qt::LeftButton && m_boxSelecting) {
        m_boxSelecting = false;
        m_rubberBand->hide();
        QRect rect = QRect(m_selectionOrigin, e->pos()).normalized();
        QList<int> ids;

        for (const auto& ent : m_scene.entities()) {
            QMatrix4x4 model;
            model.translate(ent.transform.position);
            model.rotate(ent.transform.rotationEuler.x(), 1.f, 0.f, 0.f);
            model.rotate(ent.transform.rotationEuler.y(), 0.f, 1.f, 0.f);
            model.rotate(ent.transform.rotationEuler.z(), 0.f, 0.f, 1.f);
            model.scale(ent.transform.scale);

            QVector<QPoint> pts;
            pts.reserve(8);
            for (int ix = -1; ix <= 1; ix += 2) {
                for (int iy = -1; iy <= 1; iy += 2) {
                    for (int iz = -1; iz <= 1; iz += 2) {
                        QVector3D local(0.5f * ix, 0.5f * iy, 0.5f * iz);
                        QVector3D world = (model * QVector4D(local, 1.0f)).toVector3D();
                        pts.push_back(projectWorldToScreen(world));
                    }
                }
            }

            bool allInside = true;
            for (const QPoint& pt : pts) {
                if (!rect.contains(pt)) {
                    allInside = false;
                    break;
                }
            }

            if (allInside)
                ids.push_back(ent.id);
        }

        m_scene.selectInRect(ids, e->modifiers() & Qt::ControlModifier);
        emit selectionChanged();
        update();
    }
}

void ViewportWidget::wheelEvent(QWheelEvent* e)
{
    m_distance *= (e->angleDelta().y() > 0) ? 0.9f : 1.1f;
    m_distance = qBound(1.0f, m_distance, 100.f);
    update();
}
