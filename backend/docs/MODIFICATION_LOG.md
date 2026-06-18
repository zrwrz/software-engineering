# Modification Log

该文件用于记录每次由助手执行的代码修改，便于追踪与版本回退。

## 2026-06-17 联调日志输出增强

- Scope:
  - `src/controller/AuthController.hpp`
  - `src/controller/OrderController.hpp`
  - `src/controller/ItemController.hpp`
- Changes:
  - 为认证、订单、物品相关接口新增联调日志输出。
  - 成功场景输出 `[OK]` 日志，包含 endpoint 和关键上下文（如 `userId`、`itemId`、`orderId`）。
  - 失败场景输出 `[FAIL]` 日志，记录业务异常信息和兜底异常信息。
  - `GET /api/v1/users/me` 增加异常保护，避免联调时无日志的直接 500。
- Expected Result:
  - 前后端联调的每个关键步骤在后端终端都有可读日志，便于定位请求成败。

## 2026-06-17 预约提交卡住问题修复

- Scope:
  - `../frontend/src/api/client.js`
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 前端请求层新增超时机制，默认 15000ms（可通过 `VITE_API_TIMEOUT_MS` 覆盖）。
  - 当后端无响应或网络挂起时，`request()` 会抛出超时错误，避免页面长期停留在“提交中”。
  - 预约提交前增加 `qty <= availableCount` 前置校验，避免库存不足请求反复打到后端。
- Expected Result:
  - 点击“提交预约”后即使后端卡住，也会在超时后恢复按钮并给出错误提示。
  - 库存不足时前端直接提示，不会出现“提交中”假象。

## 2026-06-17 SEED_ITEM_FLOW 预约超时（锁等待）修复

- Scope:
  - `src/service/OrderService.cpp`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将预约创建流程中两处查询从 `SELECT ... FOR UPDATE` 改为普通 `SELECT`：
    - `items` 库存/状态读取
    - `users` 信用分/状态读取
  - 该流程当前并不在创建时扣减库存或更新用户状态，`FOR UPDATE` 会引入不必要行锁，导致高概率 `Lock wait timeout exceeded`。
- Expected Result:
  - 对 `SEED_ITEM_FLOW` 的“提交预约”不再稳定触发锁等待超时。
  - 后端日志中不应再出现该接口的 lock wait timeout。

## 2026-06-17 后端高风险问题修复批次（鉴权/配置/一致性）

- Scope:
  - `src/main.cpp`
  - `src/controller/OrderController.hpp`
  - `src/db/AuditLogRepository.hpp`
  - `src/db/AuditLogRepository.cpp`
  - `src/service/OrderService.cpp`
  - `src/service/UserService.cpp`
  - `src/utils/jwt.hpp`
  - `src/db/OrderManageRepository.cpp`
  - `src/db/ItemManageRepository.cpp`
  - `src/db/CreditRepository.cpp`
  - `src/db/AdminUserRepository.cpp`
  - `src/db/CompensationRepository.cpp`
  - `test/ApiFullTest.py`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 修复 JWT 密钥误用：启动时读取 `JIEWU_JWT_SECRET`，不再把 `"/api/v1"` 当密钥传递。
  - 增加端口与绑定地址安全配置：`JIEWU_SERVER_PORT` 严格校验，支持 `JIEWU_BIND_HOST`（默认 `127.0.0.1`）。
  - 内部逾期接口改为鉴权：`POST /api/v1/internal/orders/mark-overdue` 仅 `system_admin` 可触发。
  - 修复审计日志事务一致性：新增 `createLog(sql::Connection*)`，预约创建日志写入同一事务连接。
  - 降低订单号冲突风险：毫秒时间戳追加 3 位原子序列号。
  - 限制分页 `size` 上限（100）：订单/物品/信用/用户/赔偿/审计接口统一限制，避免大查询拖垮服务。
  - 增加密码长度限制：注册和改密限制为 6~128，登录输入限制最大 128，降低 PBKDF2 超长密码 DoS 风险。
  - JWT 验签比较改为常量时间比较（`CRYPTO_memcmp`）。
  - 适配内部接口鉴权后的测试：`ApiFullTest.py` 内部逾期调用改为复用管理员 token。
