// 按《前后端接口文档》实现的接口函数集合。
// 所有函数返回响应信封中的 `data`，失败抛出 ApiError。

import { request, setToken, clearToken, getToken, ApiError } from './client'

export { ApiError, getToken }

// ---- 4. 认证与用户 ----
export const authApi = {
  // 4.1 用户注册
  register(body) {
    return request('/auth/register', { method: 'POST', body, auth: false })
  },
  // 4.2 用户登录（登录成功后写入 token）
  async login({ username, password }) {
    const data = await request('/auth/login', {
      method: 'POST',
      body: { username, password },
      auth: false,
    })
    if (data?.token) setToken(data.token)
    return data
  },
  logout() {
    clearToken()
  },
}

export const userApi = {
  // 4.3 获取当前用户信息
  me() {
    return request('/users/me')
  },
  // 4.4 修改个人资料
  updateProfile(body) {
    return request('/users/me', { method: 'PUT', body })
  },
  // 4.5 修改密码
  changePassword(body) {
    return request('/users/me/password', { method: 'PUT', body })
  },
}

// ---- 5. 物品平台 ----
export const itemApi = {
  // 5.1 物品列表（分页+筛选）
  list(query) {
    const q = query || {}
    return request('/items', {
      query: {
        page: q.page ?? 1,
        size: q.size ?? 50,
        keyword: q.keyword ?? '',
        category: q.category ?? '',
        status: q.status ?? '',
      },
    })
  },
  // 5.2 物品详情
  detail(itemId) {
    return request(`/items/${itemId}`)
  },
  // 5.3 提交物品上传申请（用户）
  submitSubmission(body) {
    return request('/items/submissions', { method: 'POST', body })
  },
  // 5.4 查询物品上传审核队列（管理员）
  adminSubmissionList(query) {
    return request('/admin/items/submissions', { query })
  },
  // 5.5 审核物品上传申请（管理员）
  auditSubmission(submissionId, { action, remark }) {
    return request(`/admin/items/submissions/${submissionId}/audit`, {
      method: 'PATCH',
      body: { action, remark },
    })
  },
  // 5.6 编辑物品（管理员）
  update(itemId, body) {
    return request(`/items/${itemId}`, { method: 'PUT', body })
  },
  // 5.7 上下架物品（管理员）
  setStatus(itemId, status) {
    return request(`/items/${itemId}/status`, { method: 'PATCH', body: { status } })
  },
}

// ---- 6. 预约借还 ----
export const orderApi = {
  // 6.1 创建预约
  createReservation(body) {
    return request('/orders/reservations', { method: 'POST', body })
  },
  // 6.x 管理端订单列表
  list(query) {
    return request('/orders', { query: { status: '', ...query } })
  },
  // 6.2 我的预约/订单列表
  my(query) {
    return request('/orders/my', { query: { status: '', ...query } })
  },
  // 6.3 订单详情
  detail(orderId) {
    return request(`/orders/${orderId}`)
  },
  // 6.4 取消预约
  cancel(orderId) {
    return request(`/orders/${orderId}/cancel`, { method: 'PATCH' })
  },
  // 6.5 审核预约（管理员）：action = APPROVE | REJECT
  audit(orderId, { action, remark }) {
    return request(`/orders/${orderId}/audit`, { method: 'PATCH', body: { action, remark } })
  },
  // 6.6 办理借出（管理员）
  borrow(orderId) {
    return request(`/orders/${orderId}/borrow`, { method: 'PATCH' })
  },
  // 6.7 办理归还（管理员）
  return(orderId, body) {
    return request(`/orders/${orderId}/return`, { method: 'PATCH', body })
  },
}

// ---- 7. 信用与统计 ----
export const creditApi = {
  // 7.1 查询用户信用记录
  records(query) {
    return request('/credits/records', { query: { userId: 0, page: 1, size: 10, ...query } })
  },
  // 7.2 手工调整信用分（管理员）
  adjust(body) {
    return request('/credits/adjust', { method: 'POST', body })
  },
}

export const compensationApi = {
  // 7.3 查询赔偿记录（管理员）
  list(query) {
    const q = query || {}
    return request('/compensations', {
      query: {
        page: q.page ?? 1,
        size: q.size ?? 20,
        status: q.status ?? '',
        userId: q.userId ?? 0,
        orderId: q.orderId ?? 0,
      },
    })
  },
  // 7.4 新增赔偿记录（管理员）
  create(body) {
    return request('/compensations', { method: 'POST', body })
  },
  // 7.5 更新赔偿状态（管理员）
  setStatus(id, status) {
    return request(`/compensations/${id}/status`, { method: 'PATCH', body: { status } })
  },
}

export const statsApi = {
  // 7.6 统计看板
  dashboard() {
    return request('/stats/dashboard')
  },
}

// ---- 8. 管理端 ----
export const adminApi = {
  // 8.1 用户管理列表
  users(query) {
    const q = query || {}
    return request('/admin/users', {
      query: {
        page: q.page ?? 1,
        size: q.size ?? 10,
        keyword: q.keyword ?? '',
        status: q.status ?? '',
      },
    })
  },
  // 8.2 启用/禁用用户
  setUserStatus(userId, status) {
    return request(`/admin/users/${userId}/status`, { method: 'PATCH', body: { status } })
  },
  // 8.3 审计日志查询
  auditLogs(query) {
    const q = query || {}
    return request('/admin/audit-logs', {
      query: {
        page: q.page ?? 1,
        size: q.size ?? 10,
        operatorId: q.operatorId ?? 0,
        startTime: q.startTime ?? '',
        endTime: q.endTime ?? '',
      },
    })
  },
}
