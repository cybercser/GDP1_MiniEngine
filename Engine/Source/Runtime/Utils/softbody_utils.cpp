#include "softbody_utils.h"

namespace gdp1 {

SoftBody* SoftBodyUtils::CreateSoftBody(SoftbodyDesc& bodyDesc, GameObject* go) {
    SoftBody* body = new SoftBody();
    body->particleMass = bodyDesc.mass;
    body->springStrength = bodyDesc.springStrength;
    body->iterations = bodyDesc.iterations;
    body->CreateParticles(go->model, go->transform);

    glm::vec3 gravity = glm::vec3(0.0, -9.8, 0.0);
    body->ApplyForce(gravity);

    return body;
}

SoftBody* SoftBodyUtils::CreateChain(glm::vec3 startPos, unsigned int chainSize, float spacing) {
    SoftBody* softBody = new SoftBody();
    softBody->particleMass = 1.0f;
    softBody->iterations = 10;
    softBody->springStrength = 1.0f;

    for (unsigned int i = 0; i <= chainSize; i++) {
        SoftBodyParticle* pParticle = new SoftBodyParticle();

        pParticle->position = glm::vec3(startPos.x - (i * spacing), startPos.y, startPos.z);
        pParticle->oldPosition = pParticle->position;
        pParticle->position = pParticle->position;

        if (i == 0 || i == chainSize) pParticle->isPinned = true;

        softBody->particles.push_back(pParticle);
    }

    // Create springs between particles
    for (int i = 0; i < softBody->particles.size() - 1; i++) {
        softBody->CreateSpring(softBody->particles[i], softBody->particles[i + 1]);
    }

    softBody->ApplyForce(glm::vec3(0.0f, -9.8f, 0.f));

    return softBody;
}

SoftBody* SoftBodyUtils::CreatePlatform(Scene* scene, std::vector<GameObject*>& platformObjects, glm::vec3 startPos,
                                        unsigned int platformSize, float spacing) {
    glm::vec3 gravity = glm::vec3(0.0, -2.8, 0.0);
    Model* chainModel = scene->FindModelByName("ChainSphere");
    Model* woodModel = scene->FindModelByName("Wood");

    SoftBody* softBody = new SoftBody();
    softBody->particleMass = 1.0f;
    softBody->iterations = 10;
    softBody->springStrength = 1.0f;

    int chainSize = (platformSize * 2) + 1;

    // Create platform objects
    for (unsigned int i = 0; i < platformSize; i++) {
        TransformDesc transformDesc;
        transformDesc.localPosition = glm::vec3(startPos.x - (i * spacing), startPos.y, startPos.z);
        transformDesc.localEulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localScale = glm::vec3(1.0f, 1.0f, 1.0f);

        GameObjectDesc bodyDesc;
        bodyDesc.name = "platform_" + std::to_string(i);
        bodyDesc.modelName = "Wood";
        bodyDesc.transform = transformDesc;

        GameObject* go = new GameObject(scene, bodyDesc);
        go->model = chainModel;
        go->visible = true;

        SoftbodyDesc softBodyDesc;
        softBodyDesc.iterations = 5;
        softBodyDesc.mass = 1.0f;
        softBodyDesc.objectName = bodyDesc.name;
        softBodyDesc.springStrength = 1.0f;

        SoftBody* sb = CreateSoftBody(softBodyDesc, go);
        go->softBody = sb;
        go->hasSoftBody = true;

        sb->particles[1]->isPinned = true;
        sb->particles[2]->isPinned = true;
        sb->particles[4]->isPinned = true;
        sb->particles[6]->isPinned = true;

        scene->AddGameObject(go);
        platformObjects.push_back(go);
    }

    for (unsigned int i = 1; i < platformObjects.size(); i++) {
        SoftBody* body = platformObjects[i]->softBody;

        softBody->particles.push_back(body->particles[2]);
        softBody->particles.push_back(body->particles[1]);
        softBody->particles.push_back(body->particles[4]);
        softBody->particles.push_back(body->particles[6]);
    }

    SoftBodyParticle* edge1 = new SoftBodyParticle();
    edge1->position = glm::vec3(startPos.x + spacing, startPos.y, startPos.z);
    edge1->oldPosition = edge1->position;
    edge1->isPinned = true;

    SoftBodyParticle* edge2 = new SoftBodyParticle();
    edge2->position = glm::vec3(startPos.x + spacing, startPos.y, startPos.z + 2.0f);
    edge2->oldPosition = edge2->position;
    edge2->isPinned = true;

    SoftBodyParticle* edge3 = new SoftBodyParticle();
    edge3->position = glm::vec3(startPos.x - (softBody->particles.size() + spacing), startPos.y, startPos.z);
    edge3->oldPosition = edge3->position;
    edge3->isPinned = true;

    SoftBodyParticle* edge4 = new SoftBodyParticle();
    edge4->position = glm::vec3(startPos.x - (softBody->particles.size() + spacing), startPos.y, startPos.z + 2.0f);
    edge4->oldPosition = edge4->position;
    edge4->isPinned = true;

    softBody->particles.push_back(edge1);
    softBody->particles.push_back(edge2);
    softBody->particles.push_back(edge3);
    softBody->particles.push_back(edge4);

    for (unsigned int i = 0; i < softBody->particles.size(); i++) {
        TransformDesc transformDesc;
        transformDesc.localPosition = softBody->particles[i]->position;
        transformDesc.localEulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localScale = glm::vec3(1.0f, 1.0f, 1.0f);

        GameObjectDesc bodyDesc;
        bodyDesc.name = "chainObj_" + std::to_string(i);
        bodyDesc.modelName = "ChainSphere";
        bodyDesc.transform = transformDesc;

        GameObject* go = new GameObject(scene, bodyDesc);
        go->model = chainModel;
        go->visible = true;

        softBody->particles[i]->go = go;

        scene->AddGameObject(go);
        platformObjects.push_back(go);
    }

    for (int i = 0; i < softBody->particles.size() - 1; i++) {
        softBody->CreateSpring(softBody->particles[i], softBody->particles[i + 1]);
    }

    softBody->ApplyForce(glm::vec3(0.0f, -9.8f, 0.f));

    return softBody;
}

void SoftBodyUtils::AddChain(GameObject* go, std::vector<GameObject*> chainObjects, unsigned int chainSize,
                             float spacing, int attachVertexIndex) {
    std::vector<SoftBodyParticle*> particles;

    glm::vec3 gravity = glm::vec3(0.0, -2.8, 0.0);
    SoftBodyParticle* firstParticle = go->softBody->particles[attachVertexIndex];

    for (unsigned int i = 0; i <= chainSize; i++) {
        SoftBodyParticle* pParticle = new SoftBodyParticle();

        pParticle->position =
            glm::vec3(firstParticle->position.x + (i * spacing), firstParticle->position.y, firstParticle->position.z);

        pParticle->oldPosition = pParticle->position;

        pParticle->isPinned = i == chainSize;
        pParticle->ApplyForce(gravity);

        pParticle->go = chainObjects[i];

        chainObjects[i]->transform->SetPosition(pParticle->position);

        particles.push_back(pParticle);
    }

    go->softBody->particles.insert(go->softBody->particles.end(), particles.begin(), particles.end());

    for (int i = 0; i < chainSize; i++) {
        go->softBody->CreateSpring(go->softBody->particles[go->softBody->particles.size() - (i + 1)],
                                   go->softBody->particles[go->softBody->particles.size() - (i + 2)]);
    }

    go->softBody->CreateSpring(go->softBody->particles[go->softBody->particles.size() - chainSize - 1],
                               go->softBody->particles[0]);
}

void SoftBodyUtils::CreateBallSprings(SoftBody* body, float radius, int latitudeSegments, int longitudeSegments,
                                      float stiffness) {
    body->particles.clear();
    body->springs.clear();

    body->springStrength = stiffness;

    // Create particles on the surface of the football
    for (int lat = 0; lat <= latitudeSegments; ++lat) {
        float theta = lat * glm::pi<float>() / latitudeSegments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);

        for (int lon = 0; lon <= longitudeSegments; ++lon) {
            float phi = lon * 2 * glm::pi<float>() / longitudeSegments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            glm::vec3 position = glm::vec3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta) * radius;
            SoftBodyParticle* particle = new SoftBodyParticle();
            particle->position = position;
            particle->oldPosition = position;
            particle->mass = 1.0f;       // Adjust mass as needed
            particle->isPinned = false;  // Adjust pinning as needed

            body->particles.push_back(particle);
        }
    }

    // Connect adjacent particles with springs
    for (int lat = 0; lat < latitudeSegments; ++lat) {
        for (int lon = 0; lon < longitudeSegments; ++lon) {
            int currentIndex = lat * (longitudeSegments + 1) + lon;
            int nextIndex = currentIndex + 1;
            int nextRowIndex = (lat + 1) * (longitudeSegments + 1) + lon;

            if (lat < latitudeSegments - 1) {
                body->CreateSpring(body->particles[currentIndex], body->particles[nextRowIndex]);
                body->CreateSpring(body->particles[nextRowIndex], body->particles[nextRowIndex + 1]);
                body->CreateSpring(body->particles[currentIndex], body->particles[nextRowIndex + 1]);
            }

            if (lon < longitudeSegments - 1) {
                body->CreateSpring(body->particles[currentIndex], body->particles[nextIndex]);
            }
        }
    }
}

}  // namespace gdp1