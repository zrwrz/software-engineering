import os
import json
from typing import Any, Dict, List, Optional, Tuple

import requests

"""
ApiFullTest.py

用途：
1. 基于 prepare_test_data.sql 的测试数据，验证用户端、资源管理员端、
   系统管理员端以及辅助功能接口是否能正常工作。
2. 本脚本不负责初始化数据库。运行前请先执行 SQL 准备数据，并确认后端连接的数据库正确无误。
3. 默认测试账号、密码与 SQL 预置数据保持一致，也可以通过环境变量覆盖。

常用运行方式：
    python test/ApiFullTest.py

可选环境变量：
    API_BASE_URL                 接口基础地址，默认 http://localhost:8081/api/v1
    TEST_PASSWORD                默认测试密码，默认 123456
    BORROWER_USERNAME            普通借用用户账号
    BORROWER_PASSWORD            普通借用用户密码
    RESOURCE_ADMIN_USERNAME      资源管理员账号
    RESOURCE_ADMIN_PASSWORD      资源管理员密码
    SYSTEM_ADMIN_USERNAME        系统管理员账号
    SYSTEM_ADMIN_PASSWORD        系统管理员密码
    TEST_VERBOSE                 是否打印详细请求日志，默认 1；设为 0 可减少输出
    TEST_PRINT_RESPONSE_DATA     是否打印完整响应 data，默认 0；设为 1 会输出完整 data
"""

BASE_URL = os.getenv("API_BASE_URL", "http://localhost:8081/api/v1").rstrip("/")
DEFAULT_PASSWORD = os.getenv("TEST_PASSWORD", "123456")

BORROWER_USERNAME = os.getenv("BORROWER_USERNAME", "test_borrower_01")
BORROWER_PASSWORD = os.getenv("BORROWER_PASSWORD", DEFAULT_PASSWORD)
RESOURCE_ADMIN_USERNAME = os.getenv("RESOURCE_ADMIN_USERNAME", "resource_admin_01")
RESOURCE_ADMIN_PASSWORD = os.getenv("RESOURCE_ADMIN_PASSWORD", DEFAULT_PASSWORD)
SYSTEM_ADMIN_USERNAME = os.getenv("SYSTEM_ADMIN_USERNAME", "system_admin_01")
SYSTEM_ADMIN_PASSWORD = os.getenv("SYSTEM_ADMIN_PASSWORD", DEFAULT_PASSWORD)

# 默认打开详细输出
VERBOSE = os.getenv("TEST_VERBOSE", "1") != "0"
PRINT_RESPONSE_DATA = os.getenv("TEST_PRINT_RESPONSE_DATA", "0") == "1"

HEADERS = {"Content-Type": "application/json"}


# =========================
# 查询参数构造函数
# =========================

def item_query(page=1, size=10, keyword="", category="", status=""):
    return {"page": page, "size": size, "keyword": keyword, "category": category, "status": status}


def submission_query(page=1, size=10, status="PENDING"):
    return {"page": page, "size": size, "status": status}


def order_query(page=1, size=20, status=""):
    return {"page": page, "size": size, "status": status}


def credit_query(page=1, size=10, user_id=0):
    return {"userId": user_id, "page": page, "size": size}


def compensation_query(page=1, size=20, status="", user_id=0, order_id=0):
    return {"page": page, "size": size, "status": status, "userId": user_id, "orderId": order_id}


def admin_user_query(page=1, size=20, keyword="", status=""):
    return {"page": page, "size": size, "keyword": keyword, "status": status}


def audit_log_query(page=1, size=20, operator_id=0, start_time="", end_time=""):
    return {"page": page, "size": size, "operatorId": operator_id, "startTime": start_time, "endTime": end_time}


# =========================
# 输出辅助函数
# =========================


def section(title: str):
    print(f"\n{'=' * 80}\n{title}\n{'=' * 80}")


def step(title: str):
    print(f"\n--- {title}")


def ok(message: str):
    print(f"  [OK] {message}")


