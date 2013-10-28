// Simple 2D OpenGL Program

//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "Angel.h"
#include <stdio.h>
#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

// My includes
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
// globals
const std::string DATA_DIRECTORY_PATH = "./Data/";
int *idx_size = new int();

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location


point4 points[NumVertices];
vec4   normals[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
int Index = 0;
void
quad( int a, int b, int c, int d )
{


  vec4 u = vertices[b] - vertices[a];
  vec4 v = vertices[c] - vertices[b];

  vec4 normal = normalize( cross(u, v) );
  normal[3] = 0.0;

  normals[Index] = normal; points[Index] = vertices[a]; Index++;
  normals[Index] = normal; points[Index] = vertices[b]; Index++;
  normals[Index] = normal; points[Index] = vertices[c]; Index++;
  normals[Index] = normal; points[Index] = vertices[a]; Index++;
  normals[Index] = normal; points[Index] = vertices[c]; Index++;
  normals[Index] = normal; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
  quad( 4, 5, 6, 7 );
  quad( 5, 4, 0, 1 );
  quad( 1, 0, 3, 2 );
  quad( 2, 3, 7, 6 );
  quad( 3, 0, 4, 7 );
  quad( 6, 5, 1, 2 );
}

//----------------------------------------------------------------------------

/* ObjObject serves to conveniently package the data found in an obj file. */
class ObjObject
{
public:
	// constructors & destructors
	ObjObject();
	ObjObject(string filename);
	~ObjObject();

	// methods
	int load_from_file(string filename);
	int add_vertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w = 0);
	int add_texture_coord(GLfloat texture_coord_u, GLfloat texture_coord_v, GLfloat texture_coord_w = 0);
	int add_normal(GLfloat normal_x, GLfloat normal_y, GLfloat normal_z, GLfloat normal_w = 0);
	int add_param_vertex(GLfloat vertex_u, GLfloat vertex_v = 0, GLfloat vertex_w = 0);
	int add_face(GLint vertex_idx, GLint texture_coord_idx = 0, GLint normal_idx = 0);

	// data
	vector<GLfloat> vertices;
	vector<GLfloat> param_space_vertices;
	vector<GLfloat> texture_coords;
	vector<GLfloat> normals;
	vector<GLuint> vertex_indicies;
	vector<GLuint> texture_coord_indicies;
	vector<GLuint> normal_indicies;

	// file characteristics / metadata
	string filename;
	bool is_loaded;
	bool bad_file;
	
	int vertex_element_size;
	int normal_element_size;
	int texture_coord_element_size;
	int param_space_vertex_element_size;
};

vector<string> inline StringSplit(const string &source, const char *delimiter = " ", bool keepEmpty = false)
{
    std::vector<std::string> results;

    size_t prev = 0;
    size_t next = 0;

    while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }

    return results;
}


ObjObject::~ObjObject()
{
	vertex_element_size = NULL;
	texture_coord_element_size = NULL;
	param_space_vertex_element_size = NULL;
	bad_file = NULL;
}

ObjObject::ObjObject()
{
	vertex_element_size = 3;
	texture_coord_element_size = 2;
	param_space_vertex_element_size = 1;
}

ObjObject::ObjObject(string in_filename)
{
	vertex_element_size = 3;
	texture_coord_element_size = 2;
	param_space_vertex_element_size = 1;
	filename = in_filename;
	
	this->load_from_file(filename);
}

int ObjObject::add_face(GLint vertex_idx, GLint texture_coord_idx, GLint normal_idx)
{
	// things are indexed starting from 1 instead of 0. this is stupid, so we fix it.
	this->vertex_indicies.push_back(vertex_idx - 1);
	if(texture_coord_idx != -1)
		this->texture_coord_indicies.push_back(texture_coord_idx - 1);
	if(normal_idx != -1)
		this->normal_indicies.push_back(normal_idx - 1);
	return vertex_indicies.size();
}

int ObjObject::add_vertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	this->vertices.push_back(x);
	this->vertices.push_back(y);
	this->vertices.push_back(z);

	if(this->vertex_element_size == 4)
		this->vertices.push_back(w);
	return this->vertices.size();
}

