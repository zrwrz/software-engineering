-- 管理端与辅助功能接口测试数据准备脚本
USE jiewu_project;

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- 1. 清理旧测试数据
DELETE FROM audit_logs
WHERE target LIKE 'SEED_%'
   OR action LIKE 'SEED_%'
   OR detail LIKE '%SEED_%'
   OR target LIKE 'seed:%';

DELETE FROM notifications
WHERE type LIKE 'SEED_%'
   OR title LIKE 'SEED_%'
   OR content LIKE '%SEED_%'
   OR related_order_id IN (SELECT id FROM orders WHERE order_no LIKE 'SEED_%');

DELETE cr FROM compensation_records cr
LEFT JOIN orders o ON cr.order_id = o.id
WHERE cr.reason LIKE 'SEED_%'
   OR o.order_no LIKE 'SEED_%';

DELETE br FROM borrow_records br
LEFT JOIN orders o ON br.order_id = o.id
WHERE br.remark LIKE 'SEED_%'
   OR o.order_no LIKE 'SEED_%';

DELETE c FROM credit_records c
LEFT JOIN orders o ON c.order_id = o.id
WHERE c.reason LIKE 'SEED_%'
   OR o.order_no LIKE 'SEED_%';

DELETE FROM orders WHERE order_no LIKE 'SEED_%';
DELETE FROM item_submissions WHERE name LIKE 'SEED_%';
DELETE FROM items WHERE name LIKE 'SEED_%';

SET FOREIGN_KEY_CHECKS = 1;

-- 2. 准备固定登录账号
-- 默认密码均为：123456
INSERT INTO users(username, password, phone, email, role, credit_score, status)
VALUES
('test_borrower_01',
 'pbkdf2_sha256$100000$000102030405060708090a0b0c0d0e0f$3e3d2422f00f2cc1d1bad045819bfb8360117d59c588035c4294f3403ac097a5',
 '19990000001', 'test_borrower_01@example.com', 'borrower', 100, 'active'),
('resource_admin_01',
 'pbkdf2_sha256$100000$101112131415161718191a1b1c1d1e1f$d8231df7ee241c110c9860dff78484b7389e8afc3ce555c5eac5fdb1c3e6e75f',
 '19990000002', 'resource_admin_01@example.com', 'resource_admin', 100, 'active'),
('system_admin_01',
 'pbkdf2_sha256$100000$202122232425262728292a2b2c2d2e2f$b7eae8ae3f8c965f0645ce256cb8be63dddbcb7f303beaa1b94dc1520f70e7e2',
 '19990000003', 'system_admin_01@example.com', 'system_admin', 100, 'active'),
('manage_target_user_01',
 'pbkdf2_sha256$100000$303132333435363738393a3b3c3d3e3f$d5d22a49272512a828f8a9dd457029c1d473fb1d49335552becd41fd3dea3ccb',
 '19990000004', 'manage_target_user_01@example.com', 'borrower', 80, 'active')
ON DUPLICATE KEY UPDATE
    password = VALUES(password),
    phone = VALUES(phone),
    email = VALUES(email),
    role = VALUES(role),
    credit_score = VALUES(credit_score),
    status = VALUES(status);

SELECT @borrower_id := id FROM users WHERE username = 'test_borrower_01' LIMIT 1;
SELECT @manager_id := id FROM users WHERE username = 'resource_admin_01' LIMIT 1;
SELECT @system_admin_id := id FROM users WHERE username = 'system_admin_01' LIMIT 1;
SELECT @target_user_id := id FROM users WHERE username = 'manage_target_user_01' LIMIT 1;

-- 3. 准备物品数据：列表、详情、编辑、上下架、统计
INSERT INTO items(name, category, description, image_url, total_count, available_count, deposit, status, created_by)
VALUES
('SEED_ITEM_FLOW', 'seed_category', 'seed item for order flow', NULL, 20, 15, 20.00, 'ON_SHELF', @manager_id),
('SEED_ITEM_EDIT_STATUS', 'seed_category', 'seed item for edit and shelf status', NULL, 6, 6, 15.00, 'ON_SHELF', @manager_id),
('SEED_ITEM_OFF_SHELF', 'seed_category', 'seed off shelf item', NULL, 2, 2, 5.00, 'OFF_SHELF', @manager_id);

SELECT @flow_item_id := id FROM items WHERE name = 'SEED_ITEM_FLOW' ORDER BY id DESC LIMIT 1;
SELECT @editable_item_id := id FROM items WHERE name = 'SEED_ITEM_EDIT_STATUS' ORDER BY id DESC LIMIT 1;
SELECT @off_item_id := id FROM items WHERE name = 'SEED_ITEM_OFF_SHELF' ORDER BY id DESC LIMIT 1;

