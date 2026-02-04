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
		//if ( != SelectionType::SHADERGRAPH)
		//	return;
		//auto& nodes = mSelectionManager->GetSelectedNodes();
		//DirectoryNode* entityNode = static_cast<DirectoryNode*>(*nodes.begin());
		auto shaderGraphGUID = mSessionManager->GetShaderGraphInspected();

		if (!shaderGraphGUID.IsValid())
			return;

		// if entities present
		if (shaderGraphGUID != mCurrShaderGraphGUID)
		{
			mCurrShaderGraphGUID = shaderGraphGUID;
			auto filenameOpt = mRegistry.GetAssetManager().GetFilenameFromGUID(shaderGraphGUID);

			if (!filenameOpt.has_value())
				return;

			std::filesystem::path filePath = mRegistry.GetAssetManager().mAssetDirectory;
			filePath /= filenameOpt.value();

			// Actual Reading Starts Here
			std::ifstream fs(filePath.string());
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
			//uniqueIDCnt = 0;

			colorExitNodeID = 0;
			roughMetExitNodeID = 0;

			mDefaultIns.clear();
			mEditableIns.clear();
			mStateNodes.clear();
			mTransitionNodes.clear();

			attrIDToNodeID.clear();
			attrIDToLinkID.clear();

			struct tempLinkIDContainer
			{
				int source_attr;			// 1 out goes into
				std::queue<int> dest_attr; // many ins
			};
			std::unordered_map<std::string, tempLinkIDContainer> tempIDLinkGet;// Func Name

			// Defaults
			auto copy = CST::dataIDS;
			for (const auto& dat : copy)
			{
				ShaderStateNode n;
				n.id = ++uniqueIDCnt;
				n.name = dat.first;
				n.out_id = ++uniqueIDCnt;

				mDefaultIns[n.id] = n;
				attrIDToNodeID[n.out_id] = n.id;
				tempIDLinkGet[dat.first].source_attr = n.out_id;
			}

			// Editables
			nlohmann::json paramsJson = cshaderJson["Params"];
			if (paramsJson.contains("Floats"))
				for (auto& [name, components] : paramsJson["Floats"].items())
				{
					ShaderEditableNode n;
					n.id = ++uniqueIDCnt;
					n.out_id = ++uniqueIDCnt;
					n.name = name;
					n.baseData = components.get<float>();

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}
			if (paramsJson.contains("Ints"))
				for (auto& [name, components] : paramsJson["Ints"].items())
				{
					ShaderEditableNode n;
					n.id = ++uniqueIDCnt;
					n.out_id = ++uniqueIDCnt;
					n.name = name;
					n.baseData = components.get<int32_t>();

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}
			if (paramsJson.contains("Uints"))
				for (auto& [name, components] : paramsJson["Uints"].items())
				{
					ShaderEditableNode n;
					n.id = ++uniqueIDCnt;
					n.out_id = ++uniqueIDCnt;
					n.name = name;
					n.baseData = components.get<uint32_t>();

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}
			if (paramsJson.contains("Bools"))
				for (auto& [name, components] : paramsJson["Bools"].items())
				{
					ShaderEditableNode n;
					n.id = ++uniqueIDCnt;
					n.out_id = ++uniqueIDCnt;
					n.name = name;
					n.baseData = components.get<bool>();

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}

			nlohmann::json mainColorJson = cshaderJson["Main"];
			for (auto& [funcName, components] : mainColorJson.items())
			{
				const auto& funcDets = CST::cShaderFuncsTemplates.find(funcName)->second;
				for (auto& [id, dependencies] : components.items())
				{
					std::vector<std::string> dep;
					dependencies.get_to(dep);
					ShaderStateNode n;
					n.id = ++uniqueIDCnt;
					n.name = funcName;
					// Ins
					for (int i{}; i < funcDets.inIDs.size(); ++i)
					{
						int tID = ++uniqueIDCnt;
						n.in_ids.push_back(tID);

						attrIDToNodeID[tID] = n.id;
						tempIDLinkGet[dep[i]].dest_attr.push(tID);
					}
					// Special Case (No Out)
					if (funcName == "END_COLOR" || funcName == "END_MET_ROUGH")
					{
						if (funcName == "END_COLOR")
							colorExitNodeID = n.id;
						else
							roughMetExitNodeID = n.id;
					}
					// Out
					else
					{
						int tID = ++uniqueIDCnt;
						n.out_id = tID;

						attrIDToNodeID[tID] = n.id;
						tempIDLinkGet[id].source_attr = tID;
					}
					mStateNodes[n.id] = n;
				}
			}
			// After all functions have been loaded
			for (auto& [oldFuncID, linkNodes] : tempIDLinkGet)
			{
				while (!linkNodes.dest_attr.empty())
				{
					ShaderLinkNode n;
					n.id = ++uniqueIDCnt;
					n.sourceAttr = linkNodes.source_attr;
					n.destAttr = linkNodes.dest_attr.front();
					linkNodes.dest_attr.pop();

					attrIDToLinkID[n.sourceAttr] = n.id;
					attrIDToLinkID[n.destAttr] = n.id;
					mTransitionNodes[n.id] = n;
				}
			}
			tempLoadPos = true;
		}
	}

	void CustomShaderWindow::SaveFileData()
	{
		if (mCurrShaderGraphGUID.IsValid())
		{
			nlohmann::json shaderGraphJson;
			nlohmann::json paramsJson;

			paramsJson["Floats"] = nlohmann::json::object();
			paramsJson["Ints"] = nlohmann::json::object();
			paramsJson["Uints"] = nlohmann::json::object();
			paramsJson["Bools"] = nlohmann::json::object();

			for (auto& [id, node] : mEditableIns)
			{
				if (std::holds_alternative<float>(node.baseData))
					paramsJson["Floats"][node.name] = std::get<float>(node.baseData);

				else if (std::holds_alternative<int32_t>(node.baseData))
					paramsJson["Ints"][node.name] = std::get<int32_t>(node.baseData);

				else if (std::holds_alternative<uint32_t>(node.baseData))
					paramsJson["Uints"][node.name] = std::get<uint32_t>(node.baseData);

				else if (std::holds_alternative<bool>(node.baseData))
					paramsJson["Bools"][node.name] = std::get<bool>(node.baseData);
			}
			shaderGraphJson["Params"] = paramsJson;

			// Color Main
			std::queue<ShaderStateNode> nodesLeftToCheck;

			nlohmann::json colorMainJson;
			if(colorExitNodeID != 0)
				nodesLeftToCheck.push(mStateNodes.at(colorExitNodeID));
			if(roughMetExitNodeID != 0)
				nodesLeftToCheck.push(mStateNodes.at(roughMetExitNodeID));
			while (!nodesLeftToCheck.empty())
			{
				ShaderStateNode node = nodesLeftToCheck.front();
				nodesLeftToCheck.pop();
				std::vector<std::string> dependenciesName;
				for (auto& i : node.in_ids)
				{
					auto linkID = attrIDToLinkID.find(i);
					if (linkID != attrIDToLinkID.end())
					{
						auto linkNode = mTransitionNodes.find(linkID->second);
						if (linkNode != mTransitionNodes.end())
						{
							auto sourceNodeID = attrIDToNodeID.find(linkNode->second.sourceAttr);
							if (sourceNodeID != attrIDToNodeID.end())
							{
								auto sourceNode = mStateNodes.find(sourceNodeID->second);
								if (sourceNode != mStateNodes.end())
								{
									dependenciesName.push_back("Node" + std::to_string(sourceNode->second.id));
									nodesLeftToCheck.push(sourceNode->second);
								}
								else
								{
									auto editableNode = mEditableIns.find(sourceNodeID->second);
									if (editableNode != mEditableIns.end())
									{
										dependenciesName.push_back(editableNode->second.name);
									}
									else
									{
										auto defaultNode = mDefaultIns.find(sourceNodeID->second);
										if (defaultNode != mDefaultIns.end())
										{
											dependenciesName.push_back(defaultNode->second.name);
										}
									}
								}
							}
						}
					}
				}
				colorMainJson[node.name]["Node" + std::to_string(node.id)] = dependenciesName;
			}
			shaderGraphJson["Main"] = colorMainJson;

			auto filenameOpt = mRegistry.GetAssetManager().GetFilenameFromGUID(mCurrShaderGraphGUID);

			if (!filenameOpt.has_value())
				return;

			std::filesystem::path filePath = mRegistry.GetAssetManager().mAssetDirectory;
			filePath /= filenameOpt.value();

			// Write
			std::ofstream ofs(filePath.string());
			if (!ofs.is_open())
			{
				SLICE_LOG_WARNING("Unable to save Custom Shader File - IMGUI");
				return;
			}
			ofs << shaderGraphJson.dump(4);
			ofs.close();
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
	
#pragma region Drawing
	void CustomShaderWindow::Draw()
	{
		CheckFileData();

		ImGui::Begin("Shader Graph");
		if(ImGui::Button("Save"))
		{
			SaveFileData();
			mSelectionManager->ClearSelection();
			//SliceEngine::Handle<SliceEngine::SliceEngineTypes::Texture> handle = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Texture>("Textures/Gideon.png");
			//auto texture = handle.get();
			//ImGui::Image(static_cast<ImU64>(texture->texture_id), ImGui::GetWindowSize());
		}

		ImNodes::EditorContextSet(*editor_context_this.get());
		ImNodes::BeginNodeEditor();

		// Ins
		for(auto& i : mDefaultIns)
			DrawDefaultInNode(i.second);
		for(auto& i : mEditableIns)
			DrawEditableInNode(i.second);
		// Mids
		for(auto& i : mStateNodes)
			DrawStateNode(i.second);
		// Transitions
		for (auto& i : mTransitionNodes)
			DrawTransitionNodes(i.second);

		// must be called right before EndNodeEditor
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopLeft);

		ImNodes::EndNodeEditor();

		DrawPostEditorElements();
		PostEditorChecks();

		if (tempLoadPos)
			TempLoadPosAll();

		ImNodes::EditorContextSet(*editor_context_other.get());
		ImGui::End();
	}

	void CustomShaderWindow::DrawDefaultInNode(ShaderStateNode& node)
	{
		ImNodes::BeginNode(node.id);
		ImGui::TextUnformatted(node.name.c_str());

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::Text(cShaderTypeName[CST::dataIDS[node.name]].c_str());
		ImNodes::EndOutputAttribute();

		if (ImNodes::IsNodeSelected(node.id))
			SelectNode(&node);

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawEditableInNode(ShaderEditableNode& node)
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

		if (ImNodes::IsNodeSelected(node.id))
			SelectNode(&node);

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawStateNode(ShaderStateNode& node)
	{
		ImNodes::BeginNode(node.id);

		if ((node.id == colorExitNodeID) ||
			(node.id == roughMetExitNodeID))
			ImGui::TextUnformatted(node.name.c_str());
		else
		{
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.name.c_str());
			ImNodes::EndNodeTitleBar();
		}

		const auto& funcDets = CST::cShaderFuncsTemplates.find(node.name)->second;
		for (size_t i{}; i < funcDets.inIDs.size(); ++i)
		{
			ImNodes::BeginInputAttribute(node.in_ids[i]);
			ImGui::Text(cShaderTypeName[funcDets.inIDs[i]].c_str());
			ImNodes::EndInputAttribute();
		}

		if (node.out_id != 0)
		{
			ImGui::SameLine();
			ImNodes::BeginOutputAttribute(node.out_id);
			ImGui::Text(cShaderTypeName[funcDets.outType].c_str());
			ImNodes::EndOutputAttribute();
		}
		if (ImNodes::IsNodeSelected(node.id))
			SelectNode(&node);

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawTransitionNodes(ShaderLinkNode& n)
	{
		if (n.sourceAttr == 0 || n.destAttr == 0)
			return;
		ImNodes::Link(n.id, n.sourceAttr, n.destAttr);
		if (ImNodes::IsLinkSelected(n.id))
			SelectNode(&n);
	}
#pragma endregion

	void CustomShaderWindow::TempLoadPosAll()
	{
		float yPos{}, xPos{};
		const float xProgress{ 150.f }, yProgress{ 50.f }, yBigProgress{ 200.f };
		for (auto& i : mDefaultIns)
		{
			InitNodePos(i.first, xPos, yPos);
			yPos += yProgress;
		}
		for (auto& i : mEditableIns)
		{
			InitNodePos(i.first, xPos, yPos);
			yPos += yProgress;
		}
		xPos += xProgress;
		yPos = 0.f;
		for (auto& i : mStateNodes)
		{
			if (i.first == colorExitNodeID || i.first == roughMetExitNodeID)
				continue;
			InitNodePos(i.first, xPos, yPos);
			yPos += yBigProgress;
			if (yPos > 3 * yBigProgress)
			{
				yPos = 0.f;
				xPos += xProgress;
			}
		}
		xPos += xProgress;
		if (colorExitNodeID != 0)
		{
			InitNodePos(colorExitNodeID, xPos, yPos);
			yPos += yProgress;
		}
		if (roughMetExitNodeID != 0)
			InitNodePos(roughMetExitNodeID, xPos, yPos);
		tempLoadPos = false;
	}
	// -ve is go up
	void CustomShaderWindow::InitNodePos(int id, float xPos, float yPos)
	{
		ImNodes::SetNodeEditorSpacePos(id, ImVec2{ xPos, yPos });
		ImNodes::SnapNodeToGrid(id);
	}
	void CustomShaderWindow::SelectNode(SelectionNode* node)
	{
		mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(node);
	}
	
	void CustomShaderWindow::DrawPostEditorElements()
	{
		if (mCurrShaderGraphGUID.IsValid())
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
	void CustomShaderWindow::PostEditorChecks()
	{
		int start_attr{}, end_attr{};
		if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) // In Node & Out Node ID
		{
			ShaderLinkNode n;
			n.sourceAttr = start_attr;
			n.destAttr = end_attr;
			n.id = ++uniqueIDCnt;
			DeleteLinkFromAttr(end_attr);
			mTransitionNodes.insert(std::make_pair(n.id, n));
			attrIDToLinkID[start_attr] = n.id;
			attrIDToLinkID[end_attr] = n.id;
		}
		if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Delete))
		{
			if (mSelectionManager)
			{
				auto selectedNodes = mSelectionManager->GetSelectedNodes();

				for (auto node : selectedNodes)
				{
					switch (node->type)
					{
					case SelectionType::SHADER_LINK_STATE:
					{
						auto linkNode = static_cast<ShaderLinkNode*>(node);
						DeleteLink(linkNode->id);
						break;
					}
					case SelectionType::SHADER_FUNCTION_STATE:
					{
						auto stateNode = static_cast<ShaderStateNode*>(node);
						if (mStateNodes.find(stateNode->id) != mStateNodes.end())
						{
							// Delete Attr To Node
							if (attrIDToNodeID.find(stateNode->out_id) != attrIDToNodeID.end())
							{
								// Delete Links from in & outs
								for (auto ins : stateNode->in_ids)
									DeleteLinkFromAttr(ins);
								DeleteLinkFromAttr(stateNode->out_id);

								attrIDToNodeID.erase(stateNode->out_id);
							}
							// Delete Node
							mStateNodes.erase(stateNode->id);
						}
						break;
					}
					}
				}
				mSelectionManager->ClearSelection();
			}

		}
	}

	void CustomShaderWindow::DeleteLink(int id)
	{
		auto node = mTransitionNodes.find(id);
		if (mTransitionNodes.find(id) != mTransitionNodes.end())
		{
			// Delete Attr to Link
			if (attrIDToLinkID.find(node->second.destAttr) != attrIDToLinkID.end())
				attrIDToLinkID.erase(node->second.destAttr);
			if (attrIDToLinkID.find(node->second.sourceAttr) != attrIDToLinkID.end())
				attrIDToLinkID.erase(node->second.sourceAttr);
			// Delete Link
			mTransitionNodes.erase(id);
		}
	}
	void CustomShaderWindow::DeleteLinkFromAttr(int attr)
	{
		if (attrIDToLinkID.find(attr) != attrIDToLinkID.end())
			DeleteLink(attrIDToLinkID.at(attr));
	}
}