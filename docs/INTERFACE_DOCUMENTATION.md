# 第六组项目接口说明文档

## 1. 文档说明

本接口文档以当前代码实现为准，覆盖认证用户、物品平台、预约借还、管理端、信用积分、赔偿处理和统计看板接口。所有接口统一使用前缀 `/api/v1`。

## 2. 通用约定

- 数据格式：`application/json`
- 时间格式：`yyyy-MM-dd HH:mm:ss`
- 鉴权方式：除注册、登录、物品公开列表与详情外，其他接口需要请求头：`Authorization: Bearer <token>`
- 角色枚举统一为：`borrower`、`resource_admin`、`system_admin`
- 用户状态：`active`、`banned`、`frozen`
- 物品状态：`ON_SHELF`、`OFF_SHELF`
- 物品上传申请状态：`PENDING`、`APPROVED`、`REJECTED`
- 订单状态：`CREATED`、`APPROVED`、`REJECTED`、`BORROWED`、`RETURNED`、`CANCELLED`、`OVERDUE`
- 赔偿状态：`PENDING`、`PAID`、`WAIVED`

## 3. 通用响应

```json
{
  "code": 0,
  "message": "ok",
  "data": {}
}
```

常用错误码：`1001` 参数校验失败，`1002` 未登录或 Token 无效，`1003` 无权限访问，`2001` 资源不存在，`2002` 状态不允许当前操作，`3001` 库存不足或不可预约，`4001` 数据库操作失败，`5000` 系统内部错误。

## 4. 认证与用户接口

### 4.1 用户注册

`POST /api/v1/auth/register`

请求体：
```json
{
  "username": "zhangsan",
  "password": "123456",
  "phone": "13800000000",
  "email": "zhangsan@example.com"
}
```

返回 data：
```json
{ "userId": 10001 }
```

### 4.2 用户登录

`POST /api/v1/auth/login`

请求体：
```json
{
  "username": "zhangsan",
  "password": "123456"
}
```

返回 data：
```json
{
  "token": "jwt-token",
  "tokenType": "Bearer",
  "expiresIn": 7200,
  "userInfo": {
    "id": 10001,
    "username": "zhangsan",
    "role": "borrower",
    "creditScore": 100
  }
}
```

### 4.3 获取当前用户信息

`GET /api/v1/users/me`

### 4.4 修改当前用户资料

`PUT /api/v1/users/me`

请求体按需传字段：
```json
{
  "phone": "13811112222",
  "email": "new@example.com"
}
```

### 4.5 修改当前用户密码

`PUT /api/v1/users/me/password`

请求体：
```json
{
  "oldPassword": "123456",
  "newPassword": "new@123"
}
```

## 5. 物品平台接口

### 5.1 物品列表

`GET /api/v1/items`

Query：`page`、`size`、`keyword`、`category`、`status`

返回 data：
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

`GET /api/v1/items/{itemId}`

### 5.3 提交物品上传申请

`POST /api/v1/items/submissions`

权限：`borrower`、`resource_admin`、`system_admin`

请求体：
```json
{
  "name": "投影仪",
  "category": "电子设备",
  "description": "课堂展示使用",
  "totalCount": 1,
  "deposit": 100.0
}
```

返回 data：
```json
{
  "submissionId": 3001,
  "status": "PENDING"
}
```

### 5.4 查询物品上传审核队列

`GET /api/v1/admin/items/submissions`

权限：`resource_admin`、`system_admin`

Query：`page`、`size`、`status`

### 5.5 审核物品上传申请

`PATCH /api/v1/admin/items/submissions/{submissionId}/audit`

权限：`resource_admin`、`system_admin`

请求体：
```json
{
  "action": "APPROVE",
  "remark": "信息完整"
}
```

### 5.6 编辑物品

`PUT /api/v1/items/{itemId}`

权限：`resource_admin`、`system_admin`

请求体按需传字段：
```json
{
  "name": "投影仪A",
  "category": "电子设备",
  "description": "更新说明",
  "totalCount": 2,
  "deposit": 100.0
}
```

### 5.7 上下架物品

`PATCH /api/v1/items/{itemId}/status`

