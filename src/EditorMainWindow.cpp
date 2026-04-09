#include <QTimer>
#include <QShowEvent>
#include "EditorMainWindow.h"
#include "ViewportWidget.h"
#include "Commands.h"
#include "SceneSerializer.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUndoStack>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QKeySequence>

EditorMainWindow::EditorMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    resize(1400, 900);
    setWindowTitle("TinyEditor");

    m_undoStack = new QUndoStack(this);
    m_viewport = new ViewportWidget(this);
    m_viewport->setUndoStack(m_undoStack);
    setCentralWidget(m_viewport);

    auto* fileMenu = menuBar()->addMenu("&File");
    auto* openAct = fileMenu->addAction("Open...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &EditorMainWindow::onLoadScene);
    auto* saveAct = fileMenu->addAction("Save...");
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &EditorMainWindow::onSaveScene);

    auto* editMenu = menuBar()->addMenu("&Edit");
    auto* undoAct = m_undoStack->createUndoAction(this, "Undo");
    auto* redoAct = m_undoStack->createRedoAction(this, "Redo");
    undoAct->setShortcut(QKeySequence::Undo);
    redoAct->setShortcut(QKeySequence::Redo);
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    auto* createMenu = menuBar()->addMenu("&Create");
    auto* addEntityAct = createMenu->addAction("Add Entity");
    addEntityAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    auto* addCubeAct = createMenu->addAction("Add Cube");
    auto* addPlaneAct = createMenu->addAction("Add Plane");
    auto* addSphereAct = createMenu->addAction("Add Sphere");
    connect(addEntityAct, &QAction::triggered, this, &EditorMainWindow::onAddEntity);
    connect(addCubeAct, &QAction::triggered, this, &EditorMainWindow::onAddCube);
    connect(addPlaneAct, &QAction::triggered, this, &EditorMainWindow::onAddPlane);
    connect(addSphereAct, &QAction::triggered, this, &EditorMainWindow::onAddSphere);

    auto* entityMenu = menuBar()->addMenu("&Entity");
    auto* duplicateAct = entityMenu->addAction("Duplicate");
    duplicateAct->setShortcut(QKeySequence("Ctrl+D"));
    auto* deleteAct = entityMenu->addAction("Delete Selected");
    deleteAct->setShortcut(QKeySequence::Delete);
    auto* frameAct = entityMenu->addAction("Frame Selected");
    frameAct->setShortcut(QKeySequence(Qt::Key_F));
    connect(duplicateAct, &QAction::triggered, this, &EditorMainWindow::onDuplicateEntity);
    connect(deleteAct, &QAction::triggered, this, &EditorMainWindow::onDeleteEntity);
    connect(frameAct, &QAction::triggered, this, &EditorMainWindow::onFrameSelected);

    auto* toolsMenu = menuBar()->addMenu("&Tools");
    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
    auto* moveAct = toolsMenu->addAction("Move Tool");
    moveAct->setCheckable(true); moveAct->setChecked(true); moveAct->setShortcut(QKeySequence(Qt::Key_W));
    auto* rotateAct = toolsMenu->addAction("Rotate Tool");
    rotateAct->setCheckable(true); rotateAct->setShortcut(QKeySequence(Qt::Key_E));
    auto* scaleAct = toolsMenu->addAction("Scale Tool");
    scaleAct->setCheckable(true); scaleAct->setShortcut(QKeySequence(Qt::Key_R));
    toolGroup->addAction(moveAct); toolGroup->addAction(rotateAct); toolGroup->addAction(scaleAct);
    connect(moveAct, &QAction::triggered, m_viewport, &ViewportWidget::setToolMove);
    connect(rotateAct, &QAction::triggered, m_viewport, &ViewportWidget::setToolRotate);
    connect(scaleAct, &QAction::triggered, m_viewport, &ViewportWidget::setToolScale);

    auto* spaceMenu = menuBar()->addMenu("&Space");
    auto* spaceGroup = new QActionGroup(this);
    spaceGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
    auto* worldAct = spaceMenu->addAction("World Space");
    worldAct->setCheckable(true); worldAct->setChecked(true); worldAct->setShortcut(QKeySequence(Qt::Key_G));
    auto* localAct = spaceMenu->addAction("Local Space");
    localAct->setCheckable(true); localAct->setShortcut(QKeySequence(Qt::Key_L));
    spaceGroup->addAction(worldAct); spaceGroup->addAction(localAct);
    connect(worldAct, &QAction::triggered, m_viewport, &ViewportWidget::setSpaceWorld);
    connect(localAct, &QAction::triggered, m_viewport, &ViewportWidget::setSpaceLocal);

    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    toolbar->addAction(undoAct);
    toolbar->addAction(redoAct);
    toolbar->addSeparator();
    toolbar->addAction(moveAct);
    toolbar->addAction(rotateAct);
    toolbar->addAction(scaleAct);
    toolbar->addSeparator();
    toolbar->addAction(worldAct);
    toolbar->addAction(localAct);
    toolbar->addSeparator();
    toolbar->addAction(addCubeAct);
    toolbar->addAction(addPlaneAct);
    toolbar->addAction(addSphereAct);
    toolbar->addSeparator();
    toolbar->addAction(duplicateAct);
    toolbar->addAction(deleteAct);

    auto* viewMenu = menuBar()->addMenu("&View");
    auto* snapAct = viewMenu->addAction("Snap");
    snapAct->setCheckable(true);
    auto* wireAct = viewMenu->addAction("Wireframe");
    wireAct->setCheckable(true);
    connect(snapAct, &QAction::toggled, m_viewport, &ViewportWidget::setSnapEnabled);
    connect(wireAct, &QAction::toggled, m_viewport, &ViewportWidget::setWireframeEnabled);

    auto* hierarchyDock = new QDockWidget("Hierarchy", this);
    m_hierarchy = new QTreeWidget(hierarchyDock);
    m_hierarchy->setHeaderHidden(true);
    m_hierarchy->setSelectionMode(QAbstractItemView::ExtendedSelection);
    hierarchyDock->setWidget(m_hierarchy);
    addDockWidget(Qt::LeftDockWidgetArea, hierarchyDock);

    auto* inspectorDock = new QDockWidget("Inspector", this);
    auto* inspectorHost = new QWidget(inspectorDock);
    auto* form = new QFormLayout(inspectorHost);
    m_nameLabel = new QLabel("No selection", inspectorHost);
    form->addRow("Name", m_nameLabel);

    auto mkSpin = [&](double mn, double mx) {
        auto* s = new QDoubleSpinBox(inspectorHost);
        s->setRange(mn, mx);
        s->setDecimals(3);
        s->setSingleStep(0.1);
        return s;
    };

    m_posX = mkSpin(-9999.0, 9999.0); m_posY = mkSpin(-9999.0, 9999.0); m_posZ = mkSpin(-9999.0, 9999.0);
    m_rotX = mkSpin(-360.0, 360.0); m_rotY = mkSpin(-360.0, 360.0); m_rotZ = mkSpin(-360.0, 360.0);
    m_sclX = mkSpin(0.001, 9999.0); m_sclY = mkSpin(0.001, 9999.0); m_sclZ = mkSpin(0.001, 9999.0);

    form->addRow("Pos X", m_posX); form->addRow("Pos Y", m_posY); form->addRow("Pos Z", m_posZ);
    form->addRow("Rot X", m_rotX); form->addRow("Rot Y", m_rotY); form->addRow("Rot Z", m_rotZ);
    form->addRow("Scale X", m_sclX); form->addRow("Scale Y", m_sclY); form->addRow("Scale Z", m_sclZ);
    inspectorDock->setWidget(inspectorHost);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock);

    auto connectSpin = [&](QDoubleSpinBox* s) {
        connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorMainWindow::onInspectorValueChanged);
        connect(s, &QDoubleSpinBox::editingFinished, this, &EditorMainWindow::onInspectorEditingFinished);
    };
    connectSpin(m_posX); connectSpin(m_posY); connectSpin(m_posZ);
    connectSpin(m_rotX); connectSpin(m_rotY); connectSpin(m_rotZ);
    connectSpin(m_sclX); connectSpin(m_sclY); connectSpin(m_sclZ);

    connect(m_hierarchy, &QTreeWidget::currentItemChanged, this, &EditorMainWindow::onHierarchyCurrentChanged);
    connect(m_viewport, &ViewportWidget::selectionChanged, this, &EditorMainWindow::refreshUiFromScene);

    statusBar()->showMessage("Ready");
    onAddEntity();
}

