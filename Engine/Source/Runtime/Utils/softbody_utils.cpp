#include "softbody_utils.h"

namespace gdp1 {

SoftBody* SoftBodyUtils::CreateChain(Transform* transform, unsigned int chainSize, float spacing) {
    SoftBody* softBody = new SoftBody();

    for (unsigned int i = 0; i <= chainSize; i++) {
        SoftBodyParticle* pParticle = new SoftBodyParticle();

        pParticle->position = glm::vec3(transform->localPosition.x, transform->localPosition.y - (i * spacing),
                                        transform->localPosition.z);

        pParticle->oldPosition = pParticle->position;
        pParticle->position = pParticle->position;

        softBody->particles.push_back(pParticle);
    }

    // Create springs between particles
    for (int i = 0; i < softBody->particles.size(); i++) {
        softBody->CreateSpring(softBody->particles[i], softBody->particles[i + 1]);
    }

    return softBody;
}

void SoftBodyUtils::AddChain(GameObject* go, std::vector<GameObject*> chainObjects, unsigned int chainSize, float spacing) {
    std::vector<SoftBodyParticle*> particles;

    glm::vec3 gravity = glm::vec3(0.0, -2.8, 0.0);
    SoftBodyParticle* firstParticle = go->softBody->particles[0];

    for (unsigned int i = 0; i <= chainSize; i++) {
        SoftBodyParticle* pParticle = new SoftBodyParticle();

        pParticle->position = glm::vec3(firstParticle->position.x + (i * spacing), firstParticle->position.y, firstParticle->position.z);

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

}  // namespace gdp1