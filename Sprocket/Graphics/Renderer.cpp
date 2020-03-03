#include "Graphics/Renderer.h"
#include "Utility/Maths.h"
#include "Utility/Log.h"

#include <vector>

#include <glad/glad.h>

namespace Sprocket {
namespace Renderer {

void render(const Entity& entity,
            const std::vector<Light>& lights,
            const Camera& camera,
            const Shader& shader,
            Window* window)
{   
    shader.bind();
    unsigned int MAX_NUM_LIGHTS = 5;

    // Load up the transform/projection/view matrices.
	Maths::mat4 transform = Maths::createTransformationMatrix(
		entity.position(),
		entity.rotation(),
		entity.scale());

    Maths::mat4 projection = Sprocket::Maths::createProjectionMatrix(
        window->aspectRatio(),
        70.0f,
        0.1f,
        1000.0f);

    Maths::mat4 view = Maths::createViewMatrix(
        camera.position(),
        camera.pitch(),
        camera.yaw(),
        camera.roll()
    );

    shader.loadUniform("transformMatrix", transform);
    shader.loadUniform("projectionMatrix", projection);
    shader.loadUniform("viewMatrix", view);

    // Load remaining entity to shader
	shader.loadUniform("shineDamper", entity.texture().shineDamper());
	shader.loadUniform("reflectivity", entity.texture().reflectivity());

    // Load lights to shader
    for (size_t i = 0; i != MAX_NUM_LIGHTS; ++i) {
		if (i < lights.size()) {
			shader.loadUniform(arrayName("lightPositions", i), lights[i].position);
			shader.loadUniform(arrayName("lightColours", i), lights[i].colour);
			shader.loadUniform(arrayName("lightAttenuations", i), lights[i].attenuation);
		}
		else {  // "Empty" lights to pad the array
			shader.loadUniform(arrayName("lightPositions", i), {0.0f, 0.0f, 0.0f});
			shader.loadUniform(arrayName("lightColours", i), {0.0f, 0.0f, 0.0f});
			shader.loadUniform(arrayName("lightAttenuations", i), {1.0f, 0.0f, 0.0f});
		}
	}
    

    entity.bind();
    glDrawElements(GL_TRIANGLES, entity.model().vertexCount(), GL_UNSIGNED_INT, nullptr);
    entity.unbind();

    shader.unbind();
}

void render(const Model& model, const Shader& shader)
{   
    shader.bind();
    model.bind();
    glDrawArrays(GL_TRIANGLES, 0, model.vertexCount());
    model.unbind();
    shader.unbind();
}

}
}