void EditorMainWindow::refreshUiFromScene()
{
    rebuildHierarchy();
    syncInspector();
}

void EditorMainWindow::rebuildHierarchy()
{
    QSignalBlocker blocker(m_hierarchy);
    m_hierarchy->clear();
    for (const auto& e : m_viewport->scene().entities()) {
        auto* item = new QTreeWidgetItem(QStringList() << QString("%1 (%2)").arg(e.name).arg(e.id));
        item->setData(0, Qt::UserRole, e.id);
        if (e.selected)
            item->setSelected(true);
        m_hierarchy->addTopLevelItem(item);
    }
}

void EditorMainWindow::selectHierarchyById(int id)
{
    QSignalBlocker blocker(m_hierarchy);
    for (int i = 0; i < m_hierarchy->topLevelItemCount(); ++i) {
        auto* item = m_hierarchy->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toInt() == id) {
            m_hierarchy->setCurrentItem(item);
            break;
        }
    }
}

void EditorMainWindow::syncInspector()
{
    m_updatingUi = true;
    auto* e = m_viewport->scene().selectedEntity();
    if (!e) {
        m_nameLabel->setText("No selection");
        m_posX->setValue(0); m_posY->setValue(0); m_posZ->setValue(0);
        m_rotX->setValue(0); m_rotY->setValue(0); m_rotZ->setValue(0);
        m_sclX->setValue(1); m_sclY->setValue(1); m_sclZ->setValue(1);
        m_updatingUi = false;
        return;
    }

    m_nameLabel->setText(e->name);
    m_posX->setValue(e->transform.position.x());
    m_posY->setValue(e->transform.position.y());
    m_posZ->setValue(e->transform.position.z());
    m_rotX->setValue(e->transform.rotationEuler.x());
    m_rotY->setValue(e->transform.rotationEuler.y());
    m_rotZ->setValue(e->transform.rotationEuler.z());
    m_sclX->setValue(e->transform.scale.x());
    m_sclY->setValue(e->transform.scale.y());
    m_sclZ->setValue(e->transform.scale.z());
    m_lastInspectorTransform = e->transform;
    m_updatingUi = false;
}

