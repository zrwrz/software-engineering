#ifndef ITEM_DETAILED_RESPONSE_HPP
#define ITEM_DETAILED_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品详情响应 DTO
 * 对应 GET /items/{itemId} 接口的响应 data 结构
 */
class ItemDetailedResponse : public oatpp::DTO {
    DTO_INIT(ItemDetailedResponse, DTO)

    DTO_FIELD(Int64, id);              // 物品唯一ID
    DTO_FIELD(String, name);           // 物品名称
    DTO_FIELD(String, category);       // 类别
    DTO_FIELD(String, description);    // 详细描述
    DTO_FIELD(Int32, totalCount);      // 总库存数量
    DTO_FIELD(Int32, availableCount);  // 可用库存数量
    DTO_FIELD(String, status);         // 状态 (如 "ON_SHELF")
    DTO_FIELD(Float64, deposit);       // 押金金额
    DTO_FIELD(Int64, createdBy);       // 创建人ID
    DTO_FIELD(String, createdAt);      // 创建时间 (字符串格式 "YYYY-MM-DD HH:mm:ss")
};

#include OATPP_CODEGEN_END(DTO)

#endif // ITEM_DETAILED_RESPONSE_HPP