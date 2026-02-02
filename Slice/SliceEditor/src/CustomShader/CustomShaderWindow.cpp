#include <pch.h>
#include "CustomShader/CustomShaderWindow.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Session/SessionManager.h"
#include "../../SliceEngine/src/Resource/Shader.h"

namespace SliceEditor
{
	namespace CST = SliceEngine::SliceEngineTypes;
	static std::unordered_map<CST::CSHAD_T, std::string> cShaderTypeName
	{
		{CST::CSHAD_T::BOOL, "bool"},
		{CST::CSHAD_T::INT, "int"},
		{CST::CSHAD_T::UINT, "uint"},
		{CST::CSHAD_T::FLOAT, "float"},
		{CST::CSHAD_T::VEC2, "vec2"},
		{CST::CSHAD_T::VEC3, "vec3"},
		{CST::CSHAD_T::VEC4, "vec4"}
	};
	CustomShaderWindow::~CustomShaderWindow()
	{
		ImNodes::EditorContextFree(*editor_context_this.get());
		ImNodes::EditorContextFree(*editor_context_other.get());
	}
	void CustomShaderWindow::CheckFileData()
	{
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		if (selectionManager->mSelectionType != SelectionType::SHADERGRAPH)
			return;

		auto& nodes = selectionManager->GetSelectedNodes();

		// if entities present
		if (nodes.size() > 0)
		{
			DirectoryNode* entityNode = static_cast<DirectoryNode*>(*nodes.begin());
			if (entityNode->fileName != mCurrCSPath)
			{
				// Actual Reading Starts Here

				mCurrCSPath = entityNode->fileName;
				std::ifstream fs(entityNode->fullPath);

				if (!fs)
				{
					SLICE_LOG_WARNING("Unable to open Custom Shader File - IMGUI");
					return;
				}
				nlohmann::json cshaderJson;
				try
				{
					cshaderJson = nlohmann::json::parse(fs);
				}
				catch (nlohmann::json::parse_error& e)
				{
					fs.close();
					SLICE_LOG_ERROR("Invalid cshader (IMGUI) JSON file" + std::string(e.what()));
					return;
				}
				fs.close();
				// -------------------------------------------------------------------
				mDefaultIns.clear();
				mEditableIns.clear();
				mDefaultOuts.clear();
				mStateNodes.clear();
				mTransitionNodes.clear();
				nodeTransitionCounter = 0;
				nodeIDCounter = 0;

				struct tempLinkIDContainer
				{
					int out;			// 1 out goes into
					std::queue<int> in; // many ins
				};
				std::unordered_map<std::string, tempLinkIDContainer> tempIDLinkGet;

				// Defaults
				auto copy = CST::dataIDS;
				for (const auto& dat : copy)
				{
					StateNode n;
					n.id = ++nodeIDCounter;
					n.name = dat.first;
					n.out_id = ++nodeTransitionCounter;
					mDefaultIns.insert(std::make_pair(n.id, n));
					tempIDLinkGet[dat.first].out = n.out_id;
				}

				// Editables
				nlohmann::json paramsJson = cshaderJson["Params"];
				if (paramsJson.contains("Floats"))
					for (auto& [name, components] : paramsJson["Floats"].items())
					{
						EditableNode n;
						n.id = ++nodeIDCounter;
						n.name = name;
						n.baseData = components.get<float>();
						n.out_id = ++nodeTransitionCounter;
						mEditableIns.insert(std::make_pair(n.id, n));
						tempIDLinkGet[name].out = n.out_id;
					}
				if (paramsJson.contains("Ints"))
					for (auto& [name, components] : paramsJson["Ints"].items())
					{
						EditableNode n;
						n.id = ++nodeIDCounter;
						n.name = name;
						n.baseData = components.get<int32_t>();
						n.out_id = ++nodeTransitionCounter;
						mEditableIns.insert(std::make_pair(n.id, n));
						tempIDLinkGet[name].out = n.out_id;
					}
				if (paramsJson.contains("Uints"))
					for (auto& [name, components] : paramsJson["Uints"].items())
					{
						EditableNode n;
						n.id = ++nodeIDCounter;
						n.name = name;
						n.baseData = components.get<uint32_t>();
						n.out_id = ++nodeTransitionCounter;
						mEditableIns.insert(std::make_pair(n.id, n));
						tempIDLinkGet[name].out = n.out_id;
					}
				if (paramsJson.contains("Bools"))
					for (auto& [name, components] : paramsJson["Bools"].items())
					{
						EditableNode n;
						n.id = ++nodeIDCounter;
						n.name = name;
						n.baseData = components.get<bool>();
						n.out_id = ++nodeTransitionCounter;
						mEditableIns.insert(std::make_pair(n.id, n));
						tempIDLinkGet[name].out = n.out_id;
					}

				nlohmann::json mainColorJson = cshaderJson["ColorMain"];
				for (auto& [funcName, components] : mainColorJson.items())
				{
					auto funcDetails = CST::cShaderFuncsTemplates.find(funcName)->second;
					
					// Special Case
					if (funcName == "END")
					{
						for (auto& [id, dependencies] : components.items())
						{
							std::vector<std::string> dep;
							dependencies.get_to(dep);
							StateNode n;
							n.id = ++nodeIDCounter;
							n.name = funcName + "_COLOR";
							n.in_id = ++nodeTransitionCounter;
							mDefaultOuts.insert(std::make_pair(n.id, n));
							tempIDLinkGet[dep[0]].in.push(n.in_id);
						}
						continue;
					}

					for (auto& [id, dependencies] : components.items())
					{
						std::vector<std::string> dep;
						dependencies.get_to(dep);
						CStateNode n;
						n.id = ++nodeIDCounter;
						n.name = funcName;
						const auto& funcDets = CST::cShaderFuncsTemplates.find(n.name)->second;
						for (int i{}; i < funcDets.inIDs.size(); ++i)
						{
							int32_t tID = ++nodeTransitionCounter;
							tempIDLinkGet[dep[i]].in.push(tID);
							n.inIDs.push_back(tID);
						}
						{
							int32_t tID = ++nodeTransitionCounter;
							n.out_id = tID;
							tempIDLinkGet[id].out = tID;
						}
						mStateNodes.insert(std::make_pair(n.id, n));
					}
				}

				for (auto& [oldFuncID, linkNodes] : tempIDLinkGet)
				{
					while (!linkNodes.in.empty())
					{
						TransitionLinkNode n;
						n.id = ++linkIDCounter;
						n.source_id = linkNodes.out;
						n.target_id = linkNodes.in.front();
						linkNodes.in.pop();
						mTransitionNodes.insert(std::make_pair(n.id, n));
					}
				}
				/*
				/*
				nlohmann::json RoughnessMetJson = cshaderJson["RoughMetMain"];
				*/
				tempLoadPos = true;
			}
		}
	}
	void CustomShaderWindow::Init()
	{
		mSelectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		mSessionManager = mRegistry.GetManager<SessionManager>("Session");
		editor_context_this = std::make_unique<ImNodesEditorContext*>(ImNodes::EditorContextCreate());
		editor_context_other = std::make_unique<ImNodesEditorContext*>(ImNodes::EditorContextCreate());
	}