-- 4. 准备物品上传申请队列：待审核、已通过、已拒绝
INSERT INTO item_submissions(submitter_id, name, category, description, total_count, deposit, status, audit_remark, auditor_id, audited_at)
VALUES
(@borrower_id, 'SEED_SUBMISSION_PENDING', 'seed_category', 'seed pending item submission', 3, 12.00, 'PENDING', NULL, NULL, NULL),
(@borrower_id, 'SEED_SUBMISSION_APPROVED', 'seed_category', 'seed approved item submission', 1, 8.00, 'APPROVED', 'SEED approved remark', @manager_id, NOW()),
(@borrower_id, 'SEED_SUBMISSION_REJECTED', 'seed_category', 'seed rejected item submission', 1, 8.00, 'REJECTED', 'SEED rejected remark', @manager_id, NOW());

SELECT @pending_submission_id := id FROM item_submissions WHERE name = 'SEED_SUBMISSION_PENDING' ORDER BY id DESC LIMIT 1;

-- 5. 准备订单数据：审核、借出、归还、逾期、列表、统计
INSERT INTO orders(order_no, item_id, user_id, quantity, reserve_start_time, reserve_end_time, pickup_time, return_time,
                   status, remark, review_remark, reviewed_by, reviewed_at, borrowed_by, returned_by, return_remark,
                   created_by, created_at)
VALUES
('SEED_CREATED_AUDIT', @flow_item_id, @borrower_id, 1, DATE_ADD(NOW(), INTERVAL 1 DAY), DATE_ADD(NOW(), INTERVAL 2 DAY), NULL, NULL,
 'CREATED', 'SEED order waiting audit', NULL, NULL, NULL, NULL, NULL, NULL, @borrower_id, NOW()),
('SEED_APPROVED_BORROW', @flow_item_id, @borrower_id, 1, DATE_ADD(NOW(), INTERVAL 2 DAY), DATE_ADD(NOW(), INTERVAL 3 DAY), NULL, NULL,
 'APPROVED', 'SEED order waiting borrow', 'SEED approved', @manager_id, NOW(), NULL, NULL, NULL, @borrower_id, NOW()),
('SEED_BORROWED_RETURN', @flow_item_id, @borrower_id, 1, DATE_SUB(NOW(), INTERVAL 1 DAY), DATE_ADD(NOW(), INTERVAL 1 DAY), DATE_SUB(NOW(), INTERVAL 1 DAY), NULL,
 'BORROWED', 'SEED order waiting return', 'SEED borrowed', @manager_id, DATE_SUB(NOW(), INTERVAL 1 DAY), @manager_id, NULL, NULL, @borrower_id, NOW()),
('SEED_BORROWED_OVERDUE', @flow_item_id, @borrower_id, 1, DATE_SUB(NOW(), INTERVAL 5 DAY), DATE_SUB(NOW(), INTERVAL 1 DAY), DATE_SUB(NOW(), INTERVAL 5 DAY), NULL,
 'BORROWED', 'SEED order waiting overdue job', 'SEED borrowed overdue', @manager_id, DATE_SUB(NOW(), INTERVAL 5 DAY), @manager_id, NULL, NULL, @borrower_id, NOW()),
('SEED_RETURNED_STATS', @flow_item_id, @borrower_id, 1, DATE_SUB(NOW(), INTERVAL 8 DAY), DATE_SUB(NOW(), INTERVAL 7 DAY), DATE_SUB(NOW(), INTERVAL 8 DAY), DATE_SUB(NOW(), INTERVAL 7 DAY),
 'RETURNED', 'SEED returned order for stats', 'SEED completed', @manager_id, DATE_SUB(NOW(), INTERVAL 8 DAY), @manager_id, @manager_id, 'SEED normal return', @borrower_id, DATE_SUB(NOW(), INTERVAL 8 DAY)),
('SEED_REJECTED_LIST', @flow_item_id, @borrower_id, 1, DATE_ADD(NOW(), INTERVAL 4 DAY), DATE_ADD(NOW(), INTERVAL 5 DAY), NULL, NULL,
 'REJECTED', 'SEED rejected order', 'SEED rejected', @manager_id, NOW(), NULL, NULL, NULL, @borrower_id, NOW()),
('SEED_CANCELLED_LIST', @flow_item_id, @borrower_id, 1, DATE_ADD(NOW(), INTERVAL 6 DAY), DATE_ADD(NOW(), INTERVAL 7 DAY), NULL, NULL,
 'CANCELLED', 'SEED cancelled order', NULL, NULL, NULL, NULL, NULL, NULL, @borrower_id, NOW());

