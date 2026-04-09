#include "SceneSerializer.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

bool SceneSerializer::saveToFile(const Scene& scene, const QString& path)
{
    QJsonArray arr;
    for (const auto& e : scene.entities()) {
        QJsonObject obj;
        obj["id"] = e.id;
        obj["name"] = e.name;

        QJsonObject pos;
        pos["x"] = e.transform.position.x();
        pos["y"] = e.transform.position.y();
        pos["z"] = e.transform.position.z();
        obj["position"] = pos;

        QJsonObject rot;
        rot["x"] = e.transform.rotationEuler.x();
        rot["y"] = e.transform.rotationEuler.y();
        rot["z"] = e.transform.rotationEuler.z();
        obj["rotation"] = rot;

        QJsonObject scl;
        scl["x"] = e.transform.scale.x();
        scl["y"] = e.transform.scale.y();
        scl["z"] = e.transform.scale.z();
        obj["scale"] = scl;

        arr.append(obj);
    }

    QJsonObject root;
    root["entities"] = arr;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SceneSerializer::loadFromFile(Scene& scene, const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;

    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return false;

    auto root = doc.object();
    auto arr = root["entities"].toArray();

    scene.clear();

    for (const auto& v : arr) {
        auto obj = v.toObject();
        Entity e;
        e.id = obj["id"].toInt();
        e.name = obj["name"].toString();

        auto pos = obj["position"].toObject();
        e.transform.position = QVector3D(pos["x"].toDouble(), pos["y"].toDouble(), pos["z"].toDouble());

        auto rot = obj["rotation"].toObject();
        e.transform.rotationEuler = QVector3D(rot["x"].toDouble(), rot["y"].toDouble(), rot["z"].toDouble());

        auto scl = obj["scale"].toObject();
        e.transform.scale = QVector3D(scl["x"].toDouble(), scl["y"].toDouble(), scl["z"].toDouble());

        scene.insertEntity(e);
    }

    if (!scene.entities().empty())
        scene.selectById(scene.entities().front().id);

    return true;
}