	void CustomShaderWindow::create_default()
	{
		//SliceEngine::SliceEngineTypes::cShaderPredefines.find("");
	}

	void CustomShaderWindow::Draw()
	{
		CheckFileData();

		ImGui::Begin("Shader Graph");
		if(ImGui::Button("Save"))
		{
			//SliceEngine::Handle<SliceEngine::SliceEngineTypes::Texture> handle = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Texture>("Textures/Gideon.png");
			//auto texture = handle.get();
			//ImGui::Image(static_cast<ImU64>(texture->texture_id), ImGui::GetWindowSize());
		}

		ImNodes::EditorContextSet(*editor_context_this.get());
		ImNodes::BeginNodeEditor();

		for(auto& i : mStateNodes)
			DrawStateNode(i.second);
		for(auto& i : mDefaultIns)
			DrawDefaultInNode(i.second);
		for(auto& i : mDefaultOuts)
			DrawDefaultOutNode(i.second);
		for(auto& i : mEditableIns)
			DrawEditableInNode(i.second);
		for (auto& i : mTransitionNodes)
			DrawTransitionNodes(i.second);

		if (tempLoadPos)
			TempLoadPosAll();

		// must be called right before EndNodeEditor
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopLeft);
		DrawPostEditorElements();
		ImNodes::EndNodeEditor();
		PostEditorChecks();