int ObjObject::add_texture_coord(GLfloat u, GLfloat v, GLfloat w)
{
	this->texture_coords.push_back(u);
	this->texture_coords.push_back(v);

	if(this->texture_coord_element_size == 4)
		this->texture_coords.push_back(w);
	return this->texture_coords.size();
}

int ObjObject::add_normal(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	this->normals.push_back(x);
	this->normals.push_back(y);
	this->normals.push_back(z);

	if(this->normal_element_size == 4)
		this->vertices.push_back(w);

	return this->normals.size();
}

int ObjObject::add_param_vertex(GLfloat u, GLfloat  v, GLfloat w)
{
	int element_size = this->param_space_vertex_element_size;

	this->param_space_vertices.push_back(u);

	if(element_size >= 2)
		this->param_space_vertices.push_back(v);

	if(element_size == 3)
		this->param_space_vertices.push_back(w);

	return this->param_space_vertices.size();
}

int ObjObject::load_from_file(string in_filename)
{
	filename = in_filename;
	ifstream in_file(in_filename);
	string line;
	GLfloat comp0, comp1, comp2, comp3; // generic names for components of various points/vectors in the obj file
	int vertex_size;
	int status = -1;
	
	auto proc_vertex_tokens = [this] (vector<string> tokens) {
		GLfloat x, y, z, w;
		GLint vertex_size;

		vertex_size = tokens.size() - 1;
		x = atof(tokens[1].c_str());
		y = atof(tokens[2].c_str());
		z = atof(tokens[3].c_str());
		w = 0;

		if(vertex_size == 4)
			w = atof(tokens[4].c_str());

		this->vertex_element_size = vertex_size;
		this->add_vertex(x, y, z, w);
	};

	auto proc_texture_coord_tokens = [this] (vector<string> tokens) {
		auto texture_coord_element_size = (tokens.size() - 1);
		auto vt0 = atof(tokens[1].c_str());
		auto vt1 = atof(tokens[2].c_str());
		auto vt2 = 0;

		if(texture_coord_element_size == 3)
			vt2 = atof(tokens[3].c_str());

		this->add_texture_coord(vt0, vt1, vt2);
		this->texture_coord_element_size = texture_coord_element_size;
	};

	auto proc_normal_tokens = [this] (vector<string> tokens) { 
		auto normal_size = tokens.size() - 1;

		auto n0 = atof(tokens[1].c_str());
		auto n1 = atof(tokens[2].c_str());
		auto n2 = atof(tokens[3].c_str());

		GLfloat w = 0;

		if(normal_size == 4)
			w = atof(tokens[4].c_str());

		this->add_normal(n0, n1, n2);
		this->normal_element_size = normal_size;
	};

	auto proc_param_space_vertex_tokens = [this] (vector<string> tokens) { 

		auto param_space_vertex_element_size = (tokens.size() - 1);
		auto u = atof(tokens[1].c_str());
		auto v = 0;
		auto w = 0;

		if(param_space_vertex_element_size >= 2)
			v = atof(tokens[2].c_str());

		if(param_space_vertex_element_size == 3)
			w = atof(tokens[3].c_str());

		this->add_param_vertex(u, v, w);
	};

	auto proc_face_tokens = [this] (vector<string> tokens) {
		for(int i = 1; i < tokens.size(); i++) {
			vector<string> face = StringSplit(tokens[i], "/", true);
			vector<GLuint> face_idx_vals;
			for(auto idx_string : face)
			{
				if(idx_string == ""){
					face_idx_vals.push_back(-1);
				} else {
					face_idx_vals.push_back(atoi(idx_string.c_str()));
				}
			}

			if(face_idx_vals.size() == 1)
			{
				this->add_face(face_idx_vals[0]);
			} else if(face_idx_vals.size() == 2) {
				this->add_face(face_idx_vals[0], face_idx_vals[1]);
			} else if(face_idx_vals.size() == 3) {
				this->add_face(face_idx_vals[0], face_idx_vals[1], face_idx_vals[2]);
			}
		} 
	};

	if(in_file.is_open())
	{
		while(!in_file.eof())
		{
			getline(in_file, line);
			auto tokens = StringSplit(line);
			for(auto t : tokens) {

				if(tokens[0] == "v") 
				{
					proc_vertex_tokens(tokens);
					break;
				}  else if(tokens[0] == "vt") {
					proc_texture_coord_tokens(tokens);
					break;
				} else if(tokens[0] == "vn") {
					proc_normal_tokens(tokens);
					break;
				} else if(tokens[0] == "vp") {
					proc_param_space_vertex_tokens(tokens);
					break;
				} else if(tokens[0] == "f") {
					proc_face_tokens(tokens);
					break;
				} else if(tokens[0][0] == '#') {
					// This is a comment line for an obj file
					break;
				} else {
					// TODO: This catches the first three lines of obj file which contains file validation data. Need to handle these three lines instead of falling through here.
					cerr << "Inconceivable!" << endl;
					break;
				}
			}
		}
		

		cout << "Loaded file '" << this->filename << "'" << endl;
		cout << "# of Verticeis: " << this->vertices.size() / this->vertex_element_size << endl;
		cout << "# of Normals: " << this->normals.size() / 3 << endl;
		cout << "# of faces: " << this->vertex_indicies.size() << endl;
		bad_file = false;
		status = 0;
		in_file.close();
	} else {
		cerr << "Unable to open file: '" << filename << "'" << endl;
		bad_file = true;
		return status;
	}
	return status;
}
// END: ObjObject implementation

