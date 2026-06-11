-- 用户表
-- 存储用户基本信息、认证信息和状态
CREATE TABLE IF NOT EXISTS users (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID，自增主键',
    
    -- 基本信息
    username VARCHAR(50) NOT NULL UNIQUE COMMENT '用户名，用于登录，唯一',
    password VARCHAR(255) NOT NULL COMMENT '密码哈希值',
    -- salt VARCHAR(64) COMMENT '盐值（如果不使用自包含的哈希算法如bcrypt则需要）',
    -- name VARCHAR(100) COMMENT '真实姓名（可选）',
    
    -- 联系方式
    phone VARCHAR(20) NOT NULL UNIQUE COMMENT '手机号，唯一',
    email VARCHAR(100) COMMENT '邮箱',
    
    -- 角色与权限
    role ENUM('borrower', 'resource_admin', 'system_admin') DEFAULT 'borrower' COMMENT '角色：借用用户/资源管理员/系统管理员',
    
    -- 信用与状态
    credit_score INT DEFAULT 100 COMMENT '信用积分，初始100分',
    status ENUM('active', 'banned', 'frozen') DEFAULT 'active' COMMENT '账号状态：正常/永久封禁/暂时冻结',
    
    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    last_login_at TIMESTAMP NULL COMMENT '最后登录时间',
    
    -- 索引
    INDEX idx_username (username),
    INDEX idx_phone (phone),
    INDEX idx_status (status),
    INDEX idx_role (role)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='用户表';


-- 物品表
-- 存储可借物品的基本信息和库存状态
CREATE TABLE IF NOT EXISTS items (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '物品ID，自增主键',
    
    -- 基本信息
    name VARCHAR(100) NOT NULL COMMENT '物品名称',
    category VARCHAR(50) COMMENT '分类（如：电子设备、体育用品、书籍等）',
    description TEXT COMMENT '物品描述',
    image_url VARCHAR(255) COMMENT '物品图片URL（占位符，以后添加）',
    
    -- 库存与押金
    total_count INT NOT NULL DEFAULT 1 COMMENT '物品总库存数量',
    available_count INT NOT NULL DEFAULT 1 COMMENT '物品可用库存数量',
    deposit DECIMAL(10,2) NOT NULL DEFAULT 0.00 COMMENT '押金或赔偿参考金额',
    
    -- 状态
    status ENUM('ON_SHELF', 'OFF_SHELF') DEFAULT 'ON_SHELF' COMMENT '物品状态：上架/下架',
    
    -- 管理员信息
    created_by BIGINT COMMENT '创建人ID（关联users.id）',
    
    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    INDEX idx_category (category),
    INDEX idx_status (status),
    INDEX idx_name (name),
    INDEX idx_created_by (created_by),
    FULLTEXT INDEX idx_search (name, description) COMMENT '全文搜索索引',
    
    -- 外键约束
    FOREIGN KEY (created_by) REFERENCES users(id),
    CHECK (total_count >= 0),
    CHECK (available_count >= 0),
    CHECK (available_count <= total_count)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='物品表';


-- 物品上传申请表
-- 存储借用用户提交的物品上传申请及管理员审核结果
CREATE TABLE IF NOT EXISTS item_submissions (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '物品上传申请ID，自增主键',
    
    -- 申请人信息
    submitter_id BIGINT NOT NULL COMMENT '提交人ID（关联users.id）',
    
    -- 申请物品信息
    name VARCHAR(100) NOT NULL COMMENT '申请上架的物品名称',
    category VARCHAR(50) COMMENT '物品分类',
    description TEXT COMMENT '物品描述',
    total_count INT NOT NULL DEFAULT 1 COMMENT '申请物品数量',
    deposit DECIMAL(10,2) NOT NULL DEFAULT 0.00 COMMENT '押金或赔偿参考金额',
    
    -- 审核信息
    status ENUM('PENDING', 'APPROVED', 'REJECTED') DEFAULT 'PENDING' COMMENT '申请状态：待审核/已通过/已拒绝',
    audit_remark VARCHAR(255) COMMENT '审核备注',
    auditor_id BIGINT COMMENT '审核人ID（关联users.id）',
    audited_at DATETIME COMMENT '审核时间',
    
    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    INDEX idx_submitter_id (submitter_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (submitter_id) REFERENCES users(id),
    FOREIGN KEY (auditor_id) REFERENCES users(id),
    CHECK (total_count > 0),
    CHECK (deposit >= 0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='物品上传申请表';


-- 订单表（orders）
-- 存储用户的借用订单信息和状态
CREATE TABLE IF NOT EXISTS orders (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '订单ID',
    
    -- 订单编号（业务唯一标识）
    order_no VARCHAR(32) NOT NULL UNIQUE COMMENT '订单编号',
    
    -- 关联信息
    item_id BIGINT NOT NULL COMMENT '物品ID',
    user_id BIGINT NOT NULL COMMENT '借用用户ID',
    quantity INT NOT NULL DEFAULT 1 COMMENT '借用数量',
    
    -- 时间信息
    reserve_start_time DATETIME NOT NULL COMMENT '预约开始时间',
    reserve_end_time DATETIME NOT NULL COMMENT '预约结束时间',
    pickup_time DATETIME COMMENT '实际借出时间',
    return_time DATETIME COMMENT '实际归还时间',
    
    -- 订单状态
    status ENUM('CREATED', 'APPROVED', 'REJECTED', 'BORROWED', 'RETURNED', 'CANCELLED', 'OVERDUE') NOT NULL DEFAULT 'CREATED' COMMENT '订单状态：已创建/已审核/已拒绝/已借出/已归还/已取消/已逾期',
    
    -- 用途说明
    remark VARCHAR(255) COMMENT '借用用途或用户备注',
    
    -- 审核备注/拒绝原因
    review_remark VARCHAR(255) COMMENT '审核备注',
    reviewed_by BIGINT COMMENT '审核人ID（关联users.id）',
    reviewed_at DATETIME COMMENT '审核时间',
    borrowed_by BIGINT COMMENT '借出操作人ID（关联users.id）',
    returned_by BIGINT COMMENT '归还操作人ID（关联users.id）',
    return_remark VARCHAR(255) COMMENT '归还备注',
    
    -- 审计
    created_by BIGINT NOT NULL COMMENT '创建人ID（借用用户）',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    INDEX idx_order_no (order_no),
    INDEX idx_item_id (item_id),
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_reserve_start_time (reserve_start_time),
    INDEX idx_reserve_end_time (reserve_end_time),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (item_id) REFERENCES items(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (created_by) REFERENCES users(id),
    FOREIGN KEY (reviewed_by) REFERENCES users(id),
    FOREIGN KEY (borrowed_by) REFERENCES users(id),
    FOREIGN KEY (returned_by) REFERENCES users(id),
    CHECK (quantity > 0),
    CHECK (reserve_start_time < reserve_end_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='订单表';


-- 借还记录表
-- 存储实际借出、归还、操作人、操作时间和物品核验结果
CREATE TABLE IF NOT EXISTS borrow_records (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '借还记录ID，自增主键',
    
    -- 关联信息
    order_id BIGINT NOT NULL COMMENT '订单ID（关联orders.id）',
    item_id BIGINT NOT NULL COMMENT '物品ID（关联items.id）',
    user_id BIGINT NOT NULL COMMENT '借用用户ID（关联users.id）',
    operator_id BIGINT NOT NULL COMMENT '操作人ID（关联users.id）',
    
    -- 操作信息
    action ENUM('BORROW', 'RETURN') NOT NULL COMMENT '操作类型：借出/归还',
    remark VARCHAR(255) COMMENT '核验或操作备注',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '操作时间',
    
    -- 索引
    INDEX idx_order_id (order_id),
    INDEX idx_item_id (item_id),
    INDEX idx_user_id (user_id),
    INDEX idx_operator_id (operator_id),
    INDEX idx_action (action),
    
    -- 外键约束
    FOREIGN KEY (order_id) REFERENCES orders(id),
    FOREIGN KEY (item_id) REFERENCES items(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (operator_id) REFERENCES users(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='借还记录表';


-- 信用记录表
-- 存储用户信用积分变更原因、变更数值、关联订单和变更时间
CREATE TABLE IF NOT EXISTS credit_records (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '信用记录ID，自增主键',
    
    -- 关联信息
    user_id BIGINT NOT NULL COMMENT '用户ID（关联users.id）',
    order_id BIGINT COMMENT '关联订单ID（关联orders.id，可为空）',
    
    -- 积分变化
    change_value INT NOT NULL COMMENT '积分变更值，正数为加分，负数为扣分',
    reason VARCHAR(255) NOT NULL COMMENT '积分变更原因',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    
    -- 索引
    INDEX idx_user_id (user_id),
    INDEX idx_order_id (order_id),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (order_id) REFERENCES orders(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='信用记录表';


-- 赔偿记录表
-- 存储损坏说明、赔偿凭证、责任认定、处理状态和处理结果
CREATE TABLE IF NOT EXISTS compensation_records (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '赔偿记录ID，自增主键',
    
    -- 关联信息
    order_id BIGINT NOT NULL COMMENT '订单ID（关联orders.id）',
    user_id BIGINT NOT NULL COMMENT '责任用户ID（关联users.id）',
    
    -- 赔偿信息
    amount DECIMAL(10,2) NOT NULL DEFAULT 0.00 COMMENT '赔偿金额',
    reason VARCHAR(255) NOT NULL COMMENT '赔偿原因或损坏说明',
    evidence_url VARCHAR(255) COMMENT '赔偿或损坏凭证URL',
    status ENUM('PENDING', 'PAID', 'WAIVED') DEFAULT 'PENDING' COMMENT '赔偿状态：待处理/已赔付/已免除',
    result_remark VARCHAR(255) COMMENT '处理结论备注',
    
    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    INDEX idx_order_id (order_id),
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (order_id) REFERENCES orders(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    CHECK (amount >= 0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='赔偿记录表';


-- 通知表
-- 存储审核通知、逾期提醒、归还提醒、赔偿进度通知等站内消息
CREATE TABLE IF NOT EXISTS notifications (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '通知ID，自增主键',
    
    -- 接收人信息
    user_id BIGINT NOT NULL COMMENT '接收用户ID（关联users.id）',
    
    -- 通知内容
    type VARCHAR(50) NOT NULL COMMENT '通知类型，如审核、逾期、归还、赔偿',
    title VARCHAR(100) NOT NULL COMMENT '通知标题',
    content TEXT COMMENT '通知内容',
    related_order_id BIGINT COMMENT '关联订单ID（关联orders.id，可为空）',
    is_read TINYINT(1) DEFAULT 0 COMMENT '是否已读：0未读/1已读',
    
    -- 审计字段
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    read_at DATETIME COMMENT '阅读时间',
    
    -- 索引
    INDEX idx_user_id (user_id),
    INDEX idx_type (type),
    INDEX idx_related_order_id (related_order_id),
    INDEX idx_is_read (is_read),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (related_order_id) REFERENCES orders(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='通知表';


-- 操作日志表
-- 存储关键操作记录、操作用户、操作对象、操作时间和操作结果
CREATE TABLE IF NOT EXISTS audit_logs (
    -- 主键
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '操作日志ID，自增主键',
    
    -- 操作人信息
    operator_id BIGINT NOT NULL COMMENT '操作人ID（关联users.id）',
    
    -- 操作内容
    action VARCHAR(100) NOT NULL COMMENT '操作类型，如ORDER_AUDIT、ITEM_UPDATE等',
    target VARCHAR(100) NOT NULL COMMENT '操作对象，如order:3001、item:2001',
    detail TEXT COMMENT '操作详情',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '操作时间',
    
    -- 索引
    INDEX idx_operator_id (operator_id),
    INDEX idx_action (action),
    INDEX idx_target (target),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (operator_id) REFERENCES users(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='操作日志表';
