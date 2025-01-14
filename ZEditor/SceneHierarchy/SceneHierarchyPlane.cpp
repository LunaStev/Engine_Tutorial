//
// Created by 32725 on 2023/3/28.
//
#include "Z/Scene/Components.h"
#include "SceneHierarchyPlane.h"
#include "Z/Script/ScriptEngine.h"
#include <filesystem>
#include "imgui/imgui.h"
#include "imgui_internal.h"

namespace Z {

	namespace Temp {

		template<class T>
		std::string GetTypeName() {
			auto temp=std::string(typeid(T).name());
			auto space = temp.rfind(':');
			auto end=temp.rfind("Component");
			return temp.substr(space + 1, end - space - 1);
		}

		template<class... T>
		void AddComponentMenu(Type<T...>, Entity entity) {
			([&]() {
				if (!entity.HasComponent<T>() && ImGui::MenuItem(GetTypeName<T>().c_str())) {
					entity.AddComponent<T>();
				}
			}(), ...);
		}
	}

	static void
	MyDrawVec(const std::string &label, glm::vec3 &value, const glm::vec3& ResetValue = glm::vec3{.0f}, float ColumnWidth = 100.f) {
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, ColumnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, {.0f, 0, .5f, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {.0f, 0, 1.f, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {.0f, 0, .5f, 1});
		ImGui::PushStyleColor(ImGuiCol_Button, {.0f, .5f, .0f, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {.0f, 1.f, .0f, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {.0f, .5f, .0f, 1});
		ImGui::PushStyleColor(ImGuiCol_Button, {.5f, 0, 0, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1.f, 0, 0, 1});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {.5f, 0, 0, 1});
		static const char *ButtonLabel[] = {"X", "Y", "Z"};
		static const char *DragLabel[] = {"##X", "##Y", "##Z"};

		auto &io = ImGui::GetIO();
		ImGui::PushFont(io.Fonts->Fonts[0]);
		for (int i = 0; i < 3; ++i) {
			if (ImGui::Button(ButtonLabel[i], buttonSize)) { value[i] = ResetValue[i]; }
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::DragFloat(DragLabel[i], &value[i], 0.1f);
			if (i != 2)
				ImGui::SameLine();
			ImGui::PopItemWidth();
		}
		ImGui::PopFont();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	void SceneHierarchyPlane::OnImGuiRender() {
		ImGui::Begin("Scene Hierarchy");
		context->registry.each([&](auto entityID) {
			Entity entity{entityID, context.get()};
			DrawEntity(entity);
		});
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			selectedEntity = {};
		}
		if (ImGui::BeginPopupContextWindow(nullptr,
		                                   ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
			if (ImGui::MenuItem("Create Empty Entity")) {
				context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();

		ImGui::Begin("Inspector");

		if (selectedEntity) {
			DrawComponents(selectedEntity);
		}
		ImGui::End();
	}

	void SceneHierarchyPlane::DrawEntity(Entity entity) {
		auto &tag = entity.GetComponent<TagComponent>().tag;
		ImGuiTreeNodeFlags flags =
				((selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void *) (uint64_t) (uint32_t) entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			selectedEntity = entity;
		}

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity")) {
				if (selectedEntity == entity)
					selectedEntity = {};
				context->DestroyEntity(entity);
			}
			ImGui::EndPopup();
		}

		if (opened) {
			ImGui::TreePop();
		}
	}


	template<>
	void SceneHierarchyPlane::DrawComponent<TagComponent>(const std::string &name, Entity entity,
	                                                      void (*drawFunc)(Entity entity,TagComponent &)) {
		auto &component = entity.GetComponent<TagComponent>();
		drawFunc(entity,component);
	}

	template<>
	void SceneHierarchyPlane::DrawComponent<TransformComponent>(const std::string &name, Entity entity,
	                                                            void (*drawFunc)(Entity entity,TransformComponent &)) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void *) typeid(TransformComponent).hash_code(),
		                              ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
		                              ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth,
		                              name.c_str());
		ImGui::PopStyleVar();

		if (open) {
			auto &component = entity.GetComponent<TransformComponent>();
			drawFunc(entity,component);
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPlane::DrawComponents(Entity entity) {
		DrawComponent<TagComponent>("Tag", entity, [](Entity entity,auto &tag) {
			static char buffer[256];
			memset(buffer, 0, 256);
			strcpy(buffer, tag.tag.c_str());
			if (ImGui::InputText("##Tag", &buffer[0], 256))
				tag = std::string(buffer);
		});

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponent");
		}
		if (ImGui::BeginPopup("AddComponent")) {
			Temp::AddComponentMenu(AllTypes{},entity);
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](Entity entity,auto &transform) {
			MyDrawVec("Position", transform.translation);
			auto rotation = glm::degrees(transform.rotation);
			MyDrawVec("Rotation", rotation);
			transform.rotation = glm::radians(rotation);
			MyDrawVec("Scale", transform.scale, glm::vec3{1.f});
		});
		DrawComponent<SpriteRendererComponent>("SpriteRenderer", entity,
		                                       [](Entity entity,SpriteRendererComponent &spriteRenderer) {
			                                       ImGui::ColorEdit4("Color", &spriteRenderer.color[0]);
			                                       ImGui::Image(spriteRenderer.texture.get() == nullptr ? nullptr :
			                                                    (void *) spriteRenderer.texture->GetRendererID(),
			                                                    ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
			                                       if (ImGui::BeginDragDropTarget()) {
				                                       if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(
						                                       "CONTENT_BROWSER_ITEM")) {
					                                       const char *path = (const char *) payload->Data;
					                                       Z_CORE_ASSERT(std::filesystem::exists(path),
					                                                     "Path does not exist!");
					                                       auto temp = Texture2D::CreateTexture(path);
					                                       if (temp->GetWidth() > 2600 || temp->GetHeight() > 2600) {
						                                       MessageBoxA(nullptr, "Texture is too large!", "Error",
						                                                   MB_OK);
					                                       } else
						                                       spriteRenderer.texture = temp;
				                                       }
				                                       ImGui::EndDragDropTarget();
			                                       }
		                                       });
		DrawComponent<CircleRendererComponent>("CircleRenderer", entity, [](Entity entity,CircleRendererComponent &component) {
			ImGui::ColorEdit4("Color", &component.color[0]);
			ImGui::DragFloat("thick", &component.thickness, .05f, 0.f, 1.f);
			ImGui::DragFloat("Fade", &component.fade, .05f, 0.f, 1.f);
		});
		DrawComponent<CameraComponent>("Camera", entity, [](Entity entity,CameraComponent &component) {
			auto &camera = component.camera;
			if (ImGui::Checkbox("Primary", &component.primary)) {//Todo:Change logic
			}
			const char *projectionTypeStrings[] = {"Perspective", "Orthographic"};
			int projectionType = (int) camera.GetProjectionType();

			if (ImGui::Combo("Projection", &projectionType, projectionTypeStrings, 2)) {
				if ((int) (camera.GetProjectionType()) != projectionType) {
					camera.SetProjectionType((SceneCamera::ProjectionType) projectionType);
				}
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float verticalFOV = glm::degrees(camera.GetPerspectiveFOV());
				if (ImGui::DragFloat("Vertical FOV", &verticalFOV)) {
					camera.SetPerspectiveFOV(glm::radians(verticalFOV));
				}
				float nearClip = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip)) {
					camera.SetPerspectiveNearClip(nearClip);
				}
				float farClip = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip)) {
					camera.SetPerspectiveFarClip(farClip);
				}
			} else {
				float orthographicSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthographicSize)) {
					camera.SetOrthographicSize(orthographicSize);
				}
				float nearClip = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip)) {
					camera.SetOrthographicNearClip(nearClip);
				}
				float farClip = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip)) {
					camera.SetOrthographicFarClip(farClip);
				}
			}
		});
		DrawComponent<RigidBody2DComponent>("RigidBody2D", entity, [](Entity entity,RigidBody2DComponent &component) {
			ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
			if (int i = (int) component.bodyType;ImGui::Combo("Body Type", &i,
			                                                  "Static\0Dynamic\0Kinematic\0\0"))
				component.bodyType = (RigidBody2DComponent::BodyType) i;
		});
		DrawComponent<BoxCollider2DComponent>("BoxCollider2D", entity, [](Entity entity,BoxCollider2DComponent &component) {
			ImGui::Checkbox("Is Trigger", &component.isTrigger);
			ImGui::DragFloat2("Size", &component.size.x,.05f);
			ImGui::DragFloat2("Offset", &component.offset.x,.05f);
			ImGui::DragFloat("Density", &component.density, .05f);
			ImGui::DragFloat("Friction", &component.friction, .05f);
			ImGui::DragFloat("Restitution", &component.restitution, .05f);
			ImGui::DragFloat("MinRestitution", &component.MinRestitution, .05f);
		});
		DrawComponent<CircleCollider2DComponent>("CircleCollider2D", entity, [](Entity entity,CircleCollider2DComponent &component) {
			ImGui::Checkbox("Is Trigger", &component.isTrigger);
			ImGui::DragFloat("Radius", &component.radius,.1f);
			ImGui::DragFloat2("Offset", &component.offset.x,.05f);
			ImGui::DragFloat("Density", &component.density, .1f);
			ImGui::DragFloat("Friction", &component.friction, .05f);
			ImGui::DragFloat("Restitution", &component.restitution, .05f);
			ImGui::DragFloat("MinRestitution", &component.MinRestitution, .05f);
		});
		DrawComponent<ScriptComponent>("Script",entity,[](Entity entity,ScriptComponent&component){
			static char buffer[257];
			std::strcpy(buffer,component.scriptName.data());
			bool exists=ScriptEngine::ClassExists(component.scriptName);
			ImGui::Text("Script Name:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if(!exists)
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(.8f,.1f,.2f,1));
			if(ImGui::InputText("##ScriptName",buffer,256)){
				component.scriptName=buffer;
				ImGui::OpenPopup("ScriptNameList");
			}
			if(!exists)
				ImGui::PopStyleColor();
			if(ImGui::BeginPopup("ScriptNameList",ImGuiWindowFlags_NoFocusOnAppearing)){
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(.1f,.1f,.8f,1));
				if(ImGui::Selectable("New"))
					component.scriptName="New";
				for(const auto&[name,script]:ScriptEngine::GetScriptList()){
					if(ImGui::Selectable(name.c_str()))
						component.scriptName=name;
				}
				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
			if(exists){
				auto instance=ScriptEngine::GetInstance(entity.GetUID());
				if(!instance)
					return;
				auto klass=instance->GetClass();
				auto&fields=klass->GetFields();
				for(const auto&[name,type]:fields){
					ImGui::Text(name.c_str());
					ImGui::SameLine();
					switch (type.Type) {
						case ScriptFieldType::Float:{
							auto ptr=instance->GetValue<float>(name);
							if(ImGui::DragFloat(("##"+name).c_str(),&ptr,.5f)){
								instance->SetValue(name,&ptr);
							}
							break;
						}
						case ScriptFieldType::Int:{
							auto ptr=instance->GetValue<int>(name);
							if(ImGui::DragInt(("##"+name).c_str(),&ptr)){
								instance->SetValue(name,&ptr);
							}
							break;
						}
						case ScriptFieldType::Float2:{
							auto ptr=instance->GetValue<glm::vec2>(name);
							if(ImGui::DragFloat2(("##"+name).c_str(),&ptr[0])){
								instance->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Float3:{
							auto ptr=instance->GetValue<glm::vec3>(name);
							if(ImGui::DragFloat3(("##"+name).c_str(),&ptr[0])){
								instance->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Float4:{
							auto ptr=instance->GetValue<glm::vec4>(name);
							if(ImGui::DragFloat4(("##"+name).c_str(),&ptr[0])){
								instance->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Bool:{
							auto ptr=instance->GetValue<bool>(name);
							if(ImGui::Checkbox(("##"+name).c_str(),&ptr)){
								instance->SetValue(name,&ptr);
							}
							break;
						}
					}
				}

			}
/*			if(exists){
				auto klass=ScriptEngine::GetScriptList().at(component.scriptName);
				auto&fields=klass->GetFields();
				for(const auto&[name,type]:fields){
					ImGui::Text(name.c_str());
					ImGui::SameLine();
					switch (type.Type) {
						case ScriptFieldType::Float:{
							auto ptr=klass->GetValue<float>(name);
							if(ImGui::DragFloat(("##"+name).c_str(),&ptr,.5f)){
								klass->SetValue(name,&ptr);
							}
							break;
						}
						case ScriptFieldType::Int:{
							auto ptr=klass->GetValue<int>(name);
							if(ImGui::DragInt(("##"+name).c_str(),&ptr)){
								klass->SetValue(name,&ptr);
							}
							break;
						}
						case ScriptFieldType::Float2:{
							auto ptr=klass->GetValue<glm::vec2>(name);
							if(ImGui::DragFloat2(("##"+name).c_str(),&ptr[0])){
								klass->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Float3:{
							auto ptr=klass->GetValue<glm::vec3>(name);
							if(ImGui::DragFloat3(("##"+name).c_str(),&ptr[0])){
								klass->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Float4:{
							auto ptr=klass->GetValue<glm::vec4>(name);
							if(ImGui::DragFloat4(("##"+name).c_str(),&ptr[0])){
								klass->SetValue(name,&ptr[0]);
							}
							break;
						}
						case ScriptFieldType::Bool:{
							auto ptr=klass->GetValue<bool>(name);
							if(ImGui::Checkbox(("##"+name).c_str(),&ptr)){
								klass->SetValue(name,&ptr);
							}
							break;
						}
					}
				}
			}*/
		});
	}

	template<typename Ty>
	void SceneHierarchyPlane::DrawComponent(const std::string &name, Entity entity, void (*drawFunc)(Entity,Ty &)) {
		if (entity.HasComponent<Ty>()) {
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			auto RegionWidth = ImGui::GetContentRegionAvail().x;
			auto lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void *) typeid(Ty).hash_code(),
			                              ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
			                              ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth,
			                              name.c_str());
			ImGui::PopStyleVar();
			bool remove = false;

			ImGui::SameLine(RegionWidth - lineHeight * .5f);
			if (ImGui::Button("+", ImVec2(lineHeight, lineHeight))) {
				ImGui::OpenPopup("Remove Component");
			}
			if (ImGui::BeginPopup("Remove Component")) {
				if (ImGui::MenuItem("Remove Component")) {
					remove = true;
				}
				ImGui::EndPopup();
			}
			if (open) {
				auto &component = entity.GetComponent<Ty>();
				drawFunc(entity,component);
				ImGui::TreePop();
			}
			if (remove) {
				entity.RemoveComponent<Ty>();
			}
		}
	}

}



