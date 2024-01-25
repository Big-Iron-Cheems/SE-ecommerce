#include "Supplier.h"

std::string Supplier::toString() const {
    std::ostringstream oss;
    oss << "Supplier: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}
