// 低层 HTTP 客户端：统一处理 Base URL、JWT、{code,message,data} 信封与错误。
// 对应《前后端接口文档》第 2 节接口约定。

const BASE_URL = import.meta.env.VITE_API_BASE || '/api/v1'
const TOKEN_KEY = 'bp_token'
const AUTH_EXPIRED_NOTICE_KEY = 'bp_auth_expired_notice'
const REQUEST_TIMEOUT_MS = Number(import.meta.env.VITE_API_TIMEOUT_MS || 15000)
let authExpiredLock = false

function tokenStorage() {
  return typeof sessionStorage !== 'undefined' ? sessionStorage : null
}

function legacyTokenStorage() {
  return typeof localStorage !== 'undefined' ? localStorage : null
}

function setAuthExpiredNotice() {
  const storage = tokenStorage()
  if (!storage) return
  storage.setItem(AUTH_EXPIRED_NOTICE_KEY, '1')
}

export function consumeAuthExpiredNotice() {
  const storage = tokenStorage()
  if (!storage) return false
  const noticed = storage.getItem(AUTH_EXPIRED_NOTICE_KEY) === '1'
  if (noticed) storage.removeItem(AUTH_EXPIRED_NOTICE_KEY)
  return noticed
}

/** 业务/网络错误统一类型 */
export class ApiError extends Error {
  constructor(message, { code = null, httpStatus = null, network = false } = {}) {
    super(message)
    this.name = 'ApiError'
    this.code = code
    this.httpStatus = httpStatus
    this.network = network
  }
}

export function getToken() {
  const storage = tokenStorage()
  if (!storage) return ''
  const token = storage.getItem(TOKEN_KEY) || ''
  if (token) return token

  // 兼容旧版本：首次访问时把 localStorage token 迁移到 sessionStorage。
  const legacyStorage = legacyTokenStorage()
  const legacyToken = legacyStorage?.getItem(TOKEN_KEY) || ''
  if (!legacyToken) return ''
  storage.setItem(TOKEN_KEY, legacyToken)
  legacyStorage?.removeItem(TOKEN_KEY)
  return legacyToken
}

export function setToken(token) {
  const storage = tokenStorage()
  if (!storage) return
  if (token) {
    storage.setItem(TOKEN_KEY, token)
    authExpiredLock = false
    storage.removeItem(AUTH_EXPIRED_NOTICE_KEY)
  } else {
    storage.removeItem(TOKEN_KEY)
  }
}

export function clearToken() {
  setToken('')
}

function isAuthExpiredPayload(status, payload) {
  if (status === 401) return true
  const msg = String(payload?.message || '')
  return /token expired|token invalid|未登录|登录过期/i.test(msg)
}

function buildUrl(path, query) {
  const url = `${BASE_URL}${path}`
  if (!query) return url
  const params = new URLSearchParams()
  Object.entries(query).forEach(([k, v]) => {
    if (v === undefined || v === null) return
    params.append(k, String(v))
  })
  const qs = params.toString()
  return qs ? `${url}?${qs}` : url
}

/**
 * 统一请求方法。
 * @returns 解析后的 `data` 字段
 * @throws {ApiError}
 */
export async function request(path, { method = 'GET', query, body, auth = true, signal } = {}) {
  if (auth && authExpiredLock) {
    setAuthExpiredNotice()
    throw new ApiError('登录状态已过期，请重新登录。', { httpStatus: 401 })
  }
  const headers = {}
  if (body !== undefined) headers['Content-Type'] = 'application/json'
  if (auth) {
    const token = getToken()
    if (token) headers['Authorization'] = `Bearer ${token}`
  }

  let res
  let timeoutId = null
  let timedOut = false
  const controller = new AbortController()
  const forwardAbort = () => controller.abort()
  if (signal) {
    if (signal.aborted) controller.abort()
    else signal.addEventListener('abort', forwardAbort, { once: true })
  }
  if (Number.isFinite(REQUEST_TIMEOUT_MS) && REQUEST_TIMEOUT_MS > 0) {
    timeoutId = setTimeout(() => {
      timedOut = true
      controller.abort()
    }, REQUEST_TIMEOUT_MS)
  }
  try {
    res = await fetch(buildUrl(path, query), {
      method,
      headers,
      body: body !== undefined ? JSON.stringify(body) : undefined,
      signal: controller.signal,
    })
  } catch (e) {
    throw new ApiError(
      timedOut
        ? `请求超时（>${REQUEST_TIMEOUT_MS}ms），请检查后端服务状态后重试。`
        : e?.name === 'AbortError'
          ? '请求已取消'
          : '网络请求失败，请检查后端服务或网络。',
      {
      network: true,
      },
    )
  } finally {
    if (timeoutId) clearTimeout(timeoutId)
    if (signal) signal.removeEventListener('abort', forwardAbort)
  }

  let payload = null
  const text = await res.text()
  if (text) {
    try {
      payload = JSON.parse(text)
    } catch {
      payload = null
    }
  }

  if (!res.ok) {
    if (auth && isAuthExpiredPayload(res.status, payload)) {
      authExpiredLock = true
      setAuthExpiredNotice()
      clearToken()
    }
    const msg = payload?.message || `请求失败（HTTP ${res.status}）`
    throw new ApiError(msg, { code: payload?.code ?? null, httpStatus: res.status })
  }

  // 统一信封 {code, message, data}
  if (payload && typeof payload === 'object' && 'code' in payload) {
    if (payload.code !== 0) {
      throw new ApiError(payload.message || `业务错误（code ${payload.code}）`, {
        code: payload.code,
        httpStatus: res.status,
      })
    }
    return payload.data
  }

  // 兜底：非信封结构直接返回
  return payload
}

export const BASE = BASE_URL