def info(message: str):
    print(f"  [INFO] {message}")


def compact_json(value: Any) -> str:
    """将请求参数/响应数据压缩成一行，避免测试输出过长。"""
    return json.dumps(value, ensure_ascii=False, separators=(",", ":"))


def mask_sensitive(value: Any) -> Any:
    """打印日志时隐藏密码和 token，避免终端输出敏感信息。"""
    if isinstance(value, dict):
        masked = {}
        for key, item in value.items():
            lower_key = str(key).lower()
            if "password" in lower_key:
                masked[key] = "***"
            elif "token" in lower_key:
                masked[key] = "***"
            else:
                masked[key] = mask_sensitive(item)
        return masked
    if isinstance(value, list):
        return [mask_sensitive(item) for item in value]
    return value


def summarize_data(data: Any) -> str:
    """给响应 data 生成简短摘要，既能看出结果结构，又不会刷屏。"""
    if data is None:
        return "data=null"
    if isinstance(data, list):
        return f"list[{len(data)}]"
    if isinstance(data, dict):
        if "list" in data and isinstance(data.get("list"), list):
            keys = [key for key in data.keys() if key != "list"]
            return f"list[{len(data.get('list') or [])}], other_keys={keys}"
        keys = list(data.keys())
        return f"dict_keys={keys}"
    return f"{type(data).__name__}: {data}"


def print_config():
    section("测试配置")
    print(f"BASE_URL                  = {BASE_URL}")
    print(f"BORROWER_USERNAME         = {BORROWER_USERNAME}")
    print(f"RESOURCE_ADMIN_USERNAME   = {RESOURCE_ADMIN_USERNAME}")
    print(f"SYSTEM_ADMIN_USERNAME     = {SYSTEM_ADMIN_USERNAME}")
    print(f"TEST_VERBOSE              = {int(VERBOSE)}")
    print(f"TEST_PRINT_RESPONSE_DATA  = {int(PRINT_RESPONSE_DATA)}")
    print("\n运行前请确认：")
    print("1. 后端服务已启动。")
    print("2. 已执行 prepare_management_test_data_v2.sql。")
    print("3. SQL 中 USE 的数据库名与后端 JIEWU_DB_NAME/DB_NAME 一致。")


# =========================
# API 客户端封装
# =========================


class ApiClient:
    """简单 API 客户端，用于统一发送请求、校验响应和打印日志。"""

    def __init__(self, token: Optional[str] = None, name: str = "anonymous"):
        self.token = token
        self.name = name

    @property
    def headers(self):
        headers = HEADERS.copy()
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"
        return headers

    def request_raw(self, method: str, path: str, **kwargs) -> Tuple[requests.Response, Any]:
        url = f"{BASE_URL}{path}"
        safe_params = mask_sensitive(kwargs.get("params"))
        safe_json = mask_sensitive(kwargs.get("json"))

        if VERBOSE:
            print(f"  -> [{self.name}] {method} {path}")
            if safe_params is not None:
                print(f"     params = {compact_json(safe_params)}")
            if safe_json is not None:
                print(f"     json   = {compact_json(safe_json)}")

        response = requests.request(method, url, headers=self.headers, timeout=15, **kwargs)
        response.encoding = "utf-8"

        try:
            body = response.json()
        except ValueError:
            raise AssertionError(
                f"{method} {path} 返回非 JSON：HTTP {response.status_code}\n"
                f"请求参数：{kwargs.get('params')}\n"
                f"请求体：{mask_sensitive(kwargs.get('json'))}\n"
                f"响应内容：{response.text}"
            )

        if VERBOSE:
            if isinstance(body, dict):
                print(f"  <- HTTP {response.status_code}, code={body.get('code')}, message={body.get('message')}")
                print(f"     data_summary = {summarize_data(body.get('data'))}")
                if PRINT_RESPONSE_DATA:
                    print(f"     data = {json.dumps(mask_sensitive(body.get('data')), ensure_ascii=False, indent=2)}")
            else:
                print(f"  <- HTTP {response.status_code}, body_type={type(body).__name__}")

        return response, body

    def request(self, method: str, path: str, **kwargs):
        response, body = self.request_raw(method, path, **kwargs)

        assert response.status_code < 500, (
            f"{method} {path} HTTP {response.status_code}: "
            f"{json.dumps(body, ensure_ascii=False, indent=2)}"
        )
        assert isinstance(body, dict), f"{method} {path} 响应不是对象：{body}"
        assert body.get("code") == 0, (
            f"{method} {path} 业务失败：HTTP {response.status_code}\n"
            f"请求参数：{kwargs.get('params')}\n"
            f"请求体：{mask_sensitive(kwargs.get('json'))}\n"
            f"响应：{json.dumps(body, ensure_ascii=False, indent=2)}"
        )

        return body.get("data") or {}


