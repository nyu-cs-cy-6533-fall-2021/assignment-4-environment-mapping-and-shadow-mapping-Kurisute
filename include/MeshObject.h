#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include <vector>
#include <string>
#include <glm/glm.hpp>  // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4

enum Renderingmode{
    PHONG = 0,
    REFLECTION = 1,
    REFRACTION = 2
};


class MeshObject{
    public:
        std::vector<glm::vec3> V;
        std::vector<glm::vec3> C;
        std::vector<glm::vec3> N_f; //the normal list for frags - flat
        std::vector<glm::vec3> N_v; //the normal list for vertices - phong
        glm::vec3 ScaleVector, RotateVector, TranslateVector;
        glm::mat4 Model;
        Renderingmode Rmode;
        unsigned int VBO_Pos;
        glm::vec3 BaryCenter;
        glm::vec3 UnitScale;

        MeshObject();
        MeshObject(std::string filepath, unsigned int vbo_pos);

        void loadOFF(std::string filepath);

        glm::mat4 get_model_matrix();
        glm::vec3 get_bary_center();
        glm::vec3 get_unit_scale();
};
#endif