<template>
  <div class="login-page">
    <div class="brand">
      <div class="emblem planet-mark" aria-hidden="true">
        <span class="planet-core" />
        <span class="planet-ring" />
        <span class="planet-dot dot-a" />
        <span class="planet-dot dot-b" />
      </div>
      <div>
        <h1 class="title">借物星球</h1>
        <p class="tagline">把校园共享借还，变成可追溯、可统计的线上流程。</p>
      </div>
      <ul class="bullets">
        <li>师生用户：预约、借用、信用与消息</li>
        <li>管理员：审核、库存与风险处置（入口独立）</li>
      </ul>
      <p class="help-link">
        找不到入口？
        <a href="#" @click.prevent>查看帮助与常见问题</a>
      </p>
    </div>

    <section class="card" aria-labelledby="login-heading">
      <h2 id="login-heading" class="card-title">登录</h2>
      <p class="card-sub">请选择身份入口；管理员与师生使用不同权限模型。</p>
      <p v-if="SHOW_API_HINT" class="card-sub">
        接口：`POST /auth/login`（注册账号/管理员登录）、
        `POST /auth/register`（注册入口在单独页面）。
      </p>

      <div v-if="justRegistered" class="banner success" role="status">
        注册成功，请使用新账号登录。
      </div>
      <div v-if="expiredNotice" class="banner warning" role="alert">登录已过期，请重新登录。</div>

      <div class="kind-tabs" role="tablist" aria-label="登录类型">
        <button
          type="button"
          class="kind-tab"
          :class="{ active: loginKind === 'user' }"
          role="tab"
          :aria-selected="loginKind === 'user'"
          @click="setKind('user')"
        >
          师生用户
        </button>
        <button
          type="button"
          class="kind-tab"
          :class="{ active: loginKind === 'admin' }"
          role="tab"
          :aria-selected="loginKind === 'admin'"
          @click="setKind('admin')"
        >
          管理员
        </button>
      </div>

      <div v-if="alreadyIn" class="banner success" role="status">
        你已以「{{ roleLabel }}」身份登录。
        <router-link :to="postLoginTarget" class="inline-link">进入系统</router-link>
        ·
        <button type="button" class="link-btn" @click="doLogout">切换账号</button>
      </div>

      <!-- 师生 -->
      <div v-show="loginKind === 'user'" class="kind-panel">
        <div v-if="DEMO_MODE" class="method-tabs" role="tablist" aria-label="师生登录方式">
          <button
            type="button"
            class="method-tab"
            :class="{ active: userMethod === 'sso' }"
            role="tab"
            :aria-selected="userMethod === 'sso'"
            @click="setUserMethod('sso')"
          >
            校内统一身份认证
          </button>
          <button
            type="button"
            class="method-tab"
            :class="{ active: userMethod === 'account' }"
            role="tab"
            :aria-selected="userMethod === 'account'"
            @click="setUserMethod('account')"
          >
            注册账号登录
          </button>
        </div>

        <div v-if="userPhase === 'logged-in'" class="banner success" role="status">
          登录成功，正在进入借物星球…
          <router-link :to="postLoginTarget" class="inline-link">立即跳转</router-link>
        </div>

        <div v-else-if="userPhase === 'error'" class="banner error" role="alert">
          <strong>登录失败</strong>
          <p>{{ userErrorMsg || '请检查后端服务或重试。' }}</p>
        </div>

        <!-- 方式一：校内统一身份认证（SSO） -->
        <div v-if="DEMO_MODE" v-show="userMethod === 'sso'">
          <p class="panel-lead">通过学校统一身份认证（SSO）登录，使用校园账号，无需单独注册。</p>

          <div class="row tools">
            <label class="field-inline">
              <span class="sr-only">界面语言</span>
              <select v-model="lang" class="select compact">
                <option value="zh">简体中文</option>
                <option value="en">English</option>
              </select>
            </label>
            <a href="#" @click.prevent class="muted-link">无障碍说明</a>
          </div>

          <div class="sso-actions">
            <button
              type="button"
              class="btn btn-primary sso"
              :disabled="userPhase === 'loading' || userPhase === 'logged-in' || alreadyIn"
              @click="startCampusSso()"
            >
              <span v-if="userPhase === 'loading'" class="spinner" aria-hidden="true" />
              {{ userPhase === 'loading' ? '正在跳转统一认证…' : '使用校园统一认证登录' }}
            </button>
          </div>

          <div
            v-if="userPhase === 'loading'"
            class="progress"
            role="progressbar"
            aria-valuemin="0"
            aria-valuemax="100"
            aria-busy="true"
          >
            <div class="progress-bar" />
          </div>

          <label class="remember">
            <input v-model="rememberDevice" type="checkbox" />
            记住本设备（若业务允许，可减少频繁登录）
          </label>
        </div>

        <!-- 方式二：注册账号登录（/auth/login） -->
        <div v-show="userMethod === 'account'">
          <p class="panel-lead">
            使用已注册的账号密码登录；还没有账号？
            <router-link to="/register" class="inline-link">立即注册</router-link>。
          </p>

          <label class="field-block">
            <span class="lbl">账号</span>
            <input
              v-model="userAccount"
              class="input"
              type="text"
              autocomplete="username"
              placeholder="注册时填写的用户名"
              :disabled="userPhase === 'loading' || userPhase === 'logged-in' || alreadyIn"
            />
          </label>
          <label class="field-block">
            <span class="lbl">密码</span>
            <input
              v-model="userPassword"
              class="input"
              type="password"
              autocomplete="current-password"
              placeholder="账号密码"
              :disabled="userPhase === 'loading' || userPhase === 'logged-in' || alreadyIn"
              @keydown.enter="submitUserAccount()"
            />
          </label>

          <div class="sso-actions">
            <button
              type="button"
              class="btn btn-primary sso"
              :disabled="userPhase === 'loading' || userPhase === 'logged-in' || alreadyIn"
              @click="submitUserAccount()"
            >
              <span v-if="userPhase === 'loading'" class="spinner" aria-hidden="true" />
              {{ userPhase === 'loading' ? '正在登录…' : '登录' }}
            </button>
          </div>
        </div>
      </div>

      <!-- 管理员 -->
      <div v-show="loginKind === 'admin'" class="kind-panel">
        <p class="panel-lead">
          <template v-if="DEMO_MODE">
            使用管理员账号登录（原型：密码 <code class="mono">admin</code>，或通过下方一键模拟）。
          </template>
          <template v-else>
            使用管理员账号登录（联调模式：仅支持后端真实鉴权）。
          </template>
        </p>

        <div v-if="adminPhase === 'logged-in'" class="banner success" role="status">
          管理员会话已建立，正在进入控制台…
          <router-link :to="postLoginTarget" class="inline-link">立即跳转</router-link>
        </div>

        <div v-else-if="adminPhase === 'error'" class="banner error" role="alert">
          <strong>登录失败</strong>
          <p>{{ adminErrorMsg || `账号或密码不正确；多次失败将临时锁定该终端 IP${DEMO_MODE ? '（示意）' : ''}。` }}</p>
        </div>

        <label class="field-block">
          <span class="lbl">管理员账号</span>
          <input
            v-model="adminAccount"
            class="input"
            type="text"
            autocomplete="username"
            placeholder="请输入管理员账号或工号"
            :disabled="adminPhase === 'loading' || adminPhase === 'logged-in' || alreadyIn"
          />
        </label>
        <label class="field-block">
          <span class="lbl">密码</span>
          <input
            v-model="adminPassword"
            class="input"
            type="password"
            autocomplete="current-password"
            :placeholder="DEMO_MODE ? '原型密码：admin' : '请输入管理员密码'"
            :disabled="adminPhase === 'loading' || adminPhase === 'logged-in' || alreadyIn"
            @keydown.enter="submitAdmin(false)"
          />
        </label>

        <div class="sso-actions">
          <button
            type="button"
            class="btn btn-primary sso"
            :disabled="adminPhase === 'loading' || adminPhase === 'logged-in' || alreadyIn"
            @click="submitAdmin(false)"
          >
            <span v-if="adminPhase === 'loading' && !adminMock" class="spinner" aria-hidden="true" />
            {{ adminPrimaryLabel }}
          </button>
          <button
            v-if="DEMO_MODE"
            type="button"
            class="btn btn-ghost"
            :disabled="adminPhase === 'loading' || adminPhase === 'logged-in' || alreadyIn"
            @click="submitAdmin(true)"
          >
            模拟管理员登录成功（原型）
          </button>
        </div>

        <div
          v-if="adminPhase === 'loading'"
          class="progress"
          role="progressbar"
          aria-valuemin="0"
          aria-valuemax="100"
          aria-busy="true"
        >
          <div class="progress-bar" />
        </div>

        <p class="admin-note">
          管理员登录后默认进入<strong>管理视角</strong>：物品页以审核与库存为主，个人中心为管理控制台。
        </p>
      </div>

      <p class="legal">
        登录即表示你同意
        <a href="#" @click.prevent>用户协议</a>
        与
        <a href="#" @click.prevent>隐私政策</a>
        。
      </p>

      <div class="security">
        <h3 class="sec-title">安全提示</h3>
        <ul>
          <li>请确认地址栏为 <strong>https://</strong> 开头，谨防钓鱼站点。</li>
          <li>管理员入口应仅限内网或 VPN；不要在公共电脑保存密码。</li>
        </ul>
      </div>

      <details class="faq">
        <summary>常见问题</summary>
        <ul>
          <li>没有学校邮箱？请联系院系管理员开通借用权限。</li>
          <li>管理员忘记密码？请联系信息化办公室重置。</li>
        </ul>
      </details>

      <p class="admin-hint">
        还没有账号？
        <router-link to="/register" class="inline-link">注册账号</router-link>
        · 师生需要借用权限？
        <a href="#" @click.prevent>联系管理员{{ DEMO_MODE ? '（示例：itsupport@example.edu）' : '' }}</a>
      </p>
    </section>
  </div>
