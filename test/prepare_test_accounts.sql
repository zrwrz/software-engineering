-- 测试账号角色准备脚本
-- 使用方式：先通过 /api/v1/auth/register 注册这两个账号，再在项目数据库中执行本脚本。
-- 如果使用了其他用户名，请同步修改下面 WHERE username = '...' 的值。

UPDATE users
SET role = 'resource_admin', status = 'active'
WHERE username = 'resource_admin_01';

UPDATE users
SET role = 'system_admin', status = 'active'
WHERE username = 'system_admin_01';

SELECT id, username, role, status
FROM users
WHERE username IN ('resource_admin_01', 'system_admin_01');
