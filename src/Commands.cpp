#include "Commands.h"
#include "ViewportWidget.h"
#include "EditorMainWindow.h"

static void refreshEditorUi(ViewportWidget* viewport, EditorMainWindow* editor)
{
    if (viewport)
        viewport->update();
    if (editor)
        editor->refreshUiFromScene();
}

SetTransformCommand::SetTransformCommand(Scene* scene, int entityId,
                                         const Transform& before,
                                         const Transform& after,
                                         ViewportWidget* viewport,
                                         EditorMainWindow* editor,
                                         QUndoCommand* parent)
    : QUndoCommand(parent),
      m_scene(scene),
      m_entityId(entityId),
      m_before(before),
      m_after(after),
      m_viewport(viewport),
      m_editor(editor)
{
    setText("Set Transform");
}

void SetTransformCommand::undo()
{
    if (!m_scene) return;
    if (auto* e = m_scene->findById(m_entityId))
        e->transform = m_before;
    refreshEditorUi(m_viewport, m_editor);
}

void SetTransformCommand::redo()
{
    if (!m_scene) return;
    if (auto* e = m_scene->findById(m_entityId))
        e->transform = m_after;
    refreshEditorUi(m_viewport, m_editor);
}

bool SetTransformCommand::mergeWith(const QUndoCommand* other)
{
    if (!other || other->id() != id()) return false;
    auto* cmd = static_cast<const SetTransformCommand*>(other);
    if (cmd->m_entityId != m_entityId) return false;
    m_after = cmd->m_after;
    return true;
}

CreateEntityCommand::CreateEntityCommand(Scene* scene, const QString& name,
                                         ViewportWidget* viewport,
                                         EditorMainWindow* editor,
                                         QUndoCommand* parent)
    : QUndoCommand(parent), m_scene(scene), m_name(name), m_viewport(viewport), m_editor(editor)
{
    setText("Create Entity");
}

void CreateEntityCommand::redo()
{
    if (!m_scene) return;
    if (!m_doneOnce) {
        auto& e = m_scene->createEntity(m_name);
        e.transform.position = {0.f, 0.f, 0.f};
        m_createdId = e.id;
        m_doneOnce = true;
    } else {
        Entity restored;
        restored.id = m_createdId;
        restored.name = m_name;
        restored.transform.position = {0.f, 0.f, 0.f};
        m_scene->insertEntity(restored);
    }

    m_scene->selectById(m_createdId);
    refreshEditorUi(m_viewport, m_editor);
}

void CreateEntityCommand::undo()
{
    if (!m_scene) return;
    m_scene->removeEntityById(m_createdId);
    refreshEditorUi(m_viewport, m_editor);
}

DeleteEntityCommand::DeleteEntityCommand(Scene* scene, int entityId,
                                         ViewportWidget* viewport,
                                         EditorMainWindow* editor,
                                         QUndoCommand* parent)
    : QUndoCommand(parent), m_scene(scene), m_entityId(entityId), m_viewport(viewport), m_editor(editor)
{
    if (m_scene) {
        if (auto* e = m_scene->findById(entityId)) {
            m_backup = *e;
            m_valid = true;
            setText("Delete Entity");
        }
    }
}

void DeleteEntityCommand::redo()
{
    if (!m_valid || !m_scene) return;
    m_scene->removeEntityById(m_entityId);
    refreshEditorUi(m_viewport, m_editor);
}

void DeleteEntityCommand::undo()
{
    if (!m_valid || !m_scene) return;
    m_scene->insertEntity(m_backup);
    m_scene->selectById(m_backup.id);
    refreshEditorUi(m_viewport, m_editor);
}

BeginGizmoMoveCommand::BeginGizmoMoveCommand(Scene* scene, int entityId,
                                             const Transform& before,
                                             ViewportWidget* viewport,
                                             EditorMainWindow* editor,
                                             QUndoCommand* parent)
    : QUndoCommand(parent),
      m_scene(scene),
      m_entityId(entityId),
      m_before(before),
      m_after(before),
      m_viewport(viewport),
      m_editor(editor)
{
    setText("Move Entity");
}

void BeginGizmoMoveCommand::setAfter(const Transform& after)
{
    m_after = after;
}

void BeginGizmoMoveCommand::undo()
{
    if (!m_scene) return;
    if (auto* e = m_scene->findById(m_entityId)) {
        e->transform = m_before;
        m_scene->selectById(m_entityId);
    }
    refreshEditorUi(m_viewport, m_editor);
}

void BeginGizmoMoveCommand::redo()
{
    if (!m_scene) return;
    if (auto* e = m_scene->findById(m_entityId)) {
        e->transform = m_after;
        m_scene->selectById(m_entityId);
    }
    refreshEditorUi(m_viewport, m_editor);
}

bool BeginGizmoMoveCommand::mergeWith(const QUndoCommand* other)
{
    if (!other || other->id() != id()) return false;
    auto* cmd = static_cast<const BeginGizmoMoveCommand*>(other);
    if (cmd->m_entityId != m_entityId) return false;
    m_after = cmd->m_after;
    return true;
}
