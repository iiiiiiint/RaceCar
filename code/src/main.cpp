#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <Camera.h>
#include <Trave_Camera.h>
#include <Model.h>

#include <Car.h>
#include <my/fixed_camera.h>

#include <iostream>

// 深度Map的ID
unsigned int depthMap;
unsigned int depthMapFBO;

// 渲染阴影时的窗口分辨率（会影响阴影的锯齿边情况）
const unsigned int SHADOW_WIDTH = 1024*5;
const unsigned int SHADOW_HEIGHT = 1024*5;

GLboolean shadows = true;
int for_travel = 0;

// 世界坐标系Y轴单位向量
glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

glm::vec3 lightPos(-80.0f, 80.0f, -40.0f);
glm::vec3 lightDirection = glm::normalize(lightPos);
glm::mat4 lightSpaceMatrix;

unsigned int quadVAO = 0;
unsigned int quadVBO;

unsigned int planeVAO;

void depthMapFBOInit();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(vector<std::string> faces);
void renderRaceTrack(Model &model, Shader &shader);
void rendercar(Model &model, Shader &shader);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Car car(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f));
Camera camera(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f));
TCamera tcamera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool trave_camera = false;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float skyboxVertices[] = {
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
    };

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).

    // stbi_set_flip_vertically_on_load(false);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // build and compile shaders
    // -------------------------
    Shader skyboxShader("../data/shaders/skybox.vs", "../data/shaders/skybox.fs");
    Shader shader("../data/shaders/light_and_shadow.vs", "../data/shaders/light_and_shadow.fs");
    // 从太阳平行光角度生成深度信息的shader
    Shader depthShader("../data/shaders/shadow_mapping_depth.vs", "../data/shaders/shadow_mapping_depth.fs");
    Shader quad_shader("../data/shaders/quad.vs", "../data/shaders/quad.fs");
    // load models
    // -----------
    Model Model_track("../data/models/track/race-track.obj");
    Model Model_car("../data/models/Lamborghini/Lamborghini.obj");

    // render loop
    // -----------
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    vector<std::string> faces{
        "../data/skybox3/right.png",  // right
        "../data/skybox3/left.png",   // left
        "../data/skybox3/top.png",    // down
        "../data/skybox3/bottom.png", // up
        "../data/skybox3/front.png",  // back
        "../data/skybox3/back.png",   // front
    };
    depthMapFBOInit();

    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 31);

    quad_shader.use();
    quad_shader.setInt("depthMap", 0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix(car);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        lightProjection = glm::ortho(
            -350.0f, 350.0f,
            -350.0f, 350.0f,
            -350.0f, 350.0f);

        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), WORLD_UP);
        lightSpaceMatrix = lightProjection * lightView;

        // 从光源角度渲染整个场景
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        depthShader.setMat4("view",view);
        depthShader.setMat4("projection",projection);

        // 改变视口大小以便于进行深度的渲染
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // 绑定 FrameBuffer

        // 使用深度shader渲染生成场景

        glClear(GL_DEPTH_BUFFER_BIT);

        renderRaceTrack(Model_track, depthShader);

        rendercar(Model_car, depthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定回默认

        // 复原视口
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        
        if(trave_camera){
            projection = glm::perspective(glm::radians(tcamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = tcamera.GetViewMatrix();
        }
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        shader.setVec3("viewPos", camera.Position);
        if(trave_camera){
            shader.setVec3("viewPos", tcamera.Position);
        }

        shader.setVec3("lightPos", lightPos);
        // shader.setVec3("lightDirection",);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setBool("shadows", shadows);

        glActiveTexture(GL_TEXTURE31);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        rendercar(Model_car, shader);
        renderRaceTrack(Model_track, shader);

        // 绘制天空盒
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix(car))); // remove translation from the view matrix
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        if(trave_camera){
            projection = glm::perspective(glm::radians(tcamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = glm::mat4(glm::mat3(tcamera.GetViewMatrix()));
        }
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
    {
        for_travel = 0;
    } 
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
        if(trave_camera == true && for_travel == 0){
            trave_camera = false;
            for_travel = 1;
        }
        else if(trave_camera == false && for_travel == 0){
            trave_camera = true;
            tcamera.Flush_tcamera(camera);
            for_travel = 1;
        }
        // trave_camera = !trave_camera;
        // tcamera.Flush_tcamera(camera);
    }
    if(trave_camera){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            tcamera.ProcessKeyboard(TFORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            tcamera.ProcessKeyboard(TBACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            tcamera.ProcessKeyboard(TLEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            tcamera.ProcessKeyboard(TRIGHT, deltaTime);
    }else{
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            car.ProcessKeyboard(CAR_FORWARD, deltaTime, camera.getFront());
            camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            car.ProcessKeyboard(CAR_BACKWARD, deltaTime, camera.getFront());
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            car.ProcessKeyboard(CAR_LEFT, deltaTime, camera.getFront());
            camera.ProcessKeyboard(LEFT, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            car.ProcessKeyboard(CAR_RIGHT, deltaTime, camera.getFront());
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_W) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_PRESS)
        {
            car.ProcessKeyboard(CAR_NO, deltaTime, camera.getFront());
            camera.ProcessKeyboard(NO, deltaTime);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    tcamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    tcamera.ProcessMouseScroll(static_cast<float>(yoffset));
}
void depthMapFBOInit()
{
    glGenFramebuffers(1, &depthMapFBO);

    // 创建深度纹理
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    //  把生成的深度纹理作为帧缓冲的深度缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void renderRaceTrack(Model &model, Shader &shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix(car);

    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
    shader.setMat4("model", modelMatrix);
    // 投影转换
    // glm::mat4 projMatrix = camera.GetProjMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
    float fov = glm::radians(camera.Zoom); // 或者任何其他视野角度
    float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    float nearPlane = 0.1f;
    float farPlane = 200.0f;
    glm::mat4 projMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    if(trave_camera){
            projMatrix = glm::perspective(glm::radians(tcamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            viewMatrix = tcamera.GetViewMatrix();
        }
    shader.setMat4("view", viewMatrix);
    shader.setMat4("projection", projMatrix);

    model.Draw(shader);
}
void rendercar(Model &model_obj, Shader &shader)
{
    // // 视图转换
    // glm::mat4 viewMatrix = camera.GetViewMatrix(car);
    // shader.setMat4("view", viewMatrix);
    // // 模型转换
    // glm::mat4 modelMatrix = glm::mat4(1.0f);
    // modelMatrix = glm::scale(modelMatrix,glm::vec3(0.01f,0.01f,0.01f));
    // modelMatrix = glm::translate(modelMatrix,glm::vec3(0.0f,1000.0f,1.0f*mytime));

    float fov = glm::radians(camera.Zoom); // 或者任何其他视野角度
    float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    float nearPlane = 0.1f;
    float farPlane = 200.0f;
    glm::mat4 projMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);



    // shader.setMat4("model", modelMatrix);

    // model.Draw(shader);
    // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix(car);
    // shader.setMat4("projection", projection);

    if(trave_camera){
        projMatrix = glm::perspective(glm::radians(tcamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = tcamera.GetViewMatrix();
    }
    shader.setMat4("view", view);
    shader.setMat4("projection", projMatrix);
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, car.getPosition());  

    model = glm::rotate(model, glm::radians(360.0f - car.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
    // std::cout << car.getYaw() <<std::endl;
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // std::cout<<car.getPosition().x<<","<<car.getPosition().y<<","<<car.getPosition().z<<std::endl;
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    shader.setMat4("model", model);
    model_obj.Draw(shader);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}