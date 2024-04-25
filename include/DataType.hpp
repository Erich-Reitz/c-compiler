#pragma once

#include <string>

namespace ast {

struct DataType {
    std::string name;
    int size = 0;
    DataType* pointsTo = nullptr;

    DataType() : name(""), size(0), pointsTo(nullptr) {}

    DataType(std::string name, int size, DataType* pointsTo)
        : name(name), size(size), pointsTo(pointsTo) {}

    DataType(const DataType& other) : name(other.name), size(other.size), pointsTo(nullptr) {
        if (other.pointsTo != nullptr) {
            pointsTo = new DataType(*other.pointsTo);
        }
    }

    DataType& operator=(const DataType& other) {
        if (this != &other) {
            name = other.name;
            size = other.size;
            delete pointsTo;
            pointsTo = nullptr;
            if (other.pointsTo != nullptr) {
                pointsTo = new DataType(*other.pointsTo);
            }
        }
        return *this;
    }

    DataType FinalPointsTo() const {
        if (pointsTo == nullptr) {
            return *this;
        }
        return pointsTo->FinalPointsTo();
    }

    std::string ToString() const {
        if (pointsTo == nullptr) {
            return name;
        }
        return name + " -> " + pointsTo->ToString();
    }
};
}  // namespace ast