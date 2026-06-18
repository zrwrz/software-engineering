<template>
  <div class="register-page">
    <section class="card" aria-labelledby="register-heading">
      <header class="head">
        <div class="emblem planet-mark" aria-hidden="true">
          <span class="planet-core" />
          <span class="planet-ring" />
        </div>
        <div>
          <h1 id="register-heading" class="title">注册账号</h1>
          <p class="sub">创建平台账号，注册成功后即可登录借物星球。</p>
          <p v-if="SHOW_API_HINT" class="sub">接口：`POST /auth/register`</p>
        </div>
      </header>

      <div v-if="phase === 'done'" class="banner success" role="status">
        <strong>注册成功！</strong>
        <p>账号 <code class="mono">{{ form.username }}</code> 已创建。</p>
        <router-link to="/" class="inline-link">前往登录</router-link>
      </div>

      <form v-else class="form" @submit.prevent="submit">
        <div v-if="errorMsg" class="banner error" role="alert">{{ errorMsg }}</div>

        <label class="field">
          <span class="lbl">用户名 <em>*</em></span>
          <input
            v-model.trim="form.username"
            class="input"
            type="text"
            autocomplete="username"
            placeholder="3–20 位字母/数字"
            :disabled="phase === 'loading'"
          />
        </label>

        <label class="field">
          <span class="lbl">密码 <em>*</em></span>
          <input
            v-model="form.password"
            class="input"
            type="password"
            autocomplete="new-password"
            placeholder="至少 6 位"
            :disabled="phase === 'loading'"
          />
        </label>

        <label class="field">
          <span class="lbl">确认密码 <em>*</em></span>
          <input
            v-model="form.confirm"
            class="input"
            type="password"
            autocomplete="new-password"
            placeholder="再次输入密码"
            :disabled="phase === 'loading'"
          />
        </label>

        <label class="field">
          <span class="lbl">手机号 <em>*</em></span>
          <input
            v-model.trim="form.phone"
            class="input"
            type="tel"
            autocomplete="tel"
            placeholder="必填，11 位手机号"
            :disabled="phase === 'loading'"
          />
        </label>

        <label class="field">
          <span class="lbl">邮箱</span>
          <input
            v-model.trim="form.email"
            class="input"
            type="email"
            autocomplete="email"
            placeholder="选填，例如 you@example.com"
            :disabled="phase === 'loading'"
          />
        </label>

        <button type="submit" class="btn btn-primary submit" :disabled="phase === 'loading'">
          <span v-if="phase === 'loading'" class="spinner" aria-hidden="true" />
          {{ phase === 'loading' ? '正在注册…' : '注册' }}
        </button>

        <p class="foot">
          已有账号？
          <router-link to="/" class="inline-link">返回登录</router-link>
        </p>
      </form>
    </section>
  </div>
</template>

<script setup>
import { reactive, ref } from 'vue'
import { useRouter } from 'vue-router'
import { authApi, ApiError } from '../api'
import { SHOW_API_HINT } from '../config'

const router = useRouter()

const phase = ref('idle') // idle | loading | done
const errorMsg = ref('')

const form = reactive({
  username: '',
  password: '',
  confirm: '',
  phone: '',
  email: '',
})

function validate() {
  if (!form.username || form.username.length < 3) return '请输入至少 3 位用户名。'
  if (!form.password || form.password.length < 6) return '密码至少 6 位。'
  if (form.password !== form.confirm) return '两次输入的密码不一致。'
  if (form.phone && !/^1\d{10}$/.test(form.phone)) return '手机号格式不正确（应为 11 位）。'
  if (form.email && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(form.email)) return '邮箱格式不正确。'
  return ''
}

async function submit() {
  errorMsg.value = ''
  const err = validate()
  if (err) {
    errorMsg.value = err
    return
  }

  phase.value = 'loading'
  try {
    await authApi.register({
      username: form.username,
      password: form.password,
      phone: form.phone || undefined,
      email: form.email || undefined,
    })
    phase.value = 'done'
    setTimeout(() => router.replace({ path: '/', query: { registered: '1' } }), 1200)
  } catch (e) {
    phase.value = 'idle'
    errorMsg.value = e instanceof ApiError ? e.message : '注册失败，请稍后重试。'
  }
}
</script>

<style scoped>
.register-page {
  min-height: 100vh;
  display: grid;
  place-items: center;
  padding: clamp(1.25rem, 4vw, 3rem);
}

.card {
  width: min(440px, 100%);
  background: var(--bg-elevated);
  border: 1px solid var(--border);
  border-radius: var(--radius-lg);
  padding: clamp(1.25rem, 3vw, 2rem);
  box-shadow: var(--shadow);
}

.head {
  display: flex;
  align-items: center;
  gap: 0.85rem;
  margin-bottom: 1.25rem;
}

.emblem {
  width: 52px;
  height: 52px;
  border-radius: 16px;
  background: linear-gradient(145deg, #2f6fe0, #1b3b6f);
  display: grid;
  place-items: center;
  position: relative;
  overflow: hidden;
  flex: 0 0 auto;
}
.planet-core {
  width: 22px;
  height: 22px;
  border-radius: 50%;
  background: radial-gradient(circle at 32% 28%, #9fd0ff 0%, #71b4ff 36%, #3f87e8 70%, #3168c0 100%);
}
.planet-ring {
  position: absolute;
  width: 38px;
  height: 13px;
  border-radius: 999px;
  border: 2px solid rgba(214, 234, 255, 0.92);
  transform: rotate(-18deg);
}

.title {
  margin: 0 0 0.25rem;
  font-size: 1.4rem;
}
.sub {
  margin: 0;
  color: var(--muted);
  font-size: 0.9rem;
  line-height: 1.5;
}

.form {
  display: flex;
  flex-direction: column;
  gap: 0.8rem;
}

.field {
  display: flex;
  flex-direction: column;
  gap: 0.35rem;
}
.lbl {
  font-size: 0.82rem;
  font-weight: 600;
  color: var(--muted);
}
.lbl em {
  color: var(--danger);
  font-style: normal;
}

.submit {
  width: 100%;
  padding: 0.8rem 1rem;
  font-size: 1rem;
  margin-top: 0.35rem;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: 0.5rem;
}

.spinner {
  width: 1.05rem;
  height: 1.05rem;
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

.banner {
  border-radius: var(--radius-md);
  padding: 0.85rem 1rem;
  font-size: 0.92rem;
  line-height: 1.5;
}
.banner.success {
  background: rgba(62, 207, 142, 0.12);
  border: 1px solid rgba(62, 207, 142, 0.35);
}
.banner.success p {
  margin: 0.35rem 0;
}
.banner.error {
  background: rgba(240, 113, 120, 0.1);
  border: 1px solid rgba(240, 113, 120, 0.35);
}

.inline-link {
  font-weight: 600;
}

.foot {
  margin: 0.35rem 0 0;
  font-size: 0.88rem;
  color: var(--muted);
  text-align: center;
}

.mono {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
  font-size: 0.85em;
  padding: 0.1em 0.35em;
  border-radius: 4px;
  background: rgba(255, 255, 255, 0.08);
}
</style>
