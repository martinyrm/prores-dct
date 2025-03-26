#include "main.h"
#include <iostream>
#include <fstream>


int main() { 
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "ProRes GLSL",
        NULL,
        NULL );

    if ( window == NULL ) {
        std::cerr << "failed to create glfw window" << std::endl;
        glfwTerminate();

        return -1;
    }

    // set context
    glfwMakeContextCurrent( window );

    // load glad before we make any opengl calls
    if ( !gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress ) ) {
        std::cerr << "failed to initialise glad" << std::endl;

        return -1;
    }

    // set gl viewport size, and set glfw callback for window resize
    glViewport( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    #if DEBUG_ACTIVE
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( gl_message_callback, 0 );
    #endif

    Compute compute_shader( "shader.comp", glm::uvec2( 1, 1 ) ); //Only implemented as a single shader invocation (inefficient)
    compute_shader.use();

    //load in the DCT blocks decoded by FFmpeg
    std::ifstream inputFile("/home/martinr/blocks_pre.dat");
    std::ofstream dctResult;
    dctResult.open("/home/martinr/output.dat", std::ios_base::app); //Create file to store transformed blocks (for verification)
    int block_num = 300;
    int16_t* post_blocks =  new int16_t[block_num * 64];
    int16_t* pre_blocks =  new int16_t[block_num * 64];
    int* blocks = new int[block_num * 64];
    inputFile.read((char*)&pre_blocks[0], block_num * 64 * sizeof(int16_t));
    for(int i = 0; i < (block_num * 64); i++)
        blocks[i]  = pre_blocks[i];           //Copy DCT blocks into a (long) int array - this can be avoided 


    GLuint ssbo;
    ssbo = compute_shader.generate_and_store_ssbo(blocks); //Function creates the SSBO data store, writes the first DCT block in
    int offset = 0;
    int block_no;
    while (!glfwWindowShouldClose( window ) && block_no < block_num) {

        process_input(window);  
        compute_shader.use();   
        compute_shader.dispatch(); //Run IDCT compute shader
        compute_shader.wait();

        auto ssbo_data = compute_shader.copy_ssbo_to_host(ssbo, 64); //Copy transformed block from GPU to CPU 

        for (int i = 0; i < 64; i++) { 
            post_blocks[i] = (uint16_t)*(ssbo_data.data() + i);
        }

        dctResult.write((char*) post_blocks, sizeof(uint16_t) * 64); 

        std::cout << std::endl;   //We now write the next block into the SSBO
        offset += 64;
        block_no += 1;
        compute_shader.update_ssbo(&blocks[offset], ssbo);
        glfwPollEvents();
        glfwSwapBuffers( window );
    } 

    glfwTerminate();

    return 0;
}

// callback function to handle when the window resizes
void framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
    glViewport( 0, 0, width, height );
}

// handle all input here
void process_input( GLFWwindow* window ) {
    // close window on pressing esc
    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
    }
}


