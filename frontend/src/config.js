// 统一环境开关：
// - true: 允许原型演示兜底（本地 mock 登录/数据）
// - false: 严格联调模式，必须依赖后端接口
export const DEMO_MODE = import.meta.env.VITE_DEMO_MODE === 'true'

// 是否在界面中显示“接口提示文案”
// - true: 渲染每个模块对应的接口清单
// - false: 隐藏所有接口提示
export const SHOW_API_HINT = import.meta.env.VITE_SHOW_API_HINT === 'true'

