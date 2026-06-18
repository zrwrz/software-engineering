import { createApp } from 'vue'
import App from './App.vue'
import router from './router/index.js'
import './assets/global.css'
import { initTheme } from './theme.js'

initTheme()
createApp(App).use(router).mount('#app')
