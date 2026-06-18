<template>
  <div class="items-page">
    <header class="topbar">
      <div class="brand-row">
        <router-link to="/" class="logo">借物星球</router-link>
        <span class="role-pill" :class="{ admin: isAdminSession }">{{ rolePillLabel }}</span>
        <div v-if="canResourceAdmin" class="role-switch" role="tablist" aria-label="视角切换">
          <button
            type="button"
            class="tab"
            :class="{ active: viewMode === 'borrow' }"
            role="tab"
            :aria-selected="viewMode === 'borrow'"
            @click="viewMode = 'borrow'"
          >
            借用视图
          </button>
          <button
            type="button"
            class="tab"
            :class="{ active: viewMode === 'admin' }"
            role="tab"
            :aria-selected="viewMode === 'admin'"
            @click="viewMode = 'admin'"
          >
            管理视图
          </button>
        </div>
        <p v-else class="perspective-hint">借用视角：预约、取货与归还流程。</p>
        <div class="brand-actions">
          <button v-if="!isAdminSession" type="button" class="btn btn-primary sm" @click="openUploadModal">
            上传物品
          </button>
          <button
            v-if="canResourceAdmin && viewMode === 'admin'"
            type="button"
            class="btn btn-primary sm"
            @click="goReviewPage"
          >
            审核队列（{{ pendingSubmissionsCount }}）
          </button>
          <router-link to="/me" class="btn btn-ghost sm">{{ isAdminSession ? '控制台' : '个人中心' }}</router-link>
          <button type="button" class="btn btn-ghost sm" @click="doLogout">退出</button>
        </div>
      </div>

      <div class="filters">
        <label class="search">
          <span class="sr-only">搜索物品</span>
          <input v-model="query" class="input" type="search" placeholder="搜索名称、编号、标签…" />
        </label>
        <select v-model="category" class="select">
          <option value="">全部分类</option>
          <option v-for="c in categories" :key="c" :value="c">{{ c }}</option>
        </select>
      </div>

    </header>

    <p v-if="offline" class="preview-banner">
      当前为离线演示数据：未连接到后端服务（{{ '/api/v1/items' }}），展示的是本地示例物品。
    </p>
    <p v-if="loadError" class="admin-banner">
      数据加载失败：{{ loadError }}
    </p>
    <p v-if="adminActionMsg" class="preview-banner">
      {{ adminActionMsg }}
    </p>
    <p v-if="userAccountAlert" class="account-alert">
      {{ userAccountAlert }}
    </p>

    <p v-if="canResourceAdmin && viewMode === 'admin'" class="admin-banner">
      管理视角：侧重视图与库存状态；物品详情抽屉以审核与备注为主，不发起师生侧预约。
    </p>
    <p v-else-if="isSystemAdmin" class="admin-banner">
      系统管理员视角：此页以浏览与联调核对为主，资源审核与库存操作由资源管理员处理。
    </p>
    <p v-else-if="isAdminSession && viewMode === 'borrow'" class="preview-banner">
      借用预览：便于核对师生侧体验；敏感操作仍受管理权限约束{{ DEMO_MODE ? '（示意）' : '' }}。
    </p>
    <p v-if="SHOW_API_HINT" class="preview-banner">
      接口：`GET /items`、`GET /items/{itemId}`、`POST /orders/reservations`
      <span v-if="canResourceAdmin && viewMode === 'admin'">
        、`PUT /items/{itemId}`、`PATCH /items/{itemId}/status`
      </span>
    </p>

    <main class="grid" :aria-busy="loading">
      <article
        v-for="item in filteredItems"
        :key="item.id"
        class="item-card"
        @click="openDetail(item)"
      >
        <div class="cover" :style="{ background: item.cover }" />
        <div class="body">
          <div class="row-title">
            <h3>{{ item.name }}</h3>
            <p class="item-category">{{ item.category }}</p>
          </div>
          <p class="meta">
            <span :class="stockClass(item.stockLevel)">{{ item.stockText }}</span>
          </p>
        </div>
      </article>
    </main>

    <transition name="fade">
      <div v-if="drawerOpen" class="drawer-backdrop" @click.self="closeDetail">
        <aside class="drawer" role="dialog" aria-modal="true" aria-labelledby="drawer-title">
          <header class="drawer-head">
            <div>
              <h2 id="drawer-title">{{ activeItem?.name }}</h2>
              <p class="drawer-sub">{{ activeItem?.category }}</p>
            </div>
            <button type="button" class="icon-btn" aria-label="关闭" @click="closeDetail">×</button>
          </header>

          <p v-if="canResourceAdmin && viewMode === 'admin'" class="drawer-admin-tip">
            管理员在此处理<strong>库存与审核</strong>；下方日历与预约表单对师生隐藏操作入口，仅作只读{{ DEMO_MODE ? '示意' : '展示' }}。
          </p>

          <section class="section">
            <h3>规则摘要</h3>
            <ul class="rules">
              <li>最长借用 <strong>14 天</strong>，逾期按日计分。</li>
              <li>押金 <strong>¥{{ activeItem?.deposit }}</strong>（归还验收后原路退回）。</li>
              <li>损坏照价赔偿；争议可走申诉流程。</li>
            </ul>
          </section>

          <section v-if="!(canResourceAdmin && viewMode === 'admin')" class="section">
            <h3>可借时段{{ DEMO_MODE ? '（示意）' : '' }}</h3>
            <div class="calendar" role="grid" aria-label="可借日期">
              <button
                v-for="d in calendarDays"
                :key="d.date"
                type="button"
                class="day"
                :class="{
                  selected: selectedDates.includes(d.date),
                  conflict: conflictDemo && selectedDates.includes(d.date),
                }"
                @click="toggleDate(d.date)"
              >
                <span class="dow">{{ d.dow }}</span>
                <span class="num">{{ d.day }}</span>
              </button>
            </div>
            <p class="hint" style="margin-top: 0.5rem">
              已选：
              <template v-if="rangeStartDate">
                {{ rangeStartDate }} 至 {{ rangeEndDate || '（请选择结束日期）' }}
              </template>
              <template v-else>请先选择开始日期，再选择结束日期</template>
            </p>
            <label v-if="DEMO_MODE" class="check">
              <input v-model="conflictDemo" type="checkbox" />
              演示「时段冲突」校验（勾选后提交）
            </label>
            <label v-if="DEMO_MODE" class="check">
              <input v-model="creditDemo" type="checkbox" />
              演示「信用不足」校验（勾选后提交）
            </label>
          </section>

          <section v-else class="section">
            <h3>管理侧动作</h3>
            <ul class="rules">
              <li>当前状态：<strong>{{ activeItem?.status }}</strong></li>
              <li>可执行：编辑基础信息、上下架、保存管理员备注</li>
            </ul>
            <div class="sso-actions" style="margin-top: 0.75rem">
              <button type="button" class="btn btn-ghost" @click="toggleItemStatus">
                {{ activeItem?.status === 'OFF_SHELF' ? '上架物品' : '下架物品' }}
              </button>
            </div>
            <div class="row-inline" style="margin-top: 0.75rem">
              <label class="field-block">
                <span class="lbl">物品名称</span>
                <input v-model="adminEdit.name" class="input" type="text" />
              </label>
              <label class="field-block">
                <span class="lbl">分类</span>
                <input v-model="adminEdit.category" class="input" type="text" />
              </label>
            </div>
            <label class="field-block">
              <span class="lbl">描述</span>
              <textarea v-model="adminEdit.description" class="textarea" />
            </label>
            <div class="row-inline">
              <label class="field-block">
                <span class="lbl">总量</span>
                <input v-model.number="adminEdit.totalCount" class="input" type="number" min="1" />
              </label>
              <label class="field-block">
                <span class="lbl">押金</span>
                <input v-model.number="adminEdit.deposit" class="input" type="number" min="0" step="0.01" />
              </label>
            </div>
          </section>

          <section v-if="!(canResourceAdmin && viewMode === 'admin')" class="section row-inline">
            <div>
              <h3>数量</h3>
              <div class="stepper">
                <button type="button" aria-label="减少" @click="qty = Math.max(1, qty - 1)">−</button>
                <span>{{ qty }}</span>
                <button type="button" aria-label="增加" @click="qty++">+</button>
              </div>
            </div>
          </section>

          <section v-if="!(canResourceAdmin && viewMode === 'admin')" class="section">
            <h3>用途说明</h3>
            <textarea
              v-model="purpose"
              class="textarea"
              placeholder="简要说明借用用途（课程实验 / 活动 / 社团物资等）"
            />
          </section>

          <section v-else class="section">
            <h3>管理员备注</h3>
            <textarea
              v-model="adminNote"
              class="textarea"
              :placeholder="DEMO_MODE ? '记录库存调整、报损处理或审核结论（仅管理端可见，示意）' : '记录库存调整、报损处理或审核结论（仅管理端可见）'"
            />
          </section>

          <div v-if="inlineErrors.length" class="inline-errors" role="alert">
            <p v-for="(e, i) in inlineErrors" :key="i">{{ e }}</p>
          </div>

          <footer v-if="canResourceAdmin && viewMode === 'admin'" class="drawer-foot">
            <button type="button" class="btn btn-ghost" @click="closeDetail">关闭</button>
            <button type="button" class="btn btn-primary" @click="saveAdminItem">保存编辑</button>
          </footer>
          <footer v-else class="drawer-foot">
            <button type="button" class="btn btn-ghost" @click="closeDetail">取消</button>
            <button type="button" class="btn btn-primary" :disabled="submitting" @click="submitReserve">
              {{ submitting ? '提交中…' : '提交预约' }}
            </button>
          </footer>
        </aside>
      </div>
    </transition>

    <transition name="fade">
      <div v-if="uploadModalOpen" class="upload-modal-backdrop" @click.self="closeUploadModal">
        <section class="upload-modal" role="dialog" aria-modal="true" aria-labelledby="upload-modal-title">
          <header class="upload-modal-head">
            <div>
              <h3 id="upload-modal-title">上传物品信息</h3>
              <p class="upload-sub">提交后进入审核队列，管理员通过后自动上架。</p>
            </div>
            <button type="button" class="upload-close-btn" aria-label="关闭" @click="closeUploadModal">×</button>
          </header>

          <div class="upload-grid">
            <label class="field-block">
              <span class="lbl">物品名称</span>
              <input v-model.trim="uploadForm.name" class="input" type="text" placeholder="例如：电子白板笔套装" />
            </label>
            <label class="field-block">
              <span class="lbl">分类</span>
              <input v-model.trim="uploadForm.category" class="input" type="text" placeholder="例如：教具" />
            </label>
            <label class="field-block">
              <span class="lbl">数量</span>
              <input v-model.number="uploadForm.totalCount" class="input" type="number" min="1" />
            </label>
            <label class="field-block">
              <span class="lbl">押金</span>
              <input v-model.number="uploadForm.deposit" class="input" type="number" min="0" step="0.01" />
            </label>
          </div>
          <label class="field-block">
            <span class="lbl">描述</span>
            <textarea v-model.trim="uploadForm.description" class="textarea" placeholder="简要描述物品用途、成色、注意事项" />
          </label>
          <footer class="upload-actions">
            <button type="button" class="btn btn-ghost sm" @click="closeUploadModal">取消</button>
            <button type="button" class="btn btn-primary sm" @click="submitItemUpload">提交审核</button>
          </footer>
        </section>
      </div>
    </transition>
  </div>
