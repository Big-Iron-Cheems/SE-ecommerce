#include "Transporter.h"

User::UserType Transporter::getUserType() const { return User::UserType::TRANSPORTER; }

std::string Transporter::toString() const {
    std::ostringstream oss;
    oss << "Transporter: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}
