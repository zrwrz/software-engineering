import { createRouter, createWebHistory } from 'vue-router'
import LoginView from '../views/LoginView.vue'
import RegisterView from '../views/RegisterView.vue'
import ItemsView from '../views/ItemsView.vue'
import DashboardView from '../views/DashboardView.vue'
import ItemReviewView from '../views/ItemReviewView.vue'
import { isGuest } from '../session.js'

// 无需登录即可访问的公开路由
const PUBLIC_PATHS = ['/', '/register']

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    { path: '/', name: 'login', component: LoginView },
    { path: '/register', name: 'register', component: RegisterView },
    { path: '/items', name: 'items', component: ItemsView },
    { path: '/items/review', name: 'item-review', component: ItemReviewView },
    { path: '/me', name: 'dashboard', component: DashboardView },
  ],
  scrollBehavior() {
    return { top: 0 }
  },
})

router.beforeEach((to) => {
  if (PUBLIC_PATHS.includes(to.path)) return true
  if (isGuest()) {
    const path = to.path + (to.hash || '')
    const safeRedirect = path.startsWith('/') && !path.startsWith('//') ? path : '/items'
    return { path: '/', query: { redirect: safeRedirect } }
  }
  return true
})

export default router