- Expected Result:
  - 后端鉴权与配置安全基线提高，联调时不再暴露高危默认行为。
  - 事务日志与业务数据一致性更好，接口在异常/并发场景下更稳定。

## 2026-06-17 信用积分图改为后端实时数据驱动

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将信用趋势图 `sparkPoints` 从写死原型常量改为 `computed` 动态计算。
  - 趋势图使用后端返回的信用流水 `ledger`（delta）和当前 `creditScore` 反推时间序列，按数据自动缩放。
  - 无流水时回退为稳定平线，避免展示误导性的固定原型曲线。
- Expected Result:
  - 信用积分图随真实后端信用记录变化，不再是固定原型图形。

## 2026-06-17 新用户信用图空白修复（显示 100 基线）

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 对信用趋势图增加“无流水”分支：当 `ledger` 为空时，固定返回 `0,40 240,40` 基线点位。
  - 确保新注册用户（无信用流水）进入信用页时，图表可见且为 100 的平行基线，不再出现空白。
- Expected Result:
  - 新用户信用积分图始终可见，默认显示一条平线基线。

## 2026-06-17 信用图可视兜底（固定基线）

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在信用趋势 `svg` 中新增固定 `<line>` 基线（`y=40`），不依赖后端数据与 polyline 计算。
  - 新增 `.spark-baseline` 样式，保证新用户和空数据场景下图区域始终可见一条线。
- Expected Result:
  - 即使趋势点未生成或为空，信用图仍有可见基线，不会出现“整块空白”。

## 2026-06-17 去除信用分下方文案

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 移除用户顶部信用分卡片下方文案行（原“后端实时信用分/等待后端返回信用分”）。
  - 删除对应的 `creditScoreHint` 计算变量，避免无用状态。
- Expected Result:
  - 顶部信用分区域仅显示“信用分”和分值，不再显示说明文字。

## 2026-06-17 回退信用图固定基线兜底

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 移除 `svg` 中固定 `<line class="spark-baseline">`，不再使用额外可视兜底线。
  - 保留基于 `ledger` + `creditScore` 的 `sparkPoints` 计算逻辑。
  - 新用户无信用流水时，仍由 `sparkPoints` 返回平线点位（表示从 100 开始的默认基线）。
- Expected Result:
  - 曲线图仅由信用记录逻辑驱动；新用户仍显示一条 100 基线平线。

## 2026-06-17 新用户信用曲线可见性加固

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 新增 `creditDeltas` 计算，统一信用流水差值来源。
  - 新增 `sparkStroke`：有流水用渐变线，无流水改用高可见纯色 `#7aa7ff`。
  - 折线增加 `stroke-linecap/linejoin`，提高平线显示稳定性。
  - 无流水点位从两点改为三点（`0,40 120,40 240,40`），提升渲染器对平线的可见性。
- Expected Result:
  - 没有信用变更记录的新用户，也能稳定看到从 100 起始的平线。

## 2026-06-17 可借时段改为从当天起两周

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 移除预约抽屉中固定写死的一周日期（2026-04-20~2026-04-26）。
  - 改为运行时动态生成从“今天”开始连续 14 天的可借日期。
  - 星期与日期展示仍保持原 UI 结构（`dow` + `day`）。
- Expected Result:
  - 用户打开预约日历时，总是看到从当前日期开始往后两周的可借时段。

## 2026-06-17 预约日期改为起止区间选择

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将原先可跳选的多日期模式改为“开始日期 + 结束日期”区间模式。
  - 第一次点击记录 `rangeStartDate`，第二次点击记录 `rangeEndDate`，中间日期自动连续选中。
  - 再次点击任意日期会重置为新的开始日期，重新选择结束日期。
  - 提交预约前新增校验：必须同时选到开始和结束日期。
  - 在日历下方新增已选区间提示文案，明确当前选择状态。
