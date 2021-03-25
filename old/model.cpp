#include <glad/glad.h>

#include <iostream>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <camera.h>
#include <shader.h>
#include <model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(const char *path);
glm::mat4 quatRotation(glm::mat4 model, glm::vec3 axis, float angle);

const GLuint WIDTH = 1200, HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2, lastY = HEIGHT / 2;
bool firstMouse = true;

float deltaTime = 0.0f;  // Time between current frame and last frame
float lastFrame = 0.0f;  // Time of last frame

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window;
    window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Practice", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // hide and capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // set mouse event callback
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize OpenGL context");
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // enable depth test
    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("shaders/multilight.vs", "shaders/multilight_alpha.fs");

    Model tree("res/tree/Tree.obj");
    Model ground("res/ground/ground.obj");
    Model chair("res/chair/chair.obj");

    glm::vec3 treePositions[] {
        glm::vec3(3.0f, 0.0f, 3.0f),
        glm::vec3(-4.0f, 0.0f, -3.0f),
        glm::vec3(4.0f, 0.0f, -4.0f)
    };

    glm::vec3 chairPositions[] {
        glm::vec3(3.5f, 0.0f, 0.0f),
        glm::vec3(-3.5f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -3.5f)
    };

    std::cout << glGetString(GL_VERSION);

    // change to wireframe draws
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        // frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // check for input
        processInput(window);

        // clear data for render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        lightingShader.use();

        // set camera pos and material shininess
        lightingShader.set3f("viewPos", camera.Position);
        lightingShader.set1f("material.shininess", 64.0f);

        // set directional light uniforms
        lightingShader.set3f("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        lightingShader.set3f("dirLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        lightingShader.set3f("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5));
        lightingShader.set3f("dirLight.specular", glm::vec3(0.05f, 0.05f, 0.05f));

        // set spotlight uniforms
        lightingShader.set3f("spotLight.position", camera.Position);
        lightingShader.set3f("spotLight.direction", camera.Front);
        lightingShader.set1f("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.set1f("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));

        lightingShader.set3f("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        lightingShader.set3f("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        lightingShader.set3f("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        lightingShader.set1f("spotLight.constant", 1.0f);
        lightingShader.set1f("spotLight.linear", 0.09f);
        lightingShader.set1f("spotLight.quadratic", 0.032f);

        // pass projection matrix to shader
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        lightingShader.setmatrix4("projection", projection);

        // pass view transform matrix
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setmatrix4("view", view);

        glm::mat4 model(1.0f);
        lightingShader.setmatrix4("model", model);

        ground.Draw(lightingShader);

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, chairPositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            model = glm::scale(model, glm::vec3(0.5f));
            lightingShader.setmatrix4("model", model);
            chair.Draw(lightingShader);
        }

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, treePositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            lightingShader.setmatrix4("model", model);
            tree.Draw(lightingShader);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

glm::mat4 quatRotation(glm::mat4 model, glm::vec3 axis, float angle) {
    angle = glm::radians(angle);

    float halfSin = sin(angle / 2);
    float halfCos = cos(angle / 2);

    float w = halfCos;
    float x = axis.x * halfSin;
    float y = axis.y * halfSin;
    float z = axis.z * halfSin;

    glm::quat quaternion = glm::quat(w, x, y, z);

    glm::mat4 quaternionMatrix = glm::toMat4(quaternion);
    model = model * quaternionMatrix;
    return model;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // reversed since y-coordinates range from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}