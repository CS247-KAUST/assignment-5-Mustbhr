// CS 247 - Scientific Visualization, KAUST
//
// Programming Assignment #5
#include <cstring>
#include "CS247_prog.h"

// cycle clear colors
static void nextClearColor()
{
    clearColor = (++clearColor) % 3;

    switch(clearColor)
    {
        case 0:
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 1:
            glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
            break;
        default:
            glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
            break;
    }
}


// callbacks
// framebuffer to fix viewport
void frameBufferCallback(GLFWwindow* window, int width, int height)
{
    view_width = width;
    view_height = height;
    glViewport(0, 0, width, height);
}

// key callback to take user inputs for both windows
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_RELEASE) {
        char* status[ 2 ];
        status[ 0 ] = "disabled";
        status[ 1 ] = "enabled";

        switch (key) {
            case '1':
                toggle_xy = 0;
                LoadData( filenames[ 0 ] );
                loaded_file = 0;
                fprintf( stderr, "Loading " );
                fprintf( stderr, filenames[ 0 ] );
                fprintf( stderr, " dataset.\n");
                break;
            case '2':
                toggle_xy = 0;
                LoadData(filenames[ 1 ] );
                loaded_file = 1;
                fprintf( stderr, "Loading " );
                fprintf( stderr, filenames[ 1 ] );
                fprintf( stderr, " dataset.\n");
                break;
            case '3':
                toggle_xy = 1;
                LoadData( filenames[ 2 ] );
                loaded_file = 2;
                fprintf( stderr, "Loading " );
                fprintf( stderr, filenames[ 2 ] );
                fprintf( stderr, " dataset.\n");
                break;
            case '0':
                if( num_timesteps > 1 ){
                    loadNextTimestep();
                    fprintf( stderr, "Timestep %d.\n", loaded_timestep );
                }
                break;
            case GLFW_KEY_A:
                en_arrow = !en_arrow;
                fprintf(stderr, "%s drawing arrows.\n", en_arrow? "enabling" : "disabling");
                break;
            case GLFW_KEY_S:
                current_scalar_field = (current_scalar_field + 1)%num_scalar_fields;
                DownloadScalarFieldAsTexture();
                fprintf( stderr, "Scalar field changed.\n");
                break;
            case GLFW_KEY_B:
                nextClearColor();
                fprintf( stderr, "Next clear color.\n");
                break;
            case GLFW_KEY_EQUAL:
                sampling_rate = std::min(sampling_rate + 5, 100);
                fprintf(stderr, "Increasing sampling rate to %d.\n", sampling_rate);
                break;
            case GLFW_KEY_MINUS:
                sampling_rate = std::max(sampling_rate - 5, 5);
                fprintf(stderr, "Decreasing sampling rate to: %d.\n", sampling_rate);
                break;
            case GLFW_KEY_I:
                dt = std::min(dt + 0.005, 1.);
                fprintf(stderr, "Increase dt: %.2f\n", dt);
                break;
            case GLFW_KEY_K:
                dt = std::max(dt - 0.005, 0.0001);
                fprintf(stderr, "Decrease dt: %.2f\n", dt);
                break;
            case GLFW_KEY_T:
                en_streamline = !en_streamline;
                fprintf(stderr, "%s drawing streamlines.\n", en_streamline? "enabling" : "disabling");
                break;
            case GLFW_KEY_P:
                en_pathline = !en_pathline;
                fprintf(stderr, "%s drawing pathlines.\n", en_pathline? "enabling" : "disabling");
                break;
            case GLFW_KEY_C:
                colormap_mode = (colormap_mode + 1) % 3;
                fprintf(stderr, "colormap mode: %d\n", colormap_mode);
                break;
            case GLFW_KEY_LEFT_BRACKET:
                blend_factor = std::max(0.0f, blend_factor - 0.1f);
                fprintf(stderr, "blend factor: %.2f\n", blend_factor);
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                blend_factor = std::min(1.0f, blend_factor + 0.1f);
                fprintf(stderr, "blend factor: %.2f\n", blend_factor);
                break;
            case GLFW_KEY_M:
                integration_method = (integration_method + 1) % 3;
                fprintf(stderr, "integration: %s\n",
                    integration_method==0?"Euler":(integration_method==1?"RK2":"RK4"));
                break;
            case GLFW_KEY_X:
                streamlines.clear();
                pathlines.clear();
                streamline_seeds.clear();
                fprintf(stderr, "cleared all seeds.\n");
                break;
            case GLFW_KEY_L:
                arrow_length_mode = 1 - arrow_length_mode;
                fprintf(stderr, "arrow length mode: %s\n",
                    arrow_length_mode==0?"constant":"by magnitude");
                break;
            case GLFW_KEY_R:
                rake_mode = !rake_mode;
                fprintf(stderr, "rake mode %s.\n", rake_mode?"on":"off");
                break;
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                exit( 0 );
                break;
            default:
                fprintf( stderr, "\nKeyboard commands:\n\n"
                                 "1, load %s dataset\n"
                                 "2, load %s dataset\n"
                                 "3, load %s dataset\n"
                                 "0, cycle through timesteps\n"
                                 "b, switch backgropund color\n"
                                 "a, en-/disable arrows.\n"
                                 "t, en-/disable streamlines.\n"
                                 "p, en-/disable pathlines.\n"
                                 "+, increase sampling rate.\n"
                                 "-, decrease sampling rate.\n"
                                 "i, increase dt.\n"
                                 "k, decrease dt.\n"
                                 "q, <esc> - Quit\n",
                         filenames[0], filenames[1], filenames[2] );
                break;
        }
    }
}

