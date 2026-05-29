#include <GLFW/glfw3.h>
#include <iostream>

// 窗口尺寸
const int WIDTH = 400;
const int HEIGHT = 400;

// 检测点击位置是否在"有效区域"（非透明区域）
// 这里定义一个圆形区域作为示例，你可以自定义为任意形状
bool isInValidArea(double x, double y) {
    // 将鼠标坐标转换为窗口中心坐标系（-1 到 1 范围）
    double nx = (x / WIDTH) * 2.0 - 1.0;
    double ny = 1.0 - (y / HEIGHT) * 2.0;

    // 示例：半径为 0.6 的圆形区域
    double radius = 0.6;
    return (nx * nx + ny * ny) <= radius * radius;
}

// 鼠标点击回调
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        std::cout << "Mouse clicked at: " << xpos << ", " << ypos << std::endl;

        // 如果点击在有效区域内，关闭窗口
        if (isInValidArea(xpos, ypos)) {
            std::cout << "Closing window..." << std::endl;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

// 渲染圆形形状（示例异形区域）
void renderShape() {
    // 清空为完全透明
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制一个圆形（非透明区域）
    glColor4f(1.0f, 0.5f, 0.2f, 1.0f);  // 橙色，完全不透明

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);  // 圆心
    int segments = 100;
    float radius = 0.6f;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * float(i) / float(segments);
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 配置窗口提示（必须在创建窗口前设置）
    // 启用透明帧缓冲
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    // 移除窗口装饰（边框、标题栏）
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    // 窗口置顶（可选）
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "异形透明窗口", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 设置回调
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    std::cout << "Window created. Click on the orange circle to close." << std::endl;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 渲染异形形状
        renderShape();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}