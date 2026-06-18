<template>
  <router-view />
  <button
    type="button"
    class="theme-toggle"
    :aria-label="theme === 'dark' ? '当前主题：夜间模式，点击切换' : '当前主题：日间模式，点击切换'"
    @click="handleToggleTheme"
  >
    <span class="theme-toggle__icon" aria-hidden="true">{{ theme === 'dark' ? '🌙' : '☀' }}</span>
  </button>
</template>

<script setup>
import { ref } from 'vue'
import { getCurrentTheme, toggleTheme } from './theme'

const theme = ref(getCurrentTheme())

function handleToggleTheme() {
  theme.value = toggleTheme()
}
</script>

<style scoped>
.theme-toggle {
  position: fixed;
  left: 0.9rem;
  bottom: calc(0.9rem + env(safe-area-inset-bottom));
  z-index: 70;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  border: 1px solid var(--border);
  border-radius: 9999px;
  background: var(--bg-elevated);
  color: var(--text);
  width: 2.25rem;
  height: 2.25rem;
  padding: 0;
  cursor: pointer;
  box-shadow: var(--shadow);
}

.theme-toggle:hover {
  filter: brightness(1.04);
}

.theme-toggle__icon {
  font-size: 1rem;
  line-height: 1;
}

@media (max-width: 640px) {
  .theme-toggle {
    left: 0.65rem;
    bottom: calc(0.65rem + env(safe-area-inset-bottom));
    width: 2.1rem;
    height: 2.1rem;
  }
}
</style>
