<template>
  <div class="review-page">
    <header class="head">
      <div>
        <h1>物品审核队列</h1>
        <p>资源管理员审核用户上传物品，审核通过后自动上架。</p>
      </div>
      <div class="actions">
        <router-link to="/items" class="btn btn-ghost sm">返回物品页</router-link>
      </div>
    </header>

    <p v-if="message" class="banner ok">{{ message }}</p>
    <p v-if="error" class="banner err">{{ error }}</p>
    <p v-if="SHOW_API_HINT" class="banner hint">
      接口：`GET /admin/items/submissions`、`PATCH /admin/items/submissions/{submissionId}/audit`
    </p>

    <div class="table-wrap">
      <table class="ledger">
        <thead>
          <tr>
            <th>申请号</th>
            <th>提交人</th>
            <th>物品</th>
            <th>分类</th>
            <th>数量</th>
            <th>押金</th>
            <th>状态</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="s in submissions" :key="s.id">
            <td class="mono">{{ s.id }}</td>
            <td>{{ s.submitter || s.submitterId || '-' }}</td>
            <td>{{ s.name }}</td>
            <td>{{ s.category }}</td>
            <td>{{ s.totalCount }}</td>
            <td>¥{{ Number(s.deposit || 0).toFixed(2) }}</td>
            <td><span class="badge badge-warn">待审核</span></td>
            <td class="ops">
              <button type="button" class="linkish" @click="approveSubmission(s)">通过并上架</button>
              <button type="button" class="linkish" @click="rejectSubmission(s)">驳回</button>
            </td>
          </tr>
          <tr v-if="!submissions.length">
            <td colspan="8" class="empty">当前无待审核上传。</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { computed, onMounted, onUnmounted, ref } from 'vue'
import { useRouter } from 'vue-router'
import { logout, session } from '../session'
import { itemApi, ApiError } from '../api'
import { DEMO_MODE, SHOW_API_HINT } from '../config'

const router = useRouter()
const SUBMISSION_STORAGE_KEY = 'borrow-planet-item-submissions'
const APPROVED_ITEMS_STORAGE_KEY = 'borrow-planet-approved-items'
const UPLOAD_HISTORY_STORAGE_KEY = 'borrow-planet-item-upload-history'
const QUEUE_POLL_MS = 2000

const submissions = ref([])
const message = ref('')
const error = ref('')
const canResourceAdmin = computed(
  () => session.role === 'admin' && (session.adminType === 'resource' || session.adminType === 'legacy'),
)
let queueTimer = null
let queueLoading = false
let authExpiredHandled = false

function isAuthExpiredError(err) {
  if (!(err instanceof ApiError)) return false
  if (err.httpStatus === 401) return true
  return /token expired|token invalid|未登录|登录过期/i.test(String(err.message || ''))
}

function handleAuthExpired() {
  if (authExpiredHandled) return
  authExpiredHandled = true
  stopQueueAutoRefresh()
  error.value = '登录状态已过期，请重新登录。'
  logout()
  router.replace('/?reason=expired')
}

function onPageActiveRefresh() {
  if (document.visibilityState === 'visible') loadQueue()
}

function onWindowFocusRefresh() {
  loadQueue()
}

function onSubmissionStorageChange(e) {
  if (e.key === SUBMISSION_STORAGE_KEY) loadQueue()
}

function startQueueAutoRefresh() {
  stopQueueAutoRefresh()
  queueTimer = window.setInterval(() => {
    loadQueue()
  }, QUEUE_POLL_MS)
  document.addEventListener('visibilitychange', onPageActiveRefresh)
  window.addEventListener('focus', onWindowFocusRefresh)
  if (DEMO_MODE) window.addEventListener('storage', onSubmissionStorageChange)
}

function stopQueueAutoRefresh() {
  if (queueTimer) {
    clearInterval(queueTimer)
    queueTimer = null
  }
  document.removeEventListener('visibilitychange', onPageActiveRefresh)
  window.removeEventListener('focus', onWindowFocusRefresh)
  window.removeEventListener('storage', onSubmissionStorageChange)
}

onMounted(() => {
  if (!canResourceAdmin.value) {
    router.replace('/items')
    return
  }
  loadQueue()
  startQueueAutoRefresh()
})

onUnmounted(() => {
  stopQueueAutoRefresh()
})

async function loadQueue() {
  if (queueLoading) return
  queueLoading = true
  if (DEMO_MODE) {
    submissions.value = loadSubmissions()
    queueLoading = false
    return
  }
  try {
    const data = await itemApi.adminSubmissionList({ page: 1, size: 50, status: 'PENDING', _t: Date.now() })
    submissions.value = Array.isArray(data) ? data : data?.list || []
  } catch (err) {
    if (isAuthExpiredError(err)) {
      submissions.value = []
      handleAuthExpired()
      return
    }
    if (err instanceof ApiError) error.value = `加载审核队列失败：${err.message}（已保留上次数据）`
  } finally {
    queueLoading = false
  }
}

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
  localStorage.setItem(SUBMISSION_STORAGE_KEY, JSON.stringify(submissions.value))
}

