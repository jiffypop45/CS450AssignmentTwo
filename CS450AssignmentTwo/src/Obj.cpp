#include <iostream>
#include <fstream>
#include "Obj.h"

using namespace std;

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

Obj::~Obj()
{
	vertex_element_size = NULL;
	texture_coord_element_size = NULL;
	param_space_vertex_element_size = NULL;
	bad_file = NULL;
}

Obj::Obj()
{
	vertex_element_size = 3;
	texture_coord_element_size = 2;
	param_space_vertex_element_size = 1;
}

Obj::Obj(string in_filename)
{
	vertex_element_size = 3;
	texture_coord_element_size = 2;
	param_space_vertex_element_size = 1;
	filename = in_filename;
	
	this->load_from_file(filename);
}

int Obj::add_face(GLint vertex_idx, GLint texture_coord_idx, GLint normal_idx)
{
	// things are indexed starting from 1 instead of 0. this is stupid, so we fix it.
	this->vertex_indicies.push_back(vertex_idx - 1);
	if(texture_coord_idx != -1)
		this->texture_coord_indicies.push_back(texture_coord_idx - 1);
	if(normal_idx != -1)
		this->normal_indicies.push_back(normal_idx - 1);
	return vertex_indicies.size();
}

int Obj::add_vertex(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	this->vertices.push_back(x);
	this->vertices.push_back(y);
	this->vertices.push_back(z);

	if(this->vertex_element_size == 4)
		this->vertices.push_back(w);
	return this->vertices.size();
}

int Obj::add_texture_coord(GLfloat u, GLfloat v, GLfloat w)
{
	this->texture_coords.push_back(u);
	this->texture_coords.push_back(v);

	if(this->texture_coord_element_size == 4)
		this->texture_coords.push_back(w);
	return this->texture_coords.size();
}

int Obj::add_normal(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	this->normals.push_back(x);
	this->normals.push_back(y);
	this->normals.push_back(z);

	if(this->normal_element_size == 4)
		this->vertices.push_back(w);

	return this->normals.size();
}

int Obj::add_param_vertex(GLfloat u, GLfloat  v, GLfloat w)
{
	int element_size = this->param_space_vertex_element_size;

	this->param_space_vertices.push_back(u);

	if(element_size >= 2)
		this->param_space_vertices.push_back(v);

	if(element_size == 3)
		this->param_space_vertices.push_back(w);

	return this->param_space_vertices.size();
}

int Obj::load_from_file(string in_filename)
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
			vector<GLint> face_idx_vals;
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