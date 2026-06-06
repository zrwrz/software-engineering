#ifndef USER_SERVICE_HPP
#define USER_SERVICE_HPP

#include "dto/request/RegisterRequest.hpp"
#include "dto/response/RegisterResponse.hpp"

#include <random>
#include <string>

class UserService {
  private:
  public:
    /**
     * @brief 生成注册盐值
     * 
     * （请参考上述文件中关联的头文件来获取方法中涉及的结构的定义）
     * 处理注册第一步请求：
     * 1. 校验请求参数（用户名、手机号、邮箱）
     * 2. 检查用户名、手机号、邮箱是否已被注册
     * 3. 生成随机盐值并缓存
     * 4. 返回盐值和有效期
     *  
     * @param request 注册请求对象（包含用户名、手机号、邮箱）
     * @return oatpp::Object<RegisterSaltResponse> 包含盐值和有效期的响应对象
     *         若校验失败，返回空指针或抛出异常
     */
    static oatpp::Object<RegisterSaltResponse> generateSalt(
        const oatpp::Object<RegisterSaltRequest>& request
    );
    
    /**
     * @brief 注册用户（注册第二步）
     * 
     * 处理注册提交请求：
     * 1. 验证盐值是否有效（存在且未过期）
     * 2. 检查用户名是否已被注册
     * 3. 存储用户信息到数据库（用户名、密码哈希、盐值、手机号、邮箱）
     * 4. 返回用户ID
     * 
     * @param request 注册提交请求（包含用户名、盐值、密码哈希）
     * @return oatpp::Object<RegisterResponse> 包含用户ID的响应对象
     *         若盐值无效、用户名已存在或数据库操作失败，抛出异常
     */
    static oatpp::Object<RegisterResponse> registerUser(
        const oatpp::Object<RegisterCommitRequest>& request
    );

    /**
     * @brief 用户登录
     * 
     * 处理登录请求：
     * 1. 根据用户名查询用户
     * 2. 验证密码（用存储的盐值加密后比对）
     * 3. 检查账号状态
     * 4. 生成并返回 JWT Token
     * 
     * @param request 登录请求对象（包含用户名、明文密码）明文传输
     * @return oatpp::Object<LoginResponse> 包含 token 和用户信息的响应对象
     *         若用户不存在、密码错误或账号被禁用，抛出异常
     */
    static oatpp::Object<LoginResponse> login(
        const oatpp::Object<LoginRequest>& request
    );
};

#endif