int load_scene_by_file(string filename, vector<string>& obj_filename_list)
{
	ifstream input_scene_file;
	string line;
	string filepath;
	int status = -1;

	filepath = DATA_DIRECTORY_PATH + filename;

	input_scene_file.open(filepath);
	if(input_scene_file.is_open())
	{
		getline(input_scene_file, line);
		cout << "Dimension(s) of file: '" << line << "'" << endl;
		while(!input_scene_file.eof())
		{
			getline(input_scene_file, line);
			obj_filename_list.push_back(line);
			cout << line << endl;
		}
		status = 0;
		input_scene_file.close();
	} else {
		status = -1;
	}
	return status;
}



//----------------------------------------------------------------------------
// Should do something aobut these globals
// Create a vertex array object
GLuint vao;

GLuint buffers[4];
GLint num_indicies;
GLint num_verts;
GLuint *indicies_data;
GLuint *n_indicies_data;
GLfloat *vertex_data;
GLfloat *normal_data;
// OpenGL initialization
void
init()
{
	ObjObject *tmp = new ObjObject(DATA_DIRECTORY_PATH + "bunnyNS.obj");
	vector<GLfloat> vertex_brute_force;
	vector<GLfloat> normal_brute_force;
	for(auto idx : tmp->vertex_indicies)
	{
		for(int i = 0; i < tmp->vertex_element_size; i++) {
			vertex_brute_force.push_back(tmp->vertices[tmp->vertex_element_size*idx + i]);
		}
	}
	for(auto n_idx : tmp->normal_indicies)
	{
		for(int i = 0; i < tmp->normal_element_size; i++) {
			normal_brute_force.push_back(tmp->normals[tmp->normal_element_size*n_idx + i]);
		}
	}
	auto num_bytes_vert_data = sizeof(GLfloat) * vertex_brute_force.size();
	auto num_bytes_norm_data = sizeof(GLfloat) * normal_brute_force.size();
	vertex_brute_force.shrink_to_fit();
	normal_brute_force.shrink_to_fit();
	num_verts = vertex_brute_force.size();
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, num_bytes_vert_data + num_bytes_norm_data,
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, num_bytes_vert_data, vertex_brute_force.data() );
	glBufferSubData( GL_ARRAY_BUFFER, num_bytes_vert_data, num_bytes_norm_data, normal_brute_force.data() );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "./src/vshader.glsl", "./src/fshader.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, tmp->vertex_element_size, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, tmp->normal_element_size, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(num_bytes_vert_data));


    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    point4 light_position( 1.5, 0.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );

    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
		  1, light_position );

    glUniform1f( glGetUniformLocation(program, "Shininess"),
		 material_shininess );


    model_view = glGetUniformLocation( program, "ModelView" );
    projection = glGetUniformLocation( program, "Projection" );



    mat4 p = Perspective(45, 1.0, 0.1, 10.0);
    point4  eye( -4.0, -4.0, 4.0, 1.0);
    point4  at( 0.0, 0.0, 0.0, 1.0 );
    vec4    up( 0.0, 1.0, 0.0, 0.0 );


    mat4  mv = LookAt( eye, at, up );
    //vec4 v = vec4(0.0, 0.0, 1.0, 1.0);

    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );


    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDrawArrays(GL_TRIANGLES, 0, num_verts);
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------