- Expected Result:
  - 用户不能再跳着选日期，只能按起止日期选择连续借用区间。

## 2026-06-17 结束日期限制为不早于开始日期

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 区间选择逻辑增加约束：结束日期必须 >= 开始日期。
  - 用户尝试选择更早日期时，保留开始日期并提示“结束日期只能选择开始日期当天或之后”。
- Expected Result:
  - 结束日期不再允许选到开始日期之前，区间选择方向固定为向后。

## 2026-06-17 去除前端“立即借/需审核/可续借”渲染

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 移除物品列表卡片上的“可立即借 / 需审核 / 可续借”标签渲染，避免展示后端 `items` 表不存在的字段。
  - 移除筛选栏中对应的快捷筛选按钮与脚本逻辑（`chips`/`chipDefs` 及过滤条件）。
  - 移除详情抽屉内“需审核”提示块，以及对应的样式定义。
  - 清理离线演示和映射对象里 `instant`/`needReview`/`renewable` 字段，保证前端字段模型与后端接口一致。
- Expected Result:
  - 物品列表与详情页不再渲染后端未提供的业务字段，联调展示与后端数据模型保持一致。

## 2026-06-17 角色文案“学生”统一为“师生用户”

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将用户顶部身份徽标中的非管理员角色文案由“学生”改为“师生用户”，与当前角色模型一致（仅区分管理员与师生用户）。
- Expected Result:
  - 前端不再展示“学生”单一身份，文案统一为“师生用户”。

## 2026-06-17 修复管理员登录被识别为普通用户

- Scope:
  - `../frontend/src/session.js`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 调整角色映射函数 `mapRole`：由仅匹配 `ADMIN`，改为兼容包含 `ADMIN` 的后端角色值（如 `ROLE_ADMIN`、`SYSTEM_ADMIN`、`RESOURCE_ADMIN`）。
  - 避免管理员账号登录后被错误映射为普通用户，从而跳到师生侧界面。
- Expected Result:
  - 管理员账号登录后会被正确识别为管理员，会话与页面视角按管理员逻辑展示。

## 2026-06-17 按系统管理员/资源管理员渲染差异化 UI

- Scope:
  - `../frontend/src/session.js`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/DashboardView.vue`
  - `../frontend/src/views/ItemReviewView.vue`
  - `../frontend/src/views/LoginView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 会话层新增管理员细分能力：保存后端原始角色 `backendRole`，并映射 `adminType`（`system` / `resource` / `legacy` / `none`）。
  - `ItemsView` 按管理员类型渲染：资源管理员可进入管理视图和审核队列；系统管理员仅保留浏览/联调视图，不显示资源审核入口。
  - `DashboardView` 导航与分区按权限差异化：
    - 资源管理员：预约审核、在借监管、系统消息。
    - 系统管理员：操作日志、信用仲裁、赔偿裁定、策略设置、系统消息。
    - 兼容 `legacy` 管理员：保留全部管理分区。
  - 管理端数据加载按权限拆分，避免因访问无权限接口（403）导致控制台整体加载失败。
  - 管理操作函数增加前端权限守卫（信用调整/赔偿/用户状态需系统管理权限；预约审核/借还办理需资源管理权限）。
  - `ItemReviewView` 入口改为仅资源管理员可访问；登录页已登录身份文案支持显示“系统管理员/资源管理员”。
- Expected Result:
  - 前端会根据后端角色（系统管理员/资源管理员）展示不同管理界面与操作入口，未授权功能不会再错误渲染或触发。