权限：`resource_admin`、`system_admin`

请求体：
```json
{ "status": "OFF_SHELF" }
```

## 6. 预约借还接口

### 6.1 创建预约

`POST /api/v1/orders/reservations`

权限：`borrower`、`resource_admin`、`system_admin`

请求体：
```json
{
  "itemId": 2001,
  "quantity": 1,
  "reserveStartTime": "2026-06-01 09:00:00",
  "reserveEndTime": "2026-06-03 18:00:00",
  "remark": "课程拍摄使用"
}
```

### 6.2 我的预约/订单列表

`GET /api/v1/orders/my`

Query：`page`、`size`、`status`

### 6.3 管理端订单列表

`GET /api/v1/orders`

权限：`resource_admin`、`system_admin`

Query：`page`、`size`、`status`

### 6.4 订单详情

`GET /api/v1/orders/{orderId}`

借用用户只能查看自己的订单；管理员可查看所有订单。

### 6.5 取消预约

`PATCH /api/v1/orders/{orderId}/cancel`

仅 `CREATED` 状态可取消。

### 6.6 审核预约

`PATCH /api/v1/orders/{orderId}/audit`

权限：`resource_admin`、`system_admin`

请求体：
```json
{
  "action": "APPROVE",
  "reason": "库存确认可借"
}
```

### 6.7 办理借出

`PATCH /api/v1/orders/{orderId}/borrow`

权限：`resource_admin`、`system_admin`

仅 `APPROVED` 状态可办理借出，成功后订单进入 `BORROWED`，并生成借出记录。

### 6.8 办理归还

`PATCH /api/v1/orders/{orderId}/return`

权限：`resource_admin`、`system_admin`

请求体：
```json
{
  "returnRemark": "物品完好",
  "needCompensation": false
}
```

仅 `BORROWED` 或 `OVERDUE` 状态可办理归还。成功后释放库存、生成归还记录，并按情况生成信用记录或赔偿记录。

### 6.9 标记逾期任务

`POST /api/v1/internal/orders/mark-overdue`

系统内部任务接口。将超过预约结束时间且仍处于 `BORROWED` 的订单更新为 `OVERDUE`，并写入通知和信用扣分记录。

## 7. 管理端用户与日志接口

### 7.1 用户列表

`GET /api/v1/admin/users`

权限：`system_admin`

Query：`page`、`size`、`keyword`、`status`

### 7.2 更新用户状态

`PATCH /api/v1/admin/users/{userId}/status`

权限：`system_admin`

请求体：
```json
{ "status": "frozen" }
```

### 7.3 审计日志查询

`GET /api/v1/admin/audit-logs`

权限：`system_admin`

Query：`page`、`size`、`operatorId`、`startTime`、`endTime`

## 8. 信用积分接口

### 8.1 信用记录查询

`GET /api/v1/credits/records`

Query：`userId`、`page`、`size`

`borrower` 只能查看自己的信用记录；管理员可按 `userId` 查询。

### 8.2 手工调整信用分

`POST /api/v1/credits/adjust`

权限：`system_admin`

请求体：
```json
{
  "userId": 10001,
  "changeValue": -5,
  "reason": "人工调整"
}
```

## 9. 赔偿处理接口

### 9.1 查询赔偿记录

`GET /api/v1/compensations`

权限：`resource_admin`、`system_admin`

Query：`page`、`size`、`status`、`userId`、`orderId`

### 9.2 新增赔偿记录

`POST /api/v1/compensations`

权限：`resource_admin`、`system_admin`

请求体：
```json
{
  "orderId": 3001,
  "userId": 10001,
  "amount": 50.0,
  "reason": "物品损坏"
}
```

### 9.3 更新赔偿状态

`PATCH /api/v1/compensations/{id}/status`

权限：`resource_admin`、`system_admin`

请求体：
```json
{ "status": "PAID" }
```

## 10. 统计看板接口

### 10.1 统计看板

`GET /api/v1/stats/dashboard`

权限：`resource_admin`、`system_admin`

返回今日预约数、借用中数量、逾期数量、热门物品排行和信用分布。
