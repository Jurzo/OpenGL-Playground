#include <glad/glad.h>

#include <iostream>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <camera.h>
#include <model.h>
#include <shader.h>

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
glm::mat4 quatRotation(glm::mat4 model, glm::vec3 axis, float angle);

void texGen(unsigned int *texture);
void fboGen(unsigned int colorBufferTexture, unsigned int *fbo);

const GLuint WIDTH = 1400, HEIGHT = 700;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
Camera sideCam = Camera(glm::vec3(0.0f, 2.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

    // set view port
    glViewport(0, 0, WIDTH, HEIGHT);

    unsigned int colBufferTex, framebuffer, colBufferTex2, framebuffer2;
    texGen(&colBufferTex);
    fboGen(colBufferTex, &framebuffer);
    texGen(&colBufferTex2);
    fboGen(colBufferTex2, &framebuffer2);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    Shader shader("shaders/multilight.vs", "shaders/multilight_alpha.fs");
    Shader fboShader("shaders/fbo.vs", "shaders/fbo.fs");

    Model tree("res/tree/Tree.obj");
    Model ground("res/ground/ground.obj");
    Model chair("res/chair/chair.obj");

    glm::vec3 treePositions[]{
        glm::vec3(3.0f, 0.0f, 3.0f),
        glm::vec3(-4.0f, 0.0f, -3.0f),
        glm::vec3(4.0f, 0.0f, -4.0f)};

    glm::vec3 chairPositions[]{
        glm::vec3(3.5f, 0.0f, 0.0f),
        glm::vec3(-3.5f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -3.5f)};

    // plane to use for fbo
    float leftQuad[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         0.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         0.0f, -1.0f,  1.0f, 0.0f,
         0.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftQuad), leftQuad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // plane to use for other fbo
    float rightQuad[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
         0.0f,  1.0f,  0.0f, 1.0f,
         0.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

         0.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int VAO2, VBO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightQuad), rightQuad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

        // ------------------------
        // left framebuffer
        // clear data for render and bind fbo
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // activate shader
        shader.use();

        // set camera pos and material shininess
        shader.set3f("viewPos", camera.Position);
        shader.set1f("material.shininess", 64.0f);

        // set directional light uniforms
        shader.set3f("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        shader.set3f("dirLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.set3f("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5));
        shader.set3f("dirLight.specular", glm::vec3(0.05f, 0.05f, 0.05f));

        // set spotlight uniforms
        shader.set3f("spotLight.position", camera.Position);
        shader.set3f("spotLight.direction", camera.Front);
        shader.set1f("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.set1f("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));

        shader.set3f("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        shader.set3f("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.set3f("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        shader.set1f("spotLight.constant", 1.0f);
        shader.set1f("spotLight.linear", 0.09f);
        shader.set1f("spotLight.quadratic", 0.032f);

        // pass projection matrix to shader
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        shader.setmatrix4("projection", projection);

        // pass view transform matrix
        glm::mat4 view = camera.GetViewMatrix();
        shader.setmatrix4("view", view);

        glm::mat4 model(1.0f);
        shader.setmatrix4("model", model);

        ground.Draw(shader);

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, treePositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            shader.setmatrix4("model", model);
            tree.Draw(shader);
        }

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, chairPositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            model = glm::scale(model, glm::vec3(0.5f));
            shader.setmatrix4("model", model);
            chair.Draw(shader);
        }

        // ------------------------
        // right framebuffer
        // clear data for render and bind fbo
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // activate shader
        shader.use();

        // set camera pos and material shininess
        shader.set3f("viewPos", sideCam.Position);
        shader.set1f("material.shininess", 64.0f);

        // set directional light uniforms
        shader.set3f("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        shader.set3f("dirLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        shader.set3f("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5));
        shader.set3f("dirLight.specular", glm::vec3(0.05f, 0.05f, 0.05f));

        // set spotlight uniforms
        shader.set3f("spotLight.position", camera.Position);
        shader.set3f("spotLight.direction", camera.Front);
        shader.set1f("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.set1f("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));

        shader.set3f("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        shader.set3f("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.set3f("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        shader.set1f("spotLight.constant", 1.0f);
        shader.set1f("spotLight.linear", 0.09f);
        shader.set1f("spotLight.quadratic", 0.032f);

        // pass projection matrix to shader
        projection = glm::perspective(glm::radians(sideCam.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        shader.setmatrix4("projection", projection);

        // pass view transform matrix
        view = sideCam.GetViewMatrix();
        shader.setmatrix4("view", view);

        model = glm::mat4(1.0f);
        shader.setmatrix4("model", model);

        ground.Draw(shader);

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, treePositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            shader.setmatrix4("model", model);
            tree.Draw(shader);
        }

        for (unsigned int i = 0; i < 3; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, chairPositions[i]);
            float angle = 20.0f * i;
            model = quatRotation(model, glm::vec3(0.0f, 1.0f, 0.0f), angle);
            model = glm::scale(model, glm::vec3(0.5f));
            shader.setmatrix4("model", model);
            chair.Draw(shader);
        }

        // draw framebuffer textures to planes
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        fboShader.use();
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, colBufferTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(VAO2);
        glBindTexture(GL_TEXTURE_2D, colBufferTex2);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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

void texGen(unsigned int *texture) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void fboGen(unsigned int colorBufferTexture, unsigned int *fbo) {
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture, 0);

    // creating a render buffer object for depth and stencil testing
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach the rbo to fbo's depth and stencil buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}