## 2026-06-17 系统管理员控制台屏蔽单接口 400 全局失败

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 调整 `loadDashboard` 管理端加载策略：对管理员接口按能力分组加载，并对 `400/403` 做单接口降级处理（返回空数据而不是抛全局错误）。
  - 增加 `tryAdminLoad` 包装：当某个接口暂不可用时，仅在页面提示“部分数据未加载”，不再触发“数据同步失败”整页错误。
  - 资源管理员仅加载资源相关接口；系统管理员仅加载系统治理相关接口，避免跨权限调用导致 `HTTP 400`。
- Expected Result:
  - 系统管理员进入个人主页时，不会因某个不适用接口返回 `HTTP 400` 而出现“数据同步失败”；页面应正常展示可访问模块，并提示部分数据降级信息（如有）。

## 2026-06-17 修复系统管理员接口 400（补齐查询参数）

- Scope:
  - `../frontend/src/api/index.js`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 对管理端 3 个查询接口补齐默认 query 参数，避免后端按参数绑定时出现缺参 `HTTP 400`：
    - `adminApi.users`：默认传 `page/size/keyword/status`。
    - `adminApi.auditLogs`：默认传 `page/size/operatorId/startTime/endTime`。
    - `compensationApi.list`：默认传 `page/size/status/userId/orderId`。
  - 保持前端传参风格与后端控制器声明一致，避免因未传可选筛选项触发框架层参数校验失败。
- Expected Result:
  - 系统管理员进入个人主页时，上述三个接口不再因缺少 query 参数返回 `HTTP 400`。

## 2026-06-17 系统管理员可查看监管订单（只读）

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 新增系统管理员“监管订单”可见能力：导航增加“监管订单”分区，首页增加入口卡片。
  - 监管订单页对系统管理员开放只读视图（可看详情与状态），借出/归还按钮仅资源管理员可用。
  - 管理端加载逻辑调整：系统管理员也会拉取 `/orders` 监管订单列表；当统计接口不可用时，改用订单数据本地回填监管统计值。
- Expected Result:
  - 系统管理员个人主页可以看到监管订单列表与逾期风险，不再出现“看不到监管订单”。
  - 资源管理员仍保留借还办理操作权限。

## 2026-06-17 移除登录页预置管理员账号 admin

- Scope:
  - `../frontend/src/views/LoginView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 取消管理员登录输入框的默认账号值：`adminAccount` 由 `admin` 改为空字符串。
  - 去除离线模拟登录中对账号 `admin` 的特判昵称映射，改为使用用户输入或默认“管理员”。
  - 调整管理员账号输入框占位文案，移除“例如 admin”提示。
- Expected Result:
  - 登录页不再预置 `admin` 管理员账号，用户需手动输入管理员账号。

## 2026-06-17 深浅色模式字体可读性增强

- Scope:
  - `../frontend/src/assets/global.css`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在主题变量中新增语义文本色：`--text-muted`、`--text-on-accent`、`--error-text`、`--tag-done-text`，并分别为深色与浅色模式设置可读值。
  - 将按钮主色文字改为 `var(--text-on-accent)`，避免组件内直接写死 `#fff`。
  - 将 `ItemsView` 错误提示文字颜色由硬编码 `#ffd3d6` 改为 `var(--error-text)`，提升浅色模式对比度。
  - 将 `DashboardView` 中提醒点和已完成标签文字色改为主题变量（替换 `#fff`、`#b8ceff`），保证深浅主题一致可读。
  - 补齐 `--text-muted` 变量定义，修复使用该变量时可能出现的颜色回退问题。
- Expected Result:
  - 主要文本与状态文字在深色/浅色模式下都具备稳定对比度，不再依赖硬编码颜色造成主题下可读性不一致。

## 2026-06-17 消息中心接入关键状态变更通知

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在 Dashboard 增加统一消息写入方法（按 `审核` / `逾期风险` / `系统` 分组、自动时间戳、未读计数、分组自动创建）。
  - 将消息中心接入关键成功动作：预约审核通过/驳回、借出办理、归还办理、用户取消预约、信用分调整、赔偿记录创建、赔偿状态更新、用户状态启停。
  - 新消息会自动进入对应消息分组并标记未读，支持现有“消息中心”页面与角标实时感知。
