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
		{CST::CSHAD_T::VEC4, "vec4"},
		{CST::CSHAD_T::SAMPLER, "sampler2D"}
	};

	CustomShaderWindow::CustomShaderWindow(Registry& reg) : EditorWindow(reg) 
	{
		mFinalNode.id = ++uniqueIDCnt;
		for (auto& [name, funcDets] : CST::cShaderFuncsTemplates)
		{
			if (funcDets.FuncType == CST::ShaderGraphFunc_T::IMMUTABLE)
			{
				mFinalNodeOutputNames.push_back(name);
				mFinalNode.in_ids.push_back(++uniqueIDCnt);
			}
		}
	}

	CustomShaderWindow::~CustomShaderWindow()
	{
		EventManager::GetInstance()->Unsubscribe<DeleteSelectedEntities, &CustomShaderWindow::DeleteButtonPress>(this);
		ImNodes::EditorContextFree(*editor_context_this.get());
		ImNodes::EditorContextFree(*editor_context_other.get());
	}

#pragma region ReadWrite
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

			for (auto tID : mFinalNode.in_ids)
				attrIDToNodeID[tID] = mFinalNode.id;

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
					n.baseDataType = CST::CSHAD_T::FLOAT;

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
					n.baseDataType = CST::CSHAD_T::INT;

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
					n.baseDataType = CST::CSHAD_T::UINT;

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
					n.baseDataType = CST::CSHAD_T::BOOL;

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}
			if (paramsJson.contains("Textures"))
				for (auto& [name, components] : paramsJson["Textures"].items())
				{
					ShaderEditableNode n;
					n.id = ++uniqueIDCnt;
					n.out_id = ++uniqueIDCnt;
					n.name = name;
					n.baseData = components.get<uint64_t>();
					n.baseDataType = CST::CSHAD_T::SAMPLER;

					mEditableIns[n.id] = n;
					attrIDToNodeID[n.out_id] = n.id;
					tempIDLinkGet[name].source_attr = n.out_id;
				}

			// Load Functions
			nlohmann::json mainColorJson = cshaderJson["Main"];
			mStateNodes[mFinalNode.id] = mFinalNode;
			for (auto& [funcName, components] : mainColorJson.items())
			{
				const auto& funcDets = CST::cShaderFuncsTemplates.find(funcName)->second;
				// Finals (If have, good. Load it in)
				if (funcDets.FuncType == CST::ShaderGraphFunc_T::IMMUTABLE)
				{
					for (size_t i{}; i < mFinalNodeOutputNames.size(); ++i)
					{
						if (funcName == mFinalNodeOutputNames[i])
						{
							for (auto& [id, dependencies] : components.items())
							{
								std::vector<std::string> dep;
								dependencies.get_to(dep);
								if(dep[0] != "0")
									tempIDLinkGet[dep[0]].dest_attr.push(mFinalNode.in_ids[i]);
							}
							break;
						}
					}
					continue;
				}
				// Normal functions
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
						if (dep[i] != "0")
							tempIDLinkGet[dep[i]].dest_attr.push(tID);
					}
					// Out
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
			isSaved = true;
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
			paramsJson["Textures"] = nlohmann::json::object();

			for (auto& [id, node] : mEditableIns)
			{
				if (node.baseData.is_type<float>())
					paramsJson["Floats"][node.name] = node.baseData.get_value<float>();

				else if (node.baseData.is_type<int32_t>())
					paramsJson["Ints"][node.name] = node.baseData.get_value<int32_t>();

				else if (node.baseData.is_type<uint32_t>())
					paramsJson["Uints"][node.name] = node.baseData.get_value<uint32_t>();

				else if (node.baseData.is_type<bool>())
					paramsJson["Bools"][node.name] = node.baseData.get_value<bool>();

				else if (node.baseData.is_type<uint64_t>())
					paramsJson["Textures"][node.name] = node.baseData.get_value<uint64_t>();
			}
			shaderGraphJson["Params"] = paramsJson;

			// Color Main
			std::queue<ShaderStateNode> nodesLeftToCheck;

			nlohmann::json colorMainJson;
			// Special Behaviour for Final Node (the 1 Internal Node, but handling 4 nodes actually
			// if mFinalNodeOutputNames.size != mFinalNode.in_ids.size CRY ;w;
			for(size_t i{}; i < mFinalNodeOutputNames.size(); ++i)
			{
				auto& nodeInAddr = mFinalNode.in_ids[i];
				std::vector<std::string> dependName{"0"};

				auto linkID = attrIDToLinkID.find(nodeInAddr);
				if (linkID != attrIDToLinkID.end())
				{
					auto linkNode = mTransitionNodes.find(linkID->second);
					if (linkNode != mTransitionNodes.end())
					{
						auto sourceNodeID = attrIDToNodeID.find(linkNode->second.sourceAttr);
						if (sourceNodeID != attrIDToNodeID.end())
						{
							auto sourceNode = mStateNodes.find(sourceNodeID->second);
							// The Node Connected to i "in parameter"
							if (sourceNode != mStateNodes.end())
							{
								dependName[0] = "Node" + std::to_string(sourceNode->second.id);
								nodesLeftToCheck.push(sourceNode->second);
							}
							else // Cannot Find any Node (function) connected
							{
								auto editableNode = mEditableIns.find(sourceNodeID->second);
								if (editableNode != mEditableIns.end())
									dependName[0] = editableNode->second.name;
								else
								{
									auto defaultNode = mDefaultIns.find(sourceNodeID->second);
									if (defaultNode != mDefaultIns.end())
										dependName[0] = defaultNode->second.name;
								}
							}
						}
					}
				}
				colorMainJson[mFinalNodeOutputNames[i]]["FinalNode" + std::to_string(i)] = dependName;
			}
			

			while (!nodesLeftToCheck.empty())
			{
				ShaderStateNode node = nodesLeftToCheck.front();
				nodesLeftToCheck.pop();
				std::vector<std::string> dependenciesName;
				for (auto& i : node.in_ids)
				{
					bool foundContinuation = false;
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
								// The Node Connected to i "in parameter"
								if (sourceNode != mStateNodes.end())
								{
									dependenciesName.push_back("Node" + std::to_string(sourceNode->second.id));
									nodesLeftToCheck.push(sourceNode->second);
									foundContinuation = true;
								}
								else // Cannot Find any Node (function) connected
								{
									auto editableNode = mEditableIns.find(sourceNodeID->second);
									if (editableNode != mEditableIns.end())
									{
										dependenciesName.push_back(editableNode->second.name);
										foundContinuation = true;
									}
									else
									{
										auto defaultNode = mDefaultIns.find(sourceNodeID->second);
										if (defaultNode != mDefaultIns.end())
										{
											dependenciesName.push_back(defaultNode->second.name);
											foundContinuation = true;
										}
									}
								}
							}
						}
					}
					if(!foundContinuation) // Fails all the prev checks
						dependenciesName.push_back("0");
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
			isSaved = true;
		}
	}
