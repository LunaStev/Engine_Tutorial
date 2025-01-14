//
// Created by 32725 on 2023/4/6.
//

#ifndef ENGINE_TUTORIAL_SCRIPTENGINE_H
#define ENGINE_TUTORIAL_SCRIPTENGINE_H

#include "Z/Core/Core.h"
#include "Z/Core/Log.h"
#include "Z/Core/GUID.h"
#include "Z/Scene/Scene.h"

extern "C" {
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClassField MonoClassField;
};

namespace Z {

	enum class ScriptFieldType {
		Char, Byte,
		Int16, UInt16, Int, UInt, Long, ULong,
		Float, Double, Float2, Float3, Float4,
		Bool, String, Entity,
	};

	struct ScriptField {
		std::string Name;
		ScriptFieldType Type;
		MonoClassField *Field;
	};

	class ScriptClass {
	public:
		ScriptClass(const std::string &nameSpace, const std::string &name);

		MonoObject *GetInstance();

		MonoMethod *GetMethod(const std::string &MethodName, int paramCount = 0);

		void InvokeMethod(MonoMethod *method, MonoObject *object, void **params = nullptr, MonoObject **exc = nullptr);

		bool IsSubClassOf(const Ref<ScriptClass> &klass);

		uint32_t GetMethodCount();

		const auto &GetFields() const { return Fields; }

		template<class T>
		T GetValue(const std::string&name){
			InnerGetValue(name,buffer);
			return *(T*)buffer;
		}

		void SetValue(const std::string&name,void*ptr);
		[[nodiscard]] MonoClass *GetClass() const { return Class; }

	private:
		void InnerGetValue(const std::string&name,void *ptr);
		std::unordered_map<std::string, ScriptField> Fields;
		MonoClass *Class;
		std::string NameSpace, ClassName;
		static unsigned char buffer[64];

		friend class ScriptEngine;
	};

	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> klass, Entity entity);

		void OnCreate();

		void OnUpdate(Entity entity, float deltaTime);

		Ref<ScriptClass> GetClass() const { return Class; }


		void SetValue(const std::string &name, void *ptr);

		template<class T>
		T GetValue(const std::string &name) {
			GetValue(name, buffer);
			return *(T*)buffer;
		}

	private:
		void GetValue(const std::string &name, void *ptr);
		Ref<ScriptClass> Class;
		MonoObject *instance;
		MonoMethod *construct, *create, *update;
		static unsigned char buffer[64];
	};

	class ScriptEngine {
	public:
		ScriptEngine() = default;

		virtual ~ScriptEngine() = default;

		static void Init();

		static void ShutDown();

		static void LoadAssembly(const std::filesystem::path &path);

		static void OnRuntimeStart(Scene *scene);

		static void OnRuntimeUpdate(Entity entity, float deltaTime);

		static void OnRuntimeStop();

		static void CreateInstance(Entity entity);

		static Scene *GetContext();

		static const std::unordered_map<std::string, Ref<ScriptClass>> &GetScriptList();

		static Ref<ScriptInstance> GetInstance(GUID id);

		static bool ClassExists(const std::string &name);

		static MonoImage *GetCoreImage();

	private:
		static void MonoInit();

		static void MonoShutDown();

		static void GetClasses(MonoAssembly *);

		static MonoObject *GetInstance(MonoClass *);

		static void LoadCoreAssembly(MonoAssembly *assembly);


		friend class ScriptClass;

		friend class ScriptReg;
	};

}


#endif //ENGINE_TUTORIAL_SCRIPTENGINE_H
