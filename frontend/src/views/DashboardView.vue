<template>
  <div class="dash">
    <header class="user-bar">
      <div class="identity">
        <div class="avatar" aria-hidden="true">{{ avatarChar }}</div>
        <div>
          <div class="name-row">
            <span class="name">{{ session.displayName || '用户' }}</span>
            <span class="badge badge-role">{{ roleBadgeLabel }}</span>
          </div>
          <p class="sub">{{ subtitle }}</p>
        </div>
      </div>
      <div class="user-bar-tail">
        <div v-if="session.role !== 'admin'" class="credit-block" aria-label="信用分">
          <svg class="ring" viewBox="0 0 36 36" width="56" height="56">
            <defs>
              <linearGradient id="credit-ring" x1="0" x2="1" y1="0" y2="1">
                <stop offset="0%" stop-color="#4f9bff" />
                <stop offset="100%" stop-color="#3ecf8e" />
              </linearGradient>
            </defs>
            <path
              class="ring-bg"
              d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
            />
            <path
              class="ring-fg"
              stroke="url(#credit-ring)"
              :stroke-dasharray="`${creditPct}, 100`"
              d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
            />
          </svg>
          <div>
            <p class="credit-label">信用分</p>
            <p class="credit-val">{{ creditScoreDisplay }}</p>
          </div>
        </div>
        <div v-else-if="canResourceAdmin" class="admin-kpis" aria-label="管理待办">
          <div class="kpi">
            <p class="kpi-val">{{ adminStats.pending }}</p>
            <p class="kpi-lbl">待审核</p>
          </div>
          <div class="kpi">
            <p class="kpi-val">{{ adminStats.overdue }}</p>
            <p class="kpi-lbl">监管中</p>
          </div>
          <div class="kpi">
            <p class="kpi-val">{{ adminStats.borrowing }}</p>
            <p class="kpi-lbl">在借中</p>
          </div>
        </div>
        <div class="user-bar-actions">
          <router-link to="/items" class="btn btn-ghost sm">{{
            session.role === 'admin' ? '物品与库存' : '去借物'
          }}</router-link>
          <button type="button" class="btn btn-ghost sm" @click="doLogout">退出</button>
        </div>
      </div>
    </header>

    <p v-if="offline" class="offline-hint">
      离线演示：未连接后端服务，以下为本地示例数据。
    </p>
    <p v-if="dashboardError" class="offline-hint">
      数据同步失败：{{ dashboardError }}
    </p>
    <p v-if="dashboardInfo" class="offline-hint">
      {{ dashboardInfo }}
    </p>
    <p v-if="userAccountAlert" class="account-alert">
      {{ userAccountAlert }}
    </p>

    <div class="layout">
      <nav class="top-nav desktop-only" aria-label="主导航">
        <button
          v-for="tab in navTabs"
          :key="tab.id"
          type="button"
          class="nav-item"
          :class="{ active: section === tab.id }"
          @click="onTabClick(tab.id)"
        >
          {{ tab.label }}
          <span v-if="navBadgeMap[tab.id]" class="dot">{{ navBadgeMap[tab.id] }}</span>
        </button>
      </nav>

      <main class="content">
        <section v-if="section === 'home'" class="panel">
          <template v-if="isAdminSession">
            <h2 class="h2">管理概览</h2>
            <p class="lead">面向管理员的总览页：根据权限展示资源监管或系统治理能力。</p>
            <p v-if="SHOW_API_HINT" class="lead api-hint">
              接口：`GET /stats/dashboard`、`GET /orders`、`GET /orders/{orderId}`、`PATCH /orders/{orderId}/audit`、`PATCH /orders/{orderId}/borrow`、`PATCH /orders/{orderId}/return`
            </p>

            <div class="cards overview-cards">
              <article v-if="canResourceAdmin" class="action-card">
                <span class="badge badge-warn">预约审核</span>
                <h3>待审核 {{ adminStats.pending }} 条</h3>
                <p>集中处理待审核预约，支持通过 / 驳回并记录审核备注。</p>
                <button type="button" class="btn btn-ghost sm" @click="jumpToSection('reserve')">进入审核队列</button>
              </article>
              <article v-if="canResourceAdmin" class="action-card">
                <span class="badge badge-danger">逾期在借</span>
                <h3>监管中 {{ adminStats.overdue }} 单</h3>
                <p>跟踪逾期与在借风险，支持通知借用人并推进归还流程。</p>
                <button type="button" class="btn btn-ghost sm" @click="jumpToSection('borrow')">打开监管列表</button>
              </article>
              <article v-if="canResourceAdmin" class="action-card">
                <span class="badge badge-success">库存</span>
                <h3>在借中 {{ adminStats.borrowing }} 单</h3>
                <p>联动物品管理视图，持续处理库存与上架状态。</p>
                <router-link to="/items" class="btn btn-ghost sm">去物品列表</router-link>
              </article>
              <article v-if="canSystemAdmin" class="action-card">
                <span class="badge badge-warn">系统治理</span>
                <h3>用户与策略管理</h3>
                <p>维护账号状态、审计日志与信用仲裁策略。</p>
                <button type="button" class="btn btn-ghost sm" @click="jumpToSection('settings')">打开策略设置</button>
              </article>
              <article v-if="canSystemAdmin" class="action-card">
                <span class="badge badge-danger">风控审计</span>
                <h3>审计追踪与风险闭环</h3>
                <p>查看操作日志并联动系统风控消息，完成治理闭环。</p>
                <button type="button" class="btn btn-ghost sm" @click="jumpToSection('history')">查看操作日志</button>
              </article>
              <article v-if="canSystemAdmin" class="action-card">
                <span class="badge badge-muted">监管订单</span>
                <h3>只读查看在借风险</h3>
                <p>系统管理员可查看监管订单与逾期风险，借还办理由资源管理员执行。</p>
                <button type="button" class="btn btn-ghost sm" @click="jumpToSection('borrow')">查看监管订单</button>
              </article>
            </div>

            <h3 class="h3">风控提醒</h3>
            <ul v-if="DEMO_MODE" class="alerts">
              <li class="alert danger">
                <strong>高风险借用</strong>：示波器 ORD-009 逾期，建议联系实验室安全员协同处理。
              </li>
              <li class="alert warn">
                <strong>系统治理待办</strong>：激光测距仪报损证据齐全，等待资源管理员完成赔偿裁定。
              </li>
              <li class="alert muted">
                <strong>策略建议</strong>：体育馆羽毛球拍借用高峰将至，可临时缩短最长借期（示意）。
              </li>
            </ul>
            <ul v-else class="alerts">
              <li class="alert muted">
                <strong>系统提示</strong>：风控提醒将基于实时订单与赔偿数据自动生成。
              </li>
            </ul>

            <h3 class="h3">监管订单预览</h3>
          </template>
          <template v-else>
            <h2 class="h2">总览</h2>
            <p class="lead">用于查看订单摘要与风险提醒；具体操作请前往对应分区。</p>
            <p v-if="SHOW_API_HINT" class="lead api-hint">
              接口：`GET /users/me`、`GET /orders/my`、`GET /orders/{orderId}`、`PATCH /orders/{orderId}/cancel`、`GET /credits/records`
            </p>

            <h3 class="h3">风险提醒</h3>
            <ul v-if="DEMO_MODE" class="alerts">
              <li class="alert danger">
                <strong>逾期</strong>：订单 ORD-009 已逾期 1 天，请尽快归还并查看消息中心处理方案。
              </li>
              <li class="alert warn">
                <strong>待上传赔偿凭证</strong>：物品「激光测距仪」报损待确认，请在 48h 内上传凭证。
              </li>
              <li class="alert muted">
                <strong>预约被驳回</strong>：「实验室示波器」因时段冲突被驳回，可改时段重新提交。
              </li>
            </ul>
            <ul v-else class="alerts">
              <li class="alert muted">
                <strong>系统提示</strong>：风险提醒将根据你的真实订单状态自动更新。
              </li>
            </ul>

            <h3 class="h3">我的订单</h3>
          </template>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.homeOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('homeOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('homeOrders', field.key)"
                    :key="`homeOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('homeOrders')" type="button" class="btn btn-ghost sm" @click="clearListFilters('homeOrders')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('homeOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('homeOrders')"
                :key="`homeOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('homeOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap desktop-only">
            <table class="orders">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>状态</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredHomeOrders" :key="o.id">
                  <td><span class="bar" :class="o.tone" />{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                </tr>
              </tbody>
            </table>
          </div>
          <div class="mobile-only card-list">
            <article v-for="o in filteredHomeOrders" :key="o.id" class="order-card">
              <div class="stripe" :class="o.tone" />
              <div class="oc-body">
                <p class="oc-id">{{ o.id }}</p>
                <p class="oc-item">{{ o.item }}</p>
                <span class="tag" :class="o.tone">{{ o.status }}</span>
              </div>
            </article>
          </div>
        </section>

        <section v-else-if="section === 'credit'" class="panel">
          <template v-if="canSystemAdmin">
            <h2 class="h2">信用仲裁</h2>
            <p class="lead">管理员可手工调整信用分，并结合流水核对处理依据。</p>
            <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`POST /credits/adjust`、`GET /credits/records`</p>
            <div class="cards">
              <article class="action-card">
                <h3>手工调整信用分</h3>
                <div class="form-grid">
                  <label>
                    <span class="kpi-lbl">用户ID</span>
                    <select v-model.number="creditAdjustForm.userId" class="select">
                      <option :value="0">请选择用户</option>
                      <option v-for="u in selectableUserOptions" :key="`credit-user-${u.id}`" :value="u.id">
                        {{ u.label }}
                      </option>
                    </select>
                  </label>
                  <label>
                    <span class="kpi-lbl">分值变化（可负）</span>
                    <input v-model.number="creditAdjustForm.changeValue" class="input" type="number" />
                  </label>
                </div>
                <label class="form-grid">
                  <span class="kpi-lbl">原因</span>
                  <input v-model="creditAdjustForm.reason" class="input" type="text" />
                </label>
                <button type="button" class="btn btn-primary sm" :disabled="savingCreditAdjust" @click="submitCreditAdjust">
                  {{ savingCreditAdjust ? '提交中…' : '提交调整' }}
                </button>
              </article>
            </div>
            <h3 class="h3">最近信用流水</h3>
            <div class="list-filter-bar">
              <div class="filter-inline">
                <label v-for="field in listFilterDefinitions.creditLedger" :key="field.key" class="filter-field">
                  <span class="kpi-lbl">{{ field.label }}</span>
                  <select class="select filter-add-select" @change="onFilterSelectChange('creditLedger', field.key, $event)">
                    <option value="">添加{{ field.label }}</option>
                    <option
                      v-for="opt in getAvailableFilterOptions('creditLedger', field.key)"
                      :key="`creditLedger-${field.key}-${opt}`"
                      :value="opt"
                    >
                      {{ opt }}
                    </option>
                  </select>
                </label>
                <button v-if="hasActiveFilters('creditLedger')" type="button" class="btn btn-ghost sm" @click="clearListFilters('creditLedger')">
                  清空筛选
                </button>
              </div>
              <div v-if="selectedFilterEntries('creditLedger').length" class="filter-chip-row">
                <button
                  v-for="entry in selectedFilterEntries('creditLedger')"
                  :key="`creditLedger-chip-${entry.fieldKey}-${entry.value}`"
                  type="button"
                  class="filter-chip"
                  @click="removeFilterValue('creditLedger', entry.fieldKey, entry.value)"
                >
                  <span class="chip-field">{{ entry.fieldLabel }}</span>
                  <span class="chip-value">{{ entry.value }}</span>
                  <span class="chip-close">×</span>
                </button>
              </div>
            </div>
            <div class="table-wrap">
              <table class="ledger">
                <thead>
                  <tr>
                    <th>ID</th>
                    <th>用户ID</th>
                    <th>时间</th>
                    <th>原因</th>
                    <th>分值</th>
                    <th>关联单号</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="row in filteredLedger" :key="row.id">
                    <td class="mono">{{ row.id }}</td>
                    <td class="mono">{{ row.userId }}</td>
                    <td>{{ row.t }}</td>
                    <td>{{ row.reason }}</td>
                    <td :class="row.delta >= 0 ? 'pos' : 'neg'">{{ row.delta >= 0 ? '+' : '' }}{{ row.delta }}</td>
                    <td class="mono">{{ row.ref }}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </template>
          <template v-else>
            <h2 class="h2">信用与积分</h2>
            <p class="lead">当前分 {{ creditScoreDisplay }} · 守约加分，逾期与报损扣分。</p>
            <div class="sparkline" aria-label="近 30 天趋势简图">
              <svg viewBox="0 0 240 80" preserveAspectRatio="none">
                <polyline
                  fill="none"
                  :stroke="sparkStroke"
                  stroke-width="2.5"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  :points="sparkPoints"
                />
                <defs>
                  <linearGradient id="spark-grad" x1="0" x2="1" y1="0" y2="0">
                    <stop offset="0%" stop-color="#3d8bfd" />
                    <stop offset="100%" stop-color="#3ecf8e" />
                  </linearGradient>
                </defs>
              </svg>
              <div class="spark-labels">
                <span>30 天前</span>
                <span>今天</span>
              </div>
            </div>
            <h3 class="h3">流水</h3>
            <div class="list-filter-bar">
              <div class="filter-inline">
                <label v-for="field in listFilterDefinitions.creditLedger" :key="field.key" class="filter-field">
                  <span class="kpi-lbl">{{ field.label }}</span>
                  <select class="select filter-add-select" @change="onFilterSelectChange('creditLedger', field.key, $event)">
                    <option value="">添加{{ field.label }}</option>
                    <option
                      v-for="opt in getAvailableFilterOptions('creditLedger', field.key)"
                      :key="`creditLedger-user-${field.key}-${opt}`"
                      :value="opt"
                    >
                      {{ opt }}
                    </option>
                  </select>
                </label>
                <button v-if="hasActiveFilters('creditLedger')" type="button" class="btn btn-ghost sm" @click="clearListFilters('creditLedger')">
                  清空筛选
                </button>
              </div>
              <div v-if="selectedFilterEntries('creditLedger').length" class="filter-chip-row">
                <button
                  v-for="entry in selectedFilterEntries('creditLedger')"
                  :key="`creditLedger-user-chip-${entry.fieldKey}-${entry.value}`"
                  type="button"
                  class="filter-chip"
                  @click="removeFilterValue('creditLedger', entry.fieldKey, entry.value)"
                >
                  <span class="chip-field">{{ entry.fieldLabel }}</span>
                  <span class="chip-value">{{ entry.value }}</span>
                  <span class="chip-close">×</span>
                </button>
              </div>
            </div>
            <div class="table-wrap">
              <table class="ledger">
                <thead>
                  <tr>
                    <th>用户ID</th>
                    <th>时间</th>
                    <th>原因</th>
                    <th>分值</th>
                    <th>关联单号</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="row in filteredLedger" :key="row.id">
                    <td class="mono">{{ row.userId }}</td>
                    <td>{{ row.t }}</td>
                    <td>{{ row.reason }}</td>
                    <td :class="row.delta >= 0 ? 'pos' : 'neg'">{{ row.delta >= 0 ? '+' : '' }}{{ row.delta }}</td>
                    <td class="mono">{{ row.ref }}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </template>
        </section>

        <section v-else-if="section === 'messages'" class="panel">
          <div class="msg-head">
            <h2 class="h2">{{ session.role === 'admin' ? '系统与风控消息' : '消息中心' }}</h2>
            <button type="button" class="btn btn-ghost sm" @click="markAllRead">一键已读</button>
          </div>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.messages" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('messages', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('messages', field.key)"
                    :key="`messages-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('messages')" type="button" class="btn btn-ghost sm" @click="clearListFilters('messages')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('messages').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('messages')"
                :key="`messages-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('messages', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div v-for="group in visibleMessageGroups" :key="group.type" class="msg-group">
            <h3 class="h3 row-between">
              {{ group.title }}
              <span v-if="group.unread" class="badge badge-warn">{{ group.unread }} 未读</span>
            </h3>
            <ul class="msg-list">
              <li
                v-for="m in filteredMessageItems(group)"
                :key="m.id"
                :class="{ unread: m.unread, clickable: true }"
                @click="openMessage(group, m)"
              >
                <p class="m-title">{{ m.title }}</p>
                <p class="m-body">{{ m.body }}</p>
                <time class="m-time">{{ m.time }}</time>
              </li>
            </ul>
          </div>
        </section>

        <section v-else-if="section === 'reserve' && canResourceAdmin" class="panel">
          <h2 class="h2">预约审核</h2>
          <p class="lead">审核用户提交的预约申请，支持通过与驳回。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">
            接口：`GET /orders`、`PATCH /orders/{orderId}/audit`、`GET /orders/{orderId}`
          </p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.adminReviewOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('adminReviewOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('adminReviewOrders', field.key)"
                    :key="`adminReviewOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button
                v-if="hasActiveFilters('adminReviewOrders')"
                type="button"
                class="btn btn-ghost sm"
                @click="clearListFilters('adminReviewOrders')"
              >
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('adminReviewOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('adminReviewOrders')"
                :key="`adminReviewOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('adminReviewOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>申请人</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredAdminReviewOrders" :key="o.id">
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td>{{ o.userId || '-' }}</td>
                  <td><span class="tag muted">{{ o.status }}</span></td>
                  <td>
                    <div class="op-buttons">
                      <button type="button" class="linkish" @click="auditReservation(o, 'APPROVE')">
                        通过
                      </button>
                      <button type="button" class="linkish" @click="auditReservation(o, 'REJECT')">
                        驳回
                      </button>
                      <button type="button" class="linkish" @click="viewOrderDetail(o)">详情</button>
                    </div>
                  </td>
                </tr>
                <tr v-if="!filteredAdminReviewOrders.length">
                  <td colspan="5" class="kpi-lbl">当前无待审核预约。</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'settings' && canSystemAdmin" class="panel">
          <h2 class="h2">用户管理</h2>
          <p class="lead">查看用户账号状态并执行启用/禁用。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /admin/users`、`PATCH /admin/users/{userId}/status`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.adminUsers" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('adminUsers', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('adminUsers', field.key)"
                    :key="`adminUsers-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('adminUsers')" type="button" class="btn btn-ghost sm" @click="clearListFilters('adminUsers')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('adminUsers').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('adminUsers')"
                :key="`adminUsers-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('adminUsers', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>用户ID</th>
                  <th>用户名</th>
                  <th>角色</th>
                  <th>状态</th>
                  <th>信用分</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="u in filteredAdminUsers" :key="u.id">
                  <td class="mono">{{ u.id }}</td>
                  <td>{{ u.username }}</td>
                  <td>{{ u.role }}</td>
                  <td>{{ u.status }}</td>
                  <td>{{ u.creditScore ?? '-' }}</td>
                  <td>
                    <button type="button" class="linkish" @click="toggleUserStatus(u)">
                      {{ u.status === 'DISABLED' ? '启用' : '禁用' }}
                    </button>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'history' && canSystemAdmin" class="panel">
          <h2 class="h2">审计日志</h2>
          <p class="lead">用于追踪管理动作与关键变更。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /admin/audit-logs`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.adminLogs" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('adminLogs', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('adminLogs', field.key)"
                    :key="`adminLogs-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('adminLogs')" type="button" class="btn btn-ghost sm" @click="clearListFilters('adminLogs')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('adminLogs').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('adminLogs')"
                :key="`adminLogs-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('adminLogs', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>日志ID</th>
                  <th>操作人</th>
                  <th>动作</th>
                  <th>目标</th>
                  <th>时间</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="l in filteredAdminLogs" :key="l.id">
                  <td class="mono">{{ l.id }}</td>
                  <td>{{ l.operatorId }}</td>
                  <td>{{ l.action }}</td>
                  <td>{{ l.target }}</td>
                  <td>{{ l.createdAt }}</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'borrow' && canViewAdminOrders" class="panel">
          <h2 class="h2">在借监管</h2>
          <p class="lead">
            {{ canResourceAdmin ? '办理借出与归还，跟踪在借状态变化。' : '查看在借与逾期风险（只读）。' }}
          </p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">
            接口：`GET /orders`、`PATCH /orders/{orderId}/borrow`、`PATCH /orders/{orderId}/return`、`GET /orders/{orderId}`
          </p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.adminBorrowOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('adminBorrowOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('adminBorrowOrders', field.key)"
                    :key="`adminBorrowOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button
                v-if="hasActiveFilters('adminBorrowOrders')"
                type="button"
                class="btn btn-ghost sm"
                @click="clearListFilters('adminBorrowOrders')"
              >
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('adminBorrowOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('adminBorrowOrders')"
                :key="`adminBorrowOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('adminBorrowOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>申请人</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredAdminBorrowManageOrders" :key="o.id">
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td>{{ o.userId || '-' }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                  <td>
                    <div class="op-buttons">
                      <button
                        v-if="canResourceAdmin && o.rawStatus === 'APPROVED'"
                        type="button"
                        class="linkish"
                        @click="processBorrowing(o, 'borrow')"
                      >
                        办理借出
                      </button>
                      <button
                        v-else-if="canResourceAdmin && (o.rawStatus === 'BORROWED' || o.rawStatus === 'OVERDUE')"
                        type="button"
                        class="linkish"
                        @click="processBorrowing(o, 'return')"
                      >
                        办理归还
                      </button>
                      <span v-else-if="!canResourceAdmin" class="kpi-lbl">只读</span>
                      <button type="button" class="linkish" @click="viewOrderDetail(o)">详情</button>
                    </div>
                  </td>
                </tr>
                <tr v-if="!filteredAdminBorrowManageOrders.length">
                  <td colspan="5" class="kpi-lbl">当前无可监管订单。</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'orders' && canViewAdminOrders" class="panel">
          <h2 class="h2">订单列表</h2>
          <p class="lead">展示当前系统订单，便于定位 `orderId` 进行后续管理操作。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /orders`、`GET /orders/{orderId}`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.adminAllOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('adminAllOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('adminAllOrders', field.key)"
                    :key="`adminAllOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('adminAllOrders')" type="button" class="btn btn-ghost sm" @click="clearListFilters('adminAllOrders')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('adminAllOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('adminAllOrders')"
                :key="`adminAllOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('adminAllOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>订单ID</th>
                  <th>单号</th>
                  <th>物品</th>
                  <th>用户ID</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredAdminAllOrders" :key="o.id">
                  <td class="mono">{{ o.orderId || '-' }}</td>
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td>{{ o.userId || '-' }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                  <td><button type="button" class="linkish" @click="viewOrderDetail(o)">详情</button></td>
                </tr>
                <tr v-if="!filteredAdminAllOrders.length">
                  <td colspan="6" class="kpi-lbl">暂无订单</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'reserve' && !isAdminSession" class="panel">
          <h2 class="h2">我的预约</h2>
          <p class="lead">查看待审核/已通过/已驳回的预约订单，并执行下一步操作。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /orders/my`、`PATCH /orders/{orderId}/cancel`、`GET /orders/{orderId}`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.userReserveOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('userReserveOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('userReserveOrders', field.key)"
                    :key="`userReserveOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button
                v-if="hasActiveFilters('userReserveOrders')"
                type="button"
                class="btn btn-ghost sm"
                @click="clearListFilters('userReserveOrders')"
              >
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('userReserveOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('userReserveOrders')"
                :key="`userReserveOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('userReserveOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredUserReserveOrders" :key="o.id">
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                  <td>
                    <div class="op-buttons">
                      <button type="button" class="linkish" @click="viewOrderDetail(o)">查看详情</button>
                      <button v-if="o.rawStatus === 'CREATED'" type="button" class="linkish" @click="cancelUserOrder(o)">
                        取消预约
                      </button>
                      <button v-else-if="o.rawStatus === 'REJECTED'" type="button" class="linkish" @click="rebookUserOrder">
                        重新预约
                      </button>
                    </div>
                  </td>
                </tr>
                <tr v-if="!filteredUserReserveOrders.length">
                  <td colspan="4" class="kpi-lbl">暂无预约订单</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'borrow' && !isAdminSession" class="panel">
          <h2 class="h2">借用中</h2>
          <p class="lead">查看进行中的借用单与逾期单，支持快速查看详情。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /orders/my`、`GET /orders/{orderId}`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.userBorrowOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('userBorrowOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('userBorrowOrders', field.key)"
                    :key="`userBorrowOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('userBorrowOrders')" type="button" class="btn btn-ghost sm" @click="clearListFilters('userBorrowOrders')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('userBorrowOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('userBorrowOrders')"
                :key="`userBorrowOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('userBorrowOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredUserBorrowOrders" :key="o.id">
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                  <td>
                    <button type="button" class="linkish" @click="viewOrderDetail(o)">查看详情</button>
                  </td>
                </tr>
                <tr v-if="!filteredUserBorrowOrders.length">
                  <td colspan="4" class="kpi-lbl">暂无借用中订单</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'history' && !isAdminSession" class="panel">
          <h2 class="h2">借用记录</h2>
          <p class="lead">查看已归还与已取消订单的借用记录。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`GET /orders/my`、`GET /orders/{orderId}`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.userHistoryOrders" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('userHistoryOrders', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('userHistoryOrders', field.key)"
                    :key="`userHistoryOrders-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button
                v-if="hasActiveFilters('userHistoryOrders')"
                type="button"
                class="btn btn-ghost sm"
                @click="clearListFilters('userHistoryOrders')"
              >
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('userHistoryOrders').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('userHistoryOrders')"
                :key="`userHistoryOrders-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('userHistoryOrders', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>单号</th>
                  <th>物品</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="o in filteredUserHistoryOrders" :key="o.id">
                  <td class="mono">{{ o.id }}</td>
                  <td>{{ o.item }}</td>
                  <td><span class="tag" :class="o.tone">{{ o.status }}</span></td>
                  <td>
                    <button type="button" class="linkish" @click="viewOrderDetail(o)">查看详情</button>
                  </td>
                </tr>
                <tr v-if="!filteredUserHistoryOrders.length">
                  <td colspan="4" class="kpi-lbl">暂无借用记录</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'uploads' && !isAdminSession" class="panel">
          <h2 class="h2">我的上传</h2>
          <p class="lead">查看你提交的物品上传申请及审核状态。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`POST /items/submissions`</p>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.userUploads" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('userUploads', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('userUploads', field.key)"
                    :key="`userUploads-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('userUploads')" type="button" class="btn btn-ghost sm" @click="clearListFilters('userUploads')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('userUploads').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('userUploads')"
                :key="`userUploads-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('userUploads', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>申请号</th>
                  <th>物品</th>
                  <th>分类</th>
                  <th>数量</th>
                  <th>押金</th>
                  <th>状态</th>
                  <th>提交时间</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="u in filteredUserUploadHistory" :key="u.id">
                  <td class="mono">{{ u.id || '-' }}</td>
                  <td>{{ u.name }}</td>
                  <td>{{ u.category }}</td>
                  <td>{{ u.totalCount }}</td>
                  <td>{{ u.deposit }}</td>
                  <td><span class="tag" :class="u.tone">{{ u.statusText }}</span></td>
                  <td>{{ u.createdAt || '-' }}</td>
                </tr>
                <tr v-if="!filteredUserUploadHistory.length">
                  <td colspan="7" class="kpi-lbl">暂无物品上传记录</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'settings' && !isAdminSession" class="panel">
          <h2 class="h2">账号设置</h2>
          <p class="lead">维护个人资料与密码安全。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">接口：`PUT /users/me`、`PUT /users/me/password`</p>

          <div class="cards">
            <article class="action-card">
              <h3>个人资料</h3>
              <div class="form-grid">
                <label>
                  <span class="kpi-lbl">手机号</span>
                  <input v-model="profileForm.phone" class="input" type="text" />
                </label>
                <label>
                  <span class="kpi-lbl">邮箱</span>
                  <input v-model="profileForm.email" class="input" type="email" />
                </label>
              </div>
              <button type="button" class="btn btn-primary sm" :disabled="savingProfile" @click="saveProfile">
                {{ savingProfile ? '保存中…' : '保存资料' }}
              </button>
            </article>

            <article class="action-card">
              <h3>修改密码</h3>
              <div class="form-grid">
                <label>
                  <span class="kpi-lbl">旧密码</span>
                  <input v-model="passwordForm.oldPassword" class="input" type="password" />
                </label>
                <label>
                  <span class="kpi-lbl">新密码</span>
                  <input v-model="passwordForm.newPassword" class="input" type="password" />
                </label>
              </div>
              <button type="button" class="btn btn-primary sm" :disabled="savingPassword" @click="savePassword">
                {{ savingPassword ? '提交中…' : '修改密码' }}
              </button>
            </article>
          </div>
          <p v-if="profileMsg" class="lead" style="margin-top: 1rem">{{ profileMsg }}</p>
        </section>

        <section v-else-if="section === 'claims' && canResourceAdmin" class="panel">
          <h2 class="h2">赔偿裁定</h2>
          <p class="lead">创建赔偿记录并更新赔偿处理状态。</p>
          <p v-if="SHOW_API_HINT" class="lead api-hint">
            接口：`GET /compensations`、`POST /compensations`、`PATCH /compensations/{id}/status`
          </p>
          <div class="cards">
            <article class="action-card">
              <h3>新增赔偿记录</h3>
              <div class="form-grid">
                <label>
                  <span class="kpi-lbl">订单ID</span>
                  <input
                    v-model="compCreateOrderKeyword"
                    class="input"
                    type="text"
                    list="comp-order-options"
                    placeholder="输入订单ID，也可下拉选择"
                    @input="onCompCreateOrderKeywordInput"
                  />
                  <datalist id="comp-order-options">
                    <option v-for="o in selectableOrderOptions" :key="`comp-order-id-${o.orderId}`" :value="String(o.orderId)" :label="o.label" />
                  </datalist>
                </label>
                <label>
                  <span class="kpi-lbl">关联用户</span>
                  <input
                    class="input"
                    type="text"
                    :value="selectedCompCreateOrder ? userDisplayLabel(selectedCompCreateOrder.userId) : '请先选择订单'"
                    readonly
                  />
                </label>
                <label>
                  <span class="kpi-lbl">金额</span>
                  <input v-model.number="compCreateForm.amount" class="input" type="number" min="0" step="0.01" />
                </label>
              </div>
              <label class="form-grid">
                <span class="kpi-lbl">原因</span>
                <input v-model="compCreateForm.reason" class="input" type="text" />
              </label>
              <button type="button" class="btn btn-primary sm" :disabled="savingCompCreate" @click="createCompensation">
                {{ savingCompCreate ? '提交中…' : '新增赔偿记录' }}
              </button>
            </article>

            <article class="action-card">
              <h3>更新赔偿状态</h3>
              <div class="form-grid">
                <label>
                  <span class="kpi-lbl">赔偿记录ID</span>
                  <input v-model.number="compStatusForm.id" class="input" type="number" min="1" />
                </label>
                <label>
                  <span class="kpi-lbl">状态</span>
                  <select v-model="compStatusForm.status" class="select">
                    <option value="PENDING">待处理</option>
                    <option value="PAID">已支付</option>
                    <option value="WAIVED">已减免</option>
                  </select>
                </label>
              </div>
              <button type="button" class="btn btn-primary sm" :disabled="savingCompStatus" @click="updateCompStatus">
                {{ savingCompStatus ? '提交中…' : '更新状态' }}
              </button>
            </article>
          </div>

          <h3 class="h3" style="margin-top: 1rem">赔偿记录列表</h3>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.compensations" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('compensations', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('compensations', field.key)"
                    :key="`compensations-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button v-if="hasActiveFilters('compensations')" type="button" class="btn btn-ghost sm" @click="clearListFilters('compensations')">
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('compensations').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('compensations')"
                :key="`compensations-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('compensations', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>ID</th>
                  <th>订单ID</th>
                  <th>用户ID</th>
                  <th>金额</th>
                  <th>状态</th>
                  <th>原因</th>
                  <th>创建时间</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="c in filteredCompensations" :key="c.id">
                  <td class="mono">{{ c.id }}</td>
                  <td>{{ c.orderId }}</td>
                  <td>{{ c.userId }}</td>
                  <td>{{ c.amount }}</td>
                  <td>{{ compensationStatusLabel(c.status) }}</td>
                  <td>{{ c.reason || '—' }}</td>
                  <td>{{ c.createdAt || '—' }}</td>
                </tr>
                <tr v-if="!filteredCompensations.length">
                  <td colspan="7" style="color: var(--text-muted)">暂无赔偿记录</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else-if="section === 'claims' && !isAdminSession" class="panel">
          <h2 class="h2">赔偿记录</h2>
          <div class="list-filter-bar">
            <div class="filter-inline">
              <label v-for="field in listFilterDefinitions.userCompensations" :key="field.key" class="filter-field">
                <span class="kpi-lbl">{{ field.label }}</span>
                <select class="select filter-add-select" @change="onFilterSelectChange('userCompensations', field.key, $event)">
                  <option value="">添加{{ field.label }}</option>
                  <option
                    v-for="opt in getAvailableFilterOptions('userCompensations', field.key)"
                    :key="`userCompensations-${field.key}-${opt}`"
                    :value="opt"
                  >
                    {{ opt }}
                  </option>
                </select>
              </label>
              <button
                v-if="hasActiveFilters('userCompensations')"
                type="button"
                class="btn btn-ghost sm"
                @click="clearListFilters('userCompensations')"
              >
                清空筛选
              </button>
            </div>
            <div v-if="selectedFilterEntries('userCompensations').length" class="filter-chip-row">
              <button
                v-for="entry in selectedFilterEntries('userCompensations')"
                :key="`userCompensations-chip-${entry.fieldKey}-${entry.value}`"
                type="button"
                class="filter-chip"
                @click="removeFilterValue('userCompensations', entry.fieldKey, entry.value)"
              >
                <span class="chip-field">{{ entry.fieldLabel }}</span>
                <span class="chip-value">{{ entry.value }}</span>
                <span class="chip-close">×</span>
              </button>
            </div>
          </div>
          <div class="table-wrap">
            <table class="ledger">
              <thead>
                <tr>
                  <th>记录号</th>
                  <th>订单号</th>
                  <th>物品</th>
                  <th>金额</th>
                  <th>状态</th>
                  <th>截止时间</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="r in filteredUserCompensations" :key="r.id">
                  <td class="mono">{{ r.id }}</td>
                  <td class="mono">{{ r.orderNo }}</td>
                  <td>{{ r.itemName }}</td>
                  <td>¥{{ r.amount }}</td>
                  <td><span class="tag" :class="r.tone">{{ r.statusText }}</span></td>
                  <td>{{ r.deadline }}</td>
                  <td>
                    <button type="button" class="linkish" @click="viewCompensationOrder(r)">查看关联订单</button>
                  </td>
                </tr>
                <tr v-if="!filteredUserCompensations.length">
                  <td colspan="7" class="kpi-lbl">暂无赔偿记录</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>

        <section v-else class="panel placeholder">
          <h2 class="h2">{{ currentTab?.label }}</h2>
          <p class="lead">
            {{ DEMO_MODE ? '此分区为原型占位，可对接 SRS 中的详细用例与字段。' : '此分区功能开发中。' }}
          </p>
          <ul class="bullets">
            <li>我的预约、借用中、历史记录：与订单中心同一数据源。</li>
            <li>赔偿记录：查看待支付、已支付与减免状态。</li>
            <li>设置：通知偏好、绑定手机、安全设备管理。</li>
          </ul>
        </section>

      </main>
    </div>

    <transition name="fade">
      <div v-if="orderDetailModalOpen && orderDetailUi" class="detail-modal-backdrop" @click.self="closeOrderDetailModal">
        <section class="detail-modal" role="dialog" aria-modal="true" aria-labelledby="order-detail-title">
          <header class="detail-modal-head">
            <div>
              <h2 id="order-detail-title" class="h2">订单详情</h2>
              <p class="lead">当前展示最近一次“查看详情”的订单信息。</p>
            </div>
            <button type="button" class="icon-btn" aria-label="关闭" @click="closeOrderDetailModal">×</button>
          </header>

          <div class="detail-head">
            <div>
              <p class="detail-title">{{ orderDetailUi.orderNo }}</p>
              <p class="detail-sub">{{ orderDetailUi.itemName }}</p>
            </div>
            <span class="tag" :class="orderDetailUi.statusTone">{{ orderDetailUi.statusLabel }}</span>
          </div>

          <div class="detail-grid">
            <article class="detail-card">
              <h3>基础信息</h3>
              <div class="detail-kv">
                <p><span>订单ID</span><strong>{{ orderDetailUi.orderId }}</strong></p>
                <p><span>用户ID</span><strong>{{ orderDetailUi.userId }}</strong></p>
                <p><span>物品ID</span><strong>{{ orderDetailUi.itemId }}</strong></p>
                <p><span>数量</span><strong>{{ orderDetailUi.quantity }}</strong></p>
              </div>
            </article>

            <article class="detail-card">
              <h3>借用信息</h3>
              <div class="detail-kv">
                <p><span>备注</span><strong>{{ orderDetailUi.remark }}</strong></p>
                <p><span>赔偿标记</span><strong>{{ orderDetailUi.needCompensation }}</strong></p>
                <p><span>来源</span><strong>{{ orderDetailUi.source }}</strong></p>
              </div>
            </article>
          </div>

          <article class="detail-card">
            <h3>时间节点</h3>
            <div class="detail-kv time">
              <p><span>预约开始</span><strong>{{ orderDetailUi.reserveStart }}</strong></p>
              <p><span>预约结束</span><strong>{{ orderDetailUi.reserveEnd }}</strong></p>
              <p><span>取货时间</span><strong>{{ orderDetailUi.pickupTime }}</strong></p>
              <p><span>归还时间</span><strong>{{ orderDetailUi.returnTime }}</strong></p>
              <p><span>创建时间</span><strong>{{ orderDetailUi.createdAt }}</strong></p>
              <p><span>更新时间</span><strong>{{ orderDetailUi.updatedAt }}</strong></p>
            </div>
          </article>

          <div v-if="SHOW_API_HINT" class="lead api-hint" style="margin-top: 0.85rem">
            接口：`GET /orders/{orderId}`
          </div>
        </section>
      </div>
    </transition>

  </div>
</template>

<script setup>
import { computed, onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { logout, session } from '../session'
import { userApi, orderApi, creditApi, compensationApi, statsApi, adminApi, ApiError } from '../api'
import { DEMO_MODE, SHOW_API_HINT } from '../config'

const router = useRouter()
const route = useRoute()
const UPLOAD_HISTORY_STORAGE_KEY = 'borrow-planet-item-upload-history'
const USER_MESSAGE_STATE_KEY = 'borrow-planet-user-message-state'
const ITEM_STATUS_NOTICE_KEY = 'borrow-planet-item-status-notices'
const USER_DASHBOARD_POLL_MS = 1000
const ADMIN_DASHBOARD_POLL_MS = 5000
let userPollTimer = null
let pollingInFlight = false
let authExpiredHandled = false

function isAuthExpiredError(err) {
  if (!(err instanceof ApiError)) return false
  if (err.httpStatus === 401) return true
  return /token expired|token invalid|未登录|登录过期/i.test(String(err.message || ''))
}

function handleAuthExpired() {
  if (authExpiredHandled) return
  authExpiredHandled = true
  stopUserPolling()
  dashboardError.value = '登录状态已过期，请重新登录。'
  logout()
  router.replace('/?reason=expired')
}

function onDashboardVisibleRefresh() {
  if (typeof document !== 'undefined' && document.visibilityState !== 'visible') return
  if (pollingInFlight) return
  pollingInFlight = true
  loadDashboard({ silent: true })
    .catch(() => {})
    .finally(() => {
      pollingInFlight = false
    })
}

function onDashboardFocusRefresh() {
  onDashboardVisibleRefresh()
}

const creditScore = ref(typeof session.creditScore === 'number' ? session.creditScore : null)
const creditScoreDisplay = computed(() => (typeof creditScore.value === 'number' ? creditScore.value : '--'))
const creditPct = computed(() =>
  typeof creditScore.value === 'number' ? Math.min(100, Math.max(0, creditScore.value)) : 0,
)
const offline = ref(false)
const dashboardError = ref('')
const dashboardInfo = ref('')
const adminUsers = ref([])
const adminLogs = ref([])
const savingProfile = ref(false)
const savingPassword = ref(false)
const savingCreditAdjust = ref(false)
const savingCompCreate = ref(false)
const savingCompStatus = ref(false)
const profileMsg = ref('')
const orderDetail = ref(null)
const orderDetailModalOpen = ref(false)
const compensations = ref([])
const userUploadHistory = ref([])
const profileForm = reactive({
  phone: '',
  email: '',
})
const passwordForm = reactive({
  oldPassword: '',
  newPassword: '',
})
const creditAdjustForm = reactive({
  userId: 0,
  changeValue: 0,
  reason: '',
})
const compCreateForm = reactive({
  orderId: 0,
  userId: 0,
  amount: 0,
  reason: '',
})
const compCreateOrderKeyword = ref('')
const compStatusForm = reactive({
  id: 0,
  status: 'PENDING',
})

const section = ref('home')
const isAdminSession = computed(() => session.role === 'admin')
const canSystemAdmin = computed(
  () => isAdminSession.value && (session.adminType === 'system' || session.adminType === 'legacy'),
)
const canResourceAdmin = computed(
  () => isAdminSession.value && (session.adminType === 'resource' || session.adminType === 'legacy'),
)
const canViewAdminOrders = computed(() => isAdminSession.value && (canResourceAdmin.value || canSystemAdmin.value))
const roleBadgeLabel = computed(() => {
  if (!isAdminSession.value) return '师生用户'
  if (session.adminType === 'system') return '系统管理员'
  if (session.adminType === 'resource') return '资源管理员'
  return '管理员'
})

const userNavTabs = [
  { id: 'home', label: '总览', short: '总览', badge: null },
  { id: 'reserve', label: '我的预约', short: '预约', badge: null },
  { id: 'borrow', label: '借用中', short: '借用', badge: null },
  { id: 'history', label: '借用记录', short: '记录', badge: null },
  { id: 'uploads', label: '我的上传', short: '上传', badge: null },
  { id: 'credit', label: '信用与积分', short: '信用', badge: null },
  { id: 'claims', label: '赔偿记录', short: '赔偿', badge: null },
  { id: 'messages', label: '消息中心', short: '消息', badge: null },
  { id: 'settings', label: '设置', short: '设置', badge: null },
]

const fullAdminNavTabs = [
  { id: 'home', label: '总览', short: '总览', badge: null },
  { id: 'reserve', label: '预约审核', short: '审核', badge: null },
  { id: 'borrow', label: '在借监管', short: '在借', badge: null },
  { id: 'orders', label: '订单列表', short: '订单', badge: null },
  { id: 'history', label: '操作日志', short: '日志', badge: null },
  { id: 'credit', label: '信用仲裁', short: '仲裁', badge: null },
  { id: 'claims', label: '赔偿裁定', short: '裁定', badge: null },
  { id: 'messages', label: '系统与风控', short: '消息', badge: null },
  { id: 'settings', label: '策略与设置', short: '设置', badge: null },
]

const systemAdminNavTabs = [
  { id: 'home', label: '总览', short: '总览', badge: null },
  { id: 'borrow', label: '监管订单', short: '监管', badge: null },
  { id: 'orders', label: '订单列表', short: '订单', badge: null },
  { id: 'history', label: '操作日志', short: '日志', badge: null },
  { id: 'credit', label: '信用仲裁', short: '仲裁', badge: null },
  { id: 'messages', label: '系统与风控', short: '消息', badge: null },
  { id: 'settings', label: '策略与设置', short: '设置', badge: null },
]

const resourceAdminNavTabs = [
  { id: 'home', label: '总览', short: '总览', badge: null },
  { id: 'reserve', label: '预约审核', short: '审核', badge: null },
  { id: 'borrow', label: '在借监管', short: '在借', badge: null },
  { id: 'orders', label: '订单列表', short: '订单', badge: null },
  { id: 'claims', label: '赔偿裁定', short: '裁定', badge: null },
  { id: 'messages', label: '系统与风控', short: '消息', badge: null },
]

const navTabs = computed(() => {
  if (!isAdminSession.value) return userNavTabs
  if (canSystemAdmin.value && canResourceAdmin.value) return fullAdminNavTabs
  if (canSystemAdmin.value) return systemAdminNavTabs
  if (canResourceAdmin.value) return resourceAdminNavTabs
  return [{ id: 'home', label: '总览', short: '总览', badge: null }]
})

const currentTab = computed(() => navTabs.value.find((t) => t.id === section.value))
const messagesUnreadCount = computed(() =>
  messageGroups.value.reduce((sum, g) => sum + Number(g.unread || 0), 0),
)
const pendingCompensationCount = computed(() =>
  compensations.value.filter((c) => String(c.status || '').toUpperCase() === 'PENDING').length,
)
const pendingUploadCount = computed(() =>
  userUploadHistory.value.filter((u) => String(u.statusText || '').includes('待审核')).length,
)
const adminSuperviseOrderCount = computed(() =>
  orders.value.filter((o) => ['APPROVED', 'BORROWED', 'OVERDUE'].includes(o.rawStatus)).length,
)
const adminUserNameMap = computed(() => {
  const map = new Map()
  for (const u of adminUsers.value) {
    const userId = Number(u?.id || 0)
    if (!Number.isFinite(userId) || userId <= 0) continue
    const username = String(u?.username || '').trim()
    if (username) map.set(userId, username)
  }
  return map
})

function userDisplayLabel(userId) {
  const id = Number(userId || 0)
  if (!Number.isFinite(id) || id <= 0) return ''
  const username = adminUserNameMap.value.get(id)
  return username ? `${username}（${id}）` : `用户${id}`
}

const selectableOrderOptions = computed(() => {
  const map = new Map()
  for (const o of orders.value) {
    const orderId = Number(o?.orderId || 0)
    if (!Number.isFinite(orderId) || orderId <= 0) continue
    if (map.has(orderId)) continue
    const displayNo = String(o?.id || `ORD-${orderId}`)
    const item = String(o?.item || '未知物品')
    const userId = Number(o?.userId || 0)
    map.set(orderId, {
      orderId,
      orderNo: displayNo,
      userId,
      label: `${displayNo} · ${item}${userId > 0 ? ` · ${userDisplayLabel(userId)}` : ''}`,
    })
  }
  return Array.from(map.values())
})
const selectedCompCreateOrder = computed(
  () => selectableOrderOptions.value.find((o) => o.orderId === Number(compCreateForm.orderId)) || null,
)
const selectableUserOptions = computed(() => {
  const map = new Map()
  for (const u of adminUsers.value) {
    const userId = Number(u?.id || 0)
    if (!Number.isFinite(userId) || userId <= 0) continue
    const name = String(u?.username || `用户${userId}`)
    map.set(userId, { id: userId, label: `${name}（${userId}）` })
  }
  for (const o of orders.value) {
    const userId = Number(o?.userId || 0)
    if (!Number.isFinite(userId) || userId <= 0 || map.has(userId)) continue
    map.set(userId, { id: userId, label: userDisplayLabel(userId) })
  }
  for (const c of compensations.value) {
    const userId = Number(c?.userId || 0)
    if (!Number.isFinite(userId) || userId <= 0 || map.has(userId)) continue
    map.set(userId, { id: userId, label: userDisplayLabel(userId) })
  }
  return Array.from(map.values())
})

function getLiveBadgeCount(tabId) {
  if (tabId === 'messages') return messagesUnreadCount.value

  if (isAdminSession.value) {
    if (tabId === 'home') return 0
    if (tabId === 'reserve' && canResourceAdmin.value) return adminReviewOrders.value.length
    if (tabId === 'borrow' && canViewAdminOrders.value) return adminStats.value.overdue || adminSuperviseOrderCount.value
    if (tabId === 'claims' && canResourceAdmin.value) return pendingCompensationCount.value
    return 0
  }

  if (tabId === 'reserve') return userReserveOrders.value.filter((o) => o.rawStatus === 'CREATED').length
  if (tabId === 'borrow') return userBorrowOrders.value.length
  if (tabId === 'uploads') return 0
  if (tabId === 'claims') return pendingCompCount.value
  return 0
}

const navBadgeMap = computed(() => {
  const map = {}
  for (const tab of navTabs.value) {
    const count = getLiveBadgeCount(tab.id)
    map[tab.id] = count > 0 ? String(count) : ''
  }
  return map
})

function onTabClick(tabId) {
  if (!navTabs.value.some((tab) => tab.id === tabId)) {
    section.value = navTabs.value[0]?.id || 'home'
    const fallbackHash = section.value === 'home' ? '' : `#${section.value}`
    if (route.hash !== fallbackHash) {
      router.replace({ path: route.path, query: route.query, hash: fallbackHash })
    }
    return
  }
  section.value = tabId
  const nextHash = tabId === 'home' ? '' : `#${tabId}`
  if (route.hash !== nextHash) {
    router.replace({ path: route.path, query: route.query, hash: nextHash })
  }
}

function syncCompCreateOrderById() {
  const selected = selectableOrderOptions.value.find((o) => o.orderId === Number(compCreateForm.orderId))
  if (selected?.userId > 0) {
    compCreateForm.userId = selected.userId
    compCreateOrderKeyword.value = String(selected.orderId)
  } else {
    compCreateForm.userId = 0
  }
}

function onCompCreateOrderKeywordInput() {
  const keyword = String(compCreateOrderKeyword.value || '').trim()
  if (!keyword) {
    compCreateForm.orderId = 0
    compCreateForm.userId = 0
    return
  }
  const selected = selectableOrderOptions.value.find((o) => String(o.orderId) === keyword)
  if (!selected) {
    compCreateForm.orderId = 0
    compCreateForm.userId = 0
    return
  }
  compCreateForm.orderId = selected.orderId
  compCreateForm.userId = selected.userId > 0 ? selected.userId : 0
  compCreateOrderKeyword.value = String(selected.orderId)
}

watch(
  [navTabs, () => route.hash],
  ([tabs, hash]) => {
    const hashSection = String(hash || '')
      .replace(/^#/, '')
      .trim()

    if (hashSection && tabs.some((tab) => tab.id === hashSection)) {
      if (section.value !== hashSection) section.value = hashSection
      return
    }

    if (!tabs.some((tab) => tab.id === section.value)) {
      section.value = tabs[0]?.id || 'home'
    }

    const expectedHash = section.value === 'home' ? '' : `#${section.value}`
    if (route.hash !== expectedHash) {
      router.replace({ path: route.path, query: route.query, hash: expectedHash })
    }
  },
  { immediate: true },
)

const avatarChar = computed(() => {
  const n = (session.displayName || '用').trim()
  return n.slice(0, 1)
})

const subtitle = computed(() => {
  if (!isAdminSession.value) return '校园借用用户'
  if (session.adminType === 'system') return '系统治理与风控中心'
  if (session.adminType === 'resource') return '资源管理与库存运营组'
  return '紫金港 · 图书馆库房管理组'
})
const userAccountAlert = computed(() => {
  if (isAdminSession.value) return ''
  const status = String(session.accountStatus || '').trim().toLowerCase()
  if (!status || status === 'active') return ''
  if (status === 'frozen') return '账号已冻结：当前账号处于冻结状态，部分操作将受限，请联系管理员处理。'
  if (status === 'banned' || status === 'disabled')
    return '账号已禁用：当前账号不可正常使用，请联系管理员恢复账号状态。'
  return `账号状态异常（${session.accountStatus}）：请联系管理员确认。`
})

function doLogout() {
  logout()
  router.push('/')
}

function jumpToSection(id) {
  onTabClick(id)
}

const orders = ref(
  DEMO_MODE
    ? [
  {
    orderId: 1042,
    rawStatus: 'BORROWED',
    id: 'ORD-1042',
    item: '三脚架',
    status: '借用中',
    tone: 'ok',
    action: session.role === 'admin' ? '查看详情' : '归还指引',
  },
  {
    orderId: 1041,
    rawStatus: 'APPROVED',
    id: 'ORD-1041',
    item: '录音笔',
    status: '已通过',
    tone: 'ok',
    action: session.role === 'admin' ? '查看详情' : '去取货',
  },
  {
    orderId: 1038,
    rawStatus: 'CREATED',
    id: 'ORD-1038',
    item: '羽毛球拍套装',
    status: '待审核',
    tone: 'muted',
    action: session.role === 'admin' ? '查看详情' : '取消预约',
  },
  {
    orderId: 9,
    rawStatus: 'OVERDUE',
    id: 'ORD-009',
    item: '示波器',
    status: '已逾期',
    tone: 'bad',
    action: '查看详情',
  },
  {
    orderId: 991,
    rawStatus: 'RETURNED',
    id: 'ORD-0991',
    item: '激光测距仪',
    status: '已归还',
    tone: 'done',
    action: session.role === 'admin' ? '查看详情' : '评价',
  },
]
    : [],
)

const ledger = ref(
  DEMO_MODE
    ? [
  { id: 1, userId: 10001, t: '2026-04-17 21:06', reason: '按时归还', delta: 4, ref: 'ORD-0991' },
  { id: 2, userId: 10001, t: '2026-04-12 09:40', reason: '预约取消（提前 24h）', delta: 1, ref: 'BP-0312' },
  { id: 3, userId: 10001, t: '2026-04-02 18:22', reason: '逾期未还（1 天）', delta: -8, ref: 'ORD-009' },
]
    : [],
)

// 订单状态 → 展示文案/色调/主操作
const ORDER_STATUS_MAP = {
  CREATED: { status: '待审核', tone: 'muted', action: '取消预约' },
  APPROVED: { status: '已通过', tone: 'ok', action: '去取货' },
  REJECTED: { status: '已驳回', tone: 'bad', action: '重新预约' },
  BORROWED: { status: '借用中', tone: 'ok', action: '归还指引' },
  RETURNED: { status: '已归还', tone: 'done', action: '评价' },
  CANCELLED: { status: '已取消', tone: 'muted', action: '查看详情' },
  OVERDUE: { status: '已逾期', tone: 'bad', action: '查看详情' },
}

const COMPENSATION_STATUS_MAP = {
  PENDING: '待处理',
  PAID: '已支付',
  WAIVED: '已减免',
}
const UPLOAD_STATUS_TONE_MAP = {
  PENDING: 'muted',
  APPROVED: 'ok',
  REJECTED: 'bad',
}

function compensationStatusLabel(status) {
  const key = String(status || '').toUpperCase()
  return COMPENSATION_STATUS_MAP[key] || (status ? String(status) : '—')
}

function loadUploadHistoryFromStorage() {
  if (typeof localStorage === 'undefined') return []
  try {
    const raw = localStorage.getItem(UPLOAD_HISTORY_STORAGE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    const list = Array.isArray(arr) ? arr : []
    const currentUserId = Number(session.userId || 0)
    return list
      .filter((r) => {
        const submitterId = Number(r?.submitterId || 0)
        if (!currentUserId) return true
        return submitterId > 0 && submitterId === currentUserId
      })
      .map((r) => {
      const status = String(r?.status || 'PENDING').toUpperCase()
      return {
        id: r?.id ?? '',
        name: r?.name || '—',
        category: r?.category || '未分类',
        totalCount: Number(r?.totalCount || 0),
        deposit: `¥${Number(r?.deposit || 0).toFixed(2)}`,
        statusText: r?.statusText || (status === 'APPROVED' ? '已通过' : status === 'REJECTED' ? '已驳回' : '待审核'),
        tone: UPLOAD_STATUS_TONE_MAP[status] || 'muted',
        createdAt: r?.createdAt ? String(r.createdAt).replace('T', ' ').slice(0, 19) : '—',
        submitterId: r?.submitterId ?? null,
      }
      })
  } catch {
    return []
  }
}

function loadItemStatusNotices() {
  if (typeof localStorage === 'undefined') return []
  try {
    const raw = localStorage.getItem(ITEM_STATUS_NOTICE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    return Array.isArray(arr) ? arr : []
  } catch {
    return []
  }
}

function getUserMessageStateStorageKey() {
  const uid = Number(session.userId || 0)
  if (Number.isFinite(uid) && uid > 0) return `${USER_MESSAGE_STATE_KEY}:${uid}`
  const name = String(session.displayName || '').trim() || 'guest'
  return `${USER_MESSAGE_STATE_KEY}:name:${name}`
}

function loadUserMessageState() {
  if (typeof localStorage === 'undefined') return { orderStatusMap: {}, uploadStatusMap: {}, accountStatus: '' }
  try {
    const raw = localStorage.getItem(getUserMessageStateStorageKey())
    const parsed = raw ? JSON.parse(raw) : null
    return {
      orderStatusMap: parsed?.orderStatusMap && typeof parsed.orderStatusMap === 'object' ? parsed.orderStatusMap : {},
      uploadStatusMap: parsed?.uploadStatusMap && typeof parsed.uploadStatusMap === 'object' ? parsed.uploadStatusMap : {},
      accountStatus: typeof parsed?.accountStatus === 'string' ? parsed.accountStatus : '',
      offShelfNoticeMap:
        parsed?.offShelfNoticeMap && typeof parsed.offShelfNoticeMap === 'object' ? parsed.offShelfNoticeMap : {},
    }
  } catch {
    return { orderStatusMap: {}, uploadStatusMap: {}, accountStatus: '', offShelfNoticeMap: {} }
  }
}

function saveUserMessageState(state) {
  if (typeof localStorage === 'undefined') return
  localStorage.setItem(getUserMessageStateStorageKey(), JSON.stringify(state))
}

function pushUserOrderStatusMessage(orderNo, itemName, status) {
  const s = String(status || '').toUpperCase()
  if (s === 'BORROWED') {
    pushMessage('overdue', '物品已借出', `订单 ${orderNo}（${itemName}）已借出，请按约定时间归还。`)
  } else if (s === 'APPROVED') {
    pushMessage('review', '预约审核通过', `订单 ${orderNo}（${itemName}）已通过，请尽快取货。`)
  } else if (s === 'REJECTED') {
    pushMessage('review', '预约审核驳回', `订单 ${orderNo}（${itemName}）被驳回，请调整时段后重试。`)
  } else if (s === 'OVERDUE') {
    pushMessage('overdue', '订单已逾期', `订单 ${orderNo}（${itemName}）已逾期，请尽快归还以减少扣分。`)
  } else if (s === 'RETURNED') {
    pushMessage('overdue', '归还已确认', `订单 ${orderNo}（${itemName}）已完成归还。`)
  } else if (s === 'CANCELLED') {
    pushMessage('review', '预约已取消', `订单 ${orderNo}（${itemName}）已取消。`)
  }
}

function pushUserUploadStatusMessage(uploadId, name, statusText) {
  const s = String(statusText || '')
  if (s.includes('已通过')) {
    pushMessage('review', '物品上传审核通过', `上传申请 ${uploadId}（${name}）已通过，物品已上架。`)
  } else if (s.includes('已驳回')) {
    pushMessage('review', '物品上传被驳回', `上传申请 ${uploadId}（${name}）被驳回，请补充信息后重提。`)
  }
}

function syncUserLifecycleMessages() {
  if (isAdminSession.value) return
  const prev = loadUserMessageState()
  const nextOrderStatusMap = {}
  const nextUploadStatusMap = {}
  const nextOffShelfNoticeMap = { ...(prev.offShelfNoticeMap || {}) }

  for (const o of orders.value) {
    const key = String(o?.id || o?.orderId || '').trim()
    if (!key) continue
    const status = String(o?.rawStatus || '').toUpperCase()
    nextOrderStatusMap[key] = status
    const oldStatus = String(prev.orderStatusMap[key] || '').toUpperCase()
    if (status && status !== oldStatus) {
      pushUserOrderStatusMessage(key, String(o?.item || '物品'), status)
    }
  }

  for (const u of userUploadHistory.value) {
    const key = String(u?.id || '').trim()
    if (!key) continue
    const statusText = String(u?.statusText || '')
    nextUploadStatusMap[key] = statusText
    const oldStatus = String(prev.uploadStatusMap[key] || '')
    if (statusText && statusText !== oldStatus) {
      pushUserUploadStatusMessage(key, String(u?.name || '上传物品'), statusText)
    }
  }

  const nextAccountStatus = String(session.accountStatus || '').trim().toLowerCase()
  const oldAccountStatus = String(prev.accountStatus || '').trim().toLowerCase()
  if (nextAccountStatus && nextAccountStatus !== oldAccountStatus) {
    if (nextAccountStatus === 'frozen') {
      pushMessage('sys', '账号状态提醒', '你的账号已冻结，部分功能将受限，请联系管理员。')
    } else if (nextAccountStatus === 'banned' || nextAccountStatus === 'disabled') {
      pushMessage('sys', '账号状态提醒', '你的账号已被禁用，请联系管理员处理。')
    } else if (oldAccountStatus && oldAccountStatus !== 'active' && nextAccountStatus === 'active') {
      pushMessage('sys', '账号状态恢复', '你的账号状态已恢复为可用。')
    }
  }

  const currentUserId = Number(session.userId || 0)
  for (const notice of loadItemStatusNotices()) {
    const noticeId = String(notice?.id || '').trim()
    if (!noticeId) continue
    const status = String(notice?.status || '').toUpperCase()
    if (status !== 'OFF_SHELF') continue
    if (nextOffShelfNoticeMap[noticeId]) continue
    const noticeSubmitterId = Number(notice?.submitterId || 0)
    if (!currentUserId || noticeSubmitterId <= 0 || noticeSubmitterId !== currentUserId) continue
    const itemName = String(notice?.itemName || '').trim()
    if (!itemName) continue
    pushMessage(
      'sys',
      '物品下架通知',
      `你上传的「${itemName}」已被管理员下架，请在“我的上传”查看状态并按需调整后重新提交。`,
    )
    nextOffShelfNoticeMap[noticeId] = true
  }

  saveUserMessageState({
    orderStatusMap: nextOrderStatusMap,
    uploadStatusMap: nextUploadStatusMap,
    accountStatus: nextAccountStatus,
    offShelfNoticeMap: nextOffShelfNoticeMap,
  })
}

function mapOrder(o) {
  const m = ORDER_STATUS_MAP[o.status] || { status: o.status, tone: 'muted', action: '查看详情' }
  return {
    orderId: o.id ?? null,
    userId: o.userId ?? null,
    rawStatus: o.status || '',
    id: o.orderNo || `ORD-${o.id}`,
    item: o.itemName || `物品 #${o.itemId}`,
    status: m.status,
    tone: m.tone,
    action: session.role === 'admin' ? '查看详情' : m.action,
  }
}

function fmtDetailValue(v) {
  if (v === null || v === undefined || v === '') return '—'
  return String(v)
}

const orderDetailUi = computed(() => {
  const d = orderDetail.value
  if (!d) return null

  const statusKey = String(d.status || d.rawStatus || '').toUpperCase()
  const statusMeta = ORDER_STATUS_MAP[statusKey]

  return {
    orderNo: fmtDetailValue(d.orderNo || (d.id ? `ORD-${d.id}` : '')),
    orderId: fmtDetailValue(d.id),
    userId: fmtDetailValue(d.userId),
    itemId: fmtDetailValue(d.itemId),
    itemName: fmtDetailValue(d.itemName),
    quantity: fmtDetailValue(d.quantity),
    remark: fmtDetailValue(d.remark || d.returnRemark),
    needCompensation: d.needCompensation === true ? '是' : d.needCompensation === false ? '否' : '—',
    source: fmtDetailValue(d.source || 'backend'),
    reserveStart: fmtDetailValue(d.reserveStartTime),
    reserveEnd: fmtDetailValue(d.reserveEndTime),
    pickupTime: fmtDetailValue(d.pickupTime),
    returnTime: fmtDetailValue(d.returnTime),
    createdAt: fmtDetailValue(d.createdAt),
    updatedAt: fmtDetailValue(d.updatedAt),
    statusLabel: statusMeta?.status || fmtDetailValue(d.status),
    statusTone: statusMeta?.tone || 'muted',
  }
})

const userReserveOrders = computed(() =>
  orders.value.filter((o) => ['CREATED', 'APPROVED', 'REJECTED'].includes(o.rawStatus)),
)
const userBorrowOrders = computed(() => orders.value.filter((o) => ['BORROWED', 'OVERDUE'].includes(o.rawStatus)))
const userHistoryOrders = computed(() =>
  orders.value.filter((o) => ['RETURNED', 'CANCELLED'].includes(o.rawStatus)),
)
const adminAllOrders = computed(() => orders.value)
const adminReviewOrders = computed(() => orders.value.filter((o) => o.rawStatus === 'CREATED'))
const adminBorrowManageOrders = computed(() =>
  orders.value.filter((o) => ['APPROVED', 'BORROWED', 'OVERDUE'].includes(o.rawStatus)),
)
const userCompensations = ref(
  DEMO_MODE
    ? [
  {
    id: 'COMP-240521-01',
    orderNo: 'ORD-1042',
    itemName: '三脚架',
    amount: '120.00',
    statusText: '待支付',
    tone: 'warn',
    deadline: '2026-06-12 18:00',
  },
  {
    id: 'COMP-240518-03',
    orderNo: 'ORD-0991',
    itemName: '激光测距仪',
    amount: '350.00',
    statusText: '已减免',
    tone: 'ok',
    deadline: '—',
  },
  {
    id: 'COMP-240510-07',
    orderNo: 'ORD-0973',
    itemName: '录音笔',
    amount: '80.00',
    statusText: '已支付',
    tone: 'done',
    deadline: '2026-05-15 18:00',
  },
]
    : [],
)
const pendingCompCount = computed(() =>
  userCompensations.value.filter((r) => r.statusText === '待支付').length,
)

const listFilterDefinitions = {
  homeOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  creditLedger: [
    { key: 'userId', label: '用户ID', getValue: (r) => r.userId },
    { key: 'reason', label: '原因', getValue: (r) => r.reason },
    { key: 'ref', label: '关联单号', getValue: (r) => r.ref },
  ],
  messages: [
    { key: 'group', label: '分组', getValue: (r) => r.groupTitle },
    { key: 'title', label: '标题', getValue: (r) => r.message.title },
    { key: 'time', label: '时间', getValue: (r) => r.message.time },
  ],
  adminReviewOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  adminUsers: [
    { key: 'id', label: '用户ID', getValue: (u) => u.id },
    { key: 'username', label: '用户名', getValue: (u) => u.username },
    { key: 'role', label: '角色', getValue: (u) => u.role },
    { key: 'status', label: '状态', getValue: (u) => u.status },
  ],
  adminLogs: [
    { key: 'operatorId', label: '操作人', getValue: (l) => l.operatorId },
    { key: 'action', label: '动作', getValue: (l) => l.action },
    { key: 'target', label: '目标', getValue: (l) => l.target },
  ],
  adminBorrowOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  adminAllOrders: [
    { key: 'orderId', label: '订单ID', getValue: (o) => o.orderId },
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  userReserveOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  userBorrowOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  userHistoryOrders: [
    { key: 'id', label: '单号', getValue: (o) => o.id },
    { key: 'item', label: '物品', getValue: (o) => o.item },
    { key: 'status', label: '状态', getValue: (o) => o.status },
  ],
  userUploads: [
    { key: 'id', label: '申请号', getValue: (u) => u.id },
    { key: 'name', label: '物品', getValue: (u) => u.name },
    { key: 'category', label: '分类', getValue: (u) => u.category },
    { key: 'statusText', label: '状态', getValue: (u) => u.statusText },
  ],
  compensations: [
    { key: 'id', label: '记录ID', getValue: (c) => c.id },
    { key: 'orderId', label: '订单ID', getValue: (c) => c.orderId },
    { key: 'userId', label: '用户ID', getValue: (c) => c.userId },
    { key: 'status', label: '状态', getValue: (c) => compensationStatusLabel(c.status) },
  ],
  userCompensations: [
    { key: 'id', label: '记录号', getValue: (r) => r.id },
    { key: 'orderNo', label: '订单号', getValue: (r) => r.orderNo },
    { key: 'itemName', label: '物品', getValue: (r) => r.itemName },
    { key: 'statusText', label: '状态', getValue: (r) => r.statusText },
  ],
}

function createDefaultListFilters() {
  const filters = {}
  Object.entries(listFilterDefinitions).forEach(([listKey, defs]) => {
    filters[listKey] = {}
    defs.forEach((f) => {
      filters[listKey][f.key] = []
    })
  })
  return filters
}

const listFilters = reactive(createDefaultListFilters())

const messageFilterRows = computed(() =>
  messageGroups.value.flatMap((g) =>
    (g.items || []).map((m) => ({
      groupType: g.type,
      groupTitle: g.title,
      message: m,
    })),
  ),
)

function normalizeFilterValue(v) {
  const text = String(v ?? '').trim()
  return text || null
}

function getFilterRows(listKey) {
  if (listKey === 'homeOrders') return orders.value
  if (listKey === 'creditLedger') return ledger.value
  if (listKey === 'messages') return messageFilterRows.value
  if (listKey === 'adminReviewOrders') return adminReviewOrders.value
  if (listKey === 'adminUsers') return adminUsers.value
  if (listKey === 'adminLogs') return adminLogs.value
  if (listKey === 'adminBorrowOrders') return adminBorrowManageOrders.value
  if (listKey === 'adminAllOrders') return adminAllOrders.value
  if (listKey === 'userReserveOrders') return userReserveOrders.value
  if (listKey === 'userBorrowOrders') return userBorrowOrders.value
  if (listKey === 'userHistoryOrders') return userHistoryOrders.value
  if (listKey === 'userUploads') return userUploadHistory.value
  if (listKey === 'compensations') return compensations.value
  if (listKey === 'userCompensations') return userCompensations.value
  return []
}

function getFilterOptions(listKey, fieldKey) {
  const defs = listFilterDefinitions[listKey] || []
  const fieldDef = defs.find((f) => f.key === fieldKey)
  if (!fieldDef) return []
  const set = new Set()
  for (const row of getFilterRows(listKey)) {
    const value = normalizeFilterValue(fieldDef.getValue(row))
    if (value) set.add(value)
  }
  return Array.from(set).sort((a, b) => a.localeCompare(b, 'zh-Hans-CN'))
}

function getAvailableFilterOptions(listKey, fieldKey) {
  const selected = listFilters[listKey]?.[fieldKey] || []
  return getFilterOptions(listKey, fieldKey).filter((opt) => !selected.includes(opt))
}

function onFilterSelectChange(listKey, fieldKey, event) {
  const value = normalizeFilterValue(event?.target?.value)
  if (!value) return
  const bucket = listFilters[listKey]?.[fieldKey]
  if (Array.isArray(bucket) && !bucket.includes(value)) bucket.push(value)
  if (event?.target) event.target.value = ''
}

function removeFilterValue(listKey, fieldKey, value) {
  const bucket = listFilters[listKey]?.[fieldKey]
  if (!Array.isArray(bucket)) return
  listFilters[listKey][fieldKey] = bucket.filter((v) => v !== value)
}

function clearListFilters(listKey) {
  const bucket = listFilters[listKey]
  if (!bucket || typeof bucket !== 'object') return
  Object.keys(bucket).forEach((fieldKey) => {
    bucket[fieldKey] = []
  })
}

function selectedFilterEntries(listKey) {
  const defs = listFilterDefinitions[listKey] || []
  const bucket = listFilters[listKey] || {}
  const entries = []
  defs.forEach((f) => {
    const selected = Array.isArray(bucket[f.key]) ? bucket[f.key] : []
    selected.forEach((value) => {
      entries.push({
        fieldKey: f.key,
        fieldLabel: f.label,
        value,
      })
    })
  })
  return entries
}

function hasActiveFilters(listKey) {
  return selectedFilterEntries(listKey).length > 0
}

function rowMatchesFilters(row, listKey) {
  const defs = listFilterDefinitions[listKey] || []
  return defs.every((f) => {
    const selected = listFilters[listKey]?.[f.key] || []
    if (!selected.length) return true
    const value = normalizeFilterValue(f.getValue(row))
    return value ? selected.includes(value) : false
  })
}

function applyListFilters(rows, listKey) {
  return (rows || []).filter((row) => rowMatchesFilters(row, listKey))
}

const filteredHomeOrders = computed(() => applyListFilters(orders.value, 'homeOrders'))
const filteredLedger = computed(() => applyListFilters(ledger.value, 'creditLedger'))
const filteredAdminReviewOrders = computed(() => applyListFilters(adminReviewOrders.value, 'adminReviewOrders'))
const filteredAdminUsers = computed(() => applyListFilters(adminUsers.value, 'adminUsers'))
const filteredAdminLogs = computed(() => applyListFilters(adminLogs.value, 'adminLogs'))
const filteredAdminBorrowManageOrders = computed(() => applyListFilters(adminBorrowManageOrders.value, 'adminBorrowOrders'))
const filteredAdminAllOrders = computed(() => applyListFilters(adminAllOrders.value, 'adminAllOrders'))
const filteredUserReserveOrders = computed(() => applyListFilters(userReserveOrders.value, 'userReserveOrders'))
const filteredUserBorrowOrders = computed(() => applyListFilters(userBorrowOrders.value, 'userBorrowOrders'))
const filteredUserHistoryOrders = computed(() => applyListFilters(userHistoryOrders.value, 'userHistoryOrders'))
const filteredUserUploadHistory = computed(() => applyListFilters(userUploadHistory.value, 'userUploads'))
const filteredCompensations = computed(() => applyListFilters(compensations.value, 'compensations'))
const filteredUserCompensations = computed(() => applyListFilters(userCompensations.value, 'userCompensations'))

function filteredMessageItems(group) {
  return (group?.items || []).filter((m) =>
    rowMatchesFilters({ groupType: group.type, groupTitle: group.title, message: m }, 'messages'),
  )
}

const visibleMessageGroups = computed(() =>
  messageGroups.value
    .map((g) => {
      const items = filteredMessageItems(g)
      return {
        ...g,
        items,
        unread: items.filter((m) => m.unread).length,
      }
    })
    .filter((g) => g.items.length > 0),
)

async function loadDashboard({ silent = false } = {}) {
  if (!silent) {
    dashboardError.value = ''
    dashboardInfo.value = ''
    orderDetail.value = null
    orderDetailModalOpen.value = false
  }
  // 离线原型会话：保留示例数据
  if (session.mock && DEMO_MODE) {
    if (!isAdminSession.value) {
      userUploadHistory.value = loadUploadHistoryFromStorage()
      syncUserLifecycleMessages()
    }
    offline.value = true
    return
  }
  try {
    if (isAdminSession.value) {
      const warnings = []
      const tryAdminLoad = async (label, fn, fallback) => {
        try {
          return await fn()
        } catch (err) {
          if (err instanceof ApiError && (err.httpStatus === 400 || err.httpStatus === 403)) {
            warnings.push(`${label}接口暂不可用（${err.httpStatus}）`)
            return fallback
          }
          throw err
        }
      }

      if (canViewAdminOrders.value) {
        const stats = await tryAdminLoad('资源统计', () => statsApi.dashboard(), null)
        if (stats) {
          adminStats.value = {
            pending: stats?.todayReservationCount ?? adminStats.value.pending,
            overdue: stats?.overdueCount ?? adminStats.value.overdue,
            borrowing: stats?.borrowingCount ?? adminStats.value.borrowing,
          }
        }

        const adminOrderData = await tryAdminLoad('订单监管', () => orderApi.list({ page: 1, size: 200 }), [])
        const adminOrders = Array.isArray(adminOrderData) ? adminOrderData : adminOrderData?.list || []
        orders.value = adminOrders.map(mapOrder)
        if (!stats) {
          adminStats.value = {
            pending: orders.value.filter((o) => o.rawStatus === 'CREATED').length,
            overdue: adminSuperviseOrderCount.value,
            borrowing: orders.value.filter((o) => o.rawStatus === 'BORROWED').length,
          }
        }
      } else {
        orders.value = []
      }

      // 根据管理员类型加载对应数据，避免无权限接口导致页面整体失败。
      if (canSystemAdmin.value) {
        const [usersData, logsData, creditData] = await Promise.all([
          tryAdminLoad('用户管理', () => adminApi.users({ page: 1, size: 10 }), []),
          tryAdminLoad('审计日志', () => adminApi.auditLogs({ page: 1, size: 10 }), []),
          tryAdminLoad('信用流水', () => creditApi.records({ page: 1, size: 20 }), []),
        ])
        adminUsers.value = (Array.isArray(usersData) ? usersData : usersData?.list || []).slice(0, 10)
        adminLogs.value = (Array.isArray(logsData) ? logsData : logsData?.list || []).slice(0, 10)
        const records = Array.isArray(creditData) ? creditData : creditData?.list || []
        ledger.value = records.map((r) => ({
          id: r.id,
          userId: r.userId ?? '—',
          t: r.createdAt,
          reason: r.reason,
          delta: r.changeValue,
          ref: r.orderId ? `ORD-${r.orderId}` : '—',
        }))
      } else {
        adminUsers.value = []
        adminLogs.value = []
        ledger.value = []
      }

      if (canResourceAdmin.value) {
        const compensationData = await tryAdminLoad('赔偿记录', () => compensationApi.list({ page: 1, size: 20 }), [])
        const compList = Array.isArray(compensationData) ? compensationData : compensationData?.list || []
        compensations.value = compList.map((c) => ({
          id: c.id,
          orderId: c.orderId,
          userId: c.userId,
          amount: c.amount,
          status: c.status || 'PENDING',
          reason: c.reason,
          createdAt: c.createdAt,
        }))
      } else {
        compensations.value = []
      }

      if (warnings.length) {
        dashboardInfo.value = `部分数据未加载：${warnings.join('；')}`
      }
    } else {
      const me = await userApi.me()
      if (me && typeof me.creditScore === 'number') {
        creditScore.value = me.creditScore
        session.creditScore = me.creditScore
      } else {
        creditScore.value = null
        session.creditScore = null
      }
      session.accountStatus = String(me?.status || '')
      profileForm.phone = me?.phone || ''
      profileForm.email = me?.email || ''

      const orderData = await orderApi.my({ page: 1, size: 20 })
      const list = Array.isArray(orderData) ? orderData : orderData?.list || []
      orders.value = list.map(mapOrder)

      const creditData = await creditApi.records({ page: 1, size: 20 })
      const records = Array.isArray(creditData) ? creditData : creditData?.list || []
      ledger.value = records.map((r) => ({
        id: r.id,
        userId: r.userId ?? session.userId ?? '—',
        t: r.createdAt,
        reason: r.reason,
        delta: r.changeValue,
        ref: r.orderId ? `ORD-${r.orderId}` : '—',
      }))
      userUploadHistory.value = loadUploadHistoryFromStorage()
      syncUserLifecycleMessages()
    }
    offline.value = false
  } catch (err) {
    if (err instanceof ApiError && DEMO_MODE) {
      offline.value = true
    } else if (isAuthExpiredError(err)) {
      offline.value = false
      handleAuthExpired()
    } else if (err instanceof ApiError) {
      offline.value = false
      if (!silent) dashboardError.value = err.message
    }
  }
}

const adminStats = ref({ pending: DEMO_MODE ? 12 : 0, overdue: DEMO_MODE ? 3 : 0, borrowing: DEMO_MODE ? 48 : 0 })

onMounted(loadDashboard)

function stopUserPolling() {
  if (userPollTimer) {
    clearInterval(userPollTimer)
    userPollTimer = null
  }
  if (typeof document !== 'undefined') {
    document.removeEventListener('visibilitychange', onDashboardVisibleRefresh)
  }
  if (typeof window !== 'undefined') {
    window.removeEventListener('focus', onDashboardFocusRefresh)
  }
}

function startUserPolling() {
  stopUserPolling()
  if (session.role === 'guest') return
  const pollMs = isAdminSession.value ? ADMIN_DASHBOARD_POLL_MS : USER_DASHBOARD_POLL_MS
  userPollTimer = setInterval(async () => {
    if (pollingInFlight) return
    if (typeof document !== 'undefined' && document.visibilityState === 'hidden') return
    pollingInFlight = true
    try {
      await loadDashboard({ silent: true })
    } finally {
      pollingInFlight = false
    }
  }, pollMs)
  if (typeof document !== 'undefined') {
    document.addEventListener('visibilitychange', onDashboardVisibleRefresh)
  }
  if (typeof window !== 'undefined') {
    window.addEventListener('focus', onDashboardFocusRefresh)
  }
}

onMounted(startUserPolling)
onUnmounted(stopUserPolling)

watch(
  () => [session.role, isAdminSession.value],
  () => {
    startUserPolling()
  },
)

async function saveProfile() {
  profileMsg.value = ''
  savingProfile.value = true
  try {
    await userApi.updateProfile({
      phone: profileForm.phone || undefined,
      email: profileForm.email || undefined,
    })
    profileMsg.value = '资料已保存。'
  } catch (err) {
    if (err instanceof ApiError) profileMsg.value = `保存失败：${err.message}`
  } finally {
    savingProfile.value = false
  }
}

async function savePassword() {
  profileMsg.value = ''
  if (!passwordForm.oldPassword || !passwordForm.newPassword) {
    profileMsg.value = '请填写旧密码和新密码。'
    return
  }
  savingPassword.value = true
  try {
    await userApi.changePassword({
      oldPassword: passwordForm.oldPassword,
      newPassword: passwordForm.newPassword,
    })
    profileMsg.value = '密码修改成功。'
    passwordForm.oldPassword = ''
    passwordForm.newPassword = ''
  } catch (err) {
    if (err instanceof ApiError) profileMsg.value = `修改失败：${err.message}`
  } finally {
    savingPassword.value = false
  }
}

async function submitCreditAdjust() {
  dashboardError.value = ''
  dashboardInfo.value = ''
  if (!canSystemAdmin.value) {
    dashboardError.value = '当前账号无系统治理权限，不能调整信用分。'
    return
  }
  if (!creditAdjustForm.userId || !creditAdjustForm.reason) {
    dashboardError.value = '请填写用户ID和调整原因。'
    return
  }
  savingCreditAdjust.value = true
  try {
    await creditApi.adjust({
      userId: Number(creditAdjustForm.userId),
      changeValue: Number(creditAdjustForm.changeValue || 0),
      reason: creditAdjustForm.reason,
    })
    dashboardInfo.value = '信用分调整已提交。'
    const delta = Number(creditAdjustForm.changeValue || 0)
    pushMessage(
      'sys',
      '信用分调整已生效',
      `用户 ${creditAdjustForm.userId} 信用分变更 ${delta >= 0 ? '+' : ''}${delta}，原因：${creditAdjustForm.reason}。`,
    )
    await loadDashboard()
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  } finally {
    savingCreditAdjust.value = false
  }
}

async function createCompensation() {
  dashboardError.value = ''
  dashboardInfo.value = ''
  if (!canResourceAdmin.value) {
    dashboardError.value = '当前账号无资源管理权限，不能创建赔偿记录。'
    return
  }
  if (compCreateForm.orderId && !compCreateForm.userId) {
    syncCompCreateOrderById()
  }
  if (!compCreateForm.orderId || !compCreateForm.userId || !compCreateForm.reason) {
    dashboardError.value = '请先选择订单并填写赔偿原因。'
    return
  }
  savingCompCreate.value = true
  try {
    const data = await compensationApi.create({
      orderId: Number(compCreateForm.orderId),
      userId: Number(compCreateForm.userId),
      amount: Number(compCreateForm.amount || 0),
      reason: compCreateForm.reason,
    })
    if (data && data.id) {
      compensations.value.unshift({
        id: data.id,
        orderId: data.orderId,
        userId: data.userId,
        amount: data.amount,
        status: data.status || 'PENDING',
        reason: data.reason || compCreateForm.reason,
        createdAt: data.createdAt || new Date().toISOString().slice(0, 19).replace('T', ' '),
      })
    }
    dashboardInfo.value = '赔偿记录已创建。'
    pushMessage(
      'overdue',
      '新增赔偿记录',
      `订单 ${compCreateForm.orderId} 已创建赔偿记录（用户 ${compCreateForm.userId}）。`,
    )
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  } finally {
    savingCompCreate.value = false
  }
}

async function updateCompStatus() {
  dashboardError.value = ''
  dashboardInfo.value = ''
  if (!canResourceAdmin.value) {
    dashboardError.value = '当前账号无资源管理权限，不能更新赔偿状态。'
    return
  }
  if (!compStatusForm.id) {
    dashboardError.value = '请填写赔偿记录ID。'
    return
  }
  savingCompStatus.value = true
  try {
    await compensationApi.setStatus(Number(compStatusForm.id), compStatusForm.status)
    compensations.value = compensations.value.map((c) =>
      Number(c.id) === Number(compStatusForm.id) ? { ...c, status: compStatusForm.status } : c,
    )
    dashboardInfo.value = '赔偿状态已更新。'
    pushMessage('sys', '赔偿状态更新', `赔偿记录 ${compStatusForm.id} 状态已更新为 ${compStatusForm.status}。`)
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  } finally {
    savingCompStatus.value = false
  }
}

async function toggleUserStatus(u) {
  if (!canSystemAdmin.value) {
    dashboardError.value = '当前账号无系统治理权限，不能修改用户状态。'
    return
  }
  if (!u?.id) return
  try {
    const nextStatus = u.status === 'DISABLED' ? 'ACTIVE' : 'DISABLED'
    await adminApi.setUserStatus(u.id, nextStatus)
    u.status = nextStatus
    dashboardInfo.value = `用户 ${u.username || u.id} 状态已更新为 ${nextStatus}。`
    pushMessage('sys', '用户状态变更', `用户 ${u.username || u.id} 状态更新为 ${nextStatus}。`)
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  }
}

async function auditReservation(o, action) {
  if (!canResourceAdmin.value) {
    dashboardError.value = '当前账号无资源管理权限，不能审核预约。'
    return
  }
  if (!o?.orderId) return
  dashboardError.value = ''
  dashboardInfo.value = ''
  const actionText = action === 'APPROVE' ? '通过' : '驳回'
  if (DEMO_MODE && (session.mock || offline.value)) {
    if (action === 'APPROVE') {
      Object.assign(o, { rawStatus: 'APPROVED', status: '已通过', tone: 'ok' })
    } else {
      Object.assign(o, { rawStatus: 'REJECTED', status: '已驳回', tone: 'bad' })
    }
    dashboardInfo.value = `演示：预约 ${o.id} 已${actionText}。`
    pushMessage('review', `预约已${actionText}`, `订单 ${o.id} 已${actionText}，请关注后续借还流程。`)
    return
  }
  try {
    await orderApi.audit(o.orderId, {
      action,
      remark: action === 'APPROVE' ? '管理员审核通过' : '管理员审核驳回',
    })
    dashboardInfo.value = `预约 ${o.id} 已${actionText}。`
    pushMessage('review', `预约已${actionText}`, `订单 ${o.id} 已${actionText}，请关注后续借还流程。`)
    await loadDashboard()
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  }
}

async function processBorrowing(o, op) {
  if (!canResourceAdmin.value) {
    dashboardError.value = '当前账号无资源管理权限，不能办理借还。'
    return
  }
  if (!o?.orderId) return
  dashboardError.value = ''
  dashboardInfo.value = ''
  if (DEMO_MODE && (session.mock || offline.value)) {
    if (op === 'borrow') {
      Object.assign(o, { rawStatus: 'BORROWED', status: '借用中', tone: 'ok' })
      dashboardInfo.value = `演示：${o.id} 已办理借出。`
      pushMessage('overdue', '借出办理完成', `订单 ${o.id} 已办理借出，进入借用中。`)
    } else {
      Object.assign(o, { rawStatus: 'RETURNED', status: '已归还', tone: 'done' })
      dashboardInfo.value = `演示：${o.id} 已办理归还。`
      pushMessage('overdue', '归还办理完成', `订单 ${o.id} 已办理归还，风险状态已解除。`)
    }
    return
  }
  try {
    if (op === 'borrow') {
      await orderApi.borrow(o.orderId)
      dashboardInfo.value = `${o.id} 已办理借出。`
      pushMessage('overdue', '借出办理完成', `订单 ${o.id} 已办理借出，进入借用中。`)
    } else {
      await orderApi.return(o.orderId, { returnRemark: '管理员确认归还', needCompensation: false })
      dashboardInfo.value = `${o.id} 已办理归还。`
      pushMessage('overdue', '归还办理完成', `订单 ${o.id} 已办理归还，风险状态已解除。`)
    }
    await loadDashboard()
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  }
}

async function handleOrderAction(o) {
  if (!o?.orderId && !o?.id) return
  dashboardError.value = ''

  // 管理员监管视角默认只读：主操作统一查看详情
  if (isAdminSession.value) {
    await viewOrderDetail(o)
    return
  }

  if (o.rawStatus === 'CREATED') {
    await cancelUserOrder(o)
    return
  }
  if (o.rawStatus === 'REJECTED') {
    rebookUserOrder()
    return
  }
  await viewOrderDetail(o)
}

async function viewOrderDetail(o) {
  if (!o?.orderId && !o?.id) return
  dashboardError.value = ''
  if (DEMO_MODE && (session.mock || offline.value)) {
    orderDetail.value = {
      id: o.orderId ?? o.id,
      orderNo: o.id,
      status: o.rawStatus,
      itemName: o.item,
      source: 'mock-demo',
    }
    orderDetailModalOpen.value = true
    dashboardInfo.value = `演示：已加载订单详情 ${o.id}。`
    return
  }
  if (!o.orderId) return
  try {
    orderDetail.value = await orderApi.detail(o.orderId)
    orderDetailModalOpen.value = true
    dashboardInfo.value = `已加载订单详情：${o.id}`
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  }
}

async function viewCompensationOrder(record) {
  dashboardError.value = ''
  dashboardInfo.value = ''
  const orderNo = String(record?.orderNo || '').trim()
  if (!orderNo) {
    dashboardError.value = '该赔偿记录缺少关联订单号。'
    return
  }
  const targetOrder = orders.value.find((o) => String(o.id || '').trim() === orderNo)
  if (!targetOrder) {
    section.value = 'history'
    dashboardInfo.value = `未在当前列表找到 ${orderNo}，已跳转到借用记录。`
    return
  }
  await viewOrderDetail(targetOrder)
}

function closeOrderDetailModal() {
  orderDetailModalOpen.value = false
}

async function cancelUserOrder(o) {
  if (!o?.orderId && !o?.id) return
  dashboardError.value = ''
  if (DEMO_MODE && (session.mock || offline.value)) {
    Object.assign(o, { rawStatus: 'CANCELLED', status: '已取消', tone: 'muted', action: '查看详情' })
    dashboardInfo.value = `演示：${o.id} 已取消预约。`
    pushMessage('review', '预约已取消', `订单 ${o.id} 已取消，可重新选择时段发起预约。`)
    return
  }
  if (!o.orderId) return
  try {
    await orderApi.cancel(o.orderId)
    dashboardInfo.value = `${o.id} 已取消预约。`
    pushMessage('review', '预约已取消', `订单 ${o.id} 已取消，可重新选择时段发起预约。`)
    await loadDashboard()
  } catch (err) {
    if (err instanceof ApiError) dashboardError.value = err.message
  }
}

function rebookUserOrder() {
  router.push('/items')
  dashboardInfo.value = '已跳转到物品页，请重新发起预约。'
}

const messageTitleMap = {
  review: '审核',
  overdue: '逾期 / 风险',
  sys: '系统',
}

function formatMessageTime(date = new Date()) {
  const hh = String(date.getHours()).padStart(2, '0')
  const mm = String(date.getMinutes()).padStart(2, '0')
  return `今天 ${hh}:${mm}`
}

function pushMessage(type, title, body, { unread = true } = {}) {
  const groupType = messageTitleMap[type] ? type : 'sys'
  let group = messageGroups.value.find((g) => g.type === groupType)
  if (!group) {
    group = { type: groupType, title: messageTitleMap[groupType], unread: 0, items: [] }
    messageGroups.value.unshift(group)
  }
  group.items.unshift({
    id: `m-${Date.now()}-${Math.random().toString(36).slice(2, 8)}`,
    title,
    body,
    time: formatMessageTime(),
    unread,
  })
  if (unread) group.unread = Number(group.unread || 0) + 1
  if (group.items.length > 50) group.items = group.items.slice(0, 50)
}

const messageGroups = ref(
  DEMO_MODE
    ? [
  {
    type: 'review',
    title: '审核',
    unread: 1,
    items: [
      {
        id: 'm1',
        title: '预约审核通过',
        body: '「三脚架」预约已通过，请在 48h 内取货。',
        time: '今天 09:12',
        unread: true,
      },
      {
        id: 'm2',
        title: '预约被驳回',
        body: '「示波器」因时段冲突被驳回，请改选日期。',
        time: '昨天 16:40',
        unread: false,
      },
    ],
  },
  {
    type: 'overdue',
    title: '逾期 / 风险',
    unread: 1,
    items: [
      {
        id: 'm3',
        title: '逾期提醒',
        body: '订单 ORD-009 已逾期，请尽快归还以避免进一步扣分。',
        time: '昨天 08:00',
        unread: true,
      },
    ],
  },
  {
    type: 'sys',
    title: '系统',
    unread: 1,
    items: [
      {
        id: 'm4',
        title: '维护公告',
        body: '本周日 01:00–03:00 平台进行例行维护。',
        time: '2026-04-10',
        unread: true,
      },
    ],
  },
]
    : [],
)

watch(orderDetailModalOpen, (open) => {
  document.body.style.overflow = open ? 'hidden' : ''
})

const creditDeltas = computed(() => {
  const rows = Array.isArray(ledger.value) ? ledger.value : []
  return rows
    .map((r) => Number(r?.delta))
    .filter((n) => Number.isFinite(n))
    .reverse()
})

const sparkStroke = computed(() => (creditDeltas.value.length ? 'url(#spark-grad)' : '#7aa7ff'))

const sparkPoints = computed(() => {
  const width = 240
  const height = 80
  const deltas = creditDeltas.value

  // New users may have no credit records yet; render a visible 100 baseline.
  if (!deltas.length) {
    return '0,40 120,40 240,40'
  }

  // Build a score timeline from historical deltas + current score.
  let series = []
  if (typeof creditScore.value === 'number') {
    let running = creditScore.value - deltas.reduce((sum, n) => sum + n, 0)
    series.push(running)
    for (const d of deltas) {
      running += d
      series.push(running)
    }
  } else {
    let running = 100
    series.push(running)
    for (const d of deltas) {
      running += d
      series.push(running)
    }
  }

  if (series.length < 2) {
    const v = series[0] ?? 100
    series = [v, v]
  }

  const minV = Math.min(...series)
  const maxV = Math.max(...series)
  const range = Math.max(1, maxV - minV)
  const pad = range * 0.2 + 1
  const low = minV - pad
  const high = maxV + pad
  const yRange = Math.max(1, high - low)

  return series
    .map((v, i) => {
      const x = (i / (series.length - 1)) * width
      const y = height - ((v - low) / yRange) * (height - 8) - 4
      return `${x.toFixed(1)},${y.toFixed(1)}`
    })
    .join(' ')
})

function markAllRead() {
  messageGroups.value = messageGroups.value.map((g) => ({
    ...g,
    unread: 0,
    items: g.items.map((m) => ({ ...m, unread: false })),
  }))
}

function openMessage(group, message) {
  if (!message.unread) return
  message.unread = false
  const sourceGroup = messageGroups.value.find((g) => g.type === group.type)
  if (!sourceGroup) return
  sourceGroup.unread = Math.max(0, sourceGroup.items.filter((m) => m.unread).length)
}
</script>

<style scoped>
.dash {
  max-width: 1320px;
  margin: 0 auto;
  padding: 1rem clamp(0.65rem, 2vw, 1.25rem) 5.5rem;
}

.user-bar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 1rem;
  padding: 1rem 1.15rem;
  border-radius: var(--radius-lg);
  background: var(--bg-elevated);
  border: 1px solid var(--border);
  margin-bottom: 1.25rem;
}

.user-bar-tail {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 0.75rem;
  margin-left: auto;
}

.user-bar-actions {
  display: flex;
  flex-wrap: wrap;
  gap: 0.45rem;
  align-items: center;
}

.admin-kpis {
  display: flex;
  flex-wrap: wrap;
  gap: 0.75rem;
}
.kpi {
  min-width: 4.5rem;
  padding: 0.45rem 0.65rem;
  border-radius: var(--radius-md);
  border: 1px solid var(--border);
  background: rgba(0, 0, 0, 0.2);
  text-align: center;
}
.kpi-val {
  margin: 0;
  font-size: 1.25rem;
  font-weight: 800;
}
.kpi-lbl {
  margin: 0.1rem 0 0;
  font-size: 0.72rem;
  color: var(--muted);
  font-weight: 600;
}

.admin-list {
  margin: 0 0 1rem;
}

.identity {
  display: flex;
  gap: 0.85rem;
  align-items: center;
  min-width: 200px;
}

.avatar {
  width: 48px;
  height: 48px;
  border-radius: 14px;
  background: linear-gradient(145deg, #4f9bff, #2a4a86);
  display: grid;
  place-items: center;
  font-weight: 800;
}

.name-row {
  display: flex;
  align-items: center;
  gap: 0.45rem;
}
.name {
  font-weight: 700;
}
.sub {
  margin: 0.15rem 0 0;
  font-size: 0.85rem;
  color: var(--muted);
}

.credit-block {
  display: flex;
  align-items: center;
  gap: 0.65rem;
}

.ring {
  transform: rotate(-90deg);
}
.ring-bg {
  fill: none;
  stroke: rgba(255, 255, 255, 0.1);
  stroke-width: 3.2;
}
.ring-fg {
  fill: none;
  stroke-width: 3.2;
  stroke-linecap: round;
}

.credit-label {
  margin: 0;
  font-size: 0.75rem;
  color: var(--muted);
  font-weight: 600;
}
.credit-val {
  margin: 0;
  font-size: 1.35rem;
  font-weight: 800;
}
.trend {
  margin: 0.1rem 0 0;
  font-size: 0.78rem;
  font-weight: 600;
}
.trend.up {
  color: var(--success);
}

.offline-hint {
  margin: -0.5rem 0 1rem;
  padding: 0.6rem 0.85rem;
  border-radius: var(--radius-md);
  background: rgba(61, 139, 253, 0.1);
  border: 1px solid rgba(61, 139, 253, 0.35);
  color: var(--text);
  font-size: 0.85rem;
}

.account-alert {
  margin: -0.5rem 0 1rem;
  padding: 0.7rem 0.9rem;
  border-radius: var(--radius-md);
  background: rgba(240, 113, 120, 0.12);
  border: 1px solid rgba(240, 113, 120, 0.45);
  color: var(--danger);
  font-size: 0.9rem;
  font-weight: 700;
}

.layout {
  display: flex;
  flex-direction: column;
  gap: 0.85rem;
  align-items: stretch;
}

.top-nav {
  display: flex;
  flex-wrap: wrap;
  gap: 0.45rem;
  width: 100%;
  padding: 0.55rem;
  border-radius: var(--radius-lg);
  background: var(--bg-elevated);
  border: 1px solid var(--border);
}

.nav-item {
  text-align: center;
  border: none;
  background: rgba(255, 255, 255, 0.02);
  color: var(--muted);
  padding: 0.5rem 0.8rem;
  border-radius: 999px;
  font-size: 0.84rem;
  font-weight: 600;
  cursor: pointer;
  display: inline-flex;
  flex: 0 0 auto;
  align-items: center;
  gap: 0.35rem;
  border: 1px solid var(--border);
}
.nav-item:hover {
  background: rgba(255, 255, 255, 0.04);
  color: var(--text);
}
.nav-item.active {
  background: var(--accent-soft);
  color: var(--text);
}
.dot {
  min-width: 1.1rem;
  height: 1.1rem;
  border-radius: 999px;
  background: var(--danger);
  color: var(--text-on-accent);
  font-size: 0.62rem;
  display: grid;
  place-items: center;
  font-weight: 800;
}

.content {
  width: 100%;
  min-width: 0;
}

.panel {
  background: var(--bg-elevated);
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  padding: 1.25rem clamp(1rem, 2.5vw, 1.5rem);
}

.h2 {
  margin: 0 0 0.35rem;
  font-size: 1.35rem;
}
.lead {
  margin: 0 0 1.25rem;
  color: var(--muted);
  line-height: 1.55;
}
.lead.api-hint {
  font-size: 0.84rem;
  margin-top: -0.7rem;
  color: var(--accent);
}
.h3 {
  margin: 1.25rem 0 0.65rem;
  font-size: 1rem;
}
.row-between {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 0.75rem;
}

.cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 0.85rem;
}

.action-card {
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
  padding: 0.85rem 1rem;
  background: rgba(0, 0, 0, 0.2);
  display: flex;
  flex-direction: column;
}
.action-card.danger {
  border-color: rgba(240, 113, 120, 0.35);
  background: rgba(240, 113, 120, 0.06);
}
.action-card h3 {
  margin: 0.45rem 0 0.35rem;
  font-size: 1rem;
}
.action-card p {
  margin: 0 0 0.65rem;
  font-size: 0.86rem;
  color: var(--muted);
  line-height: 1.45;
  flex: 1;
}
.action-card .btn {
  margin-top: auto;
  align-self: flex-start;
}

/* 管理概览三卡片：统一替换灰色底为更柔和的蓝色系 */
.overview-cards .action-card {
  background: linear-gradient(145deg, rgba(61, 139, 253, 0.16), rgba(78, 169, 255, 0.1));
  border-color: rgba(93, 165, 255, 0.45);
}

.overview-cards .action-card:hover {
  border-color: rgba(126, 188, 255, 0.65);
}

.form-grid {
  display: grid;
  gap: 0.6rem;
  margin: 0.5rem 0 0.75rem;
}

.form-grid label {
  display: grid;
  gap: 0.35rem;
}

.alerts {
  list-style: none;
  margin: 0;
  padding: 0;
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}
.alert {
  border-radius: var(--radius-md);
  padding: 0.65rem 0.8rem;
  font-size: 0.88rem;
  line-height: 1.45;
}
.alert.danger {
  background: rgba(240, 113, 120, 0.1);
  border: 1px solid rgba(240, 113, 120, 0.35);
}
.alert.warn {
  background: rgba(245, 177, 74, 0.1);
  border: 1px solid rgba(245, 177, 74, 0.35);
}
.alert.muted {
  background: rgba(255, 255, 255, 0.04);
  border: 1px solid var(--border);
  color: var(--muted);
}

.table-wrap {
  overflow: auto;
  border-radius: var(--radius-md);
  border: 1px solid var(--border);
}

.list-filter-bar {
  margin: 0.6rem 0 0.75rem;
  display: grid;
  gap: 0.45rem;
}

.filter-inline {
  display: flex;
  align-items: flex-end;
  gap: 0.5rem;
  flex-wrap: nowrap;
  overflow-x: auto;
  padding-bottom: 0.1rem;
}

.filter-field {
  display: inline-grid;
  gap: 0.3rem;
  min-width: 150px;
  flex: 0 0 150px;
}

.filter-add-select {
  min-height: 2.25rem;
}

.filter-chip-row {
  display: flex;
  flex-wrap: wrap;
  gap: 0.35rem;
}

.filter-chip {
  border: 1px solid var(--border);
  background: var(--chip-bg);
  color: var(--text);
  border-radius: 999px;
  padding: 0.22rem 0.5rem;
  display: inline-flex;
  align-items: center;
  gap: 0.35rem;
  cursor: pointer;
  font-size: 0.78rem;
}

.filter-chip:hover {
  border-color: rgba(61, 139, 253, 0.5);
  background: var(--accent-soft);
}

.chip-field {
  color: var(--muted);
}

.chip-value {
  font-weight: 600;
}

.chip-close {
  color: var(--danger);
  font-weight: 700;
}

.orders,
.ledger {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.88rem;
}
.orders th,
.orders td,
.ledger th,
.ledger td {
  padding: 0.65rem 0.75rem;
  text-align: left;
  border-bottom: 1px solid var(--border);
}
.orders th,
.ledger th {
  background: rgba(0, 0, 0, 0.25);
  color: var(--muted);
  font-weight: 600;
  font-size: 0.78rem;
  text-transform: uppercase;
  letter-spacing: 0.04em;
}
.bar {
  display: inline-block;
  width: 4px;
  height: 1.1em;
  border-radius: 3px;
  margin-right: 0.45rem;
  vertical-align: -0.2em;
}
.bar.ok {
  background: var(--success);
}
.bar.warn {
  background: var(--warning);
}
.bar.bad {
  background: var(--danger);
}
.bar.muted {
  background: var(--muted);
}
.bar.done {
  background: #7aa7ff;
}

.tag {
  display: inline-flex;
  padding: 0.2rem 0.45rem;
  border-radius: 6px;
  font-size: 0.72rem;
  font-weight: 700;
}
.tag.ok {
  background: rgba(62, 207, 142, 0.15);
  color: var(--success);
}
.tag.warn {
  background: rgba(245, 177, 74, 0.15);
  color: var(--warning);
}
.tag.bad {
  background: rgba(240, 113, 120, 0.15);
  color: var(--danger);
}
.tag.muted {
  background: rgba(255, 255, 255, 0.06);
  color: var(--muted);
}
.tag.done {
  background: rgba(122, 167, 255, 0.15);
  color: var(--tag-done-text);
}

.linkish {
  border: none;
  background: none;
  color: var(--accent);
  font-weight: 600;
  cursor: pointer;
  padding: 0;
  font-size: inherit;
}
.linkish:hover {
  text-decoration: underline;
}
.linkish.full {
  margin-top: 0.5rem;
}
.detail-modal-backdrop {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.55);
  display: grid;
  place-items: center;
  z-index: 50;
  padding: 1rem;
}
.detail-modal {
  width: min(860px, 100%);
  max-height: min(88vh, 860px);
  overflow: auto;
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  background: var(--bg-elevated);
  box-shadow: var(--shadow);
  padding: 1rem 1rem 0.9rem;
}
.detail-modal-head {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 0.8rem;
}
.detail-modal-head .lead {
  margin-bottom: 0.85rem;
}
.detail-head {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 0.75rem;
  margin-bottom: 0.85rem;
}
.detail-title {
  margin: 0;
  font-size: 1rem;
  font-weight: 800;
}
.detail-sub {
  margin: 0.25rem 0 0;
  font-size: 0.86rem;
  color: var(--muted);
}
.detail-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
  gap: 0.75rem;
  margin-bottom: 0.75rem;
}
.detail-card {
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
  background: rgba(0, 0, 0, 0.2);
  padding: 0.8rem 0.9rem;
}
.detail-card h3 {
  margin: 0 0 0.65rem;
  font-size: 0.92rem;
}
.detail-kv {
  display: grid;
  gap: 0.5rem;
}
.detail-kv p {
  margin: 0;
  display: flex;
  justify-content: space-between;
  gap: 0.8rem;
  font-size: 0.86rem;
}
.detail-kv span {
  color: var(--muted);
}
.detail-kv strong {
  text-align: right;
  font-weight: 700;
  word-break: break-word;
}
.detail-kv.time {
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
}
.op-buttons {
  display: inline-flex;
  flex-wrap: wrap;
  gap: 0.55rem;
}

.card-list {
  display: flex;
  flex-direction: column;
  gap: 0.65rem;
}
.order-card {
  display: flex;
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
  overflow: hidden;
  background: rgba(0, 0, 0, 0.2);
}
.stripe {
  width: 5px;
}
.stripe.ok {
  background: var(--success);
}
.stripe.warn {
  background: var(--warning);
}
.stripe.bad {
  background: var(--danger);
}
.stripe.muted {
  background: var(--muted);
}
.stripe.done {
  background: #7aa7ff;
}
.oc-body {
  padding: 0.75rem 0.9rem;
  flex: 1;
}
.oc-id {
  margin: 0;
  font-size: 0.78rem;
  color: var(--muted);
}
.oc-item {
  margin: 0.2rem 0 0.45rem;
  font-weight: 700;
}

.sparkline {
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
  padding: 0.5rem 0.65rem 0.35rem;
  background: rgba(0, 0, 0, 0.2);
  margin-bottom: 0.5rem;
}
.sparkline svg {
  width: 100%;
  height: 96px;
  display: block;
}
.spark-labels {
  display: flex;
  justify-content: space-between;
  font-size: 0.72rem;
  color: var(--muted);
  padding: 0 0.15rem 0.25rem;
}

.ledger .pos {
  color: var(--success);
  font-weight: 700;
}
.ledger .neg {
  color: var(--danger);
  font-weight: 700;
}
.mono {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
  font-size: 0.82rem;
}

.msg-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 0.75rem;
  margin-bottom: 0.5rem;
}
.msg-group + .msg-group {
  margin-top: 1rem;
}
.msg-list {
  list-style: none;
  margin: 0;
  padding: 0;
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
  overflow: hidden;
}
.msg-list li {
  padding: 0.75rem 0.85rem;
  border-bottom: 1px solid var(--border);
  background: rgba(0, 0, 0, 0.15);
}
.msg-list li.clickable {
  cursor: pointer;
}
.msg-list li.clickable:hover {
  background: rgba(61, 139, 253, 0.06);
}
.msg-list li:last-child {
  border-bottom: none;
}
.msg-list li.unread {
  background: rgba(61, 139, 253, 0.08);
}
.m-title {
  margin: 0;
  font-weight: 700;
  font-size: 0.92rem;
}
.m-body {
  margin: 0.35rem 0 0;
  font-size: 0.86rem;
  color: var(--muted);
  line-height: 1.45;
}
.m-time {
  display: block;
  margin-top: 0.45rem;
  font-size: 0.75rem;
  color: var(--muted);
}

.placeholder .bullets {
  margin: 0;
  padding-left: 1.1rem;
  color: var(--muted);
  line-height: 1.6;
  font-size: 0.9rem;
}

.desktop-only {
  display: block;
}
.mobile-only {
  display: none;
}

@media (max-width: 900px) {
  .desktop-only {
    display: none;
  }
  .mobile-only {
    display: block;
  }
}

.btn.sm {
  padding: 0.45rem 0.75rem;
  font-size: 0.85rem;
}
</style>