#include "physics.h"

#include <iostream>

#include "common.h"
#include "broadphase.h"
#include "collider.h"
#include "contact.h"
#include "Core/game_object.h"
#include "intersections.h"
#include "octree.h"
#include "rigidbody.h"
#include "softbody.h"
#include "Render/scene.h"
#include "Render/shader.h"

DWORD WINAPI UpdateSoftBodyThread(LPVOID lpParameter);

namespace gdp1 {

Physics::Physics(Scene* scene, const LevelDesc& levelDesc) {
    Init(scene, levelDesc);
}

Physics::~Physics() {
    for (int i = 0; i < rigidbodies_.size(); i++) {
        delete rigidbodies_[i]->collider;
        delete rigidbodies_[i];
    }
    rigidbodies_.clear();
    body_map_.clear();
}

void Physics::Init(Scene* scene, const LevelDesc& levelDesc) {
    this->scene = scene;

    for (const RigidbodyDesc& bodyDesc : levelDesc.rigidbodyDescs) {
        const std::string& objName = bodyDesc.objectName;

        Rigidbody* body = new Rigidbody{};
        body->active = bodyDesc.active;
        body->position = bodyDesc.position;
        body->orientation = bodyDesc.orientation;
        body->invMass = bodyDesc.invMass;
        body->velocity = bodyDesc.velocity;

        body->object = scene->FindObjectByName(objName);

        if (bodyDesc.collider == "SPHERE") {
            body->collider = new SphereCollider(1.0f);
        } else if (bodyDesc.collider == "MESH") {
            body->collider = new MeshCollider(body->object);
        }

        rigidbodies_.push_back(body);

        body_map_.insert({objName, body});
    }

    // Init Soft bodies
    for (const SoftbodyDesc& bodyDesc : levelDesc.softbodyDescs) {
        const std::string& objName = bodyDesc.objectName;

        GameObject* gameObject = scene->FindObjectByName(objName);
        gameObject->hasSoftBody = true;
        Model* model = gameObject->model;

        SoftBody* body = new SoftBody();
        body->particleMass = bodyDesc.mass;
        body->springStrength = bodyDesc.springStrength;
        body->iterations = bodyDesc.iterations;
        body->collider = new SphereCollider(0.1f);
        body->CreateParticles(gameObject->model, gameObject->transform);

        glm::vec3 gravity = glm::vec3(0.0, -9.8, 0.0);
        body->ApplyForce(gravity);

        gameObject->softBody = body;

        softbodies_.push_back(body);

        soft_body_map_.insert({objName, body});
    }

    // CreateBVH();
}

void Physics::CreateBVH() {
    std::vector<Collider*> colliders;
    for (Rigidbody* body : rigidbodies_) {
        // if (body->invMass == 0.0) continue;
        colliders.push_back(body->collider);
    }

    octree_ = std::make_unique<Octree>(colliders, 7, 0.1f);
}

void Physics::FixedUpdate(float deltaTime) {
    for (Rigidbody* body : rigidbodies_) {
        if (body->invMass == 0.0 || !body->active) continue;

        float mass = 1.0f / body->invMass;
        glm::vec3 impulseGravity = glm::vec3(0.0, -9.8, 0.0) * mass * deltaTime;
        body->ApplyImpulse(impulseGravity);
    }

    // Broad phase
    std::vector<CollisionInfo> collisionInfos;
    BroadPhase(rigidbodies_, collisionInfos);

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
        if (Intersect(bodyA, bodyB, contact)) {
            ResolveContact(contact);
            // bodyA->object->OnCollision(info);
            /*const glm::vec3& pt = contact.ptOnA_WorldSpace;
            printf("(%s, %s) at (%.3f, %.3f, %.3f)\n", bodyA->object->name.c_str(), bodyB->object->name.c_str(), pt.x,
                   pt.y, pt.z);*/
        }
    }

    // Broad phase
    // std::vector<SoftBodyCollisionInfo> softBodycollisionInfos;
    // BroadPhaseSoftBodies(softbodies_, rigidbodies_, softBodycollisionInfos);

    // for (size_t i = 0; i < softBodycollisionInfos.size(); i++) {
    //     const SoftBodyCollisionInfo& info = softBodycollisionInfos[i];
    //     SoftBody* bodyA = info.body1;
    //     Rigidbody* bodyB = info.body2;

    //    SoftBodyContact contact;
    //    if (Intersect(bodyA, bodyB, contact)) {
    //        //ResolveContact(contact);
    //        // bodyA->object->OnCollision(info);
    //        const glm::vec3& pt = contact.ptOnA_WorldSpace;
    //        printf("(%s, %s) at (%.3f, %.3f, %.3f)\n", "Softbody", bodyB->object->name.c_str(), pt.x, pt.y, pt.z);
    //    }
    //}

#if 0
	// the brute force way
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
    for (Rigidbody* body : rigidbodies_) {
        if (body->invMass == 0.0) continue;

        body->position += body->velocity * deltaTime;
        body->collider->centerOfMass = body->position;
        body->object->transform->SetPosition(body->position);
    }
}

void Physics::StartSoftBodyThreads() {
    // Init Softbodies

    for (SoftBody* softBody : softbodies_) {
        // softBody->Simulate();

        SoftBodyThreadInfo* params = new SoftBodyThreadInfo();
        params->body = softBody;
        params->isAlive = true;
        params->keepRunning = true;
        params->timeStep = 0.01;
        params->sleepTime = 0;

        void* pParams = (void*)params;

        DWORD threadId;
        HANDLE handle = CreateThread(NULL, 0, UpdateSoftBodyThread, pParams, 0, &(threadId));
    }
}

bool Physics::AddImpulseToObject(const std::string& objectName, const glm::vec3& impulse) {
    Rigidbody* body = FindRigidBodyByName(objectName);
    if (body == nullptr) return false;

    body->ApplyImpulse(impulse);
    return true;
}

Rigidbody* Physics::FindRigidBodyByName(const std::string& name) const {
    std::map<std::string, Rigidbody*>::const_iterator it = body_map_.find(name);
    if (it == body_map_.end()) return nullptr;

    return it->second;
}

SoftBody* Physics::FindSoftBodyByName(const std::string& name) const {
    std::map<std::string, SoftBody*>::const_iterator it = soft_body_map_.find(name);
    if (it == soft_body_map_.end()) return nullptr;

    return it->second;
}

void Physics::DrawBVH(std::shared_ptr<Shader> shader) const {
    if (octree_ == nullptr) return;

    octree_->Draw(shader);
}

}  // namespace gdp1