		ImNodes::EditorContextSet(*editor_context_other.get());
		ImGui::End();
	}

	void CustomShaderWindow::DrawDefaultInNode(StateNode& node)
	{
		ImNodes::BeginNode(node.id);
		ImGui::TextUnformatted(node.name.c_str());

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::Text(cShaderTypeName[CST::dataIDS[node.name]].c_str());
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();

	}
	void CustomShaderWindow::DrawDefaultOutNode(StateNode& node)
	{
		ImNodes::BeginNode(node.id);
		ImGui::TextUnformatted(node.name.c_str());

		ImGui::SameLine();
		ImNodes::BeginInputAttribute(node.in_id);
		if(node.name == "END_COLOR")
			ImGui::Text(cShaderTypeName[CST::CSHAD_T::VEC4].c_str());
		else if(node.name == "END_MET_ROUGH")
			ImGui::Text(cShaderTypeName[CST::CSHAD_T::VEC2].c_str());
		ImNodes::EndInputAttribute();

		ImNodes::EndNode();

	}

	void CustomShaderWindow::DrawEditableInNode(EditableNode& node)
	{
		ImNodes::BeginNode(node.id);
		ImGui::TextUnformatted(node.name.c_str());

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::PushItemWidth(50.f);
		if (std::holds_alternative<float>(node.baseData))
		{
			float temp = std::get<float>(node.baseData);
			ImGui::DragFloat("", &temp, 0.01f);
			node.baseData = temp;
		}
		if (std::holds_alternative<uint32_t>(node.baseData))
		{
			int temp = static_cast<int>(std::get<uint32_t>(node.baseData));
			ImGui::DragInt("", &temp);
			node.baseData = static_cast<int>(temp);
		}
		if (std::holds_alternative<int32_t>(node.baseData))
		{
			int temp = std::get<int32_t>(node.baseData);
			ImGui::DragInt("", &temp);
			node.baseData = temp;
		}
		if (std::holds_alternative<bool>(node.baseData))
		{
			bool temp = std::get<bool>(node.baseData);
			ImGui::Checkbox("", &temp);
			node.baseData = temp;
		}
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawStateNode(CStateNode& node)
	{
		ImNodes::BeginNode(node.id);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(node.name.c_str());
		ImNodes::EndNodeTitleBar();

		const auto& funcDets = CST::cShaderFuncsTemplates.find(node.name)->second;
		for (size_t i{}; i < funcDets.inIDs.size(); ++i)
		{
			ImNodes::BeginInputAttribute(node.inIDs[i]);
			ImGui::Text(cShaderTypeName[funcDets.inIDs[i]].c_str());
			ImNodes::EndInputAttribute();
		}
		//ImGui::SameLine();

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::Text(cShaderTypeName[funcDets.outType].c_str());
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawTransitionNodes(TransitionLinkNode& n)
	{
		if (n.source_id == 0 || n.target_id == 0)
			return;
		ImNodes::Link(n.id, n.source_id, n.target_id);
	}

	void CustomShaderWindow::DrawPostEditorElements()
	{
		if (mCurrCSPath != "")
		{
			if (ImNodes::IsEditorHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("NodeEditor_Popup");
				}
			}

			if (ImGui::BeginPopup("Node_Popup"))
			{
				if (ImGui::Selectable("Make Entry State"))
				{

				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("Link_Popup"))
			{
				if (ImGui::Selectable("Delete"))
				{

				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopup("NodeEditor_Popup"))
			{
				if (ImGui::Selectable("Create Node"))
				{

				}

				ImGui::EndPopup();
			}
		}
	}
	void CustomShaderWindow::TempLoadPosAll()
	{
		for (auto& i : mStateNodes)
			InitNodePos(i.first);
		for (auto& i : mDefaultIns)
			InitNodePos(i.first);
		for (auto& i : mDefaultOuts)
			InitNodePos(i.first);
		for (auto& i : mEditableIns)
			InitNodePos(i.first);
		tempLoadPos = false;
	}
	void CustomShaderWindow::InitNodePos(int id)
	{
		ImVec2 pos{ 0.f + 200.f * (id / 6), 0.f + 50.f * (id % 6)}; // -ve is go up
		ImNodes::SetNodeEditorSpacePos(id, pos);
		ImNodes::SnapNodeToGrid(id);
	}
	void CustomShaderWindow::PostEditorChecks()
	{
		int id_attr, start_attr, end_attr;
		if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
		{
			TransitionLinkNode n;
			n.source_id = start_attr;
			n.target_id = end_attr;
			n.id = ++nodeTransitionCounter;
			mTransitionNodes.insert(std::make_pair(n.id, n));
		}
		if (ImNodes::IsLinkDestroyed(&id_attr))
		{
			mTransitionNodes.erase(id_attr);
		}
	}
}