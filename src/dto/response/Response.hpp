#ifndef GLOBAL_RESPONSE_HPP
#define GLOBAL_RESPONSE_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

// 必须包含这两行宏，用于开启 DTO 的代码生成
#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 全局统一响应包装器
 */
class ApiResponse : public oatpp::DTO {
    DTO_INIT(ApiResponse, DTO) // 初始化宏，生成 createShared 等方法

    DTO_FIELD(Int32, code);       // 业务状态码
    DTO_FIELD(String, message);   // 提示信息
    DTO_FIELD(Any, data);         // 泛型业务数据（Any 类型可以接收任意 DTO 对象）
};

// 结束 DTO 代码生成
#include OATPP_CODEGEN_END(DTO)

#endif