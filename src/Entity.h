#pragma once
#include <QString>
#include <QMatrix4x4>
#include <QVector3D>

struct Transform
{
    QVector3D position {0.f, 0.f, 0.f};
    QVector3D rotationEuler {0.f, 0.f, 0.f};
    QVector3D scale {1.f, 1.f, 1.f};

    QMatrix4x4 toMatrix() const
    {
        QMatrix4x4 m;
        m.translate(position);
        m.rotate(rotationEuler.x(), 1.f, 0.f, 0.f);
        m.rotate(rotationEuler.y(), 0.f, 1.f, 0.f);
        m.rotate(rotationEuler.z(), 0.f, 0.f, 1.f);
        m.scale(scale);
        return m;
    }
};

struct Entity
{
    int id = -1;
    QString name;
    Transform transform;
    bool selected = false;
};
