#include "Scene.h"
#include <algorithm>

Scene::Scene()
{
    auto& e1 = createEntity("Cube A");
    e1.transform.position = {-1.5f, 0.f, 0.f};

    auto& e2 = createEntity("Cube B");
    e2.transform.position = {1.5f, 0.f, 0.f};
}

Entity& Scene::createEntity(const QString& name)
{
    Entity e;
    e.id = m_nextId++;
    e.name = name;
    m_entities.push_back(e);
    return m_entities.back();
}

void Scene::insertEntity(const Entity& entity)
{
    m_entities.push_back(entity);
    if (entity.id >= m_nextId)
        m_nextId = entity.id + 1;
}

bool Scene::removeEntityById(int id)
{
    auto it = std::remove_if(m_entities.begin(), m_entities.end(), [id](const Entity& e){ return e.id == id; });
    bool removed = it != m_entities.end();
    m_entities.erase(it, m_entities.end());
    return removed;
}

std::vector<Entity>& Scene::entities() { return m_entities; }
const std::vector<Entity>& Scene::entities() const { return m_entities; }

Entity* Scene::findById(int id)
{
    for (auto& e : m_entities)
        if (e.id == id) return &e;
    return nullptr;
}

void Scene::clearSelection()
{
    for (auto& e : m_entities)
        e.selected = false;
}

Entity* Scene::selectedEntity()
{
    for (auto& e : m_entities)
        if (e.selected) return &e;
    return nullptr;
}

void Scene::selectById(int id)
{
    clearSelection();
    if (auto* e = findById(id))
        e->selected = true;
}

void Scene::clear()
{
    m_entities.clear();
    m_nextId = 1;
}

Entity Scene::duplicateEntity(int id)
{
    Entity copy;
    if (auto* e = findById(id)) {
        copy = *e;
        copy.id = m_nextId++;
        copy.name += " Copy";
        copy.selected = false;
        copy.transform.position += QVector3D(0.5f, 0.f, 0.5f);
        m_entities.push_back(copy);
    }
    return copy;
}

void Scene::selectInRect(const QList<int>& ids, bool additive)
{
    if (!additive) clearSelection();
    for (auto& e : m_entities) {
        if (ids.contains(e.id)) e.selected = true;
    }
}

QList<int> Scene::selectedIds() const
{
    QList<int> ids;
    for (const auto& e : m_entities) if (e.selected) ids.push_back(e.id);
    return ids;
}

const Entity* Scene::selectedEntity() const
{
    for (const auto& e : m_entities) if (e.selected) return &e;
    return nullptr;
}
