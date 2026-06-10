## 1. 数据库修改

补充了 `sql/schema.sql`，新增/调整了以下表：

### 1.1 调整的表

- `users`
  - 保留原角色枚举：`borrower`、`resource_admin`、`system_admin`
  - 保留原账号状态：`active`、`banned`、`frozen`
- `items`
  - 补充 `total_count`
  - 补充 `available_count`
  - 补充 `deposit`
  - 将状态调整为接口文档中的 `ON_SHELF` / `OFF_SHELF`
- `orders`
  - 补充了 `quantity`
  - 将预约时间字段调整为了 `reserve_start_time` / `reserve_end_time`
  - 补充了 `pickup_time` / `return_time`
  - 将订单状态调整为接口文档中的 `CREATED`、`APPROVED`、`REJECTED`、`BORROWED`、`RETURNED`、`CANCELLED`、`OVERDUE`
  - 补充了审核、借出、归还相关操作人字段

### 1.2 新增的表

- `item_submissions`
- `borrow_records`
- `credit_records`
- `compensation_records`
- `notifications`
- `audit_logs`

对应物品上传申请、借还记录、信用记录、赔偿记录、通知记录和操作日志等持久化数据。

## 2. CMake 修改

- 增加了 `OpenSSL` 链接，用于 JWT HMAC。
- 增加 `mysqlcppconn` 检测。