SELECT @created_order_id := id FROM orders WHERE order_no = 'SEED_CREATED_AUDIT' LIMIT 1;
SELECT @approved_order_id := id FROM orders WHERE order_no = 'SEED_APPROVED_BORROW' LIMIT 1;
SELECT @borrowed_order_id := id FROM orders WHERE order_no = 'SEED_BORROWED_RETURN' LIMIT 1;
SELECT @overdue_order_id := id FROM orders WHERE order_no = 'SEED_BORROWED_OVERDUE' LIMIT 1;
SELECT @returned_order_id := id FROM orders WHERE order_no = 'SEED_RETURNED_STATS' LIMIT 1;

-- 6. 准备借还记录
INSERT INTO borrow_records(order_id, item_id, user_id, operator_id, action, remark)
VALUES
(@borrowed_order_id, @flow_item_id, @borrower_id, @manager_id, 'BORROW', 'SEED preset borrow record'),
(@overdue_order_id, @flow_item_id, @borrower_id, @manager_id, 'BORROW', 'SEED preset overdue borrow record'),
(@returned_order_id, @flow_item_id, @borrower_id, @manager_id, 'BORROW', 'SEED preset returned borrow record'),
(@returned_order_id, @flow_item_id, @borrower_id, @manager_id, 'RETURN', 'SEED preset returned return record');

-- 7. 准备信用、赔偿、通知、审计日志数据
INSERT INTO credit_records(user_id, order_id, change_value, reason)
VALUES
(@borrower_id, @returned_order_id, 2, 'SEED on-time return credit'),
(@borrower_id, @overdue_order_id, -5, 'SEED overdue penalty'),
(@target_user_id, NULL, -10, 'SEED target user penalty');

INSERT INTO compensation_records(order_id, user_id, amount, reason, evidence_url, status, result_remark)
VALUES
(@returned_order_id, @borrower_id, 10.00, 'SEED compensation pending', NULL, 'PENDING', NULL),
(@returned_order_id, @borrower_id, 5.00, 'SEED compensation paid', NULL, 'PAID', 'SEED paid'),
(@returned_order_id, @borrower_id, 3.00, 'SEED compensation waived', NULL, 'WAIVED', 'SEED waived');

INSERT INTO notifications(user_id, type, title, content, related_order_id, is_read)
VALUES
(@borrower_id, 'SEED_AUDIT', 'SEED audit notice', 'SEED reservation approved.', @approved_order_id, 0),
(@borrower_id, 'SEED_OVERDUE', 'SEED overdue notice', 'SEED order overdue.', @overdue_order_id, 0),
(@borrower_id, 'SEED_COMPENSATION', 'SEED compensation notice', 'SEED compensation pending.', @returned_order_id, 0);

INSERT INTO audit_logs(operator_id, action, target, detail)
VALUES
(@manager_id, 'SEED_ITEM_AUDIT', 'SEED_ITEM_SUBMISSION', 'SEED preset item audit log'),
(@manager_id, 'SEED_ORDER_AUDIT', 'SEED_ORDER', 'SEED preset order audit log'),
(@system_admin_id, 'SEED_USER_STATUS_UPDATE', 'SEED_USER', 'SEED preset user status log'),
(@system_admin_id, 'SEED_CREDIT_ADJUST', 'SEED_CREDIT', 'SEED preset credit log');

-- 8. 输出核对结果。下面 seed_check 应全部为 OK。
SELECT 'seed_accounts' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) = 4, 'OK', 'FAIL') AS seed_check
FROM users
WHERE username IN ('test_borrower_01', 'resource_admin_01', 'system_admin_01', 'manage_target_user_01');

SELECT 'seed_items' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) = 3, 'OK', 'FAIL') AS seed_check
FROM items WHERE name LIKE 'SEED_ITEM_%';

SELECT 'seed_submissions' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) = 3, 'OK', 'FAIL') AS seed_check
FROM item_submissions WHERE name LIKE 'SEED_SUBMISSION_%';

SELECT 'seed_orders' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) = 7, 'OK', 'FAIL') AS seed_check
FROM orders WHERE order_no LIKE 'SEED_%';

SELECT 'seed_compensations' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) >= 3, 'OK', 'FAIL') AS seed_check
FROM compensation_records WHERE reason LIKE 'SEED_%';

SELECT 'seed_audit_logs' AS section, COUNT(*) AS count_value,
       IF(COUNT(*) >= 4, 'OK', 'FAIL') AS seed_check
FROM audit_logs WHERE action LIKE 'SEED_%';

SELECT 'login_accounts' AS section, id, username, role, status, credit_score
FROM users
WHERE username IN ('test_borrower_01', 'resource_admin_01', 'system_admin_01', 'manage_target_user_01')
ORDER BY FIELD(username, 'test_borrower_01', 'resource_admin_01', 'system_admin_01', 'manage_target_user_01');
