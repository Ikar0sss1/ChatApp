# Simple IMChat

一个基于Qt开发的即时通讯应用程序。

## 功能特点

- 用户注册和登录
- 实时消息发送和接收
- 支持文本、图片和文件传输
- 联系人管理
- 深色/浅色主题切换
- 自定义字体大小
- 表情包支持

## 技术栈

- Qt 6.7.3
- C++11
- SQLite数据库
- TCP网络通信

## 构建要求

- CMake 3.5+
- Qt 6.7.3
- C++11兼容的编译器

## 构建步骤

1. 克隆仓库：
```bash
git clone https://github.com/你的用户名/ChatApp.git
cd ChatApp
```

2. 创建构建目录：
```bash
mkdir build
cd build
```

3. 配置项目：
```bash
cmake ..
```

4. 构建项目：
```bash
cmake --build .
```

## 使用说明

1. 运行程序后，首先需要注册账号
2. 登录后可以添加联系人
3. 点击联系人开始聊天
4. 支持发送文本、图片和文件

## 许可证

MIT License 