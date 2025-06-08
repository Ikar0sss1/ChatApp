cmake_minimum_required(VERSION 3.5)
project(chat LANGUAGES CXX)

# 基础设置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "e:/Qt/6.7.3/mingw_64")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt模块
find_package(Qt6 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Sql
    Network 
    REQUIRED
)

# 设置UI文件搜索路径
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

# 按目录组织源文件
set(CORE_SOURCES
    src/core/DatabaseManager.cpp
    src/core/DatabaseManager.h
    src/core/MessageHandler.cpp
    src/core/MessageHandler.h
    src/core/NetworkManager.cpp
    src/core/NetworkManager.h
    src/core/UserManager.cpp
    src/core/UserManager.h
)

set(MODELS_SOURCES
    src/models/Message.cpp
    src/models/Message.h
    src/models/User.cpp
    src/models/User.h
)

set(UI_SOURCES
    src/ui/ChatWindow.cpp
    src/ui/ChatWindow.h
    src/ui/ChatWindow.ui
    src/ui/ContactListWidget.cpp
    src/ui/ContactListWidget.h
    src/ui/LoginWindow.cpp
    src/ui/LoginWindow.h
    src/ui/LoginWindow.ui
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.ui
    src/ui/MessageBubble.cpp
    src/ui/MessageBubble.h
    src/ui/RegisterWindow.cpp
    src/ui/RegisterWindow.h
    src/ui/InputWindow.cpp
    src/ui/DeleteWindow.cpp
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/EmojiDialog.cpp
    src/ui/EmojiDialog.h
    src/ui/DragImageTextEdit.h
)

set(UTILS_SOURCES
    src/utils/Config.cpp
    src/utils/Config.h
    src/utils/Logger.cpp
    src/utils/Logger.h
)

# 资源文件
set(RESOURCE_FILES
    src/resources/resources.qrc
    # src/resources/qss/style.qss  # 注释或删除这一行
)

# 指定UTF-8编码
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 创建可执行文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${MODELS_SOURCES}
    ${UI_SOURCES}
    ${UTILS_SOURCES}
    ${RESOURCE_FILES}
)

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接Qt模块
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)