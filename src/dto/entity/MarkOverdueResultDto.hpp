#ifndef MARK_OVERDUE_RESULT_DTO_HPP
#define MARK_OVERDUE_RESULT_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 逾期任务结果 DTO
 */
class MarkOverdueResultDto : public oatpp::DTO {
    DTO_INIT(MarkOverdueResultDto, DTO)

    DTO_FIELD(Int32, updatedCount);
    DTO_FIELD(Vector<Int64>, orderIds);
};

#include OATPP_CODEGEN_END(DTO)

#endif
