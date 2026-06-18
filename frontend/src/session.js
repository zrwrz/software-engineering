import { reactive } from 'vue'
import { setToken, getToken, clearToken } from './api/client'
import { DEMO_MODE } from './config'

const KEY = 'bp_proto_session'

function readStorage() {
  if (typeof sessionStorage === 'undefined') return null
  try {
    const raw = sessionStorage.getItem(KEY)
    if (!raw) return null
    const o = JSON.parse(raw)
    if (o?.role !== 'user' && o?.role !== 'admin') return null
    // 联调模式下拒绝恢复任何离线 mock 会话
    if (o?.mock === true && !DEMO_MODE) return null
    return {
      role: o.role,
      adminType:
        o.role === 'admin'
          ? normalizeAdminType(typeof o.adminType === 'string' ? o.adminType : 'legacy')
          : 'none',
      backendRole: typeof o.backendRole === 'string' ? o.backendRole : '',
      displayName: typeof o.displayName === 'string' ? o.displayName : '',
      userId: o.userId ?? null,
      creditScore: typeof o.creditScore === 'number' ? o.creditScore : null,
      accountStatus: typeof o.accountStatus === 'string' ? o.accountStatus : '',
      mock: o.mock === true,
    }
  } catch {
    return null
  }
}

const initial = readStorage()
const hasToken = Boolean(getToken())

export const session = reactive({
  role: initial && (initial.mock || hasToken) ? initial.role : 'guest',
  // 管理员类型：system | resource | legacy | none
  adminType: initial && (initial.mock || hasToken) ? initial.adminType : 'none',
  // 后端原始角色值，便于前端按实际权限做 UI 分支
  backendRole: initial && (initial.mock || hasToken) ? initial.backendRole : '',
  displayName: initial && (initial.mock || hasToken) ? initial.displayName : '',
  userId: initial && (initial.mock || hasToken) ? initial.userId : null,
  creditScore: initial && (initial.mock || hasToken) ? initial.creditScore : null,
  accountStatus: initial && (initial.mock || hasToken) ? initial.accountStatus : '',
  // 标记是否为离线原型会话（未连接真实后端）
  mock: initial && (initial.mock || hasToken) ? initial.mock : false,
})

if (typeof sessionStorage !== 'undefined' && initial && !initial.mock && !hasToken) {
  sessionStorage.removeItem(KEY)
}

function persist() {
  if (typeof sessionStorage === 'undefined') return
  sessionStorage.setItem(
    KEY,
    JSON.stringify({
      role: session.role,
      adminType: session.adminType,
      backendRole: session.backendRole,
      displayName: session.displayName,
      userId: session.userId,
      creditScore: session.creditScore,
      accountStatus: session.accountStatus,
      mock: session.mock,
    }),
  )
}

/** 后端角色（USER/ADMIN）映射到本地会话角色 */
function mapRole(backendRole) {
  const normalized = String(backendRole || '').trim().toUpperCase()
  // 兼容 ADMIN / ROLE_ADMIN / SYSTEM_ADMIN / RESOURCE_ADMIN 等后端角色命名。
  if (normalized === 'ADMIN' || normalized.includes('ADMIN')) return 'admin'
  return 'user'
}

function normalizeAdminType(value) {
  return ['system', 'resource', 'legacy', 'none'].includes(value) ? value : 'none'
}

/** 后端管理员角色细分：SYSTEM_ADMIN / RESOURCE_ADMIN */
function mapAdminType(backendRole) {
  const normalized = String(backendRole || '').trim().toUpperCase()
  if (!normalized.includes('ADMIN')) return 'none'
  if (normalized.includes('SYSTEM_ADMIN')) return 'system'
  if (normalized.includes('RESOURCE_ADMIN')) return 'resource'
  return 'legacy'
}

/**
 * 使用后端登录返回的 userInfo 建立真实会话。
 * userInfo 形如 { id, username, role, creditScore }
 */
export function applyLogin(userInfo) {
  const backendRole = String(userInfo?.role || '')
  session.role = mapRole(backendRole)
  session.adminType = session.role === 'admin' ? mapAdminType(backendRole) : 'none'
  session.backendRole = backendRole
  session.displayName = userInfo?.username || userInfo?.displayName || '用户'
  session.userId = userInfo?.id ?? null
  session.creditScore = typeof userInfo?.creditScore === 'number' ? userInfo.creditScore : null
  session.accountStatus = String(userInfo?.status || '')
  session.mock = false
  persist()
}

/** 师生用户（离线原型模拟） */
export function loginAsUser(displayName = '张同学') {
  if (!DEMO_MODE) return
  session.role = 'user'
  session.adminType = 'none'
  session.backendRole = 'USER'
  session.displayName = displayName
  session.userId = null
  session.creditScore = 86
  session.accountStatus = 'active'
  session.mock = true
  persist()
}

/** 管理员（离线原型模拟） */
export function loginAsAdmin(displayName = '李老师') {
  if (!DEMO_MODE) return
  session.role = 'admin'
  session.adminType = 'legacy'
  session.backendRole = 'ADMIN'
  session.displayName = displayName
  session.userId = null
  session.creditScore = null
  session.accountStatus = 'active'
  session.mock = true
  persist()
}

export function logout() {
  session.role = 'guest'
  session.adminType = 'none'
  session.backendRole = ''
  session.displayName = ''
  session.userId = null
  session.creditScore = null
  session.accountStatus = ''
  session.mock = false
  clearToken()
  if (typeof sessionStorage !== 'undefined') sessionStorage.removeItem(KEY)
}

export function isUser() {
  return session.role === 'user'
}

export function isAdmin() {
  return session.role === 'admin'
}

export function isSystemAdmin() {
  return session.role === 'admin' && (session.adminType === 'system' || session.adminType === 'legacy')
}

export function isResourceAdmin() {
  return session.role === 'admin' && (session.adminType === 'resource' || session.adminType === 'legacy')
}

export function isGuest() {
  return session.role === 'guest'
}

// 重新导出，便于其它模块统一从 session 引入
export { setToken }
