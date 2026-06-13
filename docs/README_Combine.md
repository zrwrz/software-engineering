# 用户端管理端统合说明

## 功能完善

### 物品模块

- 物品模块接口：
  - `GET /api/v1/items`
  - `GET /api/v1/items/{itemId}`
  - `POST /api/v1/items/submissions`
  - `GET /api/v1/admin/items/submissions`
  - `PATCH /api/v1/admin/items/submissions/{submissionId}/audit`
  - `PUT /api/v1/items/{itemId}`
  - `PATCH /api/v1/items/{itemId}/status`
- 物品上传申请审核通过后创建正式物品，拒绝后仅更新申请状态。

### 预约借还模块

- 创建预约时校验物品状态、库存、用户状态和信用分。
- 审核通过时预留库存，审核拒绝时不占用库存。
- 办理借出时更新订单状态为 `BORROWED`，写入借出记录和站内通知。
- 办理归还时释放库存，写入归还记录，按时归还生成信用加分记录。
- 归还时如 `needCompensation=true`，自动生成赔偿记录和信用扣分记录。
- 逾期任务会将超期未还订单更新为 `OVERDUE`，并写入逾期通知和信用扣分记录。

### 辅助模块

- 赔偿记录创建时同步写入信用扣分和通知。
- 信用记录查询保持借用用户只能看自己、管理员可按用户筛选。
- 统计看板继续基于订单、用户信用和热门物品聚合计算。

## 数据库与环境变量

数据库连接支持以下环境变量：

```bash
export JIEWU_DB_HOST="tcp://127.0.0.1:3306"
export JIEWU_DB_USER="root"
export JIEWU_DB_PASSWORD="password"
export JIEWU_DB_NAME="jiewu_project"
```

同时兼容变量：`DB_HOST`、`DB_USER`、`DB_PASSWORD`、`DB_NAME`。

## 编译运行（详细可参考 CompileCommands.md）

```bash
mkdir -p build
cd build
cmake ..
make -j
./jiewu_server
```

依赖：Oat++、OpenSSL、MySQL Connector/C++。

## 测试

用户基础接口测试：

```bash
python test/UserTest.py
```

完整接口流程测试：

```bash
python test/ApiFullTest.py
```

完整流程测试支持以下环境变量：

```bash
export API_BASE_URL="http://localhost:8081/api/v1"
export RESOURCE_ADMIN_USERNAME="resource_admin"
export RESOURCE_ADMIN_PASSWORD="password"
export SYSTEM_ADMIN_USERNAME="system_admin"
export SYSTEM_ADMIN_PASSWORD="password"
```

如果没有配置管理员账号，测试脚本会跳过管理端和系统管理员专属接口，只运行普通用户可访问的测试。