- Expected Result:
  - 消息中心不再只是静态展示；关键业务状态变更后会即时出现可追踪消息。

## 2026-06-17 用户端消息中心接入状态变更通知

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 为用户侧新增“状态快照 + 去重”消息机制：按用户维度将订单状态、上传审核状态、账号状态写入本地快照并对比变化。
  - 在用户拉取数据后自动生成消息：
    - 订单：`APPROVED`/`REJECTED`/`OVERDUE`/`RETURNED`/`CANCELLED`
    - 上传审核：`已通过`/`已驳回`
    - 账号状态：冻结/禁用/恢复
  - 新消息自动进入现有消息分组并计入未读，不再仅管理端有消息联动。
- Expected Result:
  - 用户端在关键状态变化后会收到可见消息，消息中心与未读角标可反映真实业务变更。

## 2026-06-17 用户端个人中心增加自动轮询刷新

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在用户端个人中心增加 15 秒轮询刷新（调用 `loadDashboard`），用于自动同步订单/消息/信用等数据，无需手动刷新页面。
  - 增加轮询并发保护：上一轮请求未完成时跳过下一轮，避免重复堆积请求。
  - 页面不可见（浏览器标签页隐藏）时暂停本轮拉取，降低无效请求。
  - 组件卸载时自动清理定时器；当角色切换为管理员时停止轮询，切回用户时恢复轮询。
- Expected Result:
  - 用户端状态更新可自动刷新，关键状态变化不再必须手动刷新页面后才能看到。

## 2026-06-17 用户端轮询间隔调整为 5 秒

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将用户端个人中心自动轮询间隔从 `15000ms` 调整为 `5000ms`，提升状态更新时效。
- Expected Result:
  - 用户端关键状态变化在 5 秒内可见，无需手动刷新。

## 2026-06-17 注册页手机号文案改为必填

