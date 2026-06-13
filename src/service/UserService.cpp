#include "service/UserService.hpp"

#include "db/UserRepository.hpp"
#include "utils/jwt.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr int PBKDF2_ITERATIONS = 100000;
constexpr int SALT_BYTES = 16;
constexpr int HASH_BYTES = 32;
constexpr int64_t TOKEN_EXPIRES_IN = 7200;

std::string toStdString(const oatpp::String& value) {
    return value ? std::string(value->c_str()) : std::string();
}

std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}

std::vector<unsigned char> hexToBytes(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        throw std::runtime_error("invalid password hash");
    }

    std::vector<unsigned char> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned int value = 0;
        std::istringstream in(hex.substr(i, 2));
        in >> std::hex >> value;
        if (in.fail()) {
            throw std::runtime_error("invalid password hash");
        }
        bytes.push_back(static_cast<unsigned char>(value));
    }
    return bytes;
}

std::vector<unsigned char> derivePasswordHash(
    const std::string& password,
    const std::vector<unsigned char>& salt,
    int iterations
) {
    std::vector<unsigned char> hash(HASH_BYTES);
    int ok = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        salt.data(),
        static_cast<int>(salt.size()),
        iterations,
        EVP_sha256(),
        static_cast<int>(hash.size()),
        hash.data()
    );

    if (ok != 1) {
        throw std::runtime_error("password hash failed");
    }

    return hash;
}

bool constantTimeEquals(
    const std::vector<unsigned char>& left,
    const std::vector<unsigned char>& right
) {
    if (left.size() != right.size()) {
        return false;
    }

    unsigned char diff = 0;
    for (size_t i = 0; i < left.size(); ++i) {
        diff |= left[i] ^ right[i];
    }
    return diff == 0;
}

std::string hashPassword(const std::string& password) {
    std::vector<unsigned char> salt(SALT_BYTES);
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        throw std::runtime_error("generate password salt failed");
    }

    std::vector<unsigned char> hash = derivePasswordHash(password, salt, PBKDF2_ITERATIONS);

    return "pbkdf2_sha256$" + std::to_string(PBKDF2_ITERATIONS) + "$" +
           bytesToHex(salt) + "$" + bytesToHex(hash);
}

bool verifyPassword(const std::string& password, const std::string& storedHash) {
    try {
        const std::string prefix = "pbkdf2_sha256$";
        if (storedHash.find(prefix) != 0) {
            return false;
        }

        size_t iterationsEnd = storedHash.find('$', prefix.size());
        if (iterationsEnd == std::string::npos) {
            return false;
        }

        size_t saltEnd = storedHash.find('$', iterationsEnd + 1);
        if (saltEnd == std::string::npos) {
            return false;
        }

        int iterations = std::stoi(storedHash.substr(prefix.size(), iterationsEnd - prefix.size()));
        if (iterations <= 0) {
            return false;
        }
        std::string saltHex = storedHash.substr(iterationsEnd + 1, saltEnd - iterationsEnd - 1);
        std::string hashHex = storedHash.substr(saltEnd + 1);

        std::vector<unsigned char> salt = hexToBytes(saltHex);
        std::vector<unsigned char> expectedHash = hexToBytes(hashHex);
        std::vector<unsigned char> actualHash = derivePasswordHash(password, salt, iterations);

        return constantTimeEquals(actualHash, expectedHash);
    } catch (const std::exception&) {
        return false;
    }
}

oatpp::Object<UserDto> toUserDto(const UserRepository::UserRecord& record) {
    auto dto = UserDto::createShared();
    dto->id = record.id;
    dto->username = record.username.c_str();
    dto->role = record.role.c_str();
    dto->phone = record.phone.c_str();
    dto->email = record.email.c_str();
    dto->creditScore = record.creditScore;
    dto->status = record.status.c_str();
    dto->createdAt = record.createdAt.c_str();
    return dto;
}

oatpp::Object<LoginUserInfoResponseDto> toLoginUserInfoDto(
    const UserRepository::UserRecord& record
) {
    auto dto = LoginUserInfoResponseDto::createShared();
    dto->id = record.id;
    dto->username = record.username.c_str();
    dto->role = record.role.c_str();
    dto->creditScore = record.creditScore;
    return dto;
}

void ensureNotEmpty(const std::string& value, const std::string& message) {
    if (value.empty()) {
        throw std::runtime_error(message);
    }
}

} // namespace