#pragma endregion

	void CustomShaderWindow::Init()
	{
		EventManager::GetInstance()->Subscribe<DeleteSelectedEntities, &CustomShaderWindow::DeleteButtonPress>(this);
		mSelectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		mSessionManager = mRegistry.GetManager<SessionManager>("Session");
		editor_context_this = std::make_unique<ImNodesEditorContext*>(ImNodes::EditorContextCreate());
		editor_context_other = std::make_unique<ImNodesEditorContext*>(ImNodes::EditorContextCreate());
	}

	void CustomShaderWindow::create_default()
	{
		//SliceEngine::SliceEngineTypes::cShaderPredefines.find("");
	}

	void CustomShaderWindow::DrawSideBar()
	{
		ImGui::SetNextItemWidth(150.f);
		ImGui::BeginChild("##left_ShaderGraph_region", ImVec2(150.0f, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
		ImGui::SeparatorText("Parameters");
		int toDeleteID{};
		for (auto& [id, data] : mEditableIns)
		{
			std::string param_id = "##SG_Param" + data.name + std::to_string(id);
			std::string param_button_id = "##SG_ParamButton" + data.name + std::to_string(id);
			std::string paramName = data.name;

			ImGui::SetNextItemWidth(20.f);
			switch (data.baseDataType)
			{
			case CST::CSHAD_T::BOOL:ImGui::Text("Bool"); break;
			case CST::CSHAD_T::UINT:ImGui::Text("Uint"); break;
			case CST::CSHAD_T::INT:ImGui::Text("Int"); break;
			case CST::CSHAD_T::FLOAT:ImGui::Text("Float"); break;
			case CST::CSHAD_T::SAMPLER:ImGui::Text("Texture"); break;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(125.f);
			if (ImGui::InputText(param_id.c_str(), &paramName, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				data.name = paramName;
			}
			ImGui::SameLine();
			ImGui::Text("Del?");
			ImGui::SameLine();
			if (ImGui::Button(param_button_id.c_str(),ImVec2(20.f, 0.f)))
			{
				toDeleteID = data.id;
				if (attrIDToNodeID.find(data.out_id) != attrIDToNodeID.end())
				{
					attrIDToNodeID.erase(data.out_id);
				}
				DeleteLinkFromAttr(data.out_id);
			}
		}

		if (toDeleteID != 0)
		{
			mEditableIns.erase(toDeleteID);
		}

		ImGui::EndChild();
	}
	void CustomShaderWindow::DrawNodeEditor()
	{
		ImGui::BeginChild("##right_ShaderGraph_region", ImVec2(0.f, 0.0f), ImGuiChildFlags_Borders);
		ImNodes::BeginNodeEditor();

		// Ins
		for (auto& i : mDefaultIns)
			DrawDefaultInNode(i.second);
		for (auto& i : mEditableIns)
			DrawEditableInNode(i.second);
		// Mids
		for (auto& i : mStateNodes)
		{
			if(i.second.id != mFinalNode.id)
				DrawStateNode(i.second);
		}
		DrawFinalNode();
		// Transitions
		for (auto& i : mTransitionNodes)
			DrawTransitionNodes(i.second);

		DrawPostEditorElements();
		// must be called right before EndNodeEditor
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopLeft);
		ImNodes::EndNodeEditor();
		ImGui::EndChild();
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
		if (!isSaved)
		{
			ImGui::SameLine();
			ImGui::Text("Unsaved Data");
		}
		DrawSideBar();
		ImGui::SameLine();
		ImNodes::EditorContextSet(*editor_context_this.get());
		DrawNodeEditor();

		PostEditorChecks();

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
		{
			if (ImGui::IsWindowFocused())
			{
				SelectNode(&node);
			}
		}

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawEditableInNode(ShaderEditableNode& node)
	{
		ImNodes::BeginNode(node.id);
		ImGui::TextUnformatted(node.name.c_str());

		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::PushItemWidth(50.f);
		if (node.baseData.is_type<float>())
		{
			float temp = node.baseData.get_value<float>();
			ImGui::DragFloat("", &temp, 0.01f);
			node.baseData = temp;
		}
		if (node.baseData.is_type<uint32_t>())
		{
			int temp = static_cast<int>(node.baseData.get_value<uint32_t>());
			ImGui::DragInt("", &temp);
			node.baseData = static_cast<uint32_t>(temp);
		}
		if (node.baseData.is_type<int32_t>())
		{
			int temp = node.baseData.get_value<float>();
			ImGui::DragInt("", &temp);
			node.baseData = temp;
		}
		if (node.baseData.is_type<bool>())
		{
			bool temp = node.baseData.get_value<bool>();
			ImGui::Checkbox("", &temp);
			node.baseData = temp;
		}
		ImNodes::EndOutputAttribute();

		if (ImNodes::IsNodeSelected(node.id))
		{
			if(ImGui::IsWindowFocused())
			{
				SelectNode(&node);
			}
		}

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawStateNode(ShaderStateNode& node)
	{
		ImNodes::BeginNode(node.id);
		
		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(node.name.c_str());
		ImNodes::EndNodeTitleBar();
		
		const auto& funcDets = CST::cShaderFuncsTemplates.find(node.name)->second;
		for (size_t i{}; i < funcDets.inIDs.size(); ++i)
		{
			ImNodes::BeginInputAttribute(node.in_ids[i]);
			ImGui::Text(cShaderTypeName[funcDets.inIDs[i]].c_str());
			ImNodes::EndInputAttribute();
		}

	
		ImGui::SameLine();
		ImNodes::BeginOutputAttribute(node.out_id);
		ImGui::Text(cShaderTypeName[funcDets.outType].c_str());
		ImNodes::EndOutputAttribute();
		
		if (ImNodes::IsNodeSelected(node.id))
		{
			if (ImGui::IsWindowFocused())
			{
				SelectNode(&node);
			}
		}

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawFinalNode()
	{
		ImNodes::BeginNode(mFinalNode.id);

		for (size_t i{}; i < mFinalNodeOutputNames.size(); ++i)
		{
			ImNodes::BeginInputAttribute(mFinalNode.in_ids[i]);
			ImGui::Text(mFinalNodeOutputNames[i].substr(4).c_str());
			ImNodes::EndInputAttribute();
		}

		if (ImNodes::IsNodeSelected(mFinalNode.id))
		{
			if (ImGui::IsWindowFocused())
			{
				SelectNode(&mFinalNode);
			}
		}

		ImNodes::EndNode();
	}

	void CustomShaderWindow::DrawTransitionNodes(ShaderLinkNode& n)
	{
		if (n.sourceAttr == 0 || n.destAttr == 0)
			return;
		ImNodes::Link(n.id, n.sourceAttr, n.destAttr);
		if (ImNodes::IsLinkSelected(n.id))
		{
			if(ImGui::IsWindowFocused())
			{
				SelectNode(&n);
			}
		}
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
			InitNodePos(i.first, xPos, yPos);
			yPos += yBigProgress;
			if (yPos > 3 * yBigProgress)
			{
				yPos = 0.f;
				xPos += xProgress;
			}
		}
		xPos += xProgress;

		InitNodePos(mFinalNode.id, xPos, yPos);

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
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					mSelectionManager->ClearSelection();

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("NodeEditor_Popup");
					mouseSelectPos = ImGui::GetMousePos();
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
				if (ImGui::BeginMenu("Make New Editable"))
				{
					//if (ImGui::MenuItem("Make Bool"))
					//	newNodeID = CreateEditable(CST::CSHAD_T::BOOL);
					//if (ImGui::MenuItem("Make int"))
					//	newNodeID = CreateEditable(CST::CSHAD_T::INT);
					//if (ImGui::MenuItem("Make uint"))
					//	newNodeID = CreateEditable(CST::CSHAD_T::UINT);
					if (ImGui::MenuItem("Make float"))
						newNodeID = CreateEditable(CST::CSHAD_T::FLOAT);
					if (ImGui::MenuItem("Make Texture"))
						newNodeID = CreateEditable(CST::CSHAD_T::SAMPLER);

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Maths"))
				{
					for (auto& [funcName, funcDets] : CST::cShaderFuncsTemplates)
					{
						if (funcDets.FuncType != CST::ShaderGraphFunc_T::MATH)
							continue;

						if (ImGui::MenuItem(funcName.c_str()))
							newNodeID = CreateNode(funcName);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Utilities"))
				{
					for (auto& [funcName, funcDets] : CST::cShaderFuncsTemplates)
					{
						if (funcDets.FuncType != CST::ShaderGraphFunc_T::UTILITIES)
							continue;

						if (ImGui::MenuItem(funcName.c_str()))
							newNodeID = CreateNode(funcName);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Vector Manipulation"))
				{
					for (auto& [funcName, funcDets] : CST::cShaderFuncsTemplates)
					{
						if (funcDets.FuncType != CST::ShaderGraphFunc_T::VECTOR_MANIP)
							continue;

						if (ImGui::MenuItem(funcName.c_str()))
							newNodeID = CreateNode(funcName);
					}
					ImGui::EndMenu();
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
			// Find the Node IDs to connect
			if (attrIDToNodeID.find(start_attr) != attrIDToNodeID.end() &&
				attrIDToNodeID.find(end_attr) != attrIDToNodeID.end())
			{
				auto startNodeID = attrIDToNodeID.at(start_attr);

				CST::CSHAD_T startAttrType = CST::CSHAD_T::NIL;
				// Loop through ALL possible locations where start_attr comes from
				if (mDefaultIns.find(startNodeID) != mDefaultIns.end())
				{
					auto dets = CST::dataIDS.find(mDefaultIns.at(startNodeID).name);
					if (dets != CST::dataIDS.end())
						startAttrType = dets->second;
				}
				else if (mEditableIns.find(startNodeID) != mEditableIns.end())
					startAttrType = mEditableIns.at(startNodeID).baseDataType;
				else if (mStateNodes.find(startNodeID) != mStateNodes.end())
				{
					auto dets = CST::cShaderFuncsTemplates.find(mStateNodes.at(startNodeID).name);
					if (dets != CST::cShaderFuncsTemplates.end())
						startAttrType = dets->second.outType;
				}

				auto endNodeID = attrIDToNodeID.at(end_attr);
				// Find End Node
				if (mStateNodes.find(endNodeID) != mStateNodes.end())
				{
					auto endStateNode = mStateNodes.find(endNodeID)->second;

					// Special Case for Final Node
					if (endNodeID == mFinalNode.id)
					{
						for (size_t i{}; i < mFinalNodeOutputNames.size(); ++i)
						{
							if (mFinalNode.in_ids[i] == end_attr)
							{
								auto& endFuncDets = CST::cShaderFuncsTemplates.find(mFinalNodeOutputNames[i])->second;
								auto& endAttrType = endFuncDets.inIDs[0];

								// If type match then Link
								if (CST::cTypecast.find(PairCshad(startAttrType,endAttrType)) != CST::cTypecast.end())
								{
									ShaderLinkNode n;
									n.sourceAttr = start_attr;
									n.destAttr = end_attr;
									n.id = ++uniqueIDCnt;
									DeleteLinkFromAttr(end_attr);
									mTransitionNodes.insert(std::make_pair(n.id, n));
									attrIDToLinkID[start_attr] = n.id;
									attrIDToLinkID[end_attr] = n.id;

									isSaved = false;
								}
								break;
							}
						}
					}
					else
					{
						auto& endFuncDets = CST::cShaderFuncsTemplates.find(endStateNode.name)->second;

						// Loop through possible in's to find the correct IN attr
						for (size_t i{}; i < endStateNode.in_ids.size(); ++i)
						{
							if (endStateNode.in_ids[i] == end_attr)
							{
								auto& endAttrType = endFuncDets.inIDs[i];
								// If type match then Link
								if (CST::cTypecast.find(PairCshad(startAttrType,endAttrType)) != CST::cTypecast.end())
								{
									ShaderLinkNode n;
									n.sourceAttr = start_attr;
									n.destAttr = end_attr;
									n.id = ++uniqueIDCnt;
									DeleteLinkFromAttr(end_attr);
									mTransitionNodes.insert(std::make_pair(n.id, n));
									attrIDToLinkID[start_attr] = n.id;
									attrIDToLinkID[end_attr] = n.id;

									isSaved = false;
								}
								break;
							}
						}
					}
				}
			}
			mSelectionManager->ClearSelection();
		}
	
		if (tempLoadPos)
			TempLoadPosAll();
		if (newNodeID != 0)
		{
			ImNodes::SetNodeScreenSpacePos(newNodeID, mouseSelectPos);
			ImNodes::SnapNodeToGrid(newNodeID);
			newNodeID = 0;
		}
	}

	void CustomShaderWindow::DeleteButtonPress()
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
				isSaved = false;
				break;
			}
			case SelectionType::SHADER_FUNCTION_STATE:
			{
				auto stateNode = static_cast<ShaderStateNode*>(node);
				if (CST::cShaderFuncsTemplates.find(stateNode->name) != CST::cShaderFuncsTemplates.end())
				{
					if (CST::cShaderFuncsTemplates.find(stateNode->name)->second.FuncType == CST::ShaderGraphFunc_T::IMMUTABLE)
						break;
				}
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
					isSaved = false;
				}
				break;
			}
			}
		}
		//mSelectionManager->ClearSelection();

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
	int CustomShaderWindow::CreateNode(std::string funcName)
	{
		// Special Case (No Out) --TODO-- Prevent Deletion or making ;w; of END_COLOR
		auto function = CST::cShaderFuncsTemplates.find(funcName);
		if (function != CST::cShaderFuncsTemplates.end())
		{
			ShaderStateNode n;
			n.id = ++uniqueIDCnt;
			n.name = funcName;
			// Ins
			for (int i{}; i < function->second.inIDs.size(); ++i)
			{
				int in_attr = ++uniqueIDCnt;
				n.in_ids.push_back(in_attr);

				attrIDToNodeID[in_attr] = n.id;
			}
			int out_attr = ++uniqueIDCnt;
			n.out_id = out_attr;

			attrIDToNodeID[out_attr] = n.id;
			mStateNodes[n.id] = n;
			return n.id;
		}
		return 0;
	}
	int CustomShaderWindow::CreateEditable(SliceEngine::SliceEngineTypes::CSHAD_T type)
	{
		ShaderEditableNode node;
		node.baseDataType = type;
		node.id = ++uniqueIDCnt;
		node.out_id = ++uniqueIDCnt;
		node.name = "NODE_" + std::to_string(node.id);

		attrIDToNodeID[node.out_id] = node.id;
		switch (type)
		{
		case CST::CSHAD_T::BOOL:
		{
			node.baseData = false;
			break;
		}
		case CST::CSHAD_T::INT:
		{
			node.baseData = 0;
			break;
		}
		case CST::CSHAD_T::UINT:
		{
			uint32_t temp{};
			node.baseData = temp;
			break;
		}
		case CST::CSHAD_T::FLOAT:
		{
			node.baseData = 0.f;
			break;
		}
		case CST::CSHAD_T::SAMPLER:
		{
			node.baseData = SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			break;
		}
		}
		mEditableIns[node.id] = node;
		isSaved = false;
		return node.id;
	}
}