function appendApprovedItemLocal(submission) {
  const approved = JSON.parse(localStorage.getItem(APPROVED_ITEMS_STORAGE_KEY) || '[]')
  const next = Array.isArray(approved) ? approved : []
  next.unshift({
    localId: `LOCAL-${Date.now().toString().slice(-6)}`,
    name: submission.name,
    category: submission.category,
    description: submission.description,
    totalCount: submission.totalCount,
    deposit: submission.deposit,
    submitterId: submission.submitterId ?? null,
    status: 'ON_SHELF',
  })
  localStorage.setItem(APPROVED_ITEMS_STORAGE_KEY, JSON.stringify(next))
}

function updateUploadHistoryStatus(submissionId, status) {
  if (typeof localStorage === 'undefined') return
  try {
    const raw = localStorage.getItem(UPLOAD_HISTORY_STORAGE_KEY)
    const arr = raw ? JSON.parse(raw) : []
    const list = Array.isArray(arr) ? arr : []
    const next = list.map((r) => {
      if (String(r?.id) !== String(submissionId)) return r
      return {
        ...r,
        status,
        statusText: status === 'APPROVED' ? '已通过' : status === 'REJECTED' ? '已驳回' : '待审核',
      }
    })
    localStorage.setItem(UPLOAD_HISTORY_STORAGE_KEY, JSON.stringify(next))
  } catch {
    // ignore malformed local data
  }
}

async function approveSubmission(s) {
  message.value = ''
  error.value = ''
  try {
    if (DEMO_MODE) {
      appendApprovedItemLocal(s)
      submissions.value = submissions.value.filter((it) => it.id !== s.id)
    } else {
      await itemApi.auditSubmission(s.id, {
        action: 'APPROVE',
        remark: '管理员审核通过',
      })
    }
    updateUploadHistoryStatus(s.id, 'APPROVED')
    if (DEMO_MODE) saveSubmissions()
    await loadQueue()
    message.value = `审核通过并上架：${s.name}`
  } catch (err) {
    if (err instanceof ApiError) error.value = `审核失败：${err.message}`
  }
}

async function rejectSubmission(s) {
  message.value = ''
  error.value = ''
  try {
    if (DEMO_MODE) {
      submissions.value = submissions.value.filter((it) => it.id !== s.id)
      saveSubmissions()
    } else {
      await itemApi.auditSubmission(s.id, { action: 'REJECT', remark: '信息不完整，驳回' })
      await loadQueue()
    }
    updateUploadHistoryStatus(s.id, 'REJECTED')
    message.value = `已驳回上传申请：${s.name}`
  } catch (err) {
    if (err instanceof ApiError) error.value = `驳回失败：${err.message}`
  }
}
</script>

<style scoped>
.review-page {
  max-width: 1100px;
  margin: 0 auto;
  padding: 1rem;
}
.head {
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-bottom: 0.85rem;
}
.head h1 {
  margin: 0;
  font-size: 1.3rem;
}
.head p {
  margin: 0.35rem 0 0;
  color: var(--muted);
}
.actions {
  display: flex;
  align-items: center;
}
.banner {
  margin: 0 0 0.75rem;
  padding: 0.6rem 0.75rem;
  border-radius: var(--radius-md);
  font-size: 0.88rem;
}
.banner.ok {
  background: rgba(62, 207, 142, 0.12);
  border: 1px solid rgba(62, 207, 142, 0.35);
}
.banner.err {
  background: rgba(240, 113, 120, 0.1);
  border: 1px solid rgba(240, 113, 120, 0.35);
}
.banner.hint {
  background: rgba(61, 139, 253, 0.1);
  border: 1px solid rgba(61, 139, 253, 0.35);
}
.table-wrap {
  overflow: auto;
  border: 1px solid var(--border);
  border-radius: var(--radius-md);
}
.ledger {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.88rem;
}
.ledger th,
.ledger td {
  padding: 0.6rem 0.7rem;
  text-align: left;
  border-bottom: 1px solid var(--border);
}
.ledger th {
  text-transform: uppercase;
  font-size: 0.76rem;
  color: var(--muted);
  background: rgba(0, 0, 0, 0.18);
}
.ops {
  display: inline-flex;
  gap: 0.55rem;
}
.linkish {
  border: none;
  background: none;
  color: var(--accent);
  font-weight: 600;
  cursor: pointer;
  padding: 0;
}
.linkish:hover {
  text-decoration: underline;
}
.mono {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
}
.empty {
  color: var(--muted);
}
</style>
