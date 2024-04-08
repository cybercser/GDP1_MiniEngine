#include "contact.h"

#include "rigidbody.h"

namespace gdp1 {

// response with bounce
void ResolveContact(Contact& contact) {
    Rigidbody* bodyA = contact.bodyA;
    Rigidbody* bodyB = contact.bodyB;

    float invMassA = bodyA->invMass;
    float invMassB = bodyB->invMass;
    float invMassSum = invMassA + invMassB;

    // bodyA->velocity = bodyB->velocity = glm::vec3(0.0);  // stop for debug

    // calculate the collision impulse (conservation of momentum)
    glm::vec3 vab = bodyB->velocity - bodyA->velocity;
    float impulseJ = 2.0f * glm::dot(vab, contact.normal) / invMassSum;
    glm::vec3 vectorImpulseJ = 0.3f * contact.normal * impulseJ;

    bodyA->ApplyImpulse(vectorImpulseJ);
    bodyB->ApplyImpulse(-vectorImpulseJ);

    // pull inter-penetrating objects apart, move away from each other
    float tA = invMassA / invMassSum;
    float tB = invMassB / invMassSum;

    glm::vec3 ds = contact.ptOnB_WorldSpace - contact.ptOnA_WorldSpace;
    bodyA->position += ds * tA;
    bodyB->position -= ds * tB;
}

}  // namespace gdp1
