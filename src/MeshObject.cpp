#include "MeshObject.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>  // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

MeshObject::MeshObject(){
    ScaleVector = glm::vec3(1,1,1);
    RotateVector = glm::vec3(0,0,0);
    TranslateVector = glm::vec3(0,0,0);
    Rmode = PHONG;
}


MeshObject::MeshObject(std::string filepath, unsigned int vbo_pos){
    ScaleVector = glm::vec3(1,1,1);
    RotateVector = glm::vec3(0,0,0);
    TranslateVector = glm::vec3(0,0,0);
    Rmode = PHONG;
    VBO_Pos = vbo_pos;
    loadOFF(filepath);
}


void MeshObject::loadOFF(std::string filepath){
    std::string off_flag;
    std::ifstream fin;
    fin.open(filepath, std::ios::in);
    fin >> off_flag;
    if(off_flag != "OFF"){
        throw "Not a vaild OFF file!";
    }

    int vertex_num, face_num, edge_num;
    fin >> vertex_num >> face_num >> edge_num;
    
    std::vector<glm::vec3> vertex_list;
    for(int i = 0;i < vertex_num;i++){
        float v1, v2, v3;
        fin >> v1 >> v2 >> v3;
        vertex_list.push_back(glm::vec3(v1,v2,v3));
    }

    V.clear();
    C.clear();
    N_f.clear();
    N_v.clear();

    std::vector<glm::vec3> face_normal_list(face_num);
    std::vector<std::vector<int> > vertex_face_map(vertex_num);
    std::vector<glm::vec3> vertex_normal_list(vertex_num);

    for(int i = 0;i < face_num;i++){
        int size, id1, id2, id3;
        fin >> size >> id1 >> id2 >> id3;
        V.push_back(vertex_list[id1]);
        V.push_back(vertex_list[id2]);
        V.push_back(vertex_list[id3]);

        glm::vec3 color(0.8f,0.8f,0.8f);
        C.push_back(color);
        C.push_back(color);
        C.push_back(color);

        // calculate face normal vector
        glm::highp_vec3 face_normal = glm::cross(vertex_list[id2]-vertex_list[id1],
                                vertex_list[id3]-vertex_list[id1]);
        
        face_normal = glm::normalize(face_normal);
        face_normal_list[i] = face_normal;

        N_f.push_back(face_normal);
        N_f.push_back(face_normal);
        N_f.push_back(face_normal);

        // map face to vertices
        vertex_face_map[id1].push_back(i);
        vertex_face_map[id2].push_back(i);
        vertex_face_map[id3].push_back(i);
    }

    // calculate vertex normal
    for(int i = 0;i < vertex_num;i++){
        glm::vec3 sum(0.0,0.0,0.0);
        int faces = vertex_face_map[i].size();
        for(int j = 0;j < faces;j++){
            sum += face_normal_list[vertex_face_map[i][j]];
        }
        //std::cout << sum.x << sum.y << sum.z << std::endl;
        vertex_normal_list[i] = glm::normalize(sum);
    }

    N_v.resize(V.size());

    // put normals into N
    for(int i = 0; i < V.size();i++){
        for(int j = 0;j < vertex_num;j++){
            //search for the vertex in vertexlist
            if(V[i] == vertex_list[j]){
                N_v[i] = vertex_normal_list[j];
                break;
            }
        }
    }
    
    BaryCenter = get_bary_center();
    UnitScale = get_unit_scale();
}


glm::mat4 MeshObject::get_model_matrix(){
    glm::mat4 unitMatrix(
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 0, 1)
    );

    // fix the barycenter to the origin
    glm::mat4 fix_origin = glm::translate(unitMatrix,-BaryCenter);
    // fix the sacle to the unit cube
    glm::mat4 fix_scale = glm::scale(unitMatrix,UnitScale);

    glm::mat4 scale = glm::scale(unitMatrix, ScaleVector);
    glm::mat4 translate = glm::translate(unitMatrix,TranslateVector);
    glm::mat4 rotate = unitMatrix;
    rotate = glm::rotate(rotate,glm::radians(RotateVector.x),glm::vec3(1,0,0));
    rotate = glm::rotate(rotate,glm::radians(RotateVector.y),glm::vec3(0,1,0));
    rotate = glm::rotate(rotate,glm::radians(RotateVector.z),glm::vec3(0,0,1));

    glm::mat4 Model = translate * rotate * scale * fix_scale * fix_origin;

    return Model;
}


glm::vec3 MeshObject::get_bary_center(){
    glm::vec3 sum(0,0,0);
    int v_num = V.size();
    for(int i = 0;i < V.size();i++){
        sum += V[i];
    }
    return glm::vec3(sum.x/v_num,sum.y/v_num,sum.z/v_num);
}


glm::vec3 MeshObject::get_unit_scale(){
    float x_min = 1e-30;
    float y_min = 1e-30;
    float z_min = 1e-30;
    float x_max = -1e-30;
    float y_max = -1e-30;
    float z_max = -1e-30;

    for(int i = 0;i < V.size();i++){
        x_min = x_min < V[i].x? x_min:V[i].x;
        y_min = y_min < V[i].y? y_min:V[i].y;
        z_min = z_min < V[i].z? z_min:V[i].z;
        x_max = x_max > V[i].x? x_max:V[i].x;
        y_max = y_max > V[i].y? y_max:V[i].y;
        z_max = z_max > V[i].z? z_max:V[i].z;
    }

    float x_scale = x_max - x_min;
    float y_scale = y_max - y_min;
    float z_scale = z_max - z_min;

    float max_scale = glm::max(x_scale,glm::max(y_scale,z_scale));

    return glm::vec3(1.0/max_scale,1.0/max_scale,1.0/max_scale);
}



