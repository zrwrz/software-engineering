## 编译运行说明（仅供参考！不同设备不同环境预指令也不同，请勿照搬！）

### 编译阶段
cmake .. -A x64 ^
  -Doatpp_DIR=E:/A.SE/Project/oatpp-install/lib/cmake/oatpp-1.3.0 ^
  -DOATPP_INCLUDE_DIR=E:/A.SE/Project/oatpp-install/include/oatpp-1.3.0/oatpp ^
  -DOPENSSL_ROOT_DIR=E:/A.SE/Project/OpenSSL ^
  -DOPENSSL_INCLUDE_DIR=E:/A.SE/Project/OpenSSL/include ^
  -DOPENSSL_SSL_LIBRARY=E:/A.SE/Project/OpenSSL/lib/x64/release/libssl.lib ^
  -DOPENSSL_CRYPTO_LIBRARY=E:/A.SE/Project/OpenSSL/lib/x64/release/libcrypto.lib ^
  -DMYSQL_INCLUDE_DIR="C:/Program Files/MySQL/MySQL Connector C++ 9.7/include/jdbc" ^
  -DMYSQL_LIB="C:/Program Files/MySQL/MySQL Connector C++ 9.7/lib64/vs14/mysqlcppconn.lib"

cmake --build . --config Release

### 环境配置阶段
cd /d E:\A.SE\Project\software-engineering\build\Release

set "JIEWU_DB_HOST=tcp://127.0.0.1:3306"
set "JIEWU_DB_USER=root"
set "JIEWU_DB_PASSWORD=******"
set "JIEWU_DB_NAME=jiewu_project"

set "DB_HOST=tcp://127.0.0.1:3306"
set "DB_USER=root"
set "DB_PASSWORD=******"
set "DB_NAME=jiewu_project"

set "JIEWU_SERVER_PORT=8081"

### 后端启动与测试
jiewu_server.exe

python test/UserTest.py

### 进入项目数据库
"C:\Program Files\MySQL\MySQL Server 9.2\bin\mysql.exe" -h 127.0.0.1 -P 3306 -u root -p jiewu_project

### 注册资源管理员账号（Power Shell）
Invoke-RestMethod `
  -Uri "http://localhost:8081/api/v1/auth/register" `
  -Method POST `
  -ContentType "application/json" `
  -Body '{"username":"resource_admin_01","password":"123456","phone":"18800000001","email":"resource_admin_01@example.com"}'

### 注册系统管理员账号（Power Shell）
Invoke-RestMethod `
  -Uri "http://localhost:8081/api/v1/auth/register" `
  -Method POST `
  -ContentType "application/json" `
  -Body '{"username":"system_admin_01","password":"123456","phone":"18800000002","email":"system_admin_01@example.com"}'

### 管理端配置与测试
set RESOURCE_ADMIN_USERNAME=resource_admin_01
set RESOURCE_ADMIN_PASSWORD=123456
set SYSTEM_ADMIN_USERNAME=system_admin_01
set SYSTEM_ADMIN_PASSWORD=123456
python test/ApiFullTest.py