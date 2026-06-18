#!/usr/bin/env python3
"""
注册测试账号脚本
"""

import requests
import os

BASE_URL = os.getenv("API_BASE_URL", "http://localhost:8081/api/v1")

TEST_ACCOUNTS = [
    {
        "username": "resource_admin_01",
        "password": "123456",
        "phone": "18800000001",
        "email": "resource_admin_01@example.com"
    },
    {
        "username": "system_admin_01",
        "password": "123456",
        "phone": "18800000002",
        "email": "system_admin_01@example.com"
    }
]

def register_user(account):
    url = f"{BASE_URL}/auth/register"
    try:
        resp = requests.post(url, json=account, timeout=10)
        print(f"{account['username']}: HTTP {resp.status_code}")
        if resp.status_code == 201:
            print(f"  ✅ 注册成功")
        else:
            print(f"  ❌ {resp.text}")
    except Exception as e:
        print(f"  ❌ 异常: {e}")

def main():
    print("注册测试账号...")
    for acc in TEST_ACCOUNTS:
        register_user(acc)
    print("完成")

if __name__ == "__main__":
    main()