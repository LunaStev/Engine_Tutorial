//
// Created by 32725 on 2023/4/7.
//

#include "ScriptReg.h"
#include "Z/Core/Core.h"
#include "Z/Core/Log.h"
#include "Z/Core/GUID.h"
#include "ScriptEngine.h"
#include "Z/Scene/Entity.h"
#include "Z/Core/Input.h"
#include "glm/glm.hpp"
#include "mono/jit/jit.h"
#include "mono/metadata/reflection.h"
#include "box2d/b2_body.h"

namespace Z {
	void InternalCallPRT(MonoString *str) {
		Z_CORE_WARN("C#: {0}", mono_string_to_utf8(str));
	}

	void InternalCallWARN(glm::vec3 *v) {
		Z_CORE_WARN("C#: {0}", *v);
	}

	void InternalCallDot(glm::vec3 *v, glm::vec3 *u, glm::vec3 *o) {
		*o = glm::normalize(glm::cross(*v, *u));
	}

	void GetTranslation(GUID id, glm::vec3 *o) {
		*o = ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<TransformComponent>().translation;
	}

	void SetTranslation(GUID id, glm::vec3 *o) {
		ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<TransformComponent>().translation = *o;
	}

	bool IsKeyPressed(uint16_t keycode) {
		return Input::IsKeyPressed((KeyCode) keycode);
	}

	static std::unordered_map<MonoType* , bool (*)(Entity)> ReflectionMap;

	bool Entity_HasComponent(GUID id, MonoReflectionType *type) {
		Entity entity = ScriptEngine::GetContext()->GetEntityWithGUID(id);
		Z_CORE_ASSERT(entity, "No such entity");
		MonoType *monoType = mono_reflection_type_get_type(type);
		Z_CORE_ASSERT(ReflectionMap.find(monoType) != ReflectionMap.end(), "No such type");
		return ReflectionMap.at(monoType)(entity);
	}
	void Entity_GetVelocity(GUID id, glm::vec2 *o) {
		b2Body* body = (b2Body*)ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<RigidBody2DComponent>().ptr;
		*o = glm::vec2(body->GetLinearVelocity().x, body->GetLinearVelocity().y);
	}
	void Entity_SetVelocity(GUID id, glm::vec2 *o) {
		b2Body* body = (b2Body*)ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<RigidBody2DComponent>().ptr;
		auto vel = body->GetLinearVelocity();
		body->SetLinearVelocity(vel+b2Vec2(o->x, o->y));
	}
	void Entity_ApplyForce(GUID id, glm::vec2 *o,glm::vec2* c,bool wake) {
		b2Body* body = (b2Body*)ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<RigidBody2DComponent>().ptr;
		body->ApplyForce(b2Vec2(o->x,o->y),b2Vec2(c->x,c->y),wake);
	}
	void Entity_ApplyForceCenter(GUID id, glm::vec2 *o,bool wake) {
		b2Body* body = (b2Body*)ScriptEngine::GetContext()->GetEntityWithGUID(id).GetComponent<RigidBody2DComponent>().ptr;
		body->ApplyForceToCenter(b2Vec2(o->x,o->y),wake);
	}


	template<class... T>
	void RegComponent(Type<T...>) {
		(
				[]() {
					std::string_view OriginName = typeid(T).name();
					auto space = OriginName.rfind(':');
					auto name = fmt::format("Z.{}", OriginName.substr(space + 1));
					MonoType *type = mono_reflection_type_from_name(name.data(), ScriptEngine::GetCoreImage());
					if(!type) {
						Z_CORE_ERROR("No such type: {0}", name);
						return;
					}
					ReflectionMap[type] = [](Entity entity) -> bool { return entity.HasComponent<T>(); };
				}(), ...);
	}


	void ScriptReg::Reg() {
		Z_INTERNAL_FUNC(InternalCallPRT);
		Z_INTERNAL_FUNC(InternalCallWARN);
		Z_INTERNAL_FUNC(InternalCallDot);
		Z_INTERNAL_FUNC(GetTranslation);
		Z_INTERNAL_FUNC(SetTranslation);
		Z_INTERNAL_FUNC(IsKeyPressed);
		Z_INTERNAL_FUNC(Entity_HasComponent);
		Z_INTERNAL_FUNC(Entity_GetVelocity);
		Z_INTERNAL_FUNC(Entity_SetVelocity);
		Z_INTERNAL_FUNC(Entity_ApplyForce);
		Z_INTERNAL_FUNC(Entity_ApplyForceCenter);
	}

	void ScriptReg::RegComponents() {
		RegComponent(AllTypes{});
	}
};