- Scope:
  - `../frontend/src/views/RegisterView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将注册页手机号字段渲染说明从“选填”改为“必填”。
  - 标签改为“手机号 *”，占位文案改为“必填，11 位手机号”，与后端实际要求对齐。
- Expected Result:
  - 用户在注册页可明确感知手机号是必填字段，减少前后端认知不一致。

## 2026-06-17 用户预约红色计数仅统计待审核

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 调整用户侧导航“预约”红色计数逻辑：由统计全部预约分区订单，改为仅统计 `CREATED`（待审核）状态。
  - `APPROVED`（已通过）与 `REJECTED`（已驳回）不再计入红色计数。
- Expected Result:
  - 用户侧红色预约计数只反映当前待处理预约，不再被已通过/已驳回历史状态放大。

## 2026-06-17 用户端补充借出/归还/逾期状态消息

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 扩展用户订单状态消息映射：新增 `BORROWED`（借出）通知。
  - 保留并强化 `OVERDUE`（逾期）与 `RETURNED`（归还）通知路径，确保三类状态变化均进入消息中心。
- Expected Result:
  - 用户订单状态变更为借出、归还、逾期时，消息中心会产生对应通知并计入未读。

## 2026-06-17 修复系统管理员信用调整后无流水显示

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在系统管理员数据加载分支补充信用流水请求：新增 `creditApi.records({ page: 1, size: 20 })`。
  - 将返回数据映射到 `ledger` 表格数据源（`id/userId/createdAt/reason/changeValue/orderId`）。
  - 非系统管理员分支显式清空 `ledger`，避免状态残留。
- Expected Result:
  - 系统管理员执行信用调整后，回到“信用仲裁”页面可看到流水记录更新，不再出现“无流水显示”。

## 2026-06-17 Dashboard 各列表新增顶栏筛选

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 为 Dashboard 内各分区列表新增“顶栏筛选输入框”（关键词筛选），覆盖：
    - 总览订单、信用流水、消息列表
    - 管理端：预约审核、用户管理、审计日志、在借监管、订单列表、赔偿记录
    - 用户端：我的预约、借用中、借用记录、我的上传、赔偿记录
  - 新增统一筛选状态 `listFilters` 与对应过滤计算属性，按单号/物品/用户/状态等关键字段做匹配。
  - 消息分组支持按关键词过滤消息项（标题/正文/时间）并动态隐藏空分组。
  - 增加 `list-filter-bar` 样式，统一筛选顶栏视觉与间距。
- Expected Result:
  - Dashboard 每个列表分区都可在顶部即时筛选条目，减少手动翻找成本。

## 2026-06-17 Dashboard 列表筛选改为字段下拉多选

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将上一版“关键词输入筛选”替换为“按字段下拉多选筛选（`multiple`）”。
  - 为每个列表定义字段筛选模型（字段名/取值函数），并自动生成各字段可选项（去重）。
  - 增加通用过滤引擎：按字段多选条件组合过滤各列表数据。
  - 消息分区改为按分组/标题/时间多选筛选，同时修复筛选后消息已读计数更新逻辑。
  - 调整筛选栏样式为多列字段布局，并为多选框增加可用高度。
- Expected Result:
  - 每个列表都可按多个字段分别下拉多选完成筛选，不再依赖单输入关键词匹配。

## 2026-06-17 字段筛选改为标签式可取消交互

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将字段筛选交互从 `multiple` 原生多选框改为“单行下拉添加 + 已选标签”模式。
  - 每个字段通过下拉添加一个筛选值，已选条件显示为标签；点击标签 `×` 可单独取消。
  - 增加“清空筛选”按钮，支持按分区一键清空当前列表筛选条件。
  - 增加统一筛选交互函数：可选项排除已选值、标签集合构建、单项移除与整组重置。
  - 更新筛选栏样式为紧凑单行 + 标签行，改善可读性与可操作性。
- Expected Result:
  - 筛选区域不再占用大块空间；已选条件可直观看到并可点叉取消。

## 2026-06-17 筛选交互细化为单行标签流

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将各分区筛选栏统一为“单行下拉添加 + 标签流展示”结构，避免原生多选框占高影响观感。
  - 下拉仅显示未选选项；每次选择追加一个标签；标签点击 `×` 可即时移除。
  - 增加按分区“清空筛选”按钮，支持一键清空当前列表所有字段筛选。
  - 优化样式：筛选区横向紧凑排列，标签行独立换行，整体更轻量。
- Expected Result:
  - 筛选区更简洁，已选条件可视化明显，支持逐项取消，不会出现“选了难取消”的体验问题。

## 2026-06-17 下架物品通知来源者并隐藏下架项

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 用户侧物品列表不再展示 `OFF_SHELF` 物品：用户查询默认带 `status=ON_SHELF`，并在前端过滤中再次兜底隐藏。
  - 管理员执行“下架物品”后，写入本地下架通知事件（含物品名、时间、操作人等信息）。
  - 用户端个人中心轮询时新增下架事件消费逻辑：若命中“我上传且已通过”的物品，则向消息中心推送“物品下架通知”。
  - 上传历史补充 `submitterId` 字段，便于按来源者识别并减少跨用户误匹配。
- Expected Result:
  - 下架后的物品不会继续出现在用户可借列表。
  - 来源用户会在消息中心收到对应下架通知（并保留已读/未读状态）。

## 2026-06-17 下架后管理端列表直接移除

- Scope:
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 物品列表查询统一只拉取 `ON_SHELF`，管理端与用户端都不再展示 `OFF_SHELF` 物品。
  - 管理端下架动作改为“仅下架并移除展示”：下架成功后关闭详情抽屉并从当前列表消失。
  - 前端不再提供“下架后再上架”路径；若已下架，提示“重新上传发起新申请”。
- Expected Result:
  - 下架物品在管理端也会立即消失，业务上等同“删除展示”；后续只能重新上传申请。

## 2026-06-17 用户端轮询间隔再次缩短

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将用户端个人中心自动轮询间隔从 `5000ms` 进一步缩短为 `2000ms`，提升状态更新实时性。
- Expected Result:
  - 用户端状态变化可在约 2 秒内反映到页面，无需手动刷新。

## 2026-06-17 权限与消息归属问题修复

- Scope:
  - `../backend/src/controller/OrderController.hpp`
  - `../backend/src/controller/ItemController.hpp`
  - `../backend/src/service/OrderService.hpp`
  - `../backend/src/service/OrderService.cpp`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/ItemReviewView.vue`
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 后端权限收口：将“创建预约”接口收敛为仅 `borrower` 可调用；将预约审核/借出/归还接口、物品审核/编辑/上下架接口改为仅 `resource_admin` 可操作，避免系统管理员绕过前端“只读监管”限制直接改数据。
  - 后端取消预约权限修复：`cancelReservation` 改为“仅订单发起人本人可取消”，消除管理员可取消他人订单的越权路径。
  - 用户上传历史隔离修复：个人中心读取本地上传历史时，登录态下仅展示 `submitterId` 与当前用户一致的数据，避免旧记录跨账号可见。
  - 下架通知归属修复：下架事件增加 `submitterId`，消息消费阶段按 `submitterId` 精确匹配当前用户，不再按物品名称模糊匹配，避免同名物品误通知。
  - 补充本地演示数据透传：演示流中提交/审核的本地物品数据补充 `submitterId` 传递链路，确保通知与归属判断一致。