void EditorMainWindow::onHierarchyCurrentChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
    if (!current) return;
    int id = current->data(0, Qt::UserRole).toInt();
    m_viewport->selectEntity(id);
}

void EditorMainWindow::onInspectorValueChanged()
{
    if (m_updatingUi) return;
    auto* e = m_viewport->scene().selectedEntity();
    if (!e) return;

    e->transform.position = QVector3D(m_posX->value(), m_posY->value(), m_posZ->value());
    e->transform.rotationEuler = QVector3D(m_rotX->value(), m_rotY->value(), m_rotZ->value());
    e->transform.scale = QVector3D(m_sclX->value(), m_sclY->value(), m_sclZ->value());
    m_viewport->update();
}

void EditorMainWindow::onInspectorEditingFinished()
{
    if (m_updatingUi) return;
    auto* e = m_viewport->scene().selectedEntity();
    if (!e) return;
    Transform after = e->transform;
    m_undoStack->push(new SetTransformCommand(&m_viewport->scene(), e->id, m_lastInspectorTransform, after, m_viewport, this));
    m_lastInspectorTransform = after;
}

void EditorMainWindow::onAddEntity()
{
    m_undoStack->push(new CreateEntityCommand(&m_viewport->scene(), "Entity", m_viewport, this));
    refreshUiFromScene();
}

void EditorMainWindow::onAddCube()
{
    m_undoStack->push(new CreateEntityCommand(&m_viewport->scene(), "Cube", m_viewport, this));
    refreshUiFromScene();
}

void EditorMainWindow::onAddPlane()
{
    m_undoStack->push(new CreateEntityCommand(&m_viewport->scene(), "Plane", m_viewport, this));
    if (auto* e = m_viewport->scene().selectedEntity())
        e->transform.scale = QVector3D(2.0f, 0.1f, 2.0f);
    refreshUiFromScene();
}

void EditorMainWindow::onAddSphere()
{
    m_undoStack->push(new CreateEntityCommand(&m_viewport->scene(), "Sphere", m_viewport, this));
    if (auto* e = m_viewport->scene().selectedEntity())
        e->transform.scale = QVector3D(0.8f, 0.8f, 0.8f);
    refreshUiFromScene();
}

void EditorMainWindow::onDeleteEntity()
{
    const auto ids = m_viewport->scene().selectedIds();
    for (int i = ids.size() - 1; i >= 0; --i)
        m_undoStack->push(new DeleteEntityCommand(&m_viewport->scene(), ids[i], m_viewport, this));
    refreshUiFromScene();
}

void EditorMainWindow::onDuplicateEntity()
{
    const auto ids = m_viewport->scene().selectedIds();
    for (int id : ids) {
        Entity copy = m_viewport->scene().duplicateEntity(id);
        Q_UNUSED(copy);
    }
    refreshUiFromScene();
}

void EditorMainWindow::onFrameSelected()
{
    m_viewport->frameSelected();
}

void EditorMainWindow::onSaveScene()
{
    QString path = QFileDialog::getSaveFileName(this, "Save Scene", QString(), "JSON (*.json)");
    if (path.isEmpty()) return;
    if (!SceneSerializer::saveToFile(m_viewport->scene(), path))
        QMessageBox::warning(this, "Save failed", "Could not save scene file.");
}

void EditorMainWindow::onLoadScene()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Scene", QString(), "JSON (*.json)");
    if (path.isEmpty()) return;
    if (!SceneSerializer::loadFromFile(m_viewport->scene(), path)) {
        QMessageBox::warning(this, "Load failed", "Could not load scene file.");
        return;
    }
    refreshUiFromScene();
    m_viewport->update();
}


void EditorMainWindow::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);
    QTimer::singleShot(0, this, [this]() {
        if (!m_viewport) return;
        const QSize s = m_viewport->size();
        if (s.width() > 10 && s.height() > 10) {
            m_viewport->resize(s.width() + 1, s.height());
            m_viewport->resize(s);
            m_viewport->update();
        }
    });
}
