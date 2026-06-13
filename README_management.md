# 管理端与辅助功能开发情况说明

## 开发说明

管理端与辅助功能的补充开发基于前后端接口文档进行实现，参考了《需求报告》《总体设计报告》《设计模式报告》《测试报告》，未修改正在开发的文件。

注：发现其中 `AuthController.hpp` 中存在 include 路径问题。

## 2. 新增功能模块

### 2.1 管理端功能

新增了以下管理端功能代码：

- 物品管理
  - 物品列表、物品详情
  - 物品上传申请提交
  - 物品上传申请审核
  - 编辑物品
  - 上下架物品
- 预约借还管理
  - 管理员订单列表
  - 订单详情
  - 审核预约
  - 办理借出
  - 办理归还
- 赔偿处理
  - 查询赔偿记录
  - 新增赔偿记录
  - 更新赔偿状态
- 用户管理
  - 管理端用户列表
  - 启用/禁用用户
- 审计日志
  - 关键管理操作写入审计日志
  - 管理端查询审计日志

### 2.2 辅助功能

新增了以下辅助功能代码：

- 信用积分
  - 查询信用记录
  - 系统管理员手工调整信用分
  - 逾期归还时生成信用扣分记录
- 逾期提醒
  - 定时任务接口 `POST /api/v1/internal/orders/mark-overdue`
  - 将已超期且未归还的订单更新为 `OVERDUE`
  - 写入站内通知记录
- 统计看板
  - `GET /api/v1/stats/dashboard`
  - 返回今日预约数、借用中数量、逾期数量、热门物品排行、信用分布

## 3. 新增目录与文件

### 3.1 Controller 层

新增：

- `src/controller/AdminController.hpp`
- `src/controller/ItemController.hpp`
- `src/controller/OrderController.hpp`
- `src/controller/CreditController.hpp`
- `src/controller/CompensationController.hpp`
- `src/controller/StatsController.hpp`

说明：Controller 还没在 `main.cpp` 中注册。主程序接入路由时，还需在 `main.cpp` 中统一注册。

### 3.2 Service 层

新增：

- `src/service/AdminUserService.hpp/.cpp`
- `src/service/ItemManageService.hpp/.cpp`
- `src/service/OrderManageService.hpp/.cpp`
- `src/service/CreditService.hpp/.cpp`
- `src/service/CompensationService.hpp/.cpp`
- `src/service/StatisticsService.hpp/.cpp`

说明：Service 层负责参数合法性检查、业务动作合法性检查、调用 Repository、写入审计日志等。

### 3.3 Repository 层

新增：

- `src/db/Database.hpp/.cpp`
- `src/db/AuditLogRepository.hpp/.cpp`
- `src/db/AdminUserRepository.hpp/.cpp`
- `src/db/ItemManageRepository.hpp/.cpp`
- `src/db/OrderManageRepository.hpp/.cpp`
- `src/db/CreditRepository.hpp/.cpp`
- `src/db/CompensationRepository.hpp/.cpp`
- `src/db/StatisticsRepository.hpp/.cpp`

说明：Repository 层使用 MySQL Connector/C++ 进行数据库访问，负责物品、订单、信用、赔偿、日志和统计等数据读写。

数据库连接参数通过环境变量配置：

```bash
export JIEWU_DB_HOST="tcp://127.0.0.1:3306"
export JIEWU_DB_USER="root"
export JIEWU_DB_PASSWORD="password"
export JIEWU_DB_NAME="jiewu_project"
```

### 3.4 DTO 层

新增了管理端和辅助功能需要的请求、实体和响应 DTO，主要包括：

- `AdminUserListItemDto`
- `AuditLogDto`
- `CreditRecordDto`
- `CompensationRecordDto`
- `ItemSubmissionDto`
- `StatsDashboardDto`
- `StatusResultDto`
- `MarkOverdueResultDto`
- `AuditRequest`
- `StatusUpdateRequest`
- `ReturnOrderRequest`
- `CreditAdjustRequest`
- `CompensationCreateRequest`
- `ItemSubmissionRequest`
- `ItemUpdateRequest`
- 各类分页列表响应 DTO

### 3.5 工具类

新增：

- `src/utils/AuthHelper.hpp`

说明：用于解析 `Authorization: Bearer <token>`，并依据 SQL 中确认的三类角色枚举进行权限判断：

- `borrower`
- `resource_admin`
- `system_admin`

## 4. 数据库修改

补充了 `sql/schema.sql`，新增/调整了以下表：

### 4.1 保留并调整的表

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

### 4.2 新增表

- `item_submissions`
- `borrow_records`
- `credit_records`
- `compensation_records`
- `notifications`
- `audit_logs`

对应物品上传申请、借还记录、信用记录、赔偿记录、通知记录和操作日志等持久化数据。

## 5. CMake 修改

- 增加了 `OpenSSL` 链接，用于 JWT HMAC。
- 增加 `mysqlcppconn` 检测。
- 若本机安装了 MySQL Connector/C++，将把新增的管理端与辅助功能 `.cpp` 加入编译。
- 若本机未安装 MySQL Connector/C++，会保留原有编译。

安装 MySQL Connector/C++ 后，可执行：

```bash
mkdir -p build
cd build
cmake ..
make
```

## 6. 后续接入和调整说明

因为 `main.cpp` 中还没有注册新增 Controller。后续在路由初始化位置按 Oat++ 的方式注册：

```cpp
router->addController(ItemController::createShared(objectMapper, secret));
router->addController(OrderController::createShared(objectMapper, secret));
router->addController(CreditController::createShared(objectMapper, secret));
router->addController(CompensationController::createShared(objectMapper, secret));
router->addController(StatsController::createShared(objectMapper, secret));
router->addController(AdminController::createShared(objectMapper, secret));
```