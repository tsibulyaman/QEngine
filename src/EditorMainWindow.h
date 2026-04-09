#pragma once
#include <QMainWindow>
#include "Scene.h"

class ViewportWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QDoubleSpinBox;
class QLabel;
class QUndoStack;
class QShowEvent;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorMainWindow(QWidget* parent = nullptr);
    void refreshUiFromScene();

private slots:
    void rebuildHierarchy();
    void syncInspector();
    void onHierarchyCurrentChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onInspectorValueChanged();
    void onInspectorEditingFinished();
    void onAddEntity();
    void onDeleteEntity();
    void onDuplicateEntity();
    void onAddCube();
    void onAddPlane();
    void onAddSphere();
    void onFrameSelected();
    void onSaveScene();
    void onLoadScene();

protected:
    void showEvent(QShowEvent* e) override;

private:
    void selectHierarchyById(int id);

    ViewportWidget* m_viewport = nullptr;
    QTreeWidget* m_hierarchy = nullptr;
    QLabel* m_nameLabel = nullptr;
    QUndoStack* m_undoStack = nullptr;
    QDoubleSpinBox *m_posX = nullptr, *m_posY = nullptr, *m_posZ = nullptr;
    QDoubleSpinBox *m_rotX = nullptr, *m_rotY = nullptr, *m_rotZ = nullptr;
    QDoubleSpinBox *m_sclX = nullptr, *m_sclY = nullptr, *m_sclZ = nullptr;
    bool m_updatingUi = false;
    Transform m_lastInspectorTransform;
};
