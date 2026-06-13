# 管理端与辅助功能当前实现说明

## 1. 模块划分

当前后端采用 Controller、Service、Repository 三层结构：

- Controller：接收 HTTP 请求、读取 Token、做角色校验并返回统一响应。
- Service：做参数和业务规则校验，协调 Repository 与审计日志。
- Repository：封装 MySQL 访问，维护订单、库存、信用、赔偿、通知等数据一致性。

## 2. 当前已接入 Controller

`main.cpp` 已注册以下控制器：

- `AuthController`
- `ItemController`
- `OrderController`
- `AdminController`
- `CreditController`
- `CompensationController`
- `StatsController`

## 3. 已实现功能

### 用户端

- 注册、登录、获取当前用户信息。
- 修改个人资料、修改密码。
- 浏览物品列表、查看物品详情。
- 提交物品上传申请。
- 创建预约、查看我的订单、查看订单详情、取消预约。
- 查询个人信用记录。

### 资源管理员端

- 查询物品上传审核队列。
- 审核物品上传申请。
- 编辑物品信息、上下架物品。
- 查询订单列表和订单详情。
- 审核预约、办理借出、办理归还。
- 查询和创建赔偿记录、更新赔偿状态。
- 查看统计看板。

### 系统管理员端

- 用户列表查询。
- 启用、冻结、封禁用户账号。
- 查询审计日志。
- 手工调整信用分。
- 查询赔偿记录、更新赔偿状态。
- 查看统计看板。

## 4. 角色枚举

全项目统一使用以下角色枚举：

- `borrower`：借用用户
- `resource_admin`：资源管理员
- `system_admin`：系统管理员

接口文档、SQL 表结构、JWT Payload 和权限判断均按上述枚举处理。

## 5. 重要业务规则

- 订单创建后状态为 `CREATED`。
- 审核通过后状态为 `APPROVED`，并预留库存。
- 审核拒绝后状态为 `REJECTED`，不占用库存。
- 办理借出后状态为 `BORROWED`，并写入 `borrow_records`。
- 办理归还后状态为 `RETURNED`，释放库存并写入归还记录。
- 逾期任务将超期未归还订单更新为 `OVERDUE`，写入通知与信用扣分。
- 赔偿记录创建或归还时标记需要赔偿，都会写入信用扣分记录。

## 6. 文档状态

`docs/INTERFACE_DOCUMENTATION.md` 已按当前代码接口重写，删除了旧的重复段落，并将原有 `USER/ADMIN` 角色写法统一修正为 `borrower/resource_admin/system_admin`。
