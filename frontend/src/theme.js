const THEME_KEY = 'bp-theme'

function canUseDom() {
  return typeof window !== 'undefined' && typeof document !== 'undefined'
}

function resolveInitialTheme() {
  if (!canUseDom()) return 'dark'
  const stored = localStorage.getItem(THEME_KEY)
  if (stored === 'light' || stored === 'dark') return stored
  return 'dark'
}

export function applyTheme(theme) {
  if (!canUseDom()) return 'dark'
  const normalized = theme === 'light' ? 'light' : 'dark'
  document.documentElement.setAttribute('data-theme', normalized)
  localStorage.setItem(THEME_KEY, normalized)
  return normalized
}

export function initTheme() {
  return applyTheme(resolveInitialTheme())
}

export function getCurrentTheme() {
  if (!canUseDom()) return 'dark'
  const current = document.documentElement.getAttribute('data-theme')
  return current === 'light' ? 'light' : 'dark'
}

export function toggleTheme() {
  return applyTheme(getCurrentTheme() === 'dark' ? 'light' : 'dark')
}