oatpp::Object<RegisterResponse> UserService::registerUser(
    const oatpp::Object<RegisterRequest>& request
) {
    if (!request) {
        throw std::runtime_error("register request is empty");
    }

    std::string username = toStdString(request->username);
    std::string password = toStdString(request->password);
    std::string phone = toStdString(request->phone);
    std::string email = toStdString(request->email);

    ensureNotEmpty(username, "username cannot be empty");
    ensureNotEmpty(password, "password cannot be empty");
    ensureNotEmpty(phone, "phone cannot be empty");

    if (UserRepository::existsByUsername(username)) {
        throw std::runtime_error("username already exists");
    }
    if (UserRepository::existsByPhone(phone)) {
        throw std::runtime_error("phone already exists");
    }
    if (!email.empty() && UserRepository::existsByEmail(email)) {
        throw std::runtime_error("email already exists");
    }

    int64_t userId = UserRepository::createUser(username, hashPassword(password), phone, email);

    auto response = RegisterResponse::createShared();
    response->userId = userId;
    return response;
}

oatpp::Object<LoginResponse> UserService::login(
    const std::string& secret,
    const oatpp::Object<LoginRequest>& request
) {
    if (!request) {
        throw std::runtime_error("login request is empty");
    }

    std::string username = toStdString(request->username);
    std::string password = toStdString(request->password);

    ensureNotEmpty(username, "username cannot be empty");
    ensureNotEmpty(password, "password cannot be empty");

    auto user = UserRepository::findByUsername(username);
    if (!user.has_value()) {
        throw std::runtime_error("invalid username or password");
    }

    if (!verifyPassword(password, user->password)) {
        throw std::runtime_error("invalid username or password");
    }

    if (user->status != "active") {
        throw std::runtime_error("user account is not active");
    }

    auto response = LoginResponse::createShared();
    response->token = JWT::generate(secret, user->id, user->role, TOKEN_EXPIRES_IN).c_str();
    response->tokenType = "Bearer";
    response->expiresIn = TOKEN_EXPIRES_IN;
    response->userInfo = toLoginUserInfoDto(*user);
    return response;
}

oatpp::Object<UserDto> UserService::getUserDetailed(int64_t userId) {
    auto user = UserRepository::findById(userId);
    if (!user.has_value()) {
        throw std::runtime_error("user not found");
    }

    return toUserDto(*user);
}

oatpp::Object<UserDto> UserService::updateProfile(
    int64_t userId,
    const oatpp::Object<UpdateProfileRequest>& request
) {
    if (!request) {
        throw std::runtime_error("update profile request is empty");
    }

    auto user = UserRepository::findById(userId);
    if (!user.has_value()) {
        throw std::runtime_error("user not found");
    }

    std::string phone = request->phone ? toStdString(request->phone) : user->phone;
    std::string email = request->email ? toStdString(request->email) : user->email;

    ensureNotEmpty(phone, "phone cannot be empty");

    if (UserRepository::existsByPhoneForOtherUser(phone, userId)) {
        throw std::runtime_error("phone already exists");
    }
    if (!email.empty() && UserRepository::existsByEmailForOtherUser(email, userId)) {
        throw std::runtime_error("email already exists");
    }

    if (!UserRepository::updateProfile(userId, phone, email)) {
        throw std::runtime_error("update profile failed");
    }

    auto updatedUser = UserRepository::findById(userId);
    if (!updatedUser.has_value()) {
        throw std::runtime_error("user not found after profile update");
    }

    return toUserDto(*updatedUser);
}

bool UserService::changePassword(
    int64_t userId,
    const oatpp::Object<ChangePasswordRequest>& request
) {
    if (!request) {
        throw std::runtime_error("change password request is empty");
    }

    std::string oldPassword = toStdString(request->oldPassword);
    std::string newPassword = toStdString(request->newPassword);

    ensureNotEmpty(oldPassword, "old password cannot be empty");
    ensureNotEmpty(newPassword, "new password cannot be empty");

    auto user = UserRepository::findById(userId);
    if (!user.has_value()) {
        throw std::runtime_error("user not found");
    }

    if (!verifyPassword(oldPassword, user->password)) {
        throw std::runtime_error("old password is incorrect");
    }

    if (!UserRepository::updatePassword(userId, hashPassword(newPassword))) {
        throw std::runtime_error("change password failed");
    }

    return true;
}

bool UserService::isAdmin(int64_t userId) {
    auto user = UserRepository::findById(userId);
    if (!user.has_value()) {
        return false;
    }
    return user->role == "resource_admin" || user->role == "system_admin";
}