# =========================
# 通用查询与断言函数
# =========================


def login(username: str, password: str, expected_roles: Optional[set] = None):
    """登录并校验账号角色。角色不对时，后续管理端测试一定会失败，所以这里提前断言。"""
    step(f"登录账号：{username}")
    data = ApiClient(name="login").request("POST", "/auth/login", json={"username": username, "password": password})
    user_info = data.get("userInfo", {})

    if expected_roles:
        assert user_info.get("role") in expected_roles, f"账号角色不匹配：{user_info}，期望：{expected_roles}"

    ok(f"登录成功：userId={user_info.get('id')}, username={user_info.get('username')}, role={user_info.get('role')}")
    return ApiClient(data["token"], username), user_info


def require_list(data: Dict[str, Any], label: str) -> List[Dict[str, Any]]:
    """确认分页接口返回的 list 字段存在且为数组。"""
    items = data.get("list") or []
    assert isinstance(items, list), f"{label} list 字段不是数组：{data}"
    info(f"{label}返回 {len(items)} 条记录")
    return items


def assert_seed_visible(manager: ApiClient):
    """确认 SQL 预置的物品数据能被当前后端查到，用于排除“连错数据库”的问题。"""
    step("检查 SQL 预置物品是否可见")
    data = manager.request("GET", "/items", params=item_query(page=1, size=50, keyword="SEED_ITEM"))
    items = require_list(data, "SEED_ITEM 物品列表")

    assert len(items) >= 3, (
        "未看到 SQL 预置的 SEED_ITEM 数据。请确认：\n"
        "1) 已执行 prepare_management_test_data_v2.sql；\n"
        "2) SQL 中 USE 的数据库名与后端 JIEWU_DB_NAME/DB_NAME 一致；\n"
        "3) 后端已重启或连接到了同一个 MySQL 实例。\n"
        f"当前接口响应：{json.dumps(data, ensure_ascii=False, indent=2)}"
    )
    ok("SQL 预置物品数据可见，数据库连接和基础数据基本正常")


def find_item_by_name(client: ApiClient, name: str) -> Dict[str, Any]:
    step(f"查找物品：{name}")
    data = client.request("GET", "/items", params=item_query(page=1, size=50, keyword=name))
    for item in require_list(data, "物品列表"):
        if item.get("name") == name:
            ok(f"找到物品：id={item.get('id')}, name={item.get('name')}, status={item.get('status')}")
            return item
    raise AssertionError(f"未找到物品：{name}\n响应：{json.dumps(data, ensure_ascii=False, indent=2)}")


def find_submission_by_name(client: ApiClient, name: str, status: str = "PENDING") -> Dict[str, Any]:
    step(f"查找物品申请：{name}, status={status}")
    data = client.request("GET", "/admin/items/submissions", params=submission_query(page=1, size=50, status=status))
    for item in require_list(data, "申请列表"):
        if item.get("name") == name:
            ok(f"找到物品申请：id={item.get('id')}, name={item.get('name')}, status={item.get('status')}")
            return item
    raise AssertionError(f"未找到物品申请：{name} status={status}\n响应：{json.dumps(data, ensure_ascii=False, indent=2)}")


