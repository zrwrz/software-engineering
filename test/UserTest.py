import json
import time
import requests

BASE_URL = "http://localhost:8080/api/v1"
HEADERS = {"Content-Type": "application/json"}
SUFFIX = int(time.time())

TEST_USER = {
    "username": f"test_user_{SUFFIX}",
    "password": "secure_password_12344",
    "email": f"test_{SUFFIX}@example.com",
    "phone": f"13{str(SUFFIX)[-9:]}"
}


def pretty(data):
    return json.dumps(data, indent=2, ensure_ascii=False)


def require_ok(response, expected_status=(200, 201)):
    response.encoding = "utf-8"
    assert response.status_code in expected_status, f"HTTP {response.status_code}: {response.text}"
    body = response.json()
    assert body.get("code") == 0, pretty(body)
    return body.get("data") or {}


def test_register():
    response = requests.post(f"{BASE_URL}/auth/register", json=TEST_USER, headers=HEADERS, timeout=10)
    data = require_ok(response, (200, 201))
    assert data.get("userId")
    print("注册通过：", pretty(data))


def test_login():
    response = requests.post(
        f"{BASE_URL}/auth/login",
        json={"username": TEST_USER["username"], "password": TEST_USER["password"]},
        headers=HEADERS,
        timeout=10,
    )
    data = require_ok(response)
    assert data.get("token")
    assert data.get("userInfo", {}).get("role") == "borrower"
    print("登录通过：", data.get("userInfo"))
    return data["token"]


def auth_headers(token):
    headers = HEADERS.copy()
    headers["Authorization"] = f"Bearer {token}"
    return headers


def test_get_profile(token):
    response = requests.get(f"{BASE_URL}/users/me", headers=auth_headers(token), timeout=10)
    data = require_ok(response)
    assert data.get("username") == TEST_USER["username"]
    print("获取个人信息通过：", pretty(data))


def test_update_profile(token):
    payload = {"phone": TEST_USER["phone"], "email": f"updated_{SUFFIX}@example.com"}
    response = requests.put(f"{BASE_URL}/users/me", json=payload, headers=auth_headers(token), timeout=10)
    data = require_ok(response)
    assert data.get("email") == payload["email"]
    print("修改个人资料通过：", pretty(data))


def test_change_password(token):
    new_password = "secure_password_54321"
    response = requests.put(
        f"{BASE_URL}/users/me/password",
        json={"oldPassword": TEST_USER["password"], "newPassword": new_password},
        headers=auth_headers(token),
        timeout=10,
    )
    data = require_ok(response)
    assert data.get("success") is True
    TEST_USER["password"] = new_password
    print("修改密码通过：", pretty(data))


if __name__ == "__main__":
    try:
        test_register()
        token = test_login()
        test_get_profile(token)
        test_update_profile(token)
        test_change_password(token)
        token = test_login()
        test_get_profile(token)
        print("\n用户注册、登录、个人资料与密码接口测试通过。")
    except Exception as exc:
        print("\n测试失败：", exc)
        raise
