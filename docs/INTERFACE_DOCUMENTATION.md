# 第六组项目-前后端接口文档（基于总体设计报告）

## 1. 文档说明
- 本文档依据《第六组总体设计报告》的模块划分整理，覆盖以下核心模块：`User`、`Item`、`Order(Reservation)`、`CreditRecord`、`CompensationRecord`。
- 架构采用前后端分离 + RESTful API，服务端分层为接口层/业务层/数据访问层。
- 目标：给前端与后端提供统一联调契约。

## 2. 接口约定

### 2.1 基础信息
- Base URL：`/api/v1`
- 数据格式：`application/json`
- 字符编码：`UTF-8`
- 时间格式：`yyyy-MM-dd HH:mm:ss`

### 2.2 鉴权方式
- 使用 `JWT Bearer Token`
- 除登录/注册外，默认需要请求头：
  - `Authorization: Bearer <token>`

### 2.3 通用响应结构
```json
{
  "code": 0,
  "message": "ok",
  "data": {}
}
```
- `code=0`：成功
- 非 0：失败（见错误码）

### 2.4 通用错误码
- `1001` 参数校验失败
- `1002` 未登录或 Token 无效
- `1003` 无权限访问
- `2001` 资源不存在
- `2002` 状态不允许当前操作
- `3001` 库存不足/不可预约
- `4001` 数据库操作失败
- `5000` 系统内部错误

---

## 3. 核心数据模型

### 3.1 User（用户）
- `id`：`int64_t`
- `username`：`std::string`
- `password`：`std::string`（后端存储密文）
- `role`：`std::string`（`USER`/`ADMIN`）
- `phone`：`std::string`
- `email`：`std::string`
- `creditScore`：`int32_t`
- `status`：`std::string`（`active`/`banned`/`frozen`）
- `createdAt`：`std::string`（`yyyy-MM-dd HH:mm:ss`）

### 3.2 Item（物品）
- `id`：`int64_t`
- `name`：`std::string`
- `category`：`std::string`
- `description`：`std::string`
- `totalCount`：`int32_t`
- `availableCount`：`int32_t`
- `status`：`std::string`（`ON_SHELF`/`OFF_SHELF`）
- `deposit`：`double`
- `createdBy`：`int64_t`
- `createdAt`：`std::string`（`yyyy-MM-dd HH:mm:ss`）

### 3.3 Order（预约借还订单）
- `id`：`int64_t`
- `orderNo`：`std::string`
- `userId`：`int64_t`
- `itemId`：`int64_t`
- `quantity`：`int32_t`
- `reserveStartTime`：`std::string`（`yyyy-MM-dd HH:mm:ss`）
- `reserveEndTime`：`std::string`（`yyyy-MM-dd HH:mm:ss`）
- `pickupTime`：`std::string`
- `returnTime`：`std::string`
- `status`：`std::string`  
  - `CREATED`（已创建）
  - `APPROVED`（已审核）
  - `REJECTED`（已拒绝）
  - `BORROWED`（已借出）
  - `RETURNED`（已归还）
  - `CANCELLED`（已取消）
  - `OVERDUE`（已逾期）
- `remark`：`std::string`
- `createdAt`：`std::string`（`yyyy-MM-dd HH:mm:ss`）

### 3.4 CreditRecord（信用记录）
- `id`：`int64_t`
- `userId`：`int64_t`
- `orderId`：`int64_t`
- `changeValue`：`int32_t`（正负分）
- `reason`：`std::string`
- `createdAt`：`std::string`（`yyyy-MM-dd HH:mm:ss`）

### 3.5 CompensationRecord（赔偿记录）
- `id`：`int64_t`
- `orderId`：`int64_t`
- `userId`：`int64_t`
- `amount`：`double`
- `reason`：`std::string`
- `status`：`std::string`（`PENDING`/`PAID`/`WAIVED`）
- `createdAt`：`std::string`（`yyyy-MM-dd HH:mm:ss`）

---

## 4. 认证与用户接口

### 4.1 用户注册
- `POST /auth/register`
- 请求体：
```json
{
  "username": "zhangsan",
  "password": "123456",
  "phone": "13800000000",
  "email": "zhangsan@example.com"
}
```
- 返回 `data`：`{ "userId": 10001 }`

### 4.2 用户登录
- `POST /auth/login`
- 请求体：
```json
{
  "username": "zhangsan",
  "password": "123456"
}
```
- 返回 `data`：
```json
{
  "token": "jwt-token",
  "tokenType": "Bearer",
  "expiresIn": 7200,
  "userInfo": {
    "id": 10001,
    "username": "zhangsan",
    "role": "USER",
    "creditScore": 95
  }
}
```

### 4.3 获取当前用户信息
- `GET /users/me`

