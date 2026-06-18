import requests
import json

# ================= 配置区 =================
BASE_URL = "http://localhost:8081/api/v1"  # 对应 C++ 监听的端口 + 接口规范的版本前缀
HEADERS = {
    "Content-Type": "application/json"
}

# 模拟测试数据
TEST_USER = {
    "username": "test_user_02",
    "password": "secure_password_12344",
    "email": "test1@example.com",
    "phone": "10010001001"
}

# ==========================================

def test_register():
    """测试用户注册"""
    url = f"{BASE_URL}/auth/register"
    print(f"🚀 正在测试注册: {url}")
    
    try:
        response = requests.post(url, json=TEST_USER, headers=HEADERS)
        response.encoding = 'utf-8' 
        print(f"状态码: {response.status_code}")
        
        if response.status_code in [200, 201]:
            result = response.json()
            print(f"注册成功! 数据: {json.dumps(result, indent=2, ensure_ascii=False)}")
            return True
        else:
            try:
                error_msg = json.dumps(response.json(), indent=2, ensure_ascii=False)
            except ValueError:
                error_msg = response.text
            print(f"注册失败: {error_msg}")
            return False
            
    except Exception as e:
        print(f"请求异常 (请检查后端是否启动): {e}")
        return False


def test_login():
    """测试用户登录"""
    url = f"{BASE_URL}/auth/login"
    print(f"\n🚀 正在测试登录: {url}")
    
    login_data = {
        "username": TEST_USER["username"],
        "password": TEST_USER["password"]
    }
    
    try:
        response = requests.post(url, json=login_data, headers=HEADERS)
        response.encoding = 'utf-8' 
        
        if response.status_code == 200:
            result = response.json()
            token = result.get("data", {}).get("token")
            print(f"登录成功! Token: {token}")
            return token
        else:
            try:
                error_msg = json.dumps(response.json(), indent=2, ensure_ascii=False)
            except ValueError:
                error_msg = response.text
            print(f"登录失败: {error_msg}")
            return None
            
    except Exception as e:
        print(f"请求异常: {e}")
        return None


def test_get_profile(jwt_token):
    """测试查询当前用户信息（需要携带Token）"""
    # ⚠️ 注意：这里的 URL 路径请根据你 C++ 后端的实际路由进行调整！
    # 常见的路径有：/user/profile、/user/info、/auth/me 等
    url = f"{BASE_URL}/users/me" 
    
    print(f"\n🚀 正在测试查询个人信息: {url}")
    
    # ✅ 核心：将 Token 放入请求头中（通常格式为 Bearer <token>）
    auth_headers = HEADERS.copy()
    auth_headers["Authorization"] = f"Bearer {jwt_token}"
    
    try:
        response = requests.get(url, headers=auth_headers)
        response.encoding = 'utf-8'
        print(f"状态码: {response.status_code}")
        
        if response.status_code == 200:
            result = response.json()
            print(f"获取个人信息成功! 数据: {json.dumps(result, indent=2, ensure_ascii=False)}")
            return True
        else:
            try:
                error_msg = json.dumps(response.json(), indent=2, ensure_ascii=False)
            except ValueError:
                error_msg = response.text
            print(f"获取个人信息失败: {error_msg}")
            return False
            
    except Exception as e:
        print(f"请求异常: {e}")
        return False


if __name__ == "__main__":
    # 1. 执行注册流程
    if test_register():
        # 2. 执行登录流程，获取 Token
        jwt_token = test_login()
        
        if jwt_token:
            # 3. ✅ 新增：携带 Token 执行查询个人信息流程
            if test_get_profile(jwt_token):
                print("\n🎉 所有核心链路（注册 -> 登录 -> 查信息）测试通过！可以开始业务开发了。")
            else:
                print("\n❌ 查询个人信息未通过，请检查 /user/profile 路由或 JWT 解析逻辑。")
        else:
            print("\n❌ 登录测试未通过，无法获取 Token，后续测试终止。")
    else:
        print("\n❌ 注册测试未通过，请检查后端服务状态或数据库连接。")