- Expected Result:
  - 系统管理员可继续查看监管数据，但不能通过 API 直接执行资源操作；资源管理员职责边界清晰。
  - 取消预约仅能由预约本人发起，不再存在管理员跨用户取消风险。
  - 下架通知只会发送给真实来源用户，避免同名误触发。
  - 本地上传历史不再在不同账号之间串数据。

## 2026-06-17 用户端轮询间隔再次提速

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 将用户端个人中心自动轮询间隔从 `2000ms` 进一步缩短为 `1000ms`，提升状态变化可见速度。
- Expected Result:
  - 用户端关键状态变化可在约 1 秒内反映到页面，减少“刷新偏慢”的体感。

## 2026-06-17 全页面数据轮询统一

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/ItemReviewView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - `Dashboard` 轮询由“仅用户侧”扩展为“管理员与用户侧均启用”，并增加页面可见/窗口聚焦时的即时刷新。
  - `Items` 页面新增物品列表自动轮询（含并发保护、页面不可见跳过、可见/聚焦即时刷新）。
  - `Items` 页面原“待审核计数轮询”间隔从 `4000ms` 调整为 `2000ms`。
  - `ItemReview` 审核队列轮询间隔从 `5000ms` 调整为 `2000ms`。
- Expected Result:
  - 物品页、审核页、个人中心/管理中心均可自动刷新数据，不再依赖手动刷新。
  - 页面切回前台时会立即触发一次同步，减少“看起来没更新”的等待时间。

## 2026-06-17 管理端轮询止闪与接口降压

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - `Dashboard` 轮询改为“静默刷新”：轮询时不再重置 `dashboardError/dashboardInfo` 与订单详情弹窗状态，避免页面持续闪烁。
  - 管理员主页轮询间隔降频到 `5000ms`，用户侧保持 `1000ms`，降低多窗口并发时的后端请求压力。
  - 页面可见性/聚焦触发刷新同样改为静默模式，避免切回页面时出现明显抖动。
- Expected Result:
  - 同时打开用户与多个管理员窗口时，管理员主页不再频繁闪烁。
  - 后端请求压力下降，接口“暂不可用”的出现概率明显降低。

## 2026-06-17 轮询场景 Token 过期自动止损

