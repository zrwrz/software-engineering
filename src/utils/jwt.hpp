#ifndef JWT_HPP
#define JWT_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <openssl/hmac.h>
#include <nlohmann/json.hpp>

class JWT {
  public:
    static std::string generate(const std::string& secret, int64_t userId, const std::string& role, int64_t expiresInSeconds = 7200) {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto expiresAt = now + seconds(expiresInSeconds);
        
        // Header
        auto header = nlohmann::json{
            {"alg", "HS256"},
            {"typ", "JWT"}
        };
        
        // Payload
        auto payload = nlohmann::json{
            {"userId", userId},
            {"role", role},
            {"exp", duration_cast<seconds>(expiresAt.time_since_epoch()).count()},
            {"iat", duration_cast<seconds>(now.time_since_epoch()).count()}
        };
        
        // Base64Url 编码
        auto headerEncoded = base64UrlEncode(header.dump());
        auto payloadEncoded = base64UrlEncode(payload.dump());
        
        // 拼接签名输入
        std::string signatureInput = headerEncoded + "." + payloadEncoded;
        
        // HMAC-SHA256 签名
        unsigned char* digest = HMAC(EVP_sha256(), secret.c_str(), secret.length(),
                                     (unsigned char*)signatureInput.c_str(),
                                     signatureInput.length(), nullptr, nullptr);
        std::string signature = std::string((char*)digest, 32);
        auto signatureEncoded = base64UrlEncode(signature);
        
        return headerEncoded + "." + payloadEncoded + "." + signatureEncoded;
    }
    
    static nlohmann::json verify(const std::string& secret, const std::string& token) {
        size_t firstDot = token.find('.');
        size_t lastDot = token.rfind('.');
        if (firstDot == std::string::npos || lastDot == firstDot) {
            throw std::runtime_error("Invalid token format");
        }
        
        std::string headerEncoded = token.substr(0, firstDot);
        std::string payloadEncoded = token.substr(firstDot + 1, lastDot - firstDot - 1);
        std::string signatureEncoded = token.substr(lastDot + 1);
        
        // 验证签名
        std::string signatureInput = headerEncoded + "." + payloadEncoded;
        unsigned char* digest = HMAC(EVP_sha256(), secret.c_str(), secret.length(),
                                     (unsigned char*)signatureInput.c_str(),
                                     signatureInput.length(), nullptr, nullptr);
        std::string expectedSignature = std::string((char*)digest, 32);
        std::string receivedSignature = base64UrlDecode(signatureEncoded);
        
        if (expectedSignature != receivedSignature) {
            throw std::runtime_error("Invalid signature");
        }
        
        // 解析 payload
        auto payload = nlohmann::json::parse(base64UrlDecode(payloadEncoded));
        
        // 检查过期时间
        int64_t exp = payload.value("exp", 0);
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (exp < now) {
            throw std::runtime_error("Token expired");
        }
        
        return payload;
    }
    
  private:
    static std::string base64UrlEncode(const std::string& data) {
        // 标准 Base64 编码
        std::string encoded;
        encoded.reserve(((data.size() + 2) / 3) * 4);
        
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
        for (size_t i = 0; i < data.size(); i += 3) {
            uint32_t octet_a = i < data.size() ? bytes[i] : 0;
            uint32_t octet_b = i + 1 < data.size() ? bytes[i + 1] : 0;
            uint32_t octet_c = i + 2 < data.size() ? bytes[i + 2] : 0;
            
            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            
            encoded.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(triple >> 18) & 0x3F]);
            encoded.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(triple >> 12) & 0x3F]);
            encoded.push_back((i + 1 < data.size()) ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(triple >> 6) & 0x3F] : '=');
            encoded.push_back((i + 2 < data.size()) ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[triple & 0x3F] : '=');
        }
        
        // 替换为 URL 安全字符
        std::replace(encoded.begin(), encoded.end(), '+', '-');
        std::replace(encoded.begin(), encoded.end(), '/', '_');
        return encoded;
    }
    
    static std::string base64UrlDecode(const std::string& encoded) {
        std::string normalized = encoded;
        std::replace(normalized.begin(), normalized.end(), '-', '+');
        std::replace(normalized.begin(), normalized.end(), '_', '/');
        
        // 还原填充 '='
        switch (normalized.size() % 4) {
            case 2: normalized += "=="; break;
            case 3: normalized += "="; break;
        }
        
        // 标准 Base64 解码
        std::vector<unsigned char> decoded;
        decoded.reserve((normalized.size() / 4) * 3);
        
        const char* chars = normalized.c_str();
        for (size_t i = 0; i < normalized.size(); i += 4) {
            uint32_t sextet_a = chars[i] != '=' ? charToValue(chars[i]) : 0;
            uint32_t sextet_b = chars[i + 1] != '=' ? charToValue(chars[i + 1]) : 0;
            uint32_t sextet_c = chars[i + 2] != '=' ? charToValue(chars[i + 2]) : 0;
            uint32_t sextet_d = chars[i + 3] != '=' ? charToValue(chars[i + 3]) : 0;
            
            uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
            
            decoded.push_back((triple >> 16) & 0xFF);
            if (chars[i + 2] != '=') decoded.push_back((triple >> 8) & 0xFF);
            if (chars[i + 3] != '=') decoded.push_back(triple & 0xFF);
        }
        
        return std::string(decoded.begin(), decoded.end());
    }
    
    static uint32_t charToValue(char c) {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return 0;
    }
};

#endif // JWT_HPP