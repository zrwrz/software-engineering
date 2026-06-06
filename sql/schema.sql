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
    
    -- 状态
    status ENUM('available', 'borrowed', 'maintenance', 'pending', 'off_shelf') DEFAULT 'off_shelf' COMMENT '物品状态：可借/已借出/挂起中/待审核/已下架',
    
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
    FULLTEXT INDEX idx_search (name, description) COMMENT '全文搜索索引'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='物品表';

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
    
    -- 时间信息
    expected_start_time DATETIME NOT NULL COMMENT '预计开始时间',
    expected_end_time DATETIME NOT NULL COMMENT '预计结束时间',
    borrowed_at DATETIME COMMENT '实际借出时间',
    returned_at DATETIME COMMENT '实际归还时间',
    
    -- 订单状态
    order_status VARCHAR(50) NOT NULL DEFAULT 'pending' COMMENT '订单状态：pending/approved/rejected/borrowed/returned/cancelled/overdue',
    
    -- 用途说明
    purpose VARCHAR(255) COMMENT '借用用途',
    
    -- 审核备注/拒绝原因
    review_remark VARCHAR(255) COMMENT '审核备注',
    
    -- 审计
    created_by BIGINT NOT NULL COMMENT '创建人ID（借用用户）',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 索引
    INDEX idx_order_no (order_no),
    INDEX idx_item_id (item_id),
    INDEX idx_user_id (user_id),
    INDEX idx_order_status (order_status),
    INDEX idx_expected_start_time (expected_start_time),
    INDEX idx_expected_end_time (expected_end_time),
    INDEX idx_created_at (created_at),
    
    -- 外键约束
    FOREIGN KEY (item_id) REFERENCES items(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (created_by) REFERENCES users(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='订单表';