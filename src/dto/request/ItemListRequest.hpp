#ifndef ITEM_LIST_REQUEST_HPP
#define ITEM_LIST_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品列表查询请求
 * 对应 GET /items 接口的 Query 参数
 */
class ItemListRequest : public oatpp::DTO {
    DTO_INIT(ItemListRequest, DTO)

    // 分页参数（带默认值，防止前端未传导致异常）
    DTO_FIELD(Int32, page) = 1;
    DTO_FIELD(Int32, size) = 10;

    // 筛选参数（可选，未传时为 nullptr）
    DTO_FIELD(String, keyword);
    DTO_FIELD(String, category);
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif // ITEM_LIST_REQUEST_HPP