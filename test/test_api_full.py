import os
import time
from datetime import datetime, timedelta

import requests

BASE_URL = os.getenv("API_BASE_URL", "http://localhost:8080/api/v1")
HEADERS = {"Content-Type": "application/json"}
SUFFIX = int(time.time())

RESOURCE_ADMIN_USERNAME = os.getenv("RESOURCE_ADMIN_USERNAME")
RESOURCE_ADMIN_PASSWORD = os.getenv("RESOURCE_ADMIN_PASSWORD")
SYSTEM_ADMIN_USERNAME = os.getenv("SYSTEM_ADMIN_USERNAME")
SYSTEM_ADMIN_PASSWORD = os.getenv("SYSTEM_ADMIN_PASSWORD")


class ApiClient:
    def __init__(self, token=None):
        self.token = token

    @property
    def headers(self):
        headers = HEADERS.copy()
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"
        return headers

    def request(self, method, path, **kwargs):
        response = requests.request(method, f"{BASE_URL}{path}", headers=self.headers, timeout=15, **kwargs)
        response.encoding = "utf-8"
        try:
            body = response.json()
        except ValueError:
            raise AssertionError(f"{method} {path} 返回非 JSON：HTTP {response.status_code} {response.text}")
        assert response.status_code < 500, f"{method} {path} HTTP {response.status_code}: {body}"
        assert body.get("code") == 0, f"{method} {path} 业务失败：HTTP {response.status_code} {body}"
        return body.get("data") or {}


def login(username, password):
    data = ApiClient().request("POST", "/auth/login", json={"username": username, "password": password})
    return data["token"], data.get("userInfo", {})


def register_borrower():
    payload = {
        "username": f"api_borrower_{SUFFIX}",
        "password": "Passw0rd_123",
        "phone": f"18{str(SUFFIX)[-9:]}",
        "email": f"api_borrower_{SUFFIX}@example.com",
    }
    data = ApiClient().request("POST", "/auth/register", json=payload)
    token, user_info = login(payload["username"], payload["password"])
    assert user_info["role"] == "borrower"
    return ApiClient(token), data["userId"], payload


def maybe_admin_client(username, password, expected_roles):
    if not username or not password:
        return None, None
    token, user_info = login(username, password)
    assert user_info.get("role") in expected_roles, f"管理员账号角色不匹配：{user_info}"
    return ApiClient(token), user_info


def future_time(days=1, hour=9):
    value = datetime.now() + timedelta(days=days)
    return value.replace(hour=hour, minute=0, second=0, microsecond=0).strftime("%Y-%m-%d %H:%M:%S")


def run_user_tests(borrower):
    userinfo_res = borrower.request("GET", "/users/me")
    borrower_id = userinfo_res["id"]
    borrower.request("GET", "/items", params={"page": 1, "size": 5})
    borrower.request("GET", "/credits/records", params={"page": 1, "size": 5, "userId" : borrower_id})


def run_item_and_order_flow(borrower, borrower_id, manager):
    submission = borrower.request(
        "POST",
        "/items/submissions",
        json={
            "name": f"接口测试物品_{SUFFIX}",
            "category": "测试分类",
            "description": "自动化测试创建",
            "totalCount": 2,
            "deposit": 20.0,
        },
    )
    submission_id = submission["submissionId"]

    manager.request("GET", "/admin/items/submissions", params={"page": 1, "size": 10, "status": "PENDING"})
    audit_result = manager.request(
        "PATCH",
        f"/admin/items/submissions/{submission_id}/audit",
        json={"action": "APPROVE", "remark": "自动化测试通过"},
    )
    item_id = audit_result.get("itemId")
    assert item_id, audit_result

    manager.request("GET", f"/items/{item_id}")
    manager.request("PUT", f"/items/{item_id}", json={"description": "自动化测试更新", "totalCount": 2})
    manager.request("PATCH", f"/items/{item_id}/status", json={"status": "ON_SHELF"})

    reservation = borrower.request(
        "POST",
        "/orders/reservations",
        json={
            "itemId": item_id,
            "quantity": 1,
            "reserveStartTime": future_time(1, 9),
            "reserveEndTime": future_time(2, 18),
            "remark": "自动化测试预约",
        },
    )
    order_id = reservation["id"]

    borrower.request("GET", "/orders/my", params={"page": 1, "size": 10})
    borrower.request("GET", f"/orders/{order_id}")
    manager.request("GET", "/orders", params={"page": 1, "size": 10})
    manager.request("PATCH", f"/orders/{order_id}/audit", json={"action": "APPROVE", "reason": "自动化测试审核通过"})
    manager.request("PATCH", f"/orders/{order_id}/borrow")
    manager.request("PATCH", f"/orders/{order_id}/return", json={"returnRemark": "物品完好", "needCompensation": False})
    borrower.request("GET", "/credits/records", params={"page": 1, "size": 10})
    return item_id, order_id


def run_auxiliary_tests(manager, system_admin, borrower_id, order_id):
    manager.request("GET", "/compensations", params={"page": 1, "size": 10})
    compensation = manager.request(
        "POST",
        "/compensations",
        json={"orderId": order_id, "userId": borrower_id, "amount": 1.0, "reason": "自动化测试赔偿"},
    )
    manager.request("PATCH", f"/compensations/{compensation['id']}/status", json={"status": "WAIVED"})
    manager.request("GET", "/stats/dashboard")

    if system_admin:
        system_admin.request("GET", "/admin/users", params={"page": 1, "size": 10})
        system_admin.request("GET", "/admin/audit-logs", params={"page": 1, "size": 10})
        system_admin.request(
            "POST",
            "/credits/adjust",
            json={"userId": borrower_id, "changeValue": 1, "reason": "自动化测试恢复分"},
        )


def main():
    borrower, borrower_id, _ = register_borrower()
    run_user_tests(borrower)

    manager, manager_info = maybe_admin_client(
        RESOURCE_ADMIN_USERNAME,
        RESOURCE_ADMIN_PASSWORD,
        {"resource_admin", "system_admin"},
    )
    system_admin, _ = maybe_admin_client(
        SYSTEM_ADMIN_USERNAME,
        SYSTEM_ADMIN_PASSWORD,
        {"system_admin"},
    )

    if not manager:
        print("未配置 RESOURCE_ADMIN_USERNAME/PASSWORD，已跳过管理端主流程测试。")
        print("普通用户接口测试通过。")
        return

    item_id, order_id = run_item_and_order_flow(borrower, borrower_id, manager)
    run_auxiliary_tests(manager, system_admin, borrower_id, order_id)

    if manager_info and manager_info.get("role") == "system_admin" and not system_admin:
        run_auxiliary_tests(manager, manager, borrower_id, order_id)

    print("用户端、管理端与辅助功能接口测试通过。")


if __name__ == "__main__":
    main()
