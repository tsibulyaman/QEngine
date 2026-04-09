#pragma once
#include <QString>
#include "Scene.h"

class SceneSerializer
{
public:
    static bool saveToFile(const Scene& scene, const QString& path);
    static bool loadFromFile(Scene& scene, const QString& path);
};
