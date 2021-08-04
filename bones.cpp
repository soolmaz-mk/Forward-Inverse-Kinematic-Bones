#include <vector>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/vector_angle.hpp>

class Bone {
    glm::vec3 vector;
    Bone* parent;
    
public:
 
    float angle;
    Bone(glm::vec3 _vector, Bone* _parent, float _angle) : 
        vector(_vector), 
        parent(_parent), 
        angle(_angle) {
    }
    
    glm::mat4 relative_t() {
        return rotate(glm::mat4(1.0f), angle, glm::vec3(0.0, 0.0, 0.1)) * translate(glm::mat4(1.0f), vector);
    }
    
    glm::mat4 absolute_t() {
        if (parent == nullptr)
            return relative_t();
        else
            return parent->absolute_t() * relative_t();
    }
    
    glm::vec3 absolute_start() {
        if (parent == nullptr)
            return glm::vec3(0.0, 0.0, 0.0);
        else
            return parent->absolute_end();   
    }
    
    glm::vec3 absolute_end() {
        glm::vec4 res = absolute_t() * glm::vec4(0.0, 0.0, 0.0, 1.0);
        return glm::vec3(res.x / res.w, res.y / res.w, res.z / res.w);
    }
    
    Bone* get_parent() {
        return parent;
    }
};

void add_bone(std::vector<Bone*>& bones, glm::vec3 init_vector) {
    if (bones.empty())
        bones.push_back(new Bone(init_vector, nullptr, 0));
    else
        bones.push_back(new Bone(init_vector, bones.back(), 0));
}

class Graphics {
public:
    unsigned int VBO, VAO;
    unsigned int shaderProgram;
    GLFWwindow* window;
};


// from https://learnopengl.com/

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

Graphics* graphics_init() {
    Graphics* gg = new Graphics();
    
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
    gg->window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (gg->window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(gg->window);
    glfwSetFramebufferSizeCallback(gg->window, [](GLFWwindow* gw, int w, int h) -> void { glViewport(0, 0, w, h); });

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    gg->shaderProgram = glCreateProgram();
    glAttachShader(gg->shaderProgram, vertexShader);
    glAttachShader(gg->shaderProgram, fragmentShader);
    glLinkProgram(gg->shaderProgram);
    // check for linking errors
    glGetProgramiv(gg->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(gg->shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    glGenVertexArrays(1, &(gg->VAO));
    glGenBuffers(1, &(gg->VBO));
    
    glLineWidth(2.0);
    glPointSize(5.0);
    return gg;
}

void graphics_draw(Graphics* gg, float vertices[], int size) {
    glBindVertexArray(gg->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, gg->VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);     
    
    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

  
    glUseProgram(gg->shaderProgram);
    glBindVertexArray(gg->VAO);
    glDrawArrays(GL_LINE_STRIP, 0, size / 3);
    glDrawArrays(GL_POINTS, 0, size / 3);
    glBindVertexArray(0); 

    glfwSwapBuffers(gg->window);
    glfwPollEvents();
}

class Event {
public:
    virtual ~Event() = default;
};

class CloseEvent : public Event {
};

class DragEvent : public Event {
    double x, y;
    public:
    DragEvent(double _x, double _y) : x(_x), y(_y) {}
    double get_x() { return x; }
    double get_y() { return y; }
};

Event* graphics_event(Graphics* gg) {
    if (glfwGetKey(gg->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(gg->window, true);
        
    if (glfwWindowShouldClose(gg->window))
        return new CloseEvent;
        
    if (glfwGetMouseButton(gg->window, GLFW_MOUSE_BUTTON_LEFT == GLFW_PRESS)) {
        double xpos, ypos;
        glfwGetCursorPos(gg->window, &xpos, &ypos);
        int height, width;
        glfwGetWindowSize(gg->window, &width, &height);
        return new DragEvent(2 * (xpos / width) - 1, 2 * (-ypos / height) + 1);
    }
     
    return new Event;   
    
}

void graphics_deinit(Graphics* gg) {
    glDeleteVertexArrays(1, &(gg->VAO));
    glDeleteBuffers(1, &(gg->VBO));
    glDeleteProgram(gg->shaderProgram);

    glfwTerminate();
}


#define N 4

int main() {
    std::vector<Bone*> bones;
    
    Bone* selected_bone = nullptr;
    bool is_forward = false;
    double forward_bias;
    glm::vec3 inverse_bias;
    Bone* inverse_index;
    
    for (int i = 0; i < N; i ++)
        add_bone(bones, glm::vec3(0.2, 0, 0));
    
    Graphics* gg = graphics_init();
    bool finished = false;
    while (!finished) {
        float vertices[3 * (N + 1)] = {0};
        for (int i = 0; i < N; i ++) {
            vertices[3 * i] = bones[i]->absolute_start().x;
            vertices[3 * i + 1] = bones[i]->absolute_start().y;
            vertices[3 * i + 2] = bones[i]->absolute_start().z;
        }
        vertices[3 * N] = bones.back()->absolute_end().x;
        vertices[3 * N + 1] = bones.back()->absolute_end().y;
        vertices[3 * N + 2] = bones.back()->absolute_end().z;
        
        
        graphics_draw(gg, vertices, 3 * (N + 1));
        auto event = graphics_event(gg);
        finished = dynamic_cast<CloseEvent*>(event) != nullptr;
        if (dynamic_cast<DragEvent*>(event) != nullptr) {
            DragEvent& drag_event = *(dynamic_cast<DragEvent*>(event));
            glm::vec3 p = glm::vec3(drag_event.get_x(), drag_event.get_y(), 0);
            if (selected_bone != nullptr) {
                if (is_forward) {
                    glm::vec3 s = selected_bone->absolute_start();
                    glm::vec3 e = selected_bone->absolute_end();
                    selected_bone->angle -= glm::orientedAngle(glm::normalize(p - s), glm::normalize(e - s), glm::vec3(0.0, 0.0, 1.0)) - forward_bias;
                } else {
                    inverse_index = inverse_index->get_parent() == nullptr ? selected_bone : inverse_index->get_parent();
                    double diff = glm::orientedAngle(glm::normalize(p - inverse_bias - inverse_index->absolute_start()), 
                        glm::normalize(selected_bone->absolute_end() - inverse_index->absolute_start()), glm::vec3(0.0, 0.0, 1.0));
                    inverse_index->angle -= 1.0 / N * diff;
                }
            } else {
                double best_dist = 1000;
                for (const auto& b : bones) {
                    glm::vec3 s = b->absolute_start();
                    glm::vec3 e = b->absolute_end();
                    glm::vec3 m = glm::mat3(0.5) * (s + e);
                    if (glm::length(p - e) <= glm::length(p - m) && glm::length(p - e) <= best_dist) {
                        best_dist =  glm::length(p - e);
                        is_forward = false;
                        selected_bone = b;            
                        inverse_index = b;
                        inverse_bias = p - e;
                    }
                    if (glm::length(p - m) <= glm::length(p - e) && glm::length(p - m) <= best_dist) {
                        best_dist = glm::length(p - m);
                        is_forward = true;
                        selected_bone = b;
                        forward_bias = glm::orientedAngle(glm::normalize(p - s), glm::normalize(e - s), glm::vec3(0.0, 0.0, 1.0));
                    }
                }
                std::cout << "DEBUG :: " << "selected bone : " << selected_bone << " is_forward " << is_forward << std::endl;
            }
        } else {
            selected_bone = nullptr;
        }
        delete event;
    }
    graphics_deinit(gg);
    delete gg;
    
    for (const auto& b : bones) {
        delete b;
    }
    return 0;
}