// mouse callback to seed streamlines/pathlines
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos(window, &xpos, &ypos);

        if (view_width <= 0 || view_height <= 0) return;
        if (!grid_data_loaded) return;

        // convert screen to grid coords (flip y because screen y goes down)
        float gx = (float)xpos * (float)vol_dim[0] / (float)view_width;
        float gy = (float)(view_height - ypos) * (float)vol_dim[1] / (float)view_height;

        if (en_streamline) {
            if (rake_mode) {
                // horizontal rake: 7 seeds along x at clicked y
                int N = 7;
                for (int i = 0; i < N; i++) {
                    float fx = (i + 1) * (vol_dim[0] / (float)(N + 1));
                    computeStreamline((int)fx, (int)gy);
                }
            } else {
                computeStreamline((int)gx, (int)gy);
            }
        }
        if (en_pathline) {
            computePathline((int)gx, (int)gy, loaded_timestep);
        }
    }
}

// glfw error callback
static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// data

void loadNextTimestep( void )
{
    loaded_timestep = ( loaded_timestep + 1 ) % num_timesteps;
    DownloadScalarFieldAsTexture();

    // recompute all streamlines for the new timeslice using the saved seeds
    std::vector< std::pair<float,float> > seeds = streamline_seeds;
    streamlines.clear();
    streamline_seeds.clear();
    for (size_t i = 0; i < seeds.size(); i++) {
        computeStreamline((int)seeds[i].first, (int)seeds[i].second);
    }
}


/*
 * load .gri dataset
 * This only reads the header information and calls the dat loader
 * For now we ignore the grid data and assume a rectangular grid
 */
void LoadData( char* base_filename )
{
    //reset
    reset_rendering_props();

    char filename[ 80 ];
    strcpy( filename, base_filename );
    strcat( filename, ".gri");

    fprintf( stderr, "loading grid file %s\n", filename );

    // open grid file, read only, binary mode
    FILE* fp = fopen( filename, "rb" );
    if ( fp == NULL ) {
        fprintf( stderr, "Cannot open file %s for reading.\n", filename );
        return;
    }

    // read header
    char header[ 40 ];
    fread( header, sizeof( char ), 40, fp );
    sscanf( header, "SN4DB %d %d %d %d %d %f",
            &vol_dim[ 0 ], &vol_dim[ 1 ], &vol_dim[ 2 ],
            &num_scalar_fields, &num_timesteps ,&timestep );

    fprintf( stderr, "dimensions: x: %d, y: %d, z: %d.\n", vol_dim[ 0 ], vol_dim[ 1 ], vol_dim[ 2 ] );
    fprintf( stderr, "additional info: # scalar fields: %d, # timesteps: %d, timestep: %f.\n", num_scalar_fields, num_timesteps, timestep );

    // read data
    char dat_filename[ 80 ];
    strcpy( dat_filename, base_filename );

    if( num_timesteps <= 1 ){

        strcat( dat_filename, ".dat");

    } else {

        strcat( dat_filename, ".00000.dat");

    }

    loaded_timestep = 0;
    LoadVectorData( base_filename );

    glfwSetWindowSize(window, vol_dim[ 0 ], vol_dim[ 1 ] );
    grid_data_loaded = true;
}