</template>

<script setup>
import { computed, onMounted, ref, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { applyLogin, loginAsAdmin, loginAsUser, logout, session } from '../session'
import { authApi, ApiError } from '../api'
import { consumeAuthExpiredNotice } from '../api/client'
import { DEMO_MODE, SHOW_API_HINT } from '../config'

const router = useRouter()
const route = useRoute()

const loginKind = ref('user')
const lang = ref('zh')
const rememberDevice = ref(false)

const userPhase = ref('idle')
const userMethod = ref(DEMO_MODE ? 'sso' : 'account') // sso（校内统一身份认证） | account（注册账号登录）
const userAccount = ref('')
const userPassword = ref('')

const adminPhase = ref('idle')
const adminMock = ref(false)
const adminAccount = ref('')
const adminPassword = ref('')
const adminErrorMsg = ref('')
const userErrorMsg = ref('')
const expiredNotice = ref(false)

const alreadyIn = computed(() => session.role !== 'guest')
const roleLabel = computed(() => {
  if (session.role !== 'admin') return '师生用户'
  if (session.adminType === 'system') return '系统管理员'
  if (session.adminType === 'resource') return '资源管理员'
  return '管理员'
})
const justRegistered = computed(() => route.query.registered === '1')

const postLoginTarget = computed(() => {
  const r = route.query.redirect
  if (typeof r !== 'string' || !r.startsWith('/') || r.startsWith('//')) return '/items'
  return r
})

onMounted(() => {
  expiredNotice.value = route.query.reason === 'expired' || consumeAuthExpiredNotice()
})

function setKind(kind) {
  loginKind.value = kind
  userPhase.value = 'idle'
  adminPhase.value = 'idle'
}

watch(loginKind, () => {
  userPhase.value = 'idle'
  adminPhase.value = 'idle'
  if (!DEMO_MODE) userMethod.value = 'account'
})

const adminPrimaryLabel = computed(() => {
  if (adminPhase.value === 'loading' && !adminMock.value) return '正在校验管理员凭证…'
  if (adminPhase.value === 'loading' && adminMock.value) return '正在建立管理会话…'
  return '管理员登录'
})

function goAfterLogin() {
  setTimeout(() => router.replace(postLoginTarget.value), 700)
}

function setUserMethod(method) {
  userMethod.value = method
  userPhase.value = 'idle'
  userErrorMsg.value = ''
}

// 方式一：校内统一身份认证（SSO）。后端暂无独立 SSO 接口，原型以模拟会话登录。
function startCampusSso() {
  userErrorMsg.value = ''
  if (!DEMO_MODE) {
    userPhase.value = 'error'
    userErrorMsg.value = '当前为联调模式：SSO 后端接口未接入，请使用“注册账号登录”。'
    return
  }
  userPhase.value = 'loading'
  setTimeout(() => {
    loginAsUser('张同学')
    userPhase.value = 'logged-in'
    goAfterLogin()
  }, 800)
}

// 方式二：注册账号登录，调用真实接口 POST /auth/login
async function submitUserAccount() {
  userErrorMsg.value = ''
  if (!userAccount.value.trim() || !userPassword.value) {
    userPhase.value = 'error'
    userErrorMsg.value = '请输入账号和密码。'
    return
  }

  userPhase.value = 'loading'
  try {
    const data = await authApi.login({
      username: userAccount.value.trim(),
      password: userPassword.value,
    })
    applyLogin(data.userInfo || { username: userAccount.value.trim(), role: 'USER' })
    userPhase.value = 'logged-in'
    goAfterLogin()
  } catch (err) {
    userPhase.value = 'error'
    userErrorMsg.value = err instanceof ApiError ? err.message : '登录失败，请稍后重试。'
  }
}

async function submitAdmin(mockSuccess) {
  adminMock.value = mockSuccess
  adminErrorMsg.value = ''

  // 原型离线模拟
  if (mockSuccess) {
    adminPhase.value = 'loading'
    setTimeout(() => {
      const name = adminAccount.value.trim() || '管理员'
      loginAsAdmin(name)
      adminPhase.value = 'logged-in'
      goAfterLogin()
    }, 600)
    return
  }

  // 真实后端登录：POST /auth/login
  adminPhase.value = 'loading'
  try {
    const data = await authApi.login({
      username: adminAccount.value.trim(),
      password: adminPassword.value,
    })
    applyLogin(data.userInfo || { username: adminAccount.value.trim(), role: 'ADMIN' })
    adminPhase.value = 'logged-in'
    goAfterLogin()
  } catch (err) {
    // 仅演示模式允许离线兜底，联调模式必须依赖后端
    if (DEMO_MODE && err instanceof ApiError && err.network && adminPassword.value === 'admin') {
      const name = adminAccount.value.trim() || '管理员'
      loginAsAdmin(name)
      adminPhase.value = 'logged-in'
      goAfterLogin()
      return
    }
    adminPhase.value = 'error'
    adminErrorMsg.value = err instanceof ApiError ? err.message : '登录失败，请稍后重试。'
  }
}

function doLogout() {
  logout()
  userPhase.value = 'idle'
  adminPhase.value = 'idle'
}
</script>

<style scoped>
.login-page {
  min-height: 100vh;
  display: grid;
  grid-template-columns: minmax(0, 1.05fr) minmax(0, 0.95fr);
  gap: clamp(1.5rem, 4vw, 3rem);
  padding: clamp(1.25rem, 4vw, 3rem);
  align-items: center;
  max-width: 1120px;
  margin: 0 auto;
}

.brand {
  padding: 1rem 0.5rem;
  margin-top: -1.35rem;
}

.emblem {
  width: 72px;
  height: 72px;
  border-radius: 20px;
  background: linear-gradient(145deg, #2f6fe0, #1b3b6f);
  display: grid;
  place-items: center;
  margin-bottom: 1.25rem;
  box-shadow: var(--shadow);
  position: relative;
  overflow: hidden;
}

.planet-mark {
  isolation: isolate;
}

.planet-core {
  width: 28px;
  height: 28px;
  border-radius: 50%;
  background: radial-gradient(circle at 32% 28%, #9fd0ff 0%, #71b4ff 36%, #3f87e8 70%, #3168c0 100%);
  box-shadow: inset -4px -5px 8px rgba(21, 49, 97, 0.4);
}

.planet-ring {
  position: absolute;
  width: 48px;
  height: 16px;
  border-radius: 999px;
  border: 2px solid rgba(214, 234, 255, 0.92);
  transform: rotate(-18deg);
  filter: drop-shadow(0 0 4px rgba(179, 219, 255, 0.35));
}

.planet-dot {
  position: absolute;
  width: 5px;
  height: 5px;
  border-radius: 50%;
  background: rgba(228, 241, 255, 0.85);
}

.planet-dot.dot-a {
  top: 18px;
  right: 16px;
}

.planet-dot.dot-b {
  left: 14px;
  bottom: 18px;
}

.title {
  margin: 0 0 0.5rem;
  font-size: clamp(1.75rem, 3vw, 2.25rem);
  letter-spacing: 0.02em;
}

.tagline {
  margin: 0 0 1.25rem;
  color: var(--muted);
  line-height: 1.6;
  max-width: 32ch;
}

.bullets {
  margin: 0 0 1.5rem;
  padding-left: 1.1rem;
  color: var(--muted);
  line-height: 1.7;
}

.help-link {
  margin: 0;
  font-size: 0.9rem;
  color: var(--muted);
}

.card {
  background: var(--bg-elevated);
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  padding: clamp(1.25rem, 3vw, 2rem);
  box-shadow: var(--shadow);
}

.card-title {
  margin: 0 0 0.35rem;
  font-size: 1.35rem;
}

.card-sub {
  margin: 0 0 1rem;
  color: var(--muted);
  font-size: 0.95rem;
}

.kind-tabs {
  display: flex;
  gap: 0.35rem;
  padding: 4px;
  margin-bottom: 1rem;
  border-radius: 12px;
  border: 1px solid var(--border);
  background: rgba(0, 0, 0, 0.2);
}

.kind-tab {
  flex: 1;
  border: none;
  border-radius: 10px;
  padding: 0.55rem 0.65rem;
  font-weight: 700;
  font-size: 0.9rem;
  cursor: pointer;
  color: var(--muted);
  background: transparent;
}
.kind-tab.active {
  background: var(--surface-2);
  color: var(--text);
}

.kind-panel {
  margin-bottom: 0.25rem;
  min-height: 23rem;
}

.method-tabs {
  display: flex;
  gap: 0.5rem;
  margin-bottom: 1rem;
}

.method-tab {
  flex: 1;
  border: 1px solid var(--border);
  border-radius: 10px;
  padding: 0.5rem 0.65rem;
  font-weight: 600;
  font-size: 0.85rem;
  cursor: pointer;
  color: var(--muted);
  background: transparent;
}
.method-tab.active {
  border-color: rgba(61, 139, 253, 0.55);
  background: var(--accent-soft);
  color: var(--text);
}

.panel-lead {
  margin: 0 0 1rem;
  font-size: 0.9rem;
  color: var(--muted);
  line-height: 1.55;
}

.field-block {
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
  margin-bottom: 0.75rem;
}
.lbl {
  font-size: 0.82rem;
  font-weight: 600;
  color: var(--muted);
}

.mono {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
  font-size: 0.85em;
  padding: 0.1em 0.35em;
  border-radius: 4px;
  background: rgba(255, 255, 255, 0.08);
}

.admin-note {
  margin: 0.75rem 0 0;
  font-size: 0.82rem;
  color: var(--muted);
  line-height: 1.5;
}

.banner {
  border-radius: var(--radius-md);
  padding: 0.85rem 1rem;
  margin-bottom: 1rem;
  font-size: 0.92rem;
  line-height: 1.5;
}
.banner.success {
  background: rgba(62, 207, 142, 0.12);
  border: 1px solid rgba(62, 207, 142, 0.35);
}
.banner.error {
  background: rgba(240, 113, 120, 0.1);
  border: 1px solid rgba(240, 113, 120, 0.35);
}
.banner.error p {
  margin: 0.35rem 0 0;
  color: var(--muted);
  font-size: 0.88rem;
}
.banner.warning {
  background: rgba(245, 177, 74, 0.12);
  border: 1px solid rgba(245, 177, 74, 0.45);
  color: var(--warning);
}
.inline-link {
  margin-left: 0.35rem;
  font-weight: 600;
}

.link-btn {
  margin-left: 0.35rem;
  padding: 0;
  border: none;
  background: none;
  color: var(--accent);
  font-weight: 600;
  cursor: pointer;
  font-size: inherit;
  text-decoration: underline;
}

.row.tools {
  display: flex;
  flex-wrap: wrap;
  gap: 0.75rem;
  align-items: center;
  margin-bottom: 1rem;
}

.select.compact {
  width: auto;
  min-width: 8rem;
}

.muted-link {
  color: var(--muted);
  font-size: 0.9rem;
}

.sso-actions {
  display: flex;
  flex-direction: column;
  gap: 0.6rem;
}

.sso {
  width: 100%;
  padding: 0.85rem 1rem;
  font-size: 1rem;
}

.spinner {
  width: 1.1rem;
  height: 1.1rem;
  border: 2px solid rgba(255, 255, 255, 0.35);
  border-top-color: #fff;
  border-radius: 50%;
  animation: spin 0.75s linear infinite;
}

@keyframes spin {
  to {
    transform: rotate(360deg);
  }
}

.progress {
  margin-top: 0.75rem;
  height: 4px;
  background: rgba(255, 255, 255, 0.08);
  border-radius: 999px;
  overflow: hidden;
}
.progress-bar {
  height: 100%;
  width: 40%;
  background: linear-gradient(90deg, var(--accent), #7ec8ff);
  animation: indeterminate 1.1s ease-in-out infinite alternate;
}
@keyframes indeterminate {
  from {
    transform: translateX(-20%);
  }
  to {
    transform: translateX(180%);
  }
}

.remember {
  display: flex;
  gap: 0.5rem;
  align-items: flex-start;
  margin-top: 1rem;
  font-size: 0.88rem;
  color: var(--muted);
  cursor: pointer;
}
.remember input {
  margin-top: 0.2rem;
}

.legal {
  margin: 1rem 0 0;
  font-size: 0.82rem;
  color: var(--muted);
  line-height: 1.5;
}

.security {
  margin-top: 1.25rem;
  padding-top: 1rem;
  border-top: 1px solid var(--border);
}
.sec-title {
  margin: 0 0 0.5rem;
  font-size: 0.95rem;
}
.security ul {
  margin: 0;
  padding-left: 1.1rem;
  color: var(--muted);
  font-size: 0.88rem;
  line-height: 1.55;
}

.faq {
  margin-top: 1rem;
  color: var(--muted);
  font-size: 0.9rem;
}
.faq summary {
  cursor: pointer;
  font-weight: 600;
  color: var(--text);
}
.faq ul {
  margin: 0.5rem 0 0;
  padding-left: 1.1rem;
}

.admin-hint {
  margin: 1rem 0 0;
  font-size: 0.88rem;
  color: var(--muted);
}

@media (max-width: 880px) {
  .login-page {
    grid-template-columns: 1fr;
    align-content: start;
  }
  .brand {
    text-align: center;
    margin-top: 0;
  }
  .emblem {
    margin-left: auto;
    margin-right: auto;
  }
  .tagline {
    margin-left: auto;
    margin-right: auto;
  }
  .bullets {
    display: inline-block;
    text-align: left;
  }
  .help-link {
    text-align: center;
  }
  .kind-panel {
    min-height: auto;
  }
}
</style>
