#pragma once
#include "Instance.h"
#include <glad/glad.h>

void Instance::addInstance(std::string name, glm::mat4 modelMatrix, Material &material)
{
	bool added = false;
	for (int i = 0; i < objins.size(); i++) {
		if (objins[i].name == name){
			objins[i].modelMatrices.push_back(modelMatrix);
			added = true;
		}
	}
	if (!added) {
		objins.push_back(ObjIns(name, modelMatrix, material));
	}
}

void Instance::drawByInstance(Shader* shader, Object &object)
{
	if (instanceVBO == 0) {
		glGenBuffers(1, &instanceVBO);

	}
	glBindVertexArray(object.VAO);
	shader->use();

	for (int i = 0; i < objins.size(); i++) {
		// material properties
		shader->setVec3("material.ambient", objins[i].material.ambient);
		shader->setVec3("material.diffuse", objins[i].material.diffuse);
		shader->setVec3("material.specular", objins[i].material.specular);
		shader->setFloat("material.shininess", objins[i].material.shininess);

		// set instance VBO
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, objins[i].modelMatrices.size()*sizeof(glm::mat4),
		objins[i].modelMatrices.data(), GL_STATIC_DRAW);

		// set model matrix
		for (int j = 0; j < 4; j++) {
			float location = 2 + j;
			glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(j * sizeof(glm::vec4)));
			glEnableVertexAttribArray(location);
			glVertexAttribDivisor(location, 1);
		}

		glDrawElementsInstanced(GL_TRIANGLES, object.element_amount, GL_UNSIGNED_INT, 0, objins[i].modelMatrices.size());
	}
	//unbind VAO
	glBindVertexArray(0);
	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);

	for (int i = 0; i < objins.size(); i++) {
		objins[i].modelMatrices.clear();
	}
	
}