/*
 * load .dat dataset
 * loads vector and scalar fields
 */
void LoadVectorData( const char* filename )
{
    fprintf( stderr, "loading scalar file %s\n", filename );

    // open data file, read only, binary mode
    char ts_name[ 80 ];
    if( num_timesteps > 1 )
    {
        sprintf( ts_name, "%s.%.5d.dat", filename, 0 );
    }
    else
        sprintf( ts_name, "%s.dat",filename);

    FILE* fp = fopen( ts_name, "rb" );
    if ( fp == NULL ) {
        fprintf( stderr, "Cannot open file %s for reading.\n", filename );
        return;
    }
    else
    {
        fclose( fp );
    }

    data_size = vol_dim[ 0 ] * vol_dim[ 1 ] * vol_dim[ 2 ];

    if (!vector_array) {
        delete[] vector_array;
        vector_array = NULL;
    }
    // dim.xyz * vector.xyz * timesteps
    vector_array = new float[ data_size * 3 * num_timesteps];

    // read data
    if (!scalar_fields) {
        delete[] scalar_fields;
        scalar_fields = NULL;
        delete[] scalar_bounds;
        scalar_bounds = NULL;
    }
    // dim.xyz * scalarfields(2) * timesteps
    scalar_fields = new float[ data_size * num_scalar_fields * num_timesteps ];
    scalar_bounds = new float[ 2 * num_scalar_fields * num_timesteps ];

    int num_total_fields = num_scalar_fields + 3; // scalar fields + vec.xyz
    float *tmp = new float[ data_size * num_total_fields * num_timesteps ];

    for( int k = 0 ; k < num_timesteps; k++ )
    {
        char times_name[ 80 ];
        if( num_timesteps > 1 )
        {
            sprintf( times_name, "%s.%.5d.dat", filename, k );
            fp = fopen( times_name, "rb" );
        }
        else
            fp = fopen( ts_name, "rb" );
        // read scalar data
        fread( &tmp[k*data_size*num_total_fields], sizeof( float ), ( data_size * num_total_fields ), fp );

        // close file
        fclose( fp );

        // copy and scan for min and max values
        for( int  i = 0; i < num_scalar_fields; i++ ){

            float min_val = 99999.9f;
            float max_val = 0.0f;

            float avg = 0.0;

            int offset = i * data_size * num_timesteps;

            for( int j = 0; j < data_size; j++ ){

                float val = tmp[ j * num_total_fields + 3 + i + k*data_size*num_total_fields ];

                scalar_fields[ j + k*data_size + offset ] = val;

                if( toggle_xy ) {
                    // overwrite
                    if( i == 0 ){
                        vector_array[ 3*j + 0 + 3*k*data_size ] = tmp[ j * num_total_fields + 1 + k*data_size*num_total_fields ];//toggle x and y components in vector field
                        vector_array[ 3*j + 1 + 3*k*data_size ] = tmp[ j * num_total_fields + 0 + k*data_size*num_total_fields ];
                        vector_array[ 3*j + 2 + 3*k*data_size ] = tmp[ j * num_total_fields + 2 + k*data_size*num_total_fields ];
                    }
                } else {
                    // overwrite
                    if( i == 0 ){
                        vector_array[ 3*j + 0 + 3*k*data_size ] = tmp[ j * num_total_fields + 0 + k*data_size*num_total_fields ];
                        vector_array[ 3*j + 1 + 3*k*data_size ] = tmp[ j * num_total_fields + 1 + k*data_size*num_total_fields ];
                        vector_array[ 3*j + 2 + 3*k*data_size ] = tmp[ j * num_total_fields + 2 + k*data_size*num_total_fields ];
                    }
                }

                min_val = std::min( val, min_val );
                max_val = std::max( val, max_val );

                avg += scalar_fields[ offset + j + k*data_size ] / data_size;
            }
            scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ] = min_val;
            scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ] = max_val;
        }

        // normalize
        for( int  i = 0; i < num_scalar_fields; i++ ){

            int offset = i * data_size * num_timesteps;

            float lower_bound = scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ];
            float upper_bound = scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ];

            // scale between [0..1] where 1 is original zero
            // the boundary of the bigger abs border will be used to scale
            // meaning one boundary will likely not be hit i.e real scale
            // for -50..100 will be [0.25..1.0]
            if( lower_bound < 0.0 && upper_bound > 0.0 ){

                float scale = 0.5f / std::max( -lower_bound, upper_bound );

                for( int j = 0; j < data_size; j++ ){

                    scalar_fields[ offset + j + k*data_size ] = 0.5f + scalar_fields[ offset + j + k*data_size ] * scale;
                }
                scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ] = 0.5f + scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ] * scale;
                scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ] = 0.5f + scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ] * scale;


                // scale between [0..1]
            } else {

                float sign = upper_bound <= 0.0 ? -1.0f : 1.0f;

                float scale = 1.0f / ( upper_bound - lower_bound ) * sign;

                for( int j = 0; j < data_size; j++ ){

                    scalar_fields[ offset + j + k*data_size ] = ( scalar_fields[ offset + j + k*data_size ] - lower_bound ) * scale;
                }
                scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ] = ( scalar_bounds[ 2 * i     + k*num_scalar_fields*2 ] + lower_bound ) * scale; //should be 0.0
                scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ] = ( scalar_bounds[ 2 * i + 1 + k*num_scalar_fields*2 ] + lower_bound ) * scale; //should be 1.0
            }
        }
    }
    delete[] tmp;
    DownloadScalarFieldAsTexture();

    scalar_data_loaded = true;
}


