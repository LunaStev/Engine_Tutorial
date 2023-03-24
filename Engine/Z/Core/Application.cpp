//
// Created by 32725 on 2023/3/11.
//

#include "Application.h"
#include "Z/Events/ApplicationEvent.h"
#include "Log.h"
#include "Input.h"
#include "Time.h"
#include "Z/Renderer/Renderer.h"
#include "KeyCodes.h"

namespace Z {
	Application* Application::application=nullptr;

	Application::Application(const std::string&name){
		Z_CORE_ASSERT(!application,"Application already exists!")
		application=this;
		window=Z::Scope<zWindow>(zWindow::Create(WindowProps(name)));
		window->SetEventCallFunc(Z_BIND_EVENT_FUNC(Application::EventCall));

		Renderer::Init();

		window->SetVSync(false);
		imguiLayer=new ImGuiLayer();
		PushLayer(imguiLayer);
	}


	Application::~Application() {

	}

	void Application::Run() {
		while(Running){
			Z::Time::Update();
			imguiLayer->Begin();
			if(!MinSize){
				for (Layer *layer: LayerStack) {
					layer->OnUpdate();
				}
			}
			for(Layer* layer:LayerStack){
				layer->OnImGuiRender();
			}
			imguiLayer->End();

			window->Update();
		}
	}

	void Application::EventCall(Event &e) {
		EventDispatcher dispatcher(e);
		dispatcher.Handle<WindowCloseEvent>(Z_BIND_EVENT_FUNC(Application::OnWindowClose));
		dispatcher.Handle<WindowResizeEvent>(Z_BIND_EVENT_FUNC(Application::OnWindowResize));
		for(auto it=LayerStack.end();it!=LayerStack.begin();){
			(*--it)->OnEvent(e);
			if(e.Handled){
				break;
			}
		}
	}

	void Application::PushLayer(Layer *layer) {
		LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer *overlay) {
		LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	Application &Application::Get() {
		return *application;
	}

	bool Application::OnWindowClose(WindowCloseEvent &e) {
		Running=false;
		Z_CORE_INFO("{0}",e);
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent &e) {
		if(e.GetWidth()==0||e.GetHeight()==0){
			MinSize=true;
			return false;
		}
		MinSize=false;
		Renderer::OnWindowResize(e.GetWidth(),e.GetHeight());
		return false;
	}

}