</template>

<script setup>
import { computed, onMounted, onUnmounted, reactive, ref, watch } from 'vue'
import { useRouter } from 'vue-router'
import { logout, session } from '../session'
import { itemApi, orderApi, userApi, ApiError } from '../api'
import { DEMO_MODE, SHOW_API_HINT } from '../config'

const router = useRouter()
const SUBMISSION_STORAGE_KEY = 'borrow-planet-item-submissions'
const APPROVED_ITEMS_STORAGE_KEY = 'borrow-planet-approved-items'
const UPLOAD_HISTORY_STORAGE_KEY = 'borrow-planet-item-upload-history'
const ITEM_STATUS_NOTICE_KEY = 'borrow-planet-item-status-notices'
const SUBMISSION_POLL_MS = 2000
const ITEM_LIST_POLL_MS = 2000

const viewMode = ref('borrow')
const isAdminSession = computed(() => session.role === 'admin')
const isSystemAdmin = computed(() => isAdminSession.value && session.adminType === 'system')
const canResourceAdmin = computed(
  () => isAdminSession.value && (session.adminType === 'resource' || session.adminType === 'legacy'),
)
const rolePillLabel = computed(() => {
  if (!isAdminSession.value) return '师生用户'
  if (session.adminType === 'system') return '系统管理员'
  if (session.adminType === 'resource') return '资源管理员'
  return '管理员'
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

onMounted(() => {
  viewMode.value = canResourceAdmin.value ? 'admin' : 'borrow'
})

onMounted(async () => {
  if (isAdminSession.value || session.mock) return
  try {
    const me = await userApi.me()
    session.accountStatus = String(me?.status || session.accountStatus || '')
  } catch {
    // ignore status refresh failure
  }
})

watch(
  () => [session.role, session.adminType],
  ([role]) => {
    if (role === 'user') viewMode.value = 'borrow'
    if (role === 'admin' && canResourceAdmin.value) viewMode.value = 'admin'
    if (role === 'admin' && !canResourceAdmin.value) viewMode.value = 'borrow'
    if (canResourceAdmin.value) refreshPendingSubmissionCount()
  },
)

function doLogout() {
  logout()
  router.push('/')
}
function goReviewPage() {
  router.push('/items/review')
}
const query = ref('')
const category = ref('')

const categories = computed(() => {
  const set = new Set(items.value.map((it) => it.category).filter(Boolean))
  return Array.from(set)
})

// 离线原型演示数据（后端不可用时回退使用）
const mockItems = [
  {
    id: 1,
    name: '便携投影仪',
    category: '电子设备',
    campus: '紫金港',
    building: '图书馆',
    location: '紫金港 · 图书馆 3F 服务台',
    distance: '距你约 320m',
    heat: '本月 42 次',
    stockLevel: 'ok',
    stockText: '库存充足',
    deposit: 200,
    cover: 'linear-gradient(135deg,#355c7d,#6c5b7b)',
  },
  {
    id: 2,
    name: '羽毛球拍套装',
    category: '体育器材',
    campus: '紫金港',
    building: '体育馆',
    location: '紫金港 · 体育馆器材室',
    distance: '距你约 890m',
    heat: '本月 118 次',
    stockLevel: 'low',
    stockText: '库存紧张（剩 2）',
    deposit: 50,
    cover: 'linear-gradient(135deg,#2a9d8f,#264653)',
  },
  {
    id: 3,
    name: '激光测距仪',
    category: '工具',
    campus: '玉泉',
    building: '实验室 A 座',
    location: '玉泉 · 实验室 A 座 502',
    distance: '跨校区',
    heat: '本月 19 次',
    stockLevel: 'out',
    stockText: '已约满',
    deposit: 800,
    cover: 'linear-gradient(135deg,#e76f51,#f4a261)',
  },
]

const items = ref(DEMO_MODE ? [...mockItems] : [])
const loading = ref(false)
const offline = ref(false)
const loadError = ref('')
const adminActionMsg = ref('')
const pendingSubmissions = ref([])
const uploadModalOpen = ref(false)
const uploadForm = reactive({
  name: '',
  category: '',
  description: '',
  totalCount: 1,
  deposit: 0,
})
const pendingSubmissionsCount = computed(() => pendingSubmissions.value.length)
let submissionPollTimer = null
let itemListPollTimer = null
let itemListPollingInFlight = false
let authExpiredHandled = false

function isAuthExpiredError(err) {
  if (!(err instanceof ApiError)) return false
  if (err.httpStatus === 401) return true
  return /token expired|token invalid|未登录|登录过期/i.test(String(err.message || ''))
}

function handleAuthExpired() {
  if (authExpiredHandled) return
  authExpiredHandled = true
  stopPendingSubmissionAutoRefresh()
  stopItemListAutoRefresh()
  loadError.value = '登录状态已过期，请重新登录。'
  logout()
  router.replace('/?reason=expired')
}

function onPendingPageActiveRefresh() {
  if (document.visibilityState === 'visible') refreshPendingSubmissionCount()
}

function onPendingWindowFocusRefresh() {
  refreshPendingSubmissionCount()
}

function onPendingStorageChange(e) {
  if (e.key === SUBMISSION_STORAGE_KEY) refreshPendingSubmissionCount()
}

function startPendingSubmissionAutoRefresh() {
  stopPendingSubmissionAutoRefresh()
  submissionPollTimer = window.setInterval(() => {
    refreshPendingSubmissionCount()
  }, SUBMISSION_POLL_MS)
  document.addEventListener('visibilitychange', onPendingPageActiveRefresh)
  window.addEventListener('focus', onPendingWindowFocusRefresh)
  if (DEMO_MODE) window.addEventListener('storage', onPendingStorageChange)
}

function stopPendingSubmissionAutoRefresh() {
  if (submissionPollTimer) {
    clearInterval(submissionPollTimer)
    submissionPollTimer = null
  }
  document.removeEventListener('visibilitychange', onPendingPageActiveRefresh)
  window.removeEventListener('focus', onPendingWindowFocusRefresh)
  window.removeEventListener('storage', onPendingStorageChange)
}

function onItemListPageActiveRefresh() {
  if (document.visibilityState === 'visible') refreshItemsList()
}

function onItemListWindowFocusRefresh() {
  refreshItemsList()
}

async function refreshItemsList() {
  if (itemListPollingInFlight) return
  itemListPollingInFlight = true
  try {
    await loadItems()
  } finally {
    itemListPollingInFlight = false
  }
}

function startItemListAutoRefresh() {
  stopItemListAutoRefresh()
  itemListPollTimer = window.setInterval(() => {
    if (typeof document !== 'undefined' && document.visibilityState === 'hidden') return
    refreshItemsList()
  }, ITEM_LIST_POLL_MS)
  document.addEventListener('visibilitychange', onItemListPageActiveRefresh)
  window.addEventListener('focus', onItemListWindowFocusRefresh)
}

function stopItemListAutoRefresh() {
  if (itemListPollTimer) {
    clearInterval(itemListPollTimer)
    itemListPollTimer = null
  }
  document.removeEventListener('visibilitychange', onItemListPageActiveRefresh)
  window.removeEventListener('focus', onItemListWindowFocusRefresh)
}

const covers = [
  'linear-gradient(135deg,#355c7d,#6c5b7b)',
  'linear-gradient(135deg,#2a9d8f,#264653)',
  'linear-gradient(135deg,#e76f51,#f4a261)',
  'linear-gradient(135deg,#3d5a80,#293241)',
  'linear-gradient(135deg,#5e548e,#231942)',
]

// 将后端 Item 模型映射为卡片展示所需结构
function mapItem(api) {
  const avail = Number(api.availableCount ?? 0)
  const total = Number(api.totalCount ?? avail)
  let stockLevel = 'ok'
  let stockText = '库存充足'
  if (avail <= 0) {
    stockLevel = 'out'
    stockText = '已约满'
  } else if (avail <= 2) {
    stockLevel = 'low'
    stockText = `库存紧张（剩 ${avail}）`
  }
  const idNum = Number(api.id) || 0
  return {
    id: api.id,
    name: api.name,
    category: api.category || '未分类',
    description: api.description || '',
    status: api.status || 'ON_SHELF',
    totalCount: total,
    availableCount: avail,
    campus: '',
    building: '',
    location: '校内借还点',
    distance: total ? `在册 ${avail}/${total}` : '可自取',
    heat: api.status === 'OFF_SHELF' ? '已下架' : '可借用',
    stockLevel,
    stockText,
    deposit: api.deposit ?? 0,
    submitterId: api.createdBy ?? api.created_by ?? null,
    cover: covers[idNum % covers.length],
  }
}

async function loadItems() {
  loading.value = true
  loadError.value = ''
  try {
    const data = await itemApi.list({
      page: 1,
      size: 50,
      keyword: query.value || undefined,
      category: category.value || undefined,
      status: 'ON_SHELF',
    })
    const list = Array.isArray(data) ? data : data?.list || []
    items.value = list.map(mapItem)
    if (DEMO_MODE) {
      const approved = loadApprovedItems().map(mapApprovedLocalItem)
      if (approved.length) items.value = [...approved, ...items.value]
    }
    offline.value = false
  } catch (err) {
    if (err instanceof ApiError && DEMO_MODE) {
      // 演示模式：后端不可用时回退到离线演示数据
      const approved = loadApprovedItems().map(mapApprovedLocalItem)
      items.value = [...approved, ...mockItems]
      offline.value = true
    } else if (isAuthExpiredError(err)) {
      items.value = []
      offline.value = false
      handleAuthExpired()
    } else if (err instanceof ApiError) {
      // 联调模式：保留错误，避免静默回退掩盖接口问题
      items.value = []
      offline.value = false
      loadError.value = err.message
    }
  } finally {
    loading.value = false
  }
}

onMounted(loadItems)
onMounted(() => {
  refreshPendingSubmissionCount()
  startPendingSubmissionAutoRefresh()
  startItemListAutoRefresh()
})

onUnmounted(() => {
  stopPendingSubmissionAutoRefresh()
  stopItemListAutoRefresh()
})

function loadSubmissions() {
  try {
    const raw = localStorage.getItem(SUBMISSION_STORAGE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    return Array.isArray(arr) ? arr : []
  } catch {
    return []
  }
}

function saveSubmissions() {
  localStorage.setItem(SUBMISSION_STORAGE_KEY, JSON.stringify(pendingSubmissions.value))
}

function loadUploadHistory() {
  try {
    const raw = localStorage.getItem(UPLOAD_HISTORY_STORAGE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    return Array.isArray(arr) ? arr : []
  } catch {
    return []
  }
}

function appendUploadHistory(record) {
  const history = loadUploadHistory()
  const next = Array.isArray(history) ? history : []
  next.unshift(record)
  localStorage.setItem(UPLOAD_HISTORY_STORAGE_KEY, JSON.stringify(next))
}

async function refreshPendingSubmissionCount() {
  if (!canResourceAdmin.value) return
  if (DEMO_MODE) {
    pendingSubmissions.value = loadSubmissions()
    return
  }
  try {
    const data = await itemApi.adminSubmissionList({ page: 1, size: 50, status: 'PENDING', _t: Date.now() })
    const list = Array.isArray(data) ? data : data?.list || []
    pendingSubmissions.value = list
  } catch (err) {
    if (isAuthExpiredError(err)) {
      pendingSubmissions.value = []
      handleAuthExpired()
      return
    }
    if (err instanceof ApiError) {
      adminActionMsg.value = `待审核队列刷新失败：${err.message}（已保留上次数据）`
    }
  }
}

function loadApprovedItems() {
  try {
    const raw = localStorage.getItem(APPROVED_ITEMS_STORAGE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    return Array.isArray(arr) ? arr : []
  } catch {
    return []
  }
}

function mapApprovedLocalItem(raw, idx) {
  const baseId = 900000
  const idNum = baseId + idx + 1
  const total = Number(raw.totalCount || 1)
  return {
    id: raw.localId || idNum,
    name: raw.name || `示例物品-${idNum}`,
    category: raw.category || '未分类',
    description: raw.description || '',
    status: raw.status || 'ON_SHELF',
    totalCount: total,
    availableCount: total,
    campus: '',
    building: '',
    location: '校内借还点',
    distance: `在册 ${total}/${total}`,
    heat: '可借用',
    stockLevel: 'ok',
    stockText: '库存充足',
    deposit: Number(raw.deposit || 0),
    submitterId: raw.submitterId ?? null,
    cover: covers[idNum % covers.length],
  }
}

function openUploadModal() {
  uploadModalOpen.value = true
}

function closeUploadModal() {
  uploadModalOpen.value = false
}

const drawerOpen = ref(false)
const activeItem = ref(null)
const qty = ref(1)
const purpose = ref('')
const selectedDates = ref([])
const rangeStartDate = ref('')
const rangeEndDate = ref('')
const conflictDemo = ref(false)
const creditDemo = ref(false)
const adminNote = ref('')
const adminEdit = reactive({
  name: '',
  category: '',
  description: '',
  totalCount: 1,
  deposit: 0,
})

const WEEKDAY_LABELS = ['日', '一', '二', '三', '四', '五', '六']
const calendarDays = Array.from({ length: 14 }, (_, offset) => {
  const d = new Date()
  d.setHours(0, 0, 0, 0)
  d.setDate(d.getDate() + offset)
  const yyyy = d.getFullYear()
  const mm = String(d.getMonth() + 1).padStart(2, '0')
  const dd = String(d.getDate()).padStart(2, '0')
  return {
    date: `${yyyy}-${mm}-${dd}`,
    dow: WEEKDAY_LABELS[d.getDay()],
    day: dd,
  }
})

const inlineErrors = ref([])

const filteredItems = computed(() => {
  return items.value.filter((it) => {
    if (String(it.status || '').toUpperCase() === 'OFF_SHELF') return false
    if (query.value && !`${it.name}${it.category}`.includes(query.value)) return false
    if (category.value && it.category !== category.value) return false
    return true
  })
})

function appendItemStatusNotice(record) {
  if (typeof localStorage === 'undefined') return
  try {
    const raw = localStorage.getItem(ITEM_STATUS_NOTICE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    const next = Array.isArray(arr) ? arr : []
    next.unshift(record)
    localStorage.setItem(ITEM_STATUS_NOTICE_KEY, JSON.stringify(next.slice(0, 200)))
  } catch {
    // ignore notice persistence failure
  }
}

function stockClass(level) {
  if (level === 'ok') return 'stock ok'
  if (level === 'low') return 'stock low'
  return 'stock out'
}

async function openDetail(item) {
  activeItem.value = item
  drawerOpen.value = true
  inlineErrors.value = []
  qty.value = 1
  purpose.value = ''
  adminNote.value = ''
  selectedDates.value = []
  rangeStartDate.value = ''
  rangeEndDate.value = ''
  conflictDemo.value = false
  creditDemo.value = false
  adminEdit.name = item.name || ''
  adminEdit.category = item.category || ''
  adminEdit.description = item.description || ''
  adminEdit.totalCount = item.totalCount || 1
  adminEdit.deposit = item.deposit || 0

  // 联调模式优先拉取 5.2 物品详情，避免只展示列表摘要字段
  if (!DEMO_MODE) {
    try {
      const detail = await itemApi.detail(item.id)
      activeItem.value = {
        ...activeItem.value,
        ...mapItem(detail),
        description: detail?.description || activeItem.value.description,
        deposit: detail?.deposit ?? activeItem.value.deposit,
      }
    } catch (err) {
      if (err instanceof ApiError) {
        inlineErrors.value = [`物品详情加载失败：${err.message}`]
      }
    }
  }
}

function validateUploadForm() {
  if (!uploadForm.name) return '请填写物品名称。'
  if (!uploadForm.category) return '请填写物品分类。'
  if (!uploadForm.description) return '请填写物品描述。'
  if (!Number(uploadForm.totalCount) || Number(uploadForm.totalCount) < 1) return '数量至少为 1。'
  if (Number(uploadForm.deposit) < 0) return '押金不能为负数。'
  return ''
}

function resetUploadForm() {
  uploadForm.name = ''
  uploadForm.category = ''
  uploadForm.description = ''
  uploadForm.totalCount = 1
  uploadForm.deposit = 0
}

function submitItemUpload() {
  const err = validateUploadForm()
  if (err) {
    loadError.value = err
    return
  }
  const rec = {
    id: `SUB-${Date.now().toString().slice(-6)}`,
    submitter: session.displayName || '匿名用户',
    submitterId: session.userId ?? null,
    name: uploadForm.name,
    category: uploadForm.category,
    description: uploadForm.description,
    totalCount: Number(uploadForm.totalCount || 1),
    deposit: Number(uploadForm.deposit || 0),
    createdAt: new Date().toISOString(),
  }
  if (DEMO_MODE) {
    appendUploadHistory({
      id: rec.id,
      name: rec.name,
      category: rec.category,
      totalCount: rec.totalCount,
      deposit: rec.deposit,
      status: 'PENDING',
      statusText: '待审核',
      createdAt: rec.createdAt,
      submitterId: session.userId ?? null,
    })
    pendingSubmissions.value.unshift(rec)
    saveSubmissions()
    resetUploadForm()
    closeUploadModal()
    loadError.value = ''
    adminActionMsg.value = `上传成功：${rec.name}，等待管理员审核。`
    return
  }
  itemApi
    .submitSubmission({
      name: rec.name,
      category: rec.category,
      description: rec.description,
      totalCount: rec.totalCount,
      deposit: rec.deposit,
    })
    .then((data) => {
      appendUploadHistory({
        id: data?.submissionId || rec.id,
        name: rec.name,
        category: rec.category,
        totalCount: rec.totalCount,
        deposit: rec.deposit,
        status: 'PENDING',
        statusText: '待审核',
        createdAt: new Date().toISOString(),
        submitterId: session.userId ?? null,
      })
      resetUploadForm()
      closeUploadModal()
      loadError.value = ''
      adminActionMsg.value = `上传成功：${rec.name}，等待管理员审核。`
      refreshPendingSubmissionCount()
    })
    .catch((err) => {
      if (err instanceof ApiError) loadError.value = `上传失败：${err.message}`
    })
}

async function toggleItemStatus() {
  if (!activeItem.value?.id) return
  if (DEMO_MODE) {
    inlineErrors.value = ['演示模式下不调用后端上下架接口。']
    return
  }
  if (String(activeItem.value.status || '').toUpperCase() === 'OFF_SHELF') {
    inlineErrors.value = ['该物品已下架并从列表移除，请通过重新上传发起新申请。']
    return
  }
  try {
    const target = 'OFF_SHELF'
    await itemApi.setStatus(activeItem.value.id, target)
    if (target === 'OFF_SHELF') {
      appendItemStatusNotice({
        id: `offshelf-${activeItem.value.id}-${Date.now()}`,
        itemId: activeItem.value.id,
        itemName: activeItem.value.name || '',
        submitterId: Number(activeItem.value.submitterId || 0) || null,
        status: target,
        operatorUserId: session.userId ?? null,
        operatorName: session.displayName || '管理员',
        createdAt: new Date().toISOString(),
      })
    }
    activeItem.value.status = target
    adminActionMsg.value = `物品已下架并从管理列表移除：${activeItem.value.name || activeItem.value.id}`
    closeDetail()
    await loadItems()
  } catch (err) {
    if (err instanceof ApiError) inlineErrors.value = [err.message]
  }
}

async function saveAdminItem() {
  if (!activeItem.value?.id) return
  if (DEMO_MODE) {
    inlineErrors.value = ['演示模式下不调用后端编辑接口。']
    return
  }
  try {
    await itemApi.update(activeItem.value.id, {
      name: adminEdit.name,
      category: adminEdit.category,
      description: adminEdit.description,
      totalCount: Number(adminEdit.totalCount || 1),
      deposit: Number(adminEdit.deposit || 0),
    })
    await loadItems()
    closeDetail()
  } catch (err) {
    if (err instanceof ApiError) inlineErrors.value = [err.message]
  }
}

function closeDetail() {
  drawerOpen.value = false
}

function buildDateRange(startDate, endDate) {
  return calendarDays.filter((d) => d.date >= startDate && d.date <= endDate).map((d) => d.date)
}

function toggleDate(date) {
  if (!rangeStartDate.value || (rangeStartDate.value && rangeEndDate.value)) {
    rangeStartDate.value = date
    rangeEndDate.value = ''
    selectedDates.value = [date]
    return
  }

  if (date < rangeStartDate.value) {
    inlineErrors.value = ['结束日期只能选择开始日期当天或之后。']
    return
  }

  inlineErrors.value = []
  rangeEndDate.value = date
  selectedDates.value = buildDateRange(rangeStartDate.value, rangeEndDate.value)
}

const submitting = ref(false)

async function submitReserve() {
  const errs = []
  const availableCount = Number(activeItem.value?.availableCount ?? 0)
  if (!rangeStartDate.value) errs.push('请先选择开始日期。')
  if (!rangeEndDate.value) errs.push('请选择结束日期。')
  if (!purpose.value.trim()) errs.push('请填写用途说明，便于审核与追溯。')
  if (activeItem.value?.stockLevel === 'out') errs.push('库存不足：该物品已约满，可加入候补或更换物品。')
  if (availableCount > 0 && qty.value > availableCount) {
    errs.push(`库存不足：当前可借 ${availableCount} 件，请减少预约数量。`)
  }
  if (conflictDemo.value) errs.push('时段冲突：所选时段与他人预约重叠，请改选其他日期。')
  if (creditDemo.value)
    errs.push(
      '信用不足：当前信用分不满足预约条件。建议：完成待归还订单、上传赔偿凭证，或在「信用与积分」查看流水。',
    )
  inlineErrors.value = errs
  if (errs.length) return

  // 仅演示模式允许本地跳过后端
  if (DEMO_MODE && (session.mock || offline.value)) {
    closeDetail()
    return
  }

  const dates = [...selectedDates.value].sort()
  const body = {
    itemId: activeItem.value.id,
    quantity: qty.value,
    reserveStartTime: `${dates[0]} 09:00:00`,
    reserveEndTime: `${dates[dates.length - 1]} 18:00:00`,
    remark: purpose.value.trim(),
  }

  submitting.value = true
  try {
    await orderApi.createReservation(body)
    closeDetail()
  } catch (err) {
    inlineErrors.value = [err instanceof ApiError ? err.message : '提交失败，请稍后重试。']
  } finally {
    submitting.value = false
  }
}

watch([drawerOpen, uploadModalOpen], ([drawer, upload]) => {
  document.body.style.overflow = drawer || upload ? 'hidden' : ''
})
</script>

<style scoped>
.items-page {
  max-width: 1320px;
  margin: 0 auto;
  padding: 1rem clamp(0.65rem, 2vw, 1.25rem) 5rem;
}

.topbar {
  display: flex;
  flex-direction: column;
  gap: 1rem;
  margin-bottom: 1.25rem;
}

.brand-row {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 0.75rem;
}

.logo {
  font-weight: 800;
  font-size: 1.15rem;
  color: var(--text);
  text-decoration: none;
}
.logo:hover {
  text-decoration: none;
  color: var(--accent);
}

.role-switch {
  display: inline-flex;
  border: 1px solid var(--border);
  border-radius: 999px;
  padding: 3px;
  background: rgba(0, 0, 0, 0.2);
}

.tab {
  border: none;
  background: transparent;
  color: var(--muted);
  font-weight: 600;
  font-size: 0.85rem;
  padding: 0.35rem 0.85rem;
  border-radius: 999px;
  cursor: pointer;
}
.tab.active {
  background: var(--surface-2);
  color: var(--text);
}

.btn.sm {
  padding: 0.45rem 0.75rem;
  font-size: 0.85rem;
}

.brand-actions {
  display: flex;
  flex-wrap: wrap;
  gap: 0.45rem;
  margin-left: auto;
  align-items: center;
}

.role-pill {
  font-size: 0.72rem;
  font-weight: 800;
  letter-spacing: 0.04em;
  text-transform: uppercase;
  padding: 0.25rem 0.5rem;
  border-radius: 999px;
  border: 1px solid var(--border);
  color: var(--muted);
  background: rgba(255, 255, 255, 0.04);
}
.role-pill.admin {
  border-color: rgba(245, 177, 74, 0.45);
  color: var(--warning);
  background: rgba(245, 177, 74, 0.1);
}

.perspective-hint {
  margin: 0;
  flex: 1;
  min-width: 140px;
  font-size: 0.82rem;
  color: var(--muted);
  line-height: 1.35;
}

.preview-banner {
  margin: 0 0 1rem;
  padding: 0.65rem 0.85rem;
  border-radius: var(--radius-md);
  background: rgba(61, 139, 253, 0.1);
  border: 1px solid rgba(61, 139, 253, 0.35);
  color: var(--text);
  font-size: 0.88rem;
  line-height: 1.45;
}

.account-alert {
  margin: 0 0 1rem;
  padding: 0.7rem 0.9rem;
  border-radius: var(--radius-md);
  background: rgba(240, 113, 120, 0.12);
  border: 1px solid rgba(240, 113, 120, 0.45);
  color: var(--danger);
  font-size: 0.9rem;
  font-weight: 700;
}

.drawer-admin-tip {
  margin: 0 0 1rem;
  padding: 0.65rem 0.75rem;
  border-radius: var(--radius-md);
  background: rgba(245, 177, 74, 0.1);
  border: 1px solid rgba(245, 177, 74, 0.3);
  font-size: 0.86rem;
  line-height: 1.45;
  color: var(--muted);
}

.filters {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 0.65rem;
}

@media (max-width: 900px) {
  .filters {
    grid-template-columns: 1fr;
  }
}

.admin-banner {
  margin: 0 0 1rem;
  padding: 0.65rem 0.85rem;
  border-radius: var(--radius-md);
  background: rgba(245, 177, 74, 0.12);
  border: 1px solid rgba(245, 177, 74, 0.35);
  color: var(--warning);
  font-size: 0.88rem;
}

.upload-sub {
  margin: 0 0 0.75rem;
  color: var(--muted);
  font-size: 0.86rem;
}
.upload-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 0.6rem;
  margin-bottom: 0.7rem;
}
.upload-actions {
  display: flex;
  justify-content: flex-end;
  margin-top: 0.7rem;
  gap: 0.5rem;
}
.field-block {
  display: grid;
  gap: 0.35rem;
}
.lbl {
  font-size: 0.8rem;
  color: var(--muted);
  font-weight: 600;
}

.upload-modal-backdrop {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.55);
  display: grid;
  place-items: center;
  z-index: 50;
  padding: 1rem;
}

.upload-modal {
  width: min(980px, 100%);
  max-height: min(92vh, 920px);
  overflow: auto;
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  background: var(--bg-elevated);
  box-shadow: var(--shadow);
  padding: 1.15rem 1.15rem 1rem;
}

.upload-modal-head {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 0.8rem;
}

.upload-modal-head .upload-sub {
  margin-bottom: 0.85rem;
}
.upload-modal .textarea {
  min-height: 220px;
  resize: vertical;
}

.upload-close-btn {
  border: 1px solid var(--border);
  background: rgba(255, 255, 255, 0.06);
  color: var(--text);
  width: 2.15rem;
  height: 2.15rem;
  border-radius: 12px;
  font-size: 1.2rem;
  line-height: 1;
  cursor: pointer;
}
.upload-close-btn:hover {
  border-color: rgba(61, 139, 253, 0.45);
  background: rgba(61, 139, 253, 0.12);
}

.grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
  gap: 1rem;
}

.item-card {
  background: var(--bg-elevated);
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  overflow: hidden;
  cursor: pointer;
  transition: border-color 0.15s ease, transform 0.12s ease;
}
.item-card:hover {
  border-color: rgba(61, 139, 253, 0.45);
  transform: translateY(-2px);
}

.cover {
  height: 120px;
}

.body {
  padding: 1rem 1.1rem 1.15rem;
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  text-align: left;
}

.row-title {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 0.35rem;
}

.row-title h3 {
  min-width: 0;
  overflow-wrap: anywhere;
  word-break: break-word;
  line-height: 1.35;
}

h3 {
  margin: 0;
  font-size: 1.05rem;
}

.item-category {
  margin: 0;
  max-width: 100%;
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--muted);
  line-height: 1.35;
  overflow-wrap: anywhere;
  word-break: break-word;
  text-align: left;
}

.meta {
  margin: 0.45rem 0 0.6rem;
  width: 100%;
  font-size: 0.85rem;
  color: var(--muted);
  line-height: 1.45;
  text-align: left;
}

.stock.ok {
  color: var(--success);
  font-weight: 600;
}
.stock.low {
  color: var(--warning);
  font-weight: 600;
}
.stock.out {
  color: var(--danger);
  font-weight: 600;
}

.drawer-backdrop {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.55);
  display: flex;
  justify-content: flex-end;
  z-index: 40;
}

.drawer {
  width: min(480px, 100%);
  background: var(--bg-elevated);
  border-left: 1px solid var(--border);
  padding: 1.25rem 1.25rem 1rem;
  overflow: auto;
  max-height: 100vh;
  box-shadow: -12px 0 40px rgba(0, 0, 0, 0.45);
}

.drawer-head {
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  align-items: flex-start;
  margin-bottom: 1rem;
}

.drawer-sub {
  margin: 0.25rem 0 0;
  color: var(--muted);
  font-size: 0.88rem;
  overflow-wrap: anywhere;
  word-break: break-word;
}

.drawer-head h2 {
  margin: 0;
  overflow-wrap: anywhere;
  word-break: break-word;
}

.icon-btn {
  border: none;
  background: rgba(255, 255, 255, 0.06);
  color: var(--text);
  width: 2rem;
  height: 2rem;
  border-radius: 10px;
  font-size: 1.35rem;
  line-height: 1;
  cursor: pointer;
}

.section {
  margin-bottom: 1.1rem;
}
.section h3 {
  margin: 0 0 0.5rem;
  font-size: 0.95rem;
}

.rules {
  margin: 0;
  padding-left: 1.1rem;
  color: var(--muted);
  font-size: 0.88rem;
  line-height: 1.55;
}

.calendar {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  gap: 0.35rem;
}

.day {
  border: 1px solid var(--border);
  background: rgba(0, 0, 0, 0.2);
  color: var(--text);
  border-radius: 10px;
  padding: 0.35rem 0.2rem;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.15rem;
  font-size: 0.75rem;
}
.day .dow {
  color: var(--muted);
  font-weight: 600;
}
.day .num {
  font-weight: 700;
  font-size: 0.95rem;
}
.day.selected {
  border-color: rgba(61, 139, 253, 0.65);
  background: var(--accent-soft);
}
.day.conflict.selected {
  border-color: rgba(240, 113, 120, 0.65);
  background: rgba(240, 113, 120, 0.12);
}

.check {
  display: flex;
  gap: 0.45rem;
  align-items: center;
  margin-top: 0.65rem;
  font-size: 0.82rem;
  color: var(--muted);
  cursor: pointer;
}

.row-inline {
  display: grid;
  grid-template-columns: 1fr 1.4fr;
  gap: 0.75rem;
  align-items: start;
}
@media (max-width: 520px) {
  .row-inline {
    grid-template-columns: 1fr;
  }
}

.stepper {
  display: inline-flex;
  align-items: center;
  border: 1px solid var(--border);
  border-radius: 10px;
  overflow: hidden;
}
.stepper button {
  width: 2.25rem;
  height: 2.25rem;
  border: none;
  background: rgba(255, 255, 255, 0.06);
  color: var(--text);
  font-size: 1.1rem;
  cursor: pointer;
}
.stepper span {
  min-width: 2.5rem;
  text-align: center;
  font-weight: 700;
}

.inline-errors {
  border-radius: var(--radius-md);
  padding: 0.65rem 0.75rem;
  background: rgba(240, 113, 120, 0.1);
  border: 1px solid rgba(240, 113, 120, 0.35);
  color: var(--error-text);
  font-size: 0.86rem;
  margin-bottom: 0.75rem;
}
.inline-errors p {
  margin: 0.15rem 0;
}

.drawer-foot {
  display: flex;
  justify-content: flex-end;
  gap: 0.5rem;
  padding-top: 0.5rem;
  position: sticky;
  bottom: 0;
  background: linear-gradient(180deg, transparent, var(--bg-elevated) 28%);
  padding-bottom: 0.5rem;
}

.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.18s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
