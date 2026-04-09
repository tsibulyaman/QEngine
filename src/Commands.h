#pragma once
#include <QUndoCommand>
#include <QString>
#include <QVector3D>
#include "Scene.h"

class ViewportWidget;
class EditorMainWindow;

class SetTransformCommand : public QUndoCommand
{
public:
    SetTransformCommand(Scene* scene, int entityId,
                        const Transform& before,
                        const Transform& after,
                        ViewportWidget* viewport,
                        EditorMainWindow* editor,
                        QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 1; }
    bool mergeWith(const QUndoCommand* other) override;

private:
    Scene* m_scene;
    int m_entityId;
    Transform m_before;
    Transform m_after;
    ViewportWidget* m_viewport;
    EditorMainWindow* m_editor;
};

class CreateEntityCommand : public QUndoCommand
{
public:
    CreateEntityCommand(Scene* scene, const QString& name,
                        ViewportWidget* viewport,
                        EditorMainWindow* editor,
                        QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Scene* m_scene;
    QString m_name;
    ViewportWidget* m_viewport;
    EditorMainWindow* m_editor;
    int m_createdId = -1;
    bool m_doneOnce = false;
};

class DeleteEntityCommand : public QUndoCommand
{
public:
    DeleteEntityCommand(Scene* scene, int entityId,
                        ViewportWidget* viewport,
                        EditorMainWindow* editor,
                        QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Scene* m_scene;
    Entity m_backup;
    int m_entityId;
    ViewportWidget* m_viewport;
    EditorMainWindow* m_editor;
    bool m_valid = false;
};


class BeginGizmoMoveCommand : public QUndoCommand
{
public:
    BeginGizmoMoveCommand(Scene* scene, int entityId,
                          const Transform& before,
                          ViewportWidget* viewport,
                          EditorMainWindow* editor,
                          QUndoCommand* parent = nullptr);
    void setAfter(const Transform& after);
    void undo() override;
    void redo() override;
    int id() const override { return 2; }
    bool mergeWith(const QUndoCommand* other) override;

private:
    Scene* m_scene;
    int m_entityId;
    Transform m_before;
    Transform m_after;
    ViewportWidget* m_viewport;
    EditorMainWindow* m_editor;
};
