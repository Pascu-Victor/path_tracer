#include "Object.h"

Object::Object() : material(nullptr) {}

Object::Object(std::shared_ptr<Material> material) : material(material) {}