void DownloadScalarFieldAsTexture( void )
{
    fprintf( stderr, "downloading scalar field to 2D texture\n" );

    glEnable( GL_TEXTURE_2D );

    // generate and bind 2D texture
    glGenTextures( 1, &scalar_field_texture );
    glBindTexture( GL_TEXTURE_2D, scalar_field_texture );

    // set necessary texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    int datasize = vol_dim[0] * vol_dim[1];
    //download texture (core profile: use GL_R16 / GL_RED instead of GL_INTENSITY16 / GL_LUMINANCE)
    glTexImage2D( GL_TEXTURE_2D, 0,  GL_R16, vol_dim[ 0 ], vol_dim[ 1 ], 0, GL_RED, GL_FLOAT, &scalar_fields[ (loaded_timestep + current_scalar_field * num_timesteps)*datasize ] );


    glDisable( GL_TEXTURE_2D );
}

char *textFileRead( char *fn ){

    FILE *fp;
    char *content = NULL;

    int count=0;

    if (!fn) {
        fp = fopen(fn,"rt");

        if (!fp) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = fread(content,sizeof(char),count,fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}


// initializations
// init application
bool initApplication(int argc, char **argv)
{

    std::string version((const char *)glGetString(GL_VERSION));
    std::stringstream stream(version);
    unsigned major, minor;
    char dot;

    stream >> major >> dot >> minor;

    assert(dot == '.');
    if (major > 3 || (major == 2 && minor >= 0)) {
        std::cout << "OpenGL Version " << major << "." << minor << std::endl;
    } else {
        std::cout << "The minimum required OpenGL version is not supported on this machine. Supported is only " << major << "." << minor << std::endl;
        return false;
    }

    return true;
}

void reset_rendering_props( void )
{
    num_scalar_fields = 0;
}

// set up the scene
void setup() {
    LoadData( filenames[ 0 ] );
    loaded_file = 0;

    DownloadScalarFieldAsTexture();


    // compile & link shader
#ifdef __APPLE__
    vectorProgram.compileShader("../shaders/vertex.vs");
    vectorProgram.compileShader("../shaders/fragment.fs");
#else
    vectorProgram.compileShader("../../../shaders/vertex.vs");
    vectorProgram.compileShader("../../../shaders/fragment.fs");
#endif
    vectorProgram.link();

    // make quad to render texture
    // see: vboquad.h and vboquad.cpp
    quad.init();

    // set up VAO/VBO for line drawing (glyphs/streamlines/pathlines)
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// rendering
void render() {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_TEXTURE_2D );

    // draw the texture
    glBindTexture(GL_TEXTURE_2D, scalar_field_texture);
    vectorProgram.use();

    model = mat4(1);

    vectorProgram.setUniform("vertexColor", glm::vec4(0));
    vectorProgram.setUniform("model", model);
    vectorProgram.setUniform("colormapMode", colormap_mode);
    vectorProgram.setUniform("blendFactor", blend_factor);

    quad.render();
    glDisable( GL_TEXTURE_2D );

    // switch to solid color mode for overlays
    vectorProgram.setUniform("colormapMode", 3);

    if (en_arrow) {
        vectorProgram.setUniform("vertexColor", glm::vec4(1, 1, 1, 1));
        drawGlyphs();
    }

    if (en_streamline) {
        vectorProgram.setUniform("vertexColor", glm::vec4(1, 1, 0, 1)); // yellow
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        for (size_t i = 0; i < streamlines.size(); i++) {
            glBufferData(GL_ARRAY_BUFFER, streamlines[i].size()*sizeof(float),
                         streamlines[i].data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_STRIP, 0, streamlines[i].size()/2);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    if (en_pathline) {
        vectorProgram.setUniform("vertexColor", glm::vec4(0, 1, 1, 1)); // cyan
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        for (size_t i = 0; i < pathlines.size(); i++) {
            glBufferData(GL_ARRAY_BUFFER, pathlines[i].size()*sizeof(float),
                         pathlines[i].data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_STRIP, 0, pathlines[i].size()/2);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

// entry point
int main(int argc, char** argv)
{
    // init variables
    view_width = 0;
    view_height = 0;

    toggle_xy = 0;

    en_arrow = false;
    en_streamline = false;
    en_pathline = false;
    sampling_rate = 15;
    dt = 0.1;
    integration_method = 0;
    colormap_mode = 0;
    blend_factor = 0.5f;
    arrow_length_mode = 0;
    rake_mode = false;

    reset_rendering_props();

    vector_array = NULL;
    scalar_fields = NULL;
    scalar_bounds = NULL;
    grid_data_loaded = false;
    scalar_data_loaded = false;
    current_scalar_field = 0;
    clearColor = 0;


#ifdef __APPLE__
    filenames[ 0 ] = "../data/block/c_block";
    filenames[ 1 ] = "../data/tube/tube";
    filenames[ 2 ] = "../data/hurricane/hurricane_p_tc";
#else
    filenames[ 0 ] = "../../../data/block/c_block";
    filenames[ 1 ] = "../../../data/tube/tube";
    filenames[ 2 ] = "../../../data/hurricane/hurricane_p_tc";
#endif



    // set glfw error callback
    glfwSetErrorCallback(errorCallback);

    // init glfw
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // need core profile (macOS requires this for OpenGL > 2.1)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // init glfw window
    window = glfwCreateWindow(gWindowWidth, gWindowHeight, "AMCS/CS247 Scientific Visualization", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set GLFW callback functions
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, frameBufferCallback);

    // make context current (once is sufficient)
    glfwMakeContextCurrent(window);

    // get the frame buffer size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // init the OpenGL API (we need to do this once before any calls to the OpenGL API)
    gladLoadGL();

    // init our application
    if (!initApplication(argc, argv)) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    // set up the scene
    setup();

    // print menu
    keyCallback(window, GLFW_KEY_BACKSLASH, 0, GLFW_PRESS, 0);

    // start traversing the main loop
    // loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // clear frame buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render one frame
        render();

        // swap front and back buffers
        glfwSwapBuffers(window);

        // poll and process input events (keyboard, mouse, window, ...)
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}

// helpers

// bilinear interpolation of the vector at fractional grid position (x,y) for timestep t
// returns (vx, vy); returns 0 if out of bounds
static void sampleVec(float x, float y, int t, float &vx, float &vy)
{
    vx = 0; vy = 0;
    if (x < 0 || y < 0) return;
    if (x >= vol_dim[0]-1 || y >= vol_dim[1]-1) return;

    int x0 = (int)x;
    int y0 = (int)y;
    float fx = x - x0;
    float fy = y - y0;

    int W = vol_dim[0];
    int offset = 3 * t * data_size;

    // 4 surrounding samples
    float vx00 = vector_array[offset + 3*(x0   + y0*W)     ];
    float vy00 = vector_array[offset + 3*(x0   + y0*W)   +1];
    float vx10 = vector_array[offset + 3*(x0+1 + y0*W)     ];
    float vy10 = vector_array[offset + 3*(x0+1 + y0*W)   +1];
    float vx01 = vector_array[offset + 3*(x0   + (y0+1)*W) ];
    float vy01 = vector_array[offset + 3*(x0   + (y0+1)*W)+1];
    float vx11 = vector_array[offset + 3*(x0+1 + (y0+1)*W) ];
    float vy11 = vector_array[offset + 3*(x0+1 + (y0+1)*W)+1];

    float a = (1-fx)*(1-fy);
    float b =    fx *(1-fy);
    float c = (1-fx)*   fy ;
    float d =    fx *   fy ;
    vx = a*vx00 + b*vx10 + c*vx01 + d*vx11;
    vy = a*vy00 + b*vy10 + c*vy01 + d*vy11;
}

// trilinear: bilinear in space + linear in time
static void sampleVecTime(float x, float y, float tf, float &vx, float &vy)
{
    vx = 0; vy = 0;
    if (tf < 0 || tf > num_timesteps - 1) return;
    int t0 = (int)tf;
    int t1 = t0 + 1;
    if (t1 >= num_timesteps) t1 = t0;
    float ft = tf - t0;

    float vx0, vy0, vx1, vy1;
    sampleVec(x, y, t0, vx0, vy0);
    sampleVec(x, y, t1, vx1, vy1);
    vx = (1-ft)*vx0 + ft*vx1;
    vy = (1-ft)*vy0 + ft*vy1;
}

// convert grid coords to NDC for drawing
static inline float toNDC_x(float gx) { return 2.0f * gx / (float)vol_dim[0] - 1.0f; }
static inline float toNDC_y(float gy) { return 2.0f * gy / (float)vol_dim[1] - 1.0f; }

// single integration step for streamlines (constant time slice)
static void stepStream(float &x, float &y, float h)
{
    int t = loaded_timestep;
    if (integration_method == 0) {
        // Euler
        float vx, vy;
        sampleVec(x, y, t, vx, vy);
        x += h * vx;
        y += h * vy;
    } else if (integration_method == 1) {
        // RK2 (midpoint)
        float k1x, k1y;
        sampleVec(x, y, t, k1x, k1y);
        float mx = x + 0.5f*h*k1x;
        float my = y + 0.5f*h*k1y;
        float k2x, k2y;
        sampleVec(mx, my, t, k2x, k2y);
        x += h*k2x;
        y += h*k2y;
    } else {
        // RK4
        float k1x,k1y,k2x,k2y,k3x,k3y,k4x,k4y;
        sampleVec(x, y, t, k1x, k1y);
        sampleVec(x+0.5f*h*k1x, y+0.5f*h*k1y, t, k2x, k2y);
        sampleVec(x+0.5f*h*k2x, y+0.5f*h*k2y, t, k3x, k3y);
        sampleVec(x+    h*k3x, y+    h*k3y, t, k4x, k4y);
        x += h*(k1x + 2*k2x + 2*k3x + k4x)/6.0f;
        y += h*(k1y + 2*k2y + 2*k3y + k4y)/6.0f;
    }
}

// single integration step for pathlines (also advances in time)
static void stepPath(float &x, float &y, float &tf, float h)
{
    if (integration_method == 0) {
        float vx, vy;
        sampleVecTime(x, y, tf, vx, vy);
        x += h * vx;
        y += h * vy;
        tf += h;
    } else if (integration_method == 1) {
        float k1x, k1y;
        sampleVecTime(x, y, tf, k1x, k1y);
        float mx = x + 0.5f*h*k1x;
        float my = y + 0.5f*h*k1y;
        float mt = tf + 0.5f*h;
        float k2x, k2y;
        sampleVecTime(mx, my, mt, k2x, k2y);
        x += h*k2x;
        y += h*k2y;
        tf += h;
    } else {
        float k1x,k1y,k2x,k2y,k3x,k3y,k4x,k4y;
        sampleVecTime(x, y, tf,            k1x, k1y);
        sampleVecTime(x+0.5f*h*k1x, y+0.5f*h*k1y, tf+0.5f*h, k2x, k2y);
        sampleVecTime(x+0.5f*h*k2x, y+0.5f*h*k2y, tf+0.5f*h, k3x, k3y);
        sampleVecTime(x+    h*k3x, y+    h*k3y, tf+    h,    k4x, k4y);
        x += h*(k1x + 2*k2x + 2*k3x + k4x)/6.0f;
        y += h*(k1y + 2*k2y + 2*k3y + k4y)/6.0f;
        tf += h;
    }
}


void computeStreamline(int x, int y)
{
    if (!grid_data_loaded) return;

    // save the seed so we can recompute if timestep changes
    streamline_seeds.push_back(std::make_pair((float)x, (float)y));

    // max length & step count
    float maxLen = 5.0f * (float)std::max(vol_dim[0], vol_dim[1]);
    int   maxSteps = 5000;
    float minSpeed = 1e-5f;

    // backward integration (negative dt), collect, reverse
    std::vector<float> back;
    {
        float px = x, py = y;
        float len = 0.0f;
        for (int s = 0; s < maxSteps; s++) {
            float vx, vy;
            sampleVec(px, py, loaded_timestep, vx, vy);
            float sp = sqrtf(vx*vx + vy*vy);
            if (sp < minSpeed) break;
            float ox = px, oy = py;
            stepStream(px, py, -dt);
            if (px < 0 || py < 0 || px >= vol_dim[0]-1 || py >= vol_dim[1]-1) break;
            len += sqrtf((px-ox)*(px-ox) + (py-oy)*(py-oy));
            if (len > maxLen) break;
            back.push_back(toNDC_x(px));
            back.push_back(toNDC_y(py));
        }
    }

    // assemble: reversed backward + seed + forward
    std::vector<float> line;
    for (int i = (int)back.size()/2 - 1; i >= 0; i--) {
        line.push_back(back[2*i]);
        line.push_back(back[2*i+1]);
    }
    line.push_back(toNDC_x((float)x));
    line.push_back(toNDC_y((float)y));

    // forward
    {
        float px = x, py = y;
        float len = 0.0f;
        for (int s = 0; s < maxSteps; s++) {
            float vx, vy;
            sampleVec(px, py, loaded_timestep, vx, vy);
            float sp = sqrtf(vx*vx + vy*vy);
            if (sp < minSpeed) break;
            float ox = px, oy = py;
            stepStream(px, py, dt);
            if (px < 0 || py < 0 || px >= vol_dim[0]-1 || py >= vol_dim[1]-1) break;
            len += sqrtf((px-ox)*(px-ox) + (py-oy)*(py-oy));
            if (len > maxLen) break;
            line.push_back(toNDC_x(px));
            line.push_back(toNDC_y(py));
        }
    }

    streamlines.push_back(line);
}

void computePathline(int x, int y, int t)
{
    if (!grid_data_loaded) return;

    float maxLen = 5.0f * (float)std::max(vol_dim[0], vol_dim[1]);
    int   maxSteps = 5000;
    float minSpeed = 1e-5f;

    // backward
    std::vector<float> back;
    {
        float px = x, py = y;
        float tf = (float)t;
        float len = 0.0f;
        for (int s = 0; s < maxSteps; s++) {
            float vx, vy;
            sampleVecTime(px, py, tf, vx, vy);
            float sp = sqrtf(vx*vx + vy*vy);
            if (sp < minSpeed) break;
            float ox = px, oy = py;
            stepPath(px, py, tf, -dt);
            if (px < 0 || py < 0 || px >= vol_dim[0]-1 || py >= vol_dim[1]-1) break;
            if (tf < 0 || tf > num_timesteps - 1) break;
            len += sqrtf((px-ox)*(px-ox) + (py-oy)*(py-oy));
            if (len > maxLen) break;
            back.push_back(toNDC_x(px));
            back.push_back(toNDC_y(py));
        }
    }

    std::vector<float> line;
    for (int i = (int)back.size()/2 - 1; i >= 0; i--) {
        line.push_back(back[2*i]);
        line.push_back(back[2*i+1]);
    }
    line.push_back(toNDC_x((float)x));
    line.push_back(toNDC_y((float)y));

    // forward
    {
        float px = x, py = y;
        float tf = (float)t;
        float len = 0.0f;
        for (int s = 0; s < maxSteps; s++) {
            float vx, vy;
            sampleVecTime(px, py, tf, vx, vy);
            float sp = sqrtf(vx*vx + vy*vy);
            if (sp < minSpeed) break;
            float ox = px, oy = py;
            stepPath(px, py, tf, dt);
            if (px < 0 || py < 0 || px >= vol_dim[0]-1 || py >= vol_dim[1]-1) break;
            if (tf < 0 || tf > num_timesteps - 1) break;
            len += sqrtf((px-ox)*(px-ox) + (py-oy)*(py-oy));
            if (len > maxLen) break;
            line.push_back(toNDC_x(px));
            line.push_back(toNDC_y(py));
        }
    }

    pathlines.push_back(line);
}

void drawGlyphs() {
    if (!grid_data_loaded) return;

    std::vector<float> verts; // pairs of x,y for GL_LINES

    // compute max magnitude for normalization (rough)
    float maxMag = 0.0f;
    for (int j = 0; j < vol_dim[1]; j += sampling_rate) {
        for (int i = 0; i < vol_dim[0]; i += sampling_rate) {
            float vx, vy;
            sampleVec((float)i, (float)j, loaded_timestep, vx, vy);
            float m = sqrtf(vx*vx + vy*vy);
            if (m > maxMag) maxMag = m;
        }
    }
    if (maxMag < 1e-8f) maxMag = 1.0f;

    // arrow length in NDC ~ a bit smaller than the grid cell
    float arrowLen = (float)sampling_rate * 2.0f / (float)std::max(vol_dim[0], vol_dim[1]);

    for (int j = 0; j < vol_dim[1]; j += sampling_rate) {
        for (int i = 0; i < vol_dim[0]; i += sampling_rate) {
            float vx, vy;
            sampleVec((float)i, (float)j, loaded_timestep, vx, vy);
            float mag = sqrtf(vx*vx + vy*vy);
            if (mag < 1e-8f) continue;

            // direction
            float dx = vx / mag;
            float dy = vy / mag;

            // length
            float L;
            if (arrow_length_mode == 0) L = arrowLen;
            else                        L = arrowLen * (mag / maxMag);

            // shaft
            float sx = toNDC_x((float)i);
            float sy = toNDC_y((float)j);
            // tip in NDC: need to scale dx,dy by NDC ratios
            float tx = sx + L * dx;
            float ty = sy + L * dy;

            verts.push_back(sx); verts.push_back(sy);
            verts.push_back(tx); verts.push_back(ty);

            // arrowhead: two short lines from tip, flared ~30 deg
            // from the backward direction (so the wings form a V pointing
            // back toward the shaft, not forward past the tip).
            float headLen = L * 0.35f;
            float ca = cosf(0.5f); // ~30 deg
            float sa = sinf(0.5f);
            float h1x = tx + headLen * ( ca*(-dx) - sa*(-dy));
            float h1y = ty + headLen * ( sa*(-dx) + ca*(-dy));
            float h2x = tx + headLen * ( ca*(-dx) + sa*(-dy));
            float h2y = ty + headLen * (-sa*(-dx) + ca*(-dy));

            verts.push_back(tx); verts.push_back(ty);
            verts.push_back(h1x); verts.push_back(h1y);
            verts.push_back(tx); verts.push_back(ty);
            verts.push_back(h2x); verts.push_back(h2y);
        }
    }

    if (verts.empty()) return;

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, verts.size()/2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
