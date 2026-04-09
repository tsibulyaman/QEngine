#pragma once
#include <vector>
#include "Entity.h"

class Scene
{
public:
    Scene();

    Entity& createEntity(const QString& name);
    void insertEntity(const Entity& entity);
    bool removeEntityById(int id);
    std::vector<Entity>& entities();
    const std::vector<Entity>& entities() const;

    Entity* findById(int id);
    void clearSelection();
    Entity* selectedEntity();
    const Entity* selectedEntity() const;
    QList<int> selectedIds() const;
    void selectById(int id);
    void clear();
    Entity duplicateEntity(int id);
    void selectInRect(const QList<int>& ids, bool additive);

private:
    int m_nextId = 1;
    std::vector<Entity> m_entities;
};