def find_order_by_no(client: ApiClient, order_no: str, status: str = "") -> Dict[str, Any]:
    step(f"查找订单：orderNo={order_no}, status={status or '任意'}")
    data = client.request("GET", "/orders", params=order_query(page=1, size=50, status=status))
    for item in require_list(data, "订单列表"):
        if item.get("orderNo") == order_no:
            ok(f"找到订单：id={item.get('id')}, orderNo={item.get('orderNo')}, status={item.get('status')}")
            return item
    raise AssertionError(f"未找到订单：{order_no} status={status}\n响应：{json.dumps(data, ensure_ascii=False, indent=2)}")


def find_user_by_username(system_admin: ApiClient, username: str) -> Dict[str, Any]:
    step(f"查找用户：{username}")
    data = system_admin.request("GET", "/admin/users", params=admin_user_query(page=1, size=50, keyword=username))
    for item in require_list(data, "用户列表"):
        if item.get("username") == username:
            ok(f"找到用户：id={item.get('id')}, username={item.get('username')}, role={item.get('role')}, status={item.get('status')}")
            return item
    raise AssertionError(f"未找到用户：{username}\n响应：{json.dumps(data, ensure_ascii=False, indent=2)}")


# =========================
# 测试分组
# =========================


def run_user_tests(borrower: ApiClient):
    section("[1/4] 普通用户基础接口")

    step("获取当前登录用户信息")
    borrower.request("GET", "/users/me")
    ok("普通用户个人信息接口通过")

    step("普通用户查看物品列表")
    borrower.request("GET", "/items", params=item_query(page=1, size=5))
    ok("普通用户物品列表接口通过")

    step("普通用户查看个人信用记录")
    borrower.request("GET", "/credits/records", params=credit_query(page=1, size=5))
    ok("普通用户信用记录接口通过")


def run_resource_manager_tests(manager: ApiClient):
    section("[2/4] 资源管理员：物品、申请、订单主流程")
    assert_seed_visible(manager)

    step("测试物品详情、编辑与上下架")
    editable = find_item_by_name(manager, "SEED_ITEM_EDIT_STATUS")
    editable_id = editable["id"]
    manager.request("GET", f"/items/{editable_id}")
    ok(f"物品详情接口通过：itemId={editable_id}")

    manager.request("PUT", f"/items/{editable_id}", json={"description": "SEED updated by api", "totalCount": 6, "deposit": 16.0})
    ok("物品编辑接口通过")

    manager.request("PATCH", f"/items/{editable_id}/status", json={"status": "OFF_SHELF"})
    ok("物品下架接口通过")

    manager.request("PATCH", f"/items/{editable_id}/status", json={"status": "ON_SHELF"})
    ok("物品上架接口通过")

    step("测试物品申请审核通过后生成正式物品")
    pending = find_submission_by_name(manager, "SEED_SUBMISSION_PENDING", "PENDING")
    audit_result = manager.request(
        "PATCH",
        f"/admin/items/submissions/{pending['id']}/audit",
        json={"action": "APPROVE", "remark": "SEED approved by api"},
    )
    ok(f"物品申请审核接口通过：submissionId={pending['id']}")

    generated = find_item_by_name(manager, "SEED_SUBMISSION_PENDING")
    assert generated.get("id"), f"审核通过后没有生成可查询物品。审核响应：{audit_result}"
    ok(f"审核通过后正式物品可查询：itemId={generated.get('id')}")

    step("测试订单审核、借出、归还、逾期任务")
    created = find_order_by_no(manager, "SEED_CREATED_AUDIT", "CREATED")
    approved = find_order_by_no(manager, "SEED_APPROVED_BORROW", "APPROVED")
    borrowed = find_order_by_no(manager, "SEED_BORROWED_RETURN", "BORROWED")

    manager.request("GET", f"/orders/{created['id']}")
    ok("订单详情接口通过")

    manager.request("PATCH", f"/orders/{created['id']}/audit", json={"action": "APPROVE", "reason": "SEED approved by api"})
    ok("订单审核通过接口通过")

    manager.request("PATCH", f"/orders/{approved['id']}/borrow")
    ok("办理借出接口通过")

    manager.request("PATCH", f"/orders/{borrowed['id']}/return", json={"returnRemark": "SEED returned by api", "needCompensation": False})
    ok("办理归还接口通过")

    manager.request("POST", "/internal/orders/mark-overdue")
    ok("逾期标记任务接口通过")

    return borrowed["id"]