### 4.4 修改个人资料
- `PUT /users/me`
- 请求体（按需传）：
```json
{
  "phone": "13811112222",
  "email": "new@example.com"
}
```

### 4.5 修改密码
- `PUT /users/me/password`
- 请求体：
```json
{
  "oldPassword": "123456",
  "newPassword": "new@123"
}
```

---

## 5. 物品平台接口（Item Module）

### 5.1 物品列表（分页+筛选）
- `GET /items`
- Query 参数：
  - `page` `size`
  - `keyword`
  - `category`
  - `status`
- 返回 `data`：
```json
{
  "list": [
    {
      "id": 2001,
      "name": "相机A",
      "category": "电子设备",
      "availableCount": 3,
      "status": "ON_SHELF"
    }
  ],
  "page": 1,
  "size": 10,
  "total": 35
}
```

### 5.2 物品详情
- `GET /items/{itemId}`

### 5.3 新增物品（管理员）
- `POST /items`

### 5.4 编辑物品（管理员）
- `PUT /items/{itemId}`

### 5.5 上下架物品（管理员）
- `PATCH /items/{itemId}/status`
- 请求体：
```json
{
  "status": "OFF_SHELF"
}
```

---

## 6. 预约借还接口（Reservation Module）

### 6.1 创建预约
- `POST /orders/reservations`
- 请求体：
```json
{
  "itemId": 2001,
  "quantity": 1,
  "reserveStartTime": "2026-06-01 09:00:00",
  "reserveEndTime": "2026-06-03 18:00:00",
  "remark": "课程拍摄使用"
}
```
- 业务规则：
  - 校验 `availableCount`
  - 时间区间合法性校验
  - 订单初始状态为 `CREATED`

### 6.2 我的预约/订单列表
- `GET /orders/my`
- Query 参数：`page` `size` `status`

### 6.3 订单详情
- `GET /orders/{orderId}`

### 6.4 取消预约（用户）
- `PATCH /orders/{orderId}/cancel`
- 仅 `CREATED` 状态可取消

### 6.5 审核预约（管理员）
- `PATCH /orders/{orderId}/audit`
- 请求体：
```json
{
  "action": "APPROVE",
  "remark": "库存确认可借"
}
```

### 6.6 办理借出（管理员）
- `PATCH /orders/{orderId}/borrow`
- 状态流转：`APPROVED -> BORROWED`

### 6.7 办理归还（管理员）
- `PATCH /orders/{orderId}/return`
- 请求体：
```json
{
  "returnRemark": "外观完好",
  "needCompensation": false
}
```
- 状态流转：`BORROWED -> RETURNED`

### 6.8 逾期任务（系统任务）
- `POST /internal/orders/mark-overdue`
- 供定时任务调用，将超期未归还订单更新为 `OVERDUE`

---

## 7. 信用与统计接口（Credit & Statistics Module）

### 7.1 查询用户信用记录
- `GET /credits/records`
- Query 参数：`userId`（管理员可查全部，用户默认查自己）

### 7.2 手工调整信用分（管理员）
- `POST /credits/adjust`
- 请求体：
```json
{
  "userId": 10001,
  "changeValue": -5,
  "reason": "逾期归还"
}
```

### 7.3 新增赔偿记录（管理员）
- `POST /compensations`
- 请求体：
```json
{
  "orderId": 3001,
  "userId": 10001,
  "amount": 120.5,
  "reason": "设备部件损坏"
}
```

### 7.4 更新赔偿状态（管理员）
- `PATCH /compensations/{id}/status`
- 请求体：
```json
{
  "status": "PAID"
}
```

### 7.5 统计看板
- `GET /stats/dashboard`
- 返回示例：
```json
{
  "todayReservationCount": 12,
  "borrowingCount": 48,
  "overdueCount": 4,
  "topBorrowItems": [
    { "itemId": 2001, "name": "相机A", "count": 21 }
  ],
  "creditDistribution": {
    "excellent": 120,
    "normal": 30,
    "risk": 6
  }
}
```

---

## 8. 管理端接口补充

### 8.1 用户管理列表（管理员）
- `GET /admin/users`

### 8.2 启用/禁用用户（管理员）
- `PATCH /admin/users/{userId}/status`

### 8.3 审计日志查询（管理员）
- `GET /admin/audit-logs`

---

## 9. 前后端联调建议
- 所有枚举值（订单状态、角色、赔偿状态）由后端统一返回字典或在前端集中常量化。
- 对关键状态流转增加幂等处理（重复借出/重复归还请求）。
- 对库存与订单创建采用事务，避免超卖与脏状态。
- 前端按接口错误码做统一提示与兜底。

## 10. 版本记录
- `v1.0`：依据总体设计报告初版输出，覆盖核心业务接口与联调规则。
