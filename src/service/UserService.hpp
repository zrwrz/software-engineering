#ifndef USER_SERVICE_HPP
#define USER_SERVICE_HPP

#include "dto/request/RegisterRequest.hpp"
#include "dto/response/RegisterResponse.hpp"
#include "dto/request/LoginRequest.hpp"
#include "dto/response/LoginResponse.hpp"
#include "dto/request/UpdateProfileRequest.hpp"
#include "dto/request/ChangePasswordRequest.hpp"
#include "dto/entity/UserDto.hpp"

class UserService {
  private:

  public:
    
    /**
     * @brief 注册用户
     * 
     * 处理注册提交请求：
     * 1. 检查用户名是否已被注册
     * 2. 存储用户信息到数据库（用户名、密码哈希、手机号、邮箱）
     * 3. 返回用户ID
     * 4. 请自行设计一个合理的密码哈希算法。
     * 
     * @param request 注册提交请求（包含用户名、密码明文、手机号、邮箱）
     * @return oatpp::Object<RegisterResponse> 包含用户ID的响应对象
     *         若用户名已存在或数据库操作失败，抛出异常
     */
    static oatpp::Object<RegisterResponse> registerUser(
        const oatpp::Object<RegisterRequest>& request
    );

    /**
     * @brief 用户登录
     * 
     * 处理登录请求：
     * 1. 根据用户名查询用户
     * 2. 验证密码哈希值
     * 3. 检查账号状态
     * 4. 生成并返回 JWT Token
     * 
     * @param secret std::string 类型的字符串，生成 token 时要用到
     * @param request 登录请求对象（包含用户名、明文密码）明文传输
     * @return oatpp::Object<LoginResponse> 包含 token 和用户信息的响应对象，结构：
     *         {
     *              String token     (JWT token 内容)
     *              String tokenType (默认为 "Bearer")
     *              Int64 expiresIn  (默认 7200 秒)
     *              UserDto userInfo (包含 id, username, role, creditScore 字段)
     *         }
     *         若用户不存在、密码错误或账号被禁用，抛出异常
     */
    static oatpp::Object<LoginResponse> login(
        const std::string &secret,
        const oatpp::Object<LoginRequest>& request
    );

    /**
     * @brief 获取指定用户信息
     * @param userId 需要查询信息的用户的 Id
     * @return oatpp::Object<UserDto> 用户信息结构
     */
    static oatpp::Object<UserDto> getUserDetailed(int64_t userId);

    /**
     * @brief 修改个人资料
     * 
     * @param userId 用户ID
     * @param request 修改请求（包含手机号、邮箱）
     * @return oatpp:Object<UserDto> 更新之后的用户信息结构
     * @throws std::runtime_error 用户不存在或操作失败
     */
    static oatpp::Object<UserDto> updateProfile(
        int64_t userId,
        const oatpp::Object<UpdateProfileRequest>& request
    );
    
    /**
     * @brief 修改密码
     * 
     * @param userId 用户ID
     * @param request 修改密码请求（包含旧密码、新密码）
     * @return bool 修改密码成功与否
     * @throws std::runtime_error 用户不存在、旧密码错误或操作失败
     */
    static bool changePassword(
        int64_t userId,
        const oatpp::Object<ChangePasswordRequest>& request
    );

    
};

#endif