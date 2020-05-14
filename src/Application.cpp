#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(),ss[1].str() };
}

static unsigned int CompilerShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type); // Creates empty shader obj
    const char* src = source.c_str();

    glShaderSource(id, 1, &src, nullptr); // Specify the source (can replace a code to)
    glCompileShader(id);    // Compiles the previously stored code, turning into an obj

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failde to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// Function used to create the shaders
// Receives the actual source code of the shader as strings
// It returns an unique ID for the shader
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();   // Just creates the object
    unsigned int vs = CompilerShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompilerShader(GL_FRAGMENT_SHADER, fragmentShader);

    // This is just specifies what must be linked
    glAttachShader(program, vs);    
    glAttachShader(program, fs);
    // Here is where we tie objects together.. in this case, shader objects
    glLinkProgram(program);
    // Check if the generated program is valid
    glValidateProgram(program);

    // An obj that is already bound to a program, can be marked to be deleted
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Erro!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;

    float positions[] = {
        -0.5f, -0.5f, // 0
         0.5f, -0.5f, // 1
         0.5f,  0.5f, // 2
        -0.5f,  0.5f, // 3
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    // Parametros:
    // indice do atributo, no nosso caso é o primeiro, então indice é zero
    // tamanho do atributo, não em bytes, mas em quantidade de dados, no nosso caso 2 por ter x e y
    // tipo do atributo, nesse caso são floats
    // usa normal, nesse caso não
    // stride - seria o passo.. ou o quanto preciso andar pra chegar no próximo atributo
    // ponteiro pro atributo, é como se fosse um offset_of pra ficar mais claro
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    unsigned int ibo;   // Index buffer object
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;

    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Don't what the last parameter uses
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}