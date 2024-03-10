#include "physics.h"

#include "broadphase.h"
#include "collider.h"
#include "contact.h"
#include "Core/game_object.h"

#include "intersections.h"
#include "octree.h"
#include "rigidbody.h"
#include "Render/scene.h"

namespace gdp1 {

Physics::Physics(Scene* scene, const std::vector<RigidbodyDesc>& rigidbodyDescs)
    : m_Scene(scene) {
    if (scene != nullptr) {
        Init(rigidbodyDescs);
    }
}

Physics::~Physics() {
    for (int i = 0; i < m_Rigidbodies.size(); i++) {
        delete m_Rigidbodies[i]->collider;
        delete m_Rigidbodies[i];
    }
    m_Rigidbodies.clear();
    m_RigidbodyMap.clear();
}

void Physics::Init(const std::vector<RigidbodyDesc>& rigidbodyDescs) {
    glm::vec3 zero{0.0f};
    for (const RigidbodyDesc& bodyDesc : rigidbodyDescs) {
        const std::string& objName = bodyDesc.objectName;

        Rigidbody* body = new Rigidbody{};
        body->active = bodyDesc.active;
        body->position = bodyDesc.position;
        body->orientation = bodyDesc.orientation;
        body->invMass = bodyDesc.invMass;
        body->velocity = bodyDesc.velocity;

        body->gameObject = m_Scene->FindObjectByName(objName);

        if (bodyDesc.colliderName == "Sphere") {
            body->collider = new SphereCollider(zero, 1.0f);
        } else if (bodyDesc.colliderName == "Box") {
            body->collider = new BoxCollider(zero, glm::vec3(1.0f));
        } else if (bodyDesc.colliderName == "Capsule") {
            body->collider = new CapsuleCollider(zero, glm::vec3(0.0f, 2.0f, 0.0f), 0.5f);
        } else if (bodyDesc.colliderName == "Mesh") {
            body->collider = new MeshCollider(body->gameObject);
        }
        m_Rigidbodies.push_back(body);

        m_RigidbodyMap.insert({objName, body});
    }

    CreateOctree();
}

void Physics::CreateOctree() {
    std::vector<Collider*> colliders;
    for (Rigidbody* body : m_Rigidbodies) {
        // if (body->invMass == 0.0) continue;
        colliders.push_back(body->collider);
    }

    m_Octree = std::make_unique<Octree>(colliders, 7, 0.1f);
}

void Physics::FixedUpdate(float deltaTime) {
    for (Rigidbody* body : m_Rigidbodies) {
        if (body->invMass == 0.0 || !body->active) continue;

        float mass = 1.0f / body->invMass;
        glm::vec3 impulseGravity = glm::vec3(0.0, -0.01, 0.0) * mass * deltaTime;
        body->ApplyImpulse(impulseGravity);
    }

    // Broad phase
    std::vector<CollisionInfo> collisionInfos;
    BroadPhase(m_Rigidbodies, collisionInfos);

    // Narrow phase
    // We need sphere-triangle, sphere-sphere intersection test
    for (size_t i = 0; i < collisionInfos.size(); i++) {
        const CollisionInfo& info = collisionInfos[i];
        Rigidbody* bodyA = info.body1;
        Rigidbody* bodyB = info.body2;

        // skip static body pairs (infinite mass)
        if (0.0f == bodyA->invMass && 0.0f == bodyB->invMass) {
            continue;
        }

        if (!bodyA->active || !bodyB->active) {
            continue;
        }

        Contact contact;
        if (Intersect(bodyA->collider, bodyB->collider, contact)) {
            // #TODO ResolveContact(contact);
        }
    }

#if 0
	// the brute force way without broad phase
	for (size_t i = 0; i < bodies_.size(); i++) {
		for (size_t j = i + 1; j < bodies_.size(); j++) {
			Rigidbody* bodyA = bodies_[i];
			Rigidbody* bodyB = bodies_[j];

			// skip static body pairs (infinite mass)
			if (0.0f == bodyA->invMass && 0.0f == bodyB->invMass) {
				continue;
			}

			if (!bodyA->active || !bodyB->active) {
				continue;
			}

			Contact contact;
			if (Intersect(bodyA, bodyB, contact)) {
				ResolveContact(contact);
				const glm::vec3& pt = contact.ptOnA_WorldSpace;
				printf("(%s, %s) at (%.3f, %.3f, %.3f)\n", bodyA->xform->name.c_str(), bodyB->xform->name.c_str(), pt.x, pt.y, pt.z);
			}
		}
	}
#endif

    // update position
    for (Rigidbody* body : m_Rigidbodies) {
        if (body->invMass == 0.0) continue;

        body->position += body->velocity * deltaTime;
        body->collider->center = body->position;
        body->gameObject->transform->SetPosition(body->position);
    }
}

bool Physics::AddImpulseToObject(const std::string& objectName, const glm::vec3& impulse) {
    Rigidbody* body = FindRigidbodyByName(objectName);
    if (body == nullptr) return false;

    body->ApplyImpulse(impulse);
    return true;
}

Rigidbody* Physics::FindRigidbodyByName(const std::string& name) const {
    std::unordered_map<std::string, Rigidbody*>::const_iterator it = m_RigidbodyMap.find(name);
    if (it == m_RigidbodyMap.end()) return nullptr;

    return it->second;
}

}  // namespace gdp1