- Scope:
  - `../frontend/src/views/DashboardView.vue`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/ItemReviewView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 为三个轮询页面增加统一鉴别：当接口返回 `401` 或 `Token expired` 时，判定为登录态失效。
  - 登录态失效后立即停止对应页面轮询定时器，避免继续向后端发送无效请求。
  - 触发自动退出并跳转登录页，同时给出“登录状态已过期，请重新登录”提示。
- Expected Result:
  - Token 过期时不会再出现每隔几秒持续刷失败接口日志。
  - 多窗口场景下，过期会话会自动收敛，不再长期占用后端资源。

## 2026-06-17 客户端全局 Token 过期锁

- Scope:
  - `../frontend/src/api/client.js`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 在请求客户端增加全局 `authExpiredLock`：任一鉴权请求命中 `401/Token expired` 后，立即锁定后续鉴权请求。
  - 命中过期后自动清理本地 token，后续请求直接本地短路返回“登录状态已过期”，不再继续打后端。
  - 登录成功重新写入 token 时自动解除锁定，恢复正常请求。
- Expected Result:
  - 多窗口中只要有一个会话 token 过期，不会继续持续刷后端失败日志。
  - 管理端/用户端会明确进入“需要重新登录”状态，避免“接口像是全挂”的错觉。

## 2026-06-17 Token 过期登录页提醒

- Scope:
  - `../frontend/src/api/client.js`
  - `../frontend/src/views/LoginView.vue`
  - `../frontend/src/views/DashboardView.vue`
  - `../frontend/src/views/ItemsView.vue`
  - `../frontend/src/views/ItemReviewView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 客户端新增“过期提醒标记”存取：命中 `401/Token expired` 或本地过期锁时写入提醒标记。
  - 登录页新增黄色提醒横幅：检测到过期标记或 `reason=expired` 后显示“登录已过期，请重新登录”。
  - 三个业务页在会话过期后统一跳转到 `/?reason=expired`，确保跳转后能明确提示用户。
- Expected Result:
  - 会话过期时，前端会明确提醒用户重新登录，不再出现“无提示直接不可用”的体验。

## 2026-06-17 审查问题修复（并发/401/轮询）

- Scope:
  - `src/service/OrderService.cpp`
  - `src/controller/OrderController.hpp`
  - `../frontend/src/views/ItemsView.vue`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 预约创建并发修复：对物品行 `FOR UPDATE` 并扣除 `CREATED` 待审核占用量后再校验库存，避免并发下重复预约超卖。
  - `/orders/my` 鉴权语义修复：Token 解析失败返回 `401`，业务查询失败返回 `400`，避免非鉴权错误误触发前端“登录过期”。
  - 物品页待审核轮询失败时保留上次列表数据，并提示“已保留上次数据”，不再误清空。
- Expected Result:
  - 同一物品并发预约时库存判断更准确。
  - 用户端轮询不再因普通业务错误被误判为登录过期。
  - 管理端轮询短暂失败时列表不会突然变空。

## 2026-06-17 修复并发建连导致 bus error

- Scope:
  - `src/db/Database.cpp`
  - `src/main.cpp`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - `Database::getConnection()` 增加互斥锁，串行化 MySQL Connector/C++ 的 `get_driver_instance()` + `connect()`，避免 oatpp 多线程同时建连触发原生驱动对齐崩溃（SIGBUS）。
  - `main.cpp` 补充 `<chrono>` 头文件（逾期检查线程使用 `std::chrono::hours`）。
- Expected Result:
  - 前端多窗口/高频轮询同时打后端时，服务不再随机 bus error 退出。

## 2026-06-17 订单详情返回 updatedAt

- Scope:
  - `src/dto/response/ReservationDetailedResponse.hpp`
  - `src/service/OrderService.cpp`
  - `docs/MODIFICATION_LOG.md`
- Changes:
  - 订单详情 DTO 与查询 SQL 补充 `updatedAt` 字段，映射 `orders.updated_at`。
- Expected Result:
  - 前端订单详情「时间节点 → 更新时间」可正常显示。