def run_auxiliary_tests(manager: ApiClient, system_admin: ApiClient, borrower_info: Dict[str, Any], returned_order_id: int):
    section("[3/4] 赔偿、信用、统计、审计辅助功能")
    borrower_id = borrower_info["id"]

    step("测试赔偿记录列表、创建和状态更新")
    manager.request("GET", "/compensations", params=compensation_query(page=1, size=20))
    ok("赔偿记录列表接口通过")

    created = manager.request(
        "POST",
        "/compensations",
        json={"orderId": returned_order_id, "userId": borrower_id, "amount": 1.0, "reason": "SEED compensation created by api"},
    )
    compensation_id = created.get("id") or created.get("compensationId")
    assert compensation_id, f"赔偿创建响应缺少 id：{created}"
    ok(f"赔偿记录创建接口通过：compensationId={compensation_id}")

    manager.request("GET", "/compensations", params=compensation_query(page=1, size=20, order_id=returned_order_id))
    ok("按订单查询赔偿记录接口通过")

    manager.request("PATCH", f"/compensations/{compensation_id}/status", json={"status": "WAIVED"})
    ok("赔偿状态更新接口通过")

    step("测试统计看板和信用记录")
    manager.request("GET", "/stats/dashboard")
    ok("统计看板接口通过")

    manager.request("GET", "/credits/records", params=credit_query(page=1, size=20, user_id=borrower_id))
    ok("信用记录列表接口通过")

    step("测试系统管理员人工调分和审计日志")
    system_admin.request("POST", "/credits/adjust", json={"userId": borrower_id, "changeValue": 1, "reason": "SEED credit adjusted by system admin"})
    ok("系统管理员人工调分接口通过")

    system_admin.request("GET", "/admin/audit-logs", params=audit_log_query(page=1, size=20))
    ok("审计日志查询接口通过")


def run_system_admin_tests(system_admin: ApiClient):
    section("[4/4] 系统管理员：用户管理")

    step("测试用户列表")
    system_admin.request("GET", "/admin/users", params=admin_user_query(page=1, size=20))
    ok("用户列表接口通过")

    step("测试冻结和恢复指定用户")
    target = find_user_by_username(system_admin, "manage_target_user_01")
    system_admin.request("PATCH", f"/admin/users/{target['id']}/status", json={"status": "frozen"})
    ok(f"冻结用户接口通过：userId={target['id']}")

    system_admin.request("PATCH", f"/admin/users/{target['id']}/status", json={"status": "active"})
    ok(f"恢复用户接口通过：userId={target['id']}")


def main():
    print_config()

    section("账号登录与角色校验")
    borrower, borrower_info = login(BORROWER_USERNAME, BORROWER_PASSWORD, {"borrower"})
    manager, manager_info = login(RESOURCE_ADMIN_USERNAME, RESOURCE_ADMIN_PASSWORD, {"resource_admin", "system_admin"})
    system_admin, system_admin_info = login(SYSTEM_ADMIN_USERNAME, SYSTEM_ADMIN_PASSWORD, {"system_admin"})

    info(f"普通用户：{borrower_info}")
    info(f"资源管理员：{manager_info}")
    info(f"系统管理员：{system_admin_info}")

    run_user_tests(borrower)
    returned_order_id = run_resource_manager_tests(manager)
    run_auxiliary_tests(manager, system_admin, borrower_info, returned_order_id)
    run_system_admin_tests(system_admin)

    section("测试结论")
    print("SQL 预置数据下，用户端、管理端与辅助功能接口测试通过。")


if __name__ == "__main__":
    main()