/*
 *  simple.c
 *  This program draws a red rectangle on a white background.
 *
 * Still missing the machinery to move to 3D
 */

/* glut.h includes gl.h and glu.h*/

int main(int argc, char** argv)
{
	string data_filename = "NO_DATA_FILE";
	string application_info = "CS450AssignmentTwo: ";
	string *window_title = new string;
	GLfloat eye_position[] = { 0., 0., 1., 1.};
	GLfloat at_position[] = { 0., 0., 0., 1. };
	GLfloat up_vector[] = { 0., 1., 0., 0. };

	if(argc != 11) {
		cerr << "USAGE: Expected 10 arguments but found '" << (argc - 1) << "'" << endl;
		cerr << "CS450AssignmentTwo SCENE_FILENAME FROM_X FROM_Y FROM_Z AT_X AT_Y AT_Z UP_X UP_Z UP_Y" << endl;
		cerr << "SCENE_FILENAME: A .scn filename existing in the ./CS450AssignmentTwo/Data/ directory." << endl;
		cerr << "FROM_X, FROM_Y, FROM_Z*: Floats passed to the LookAt function representing the point in the scene of the eye." << endl;
		cerr << "AT_X, AT_Y, AT_Z*: Floats passed to the LookAt function representing the point in the scene where the eye is looking." << endl;
		cerr << "UP_X, UP_Y, UP_Z*: Floats passed to the LookAt function representing the vector that describes the up direction within scene for the eye." << endl;
		cerr << "*These points and vectors will be converted to a homogenous coordinate system." << endl;
		return -1;
	}
	data_filename = argv[1];

	eye_position[0] = atof(argv[2]);
	eye_position[1] = atof(argv[3]);
	eye_position[2] = atof(argv[4]);

	at_position[0] = atof(argv[5]);
	at_position[1] = atof(argv[6]);
	at_position[2] = atof(argv[7]);

	up_vector[0] = atof(argv[8]);
	up_vector[1] = atof(argv[9]);
	up_vector[2] = atof(argv[10]);

	cout << "Loading scene file: '" << data_filename.c_str() << "'" << endl;
	cout << "Eye position: {" << eye_position[0] << ", " << eye_position[1] << ", " << eye_position[2] << "}" << endl;
	cout << "At position: {" << at_position[0] << ", " << at_position[1] << ", " << at_position[2] << "}" << endl;
	cout << "Up vector: {" << up_vector[0] << ", " << up_vector[1] << ", " << up_vector[2] << "}" << endl;
	
	vector<string> obj_filenames;
	int scene_load_status = load_scene_by_file(data_filename, obj_filenames);
    if(scene_load_status == -1)
	{
		cerr << "Unable to load file: '" << data_filename << "'" << endl;
		return -1;
	}
	vector<ObjObject*> obj_object_data;
	for(auto filename : obj_filenames)
	{
		
		obj_object_data.push_back(new ObjObject(DATA_DIRECTORY_PATH + filename));
		if(obj_object_data.back()->bad_file)
		{
			cerr << "Unable to load obj files." << endl;
			return -1;
		}
	}

	glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
	window_title->append(application_info);
	window_title->append(data_filename);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(500, 300);
    glutCreateWindow(window_title->c_str());
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    init();

    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();

    return(0);
}
