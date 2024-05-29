#pragma once
#include "includes.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

//my includes
#include "offsets.h"
#include "Vec3.h"

struct Vec2 {
	float x, y;
};
struct Vec4 {
	float x, y, z, w;
};

bool WorldToScreen(Vec3& pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight)
{
	//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
	Vec4 clipCoords;
	clipCoords.x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
	clipCoords.y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
	clipCoords.z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
	clipCoords.w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];

	if (clipCoords.w < 0.1f)
		return false;

	//perspective division, dividing by clip.W = Normalized Device Coordinates
	Vec3 NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;

	//Transform to window coordinates
	screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
	screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
	return true;
}


//my global stuff
Vec3 entPlayerBody;
Vec3 entPlayerBody2;
Vec3 locPlayerBody;
Vec3 nuEntPlayerHead;
Vec3 nuEntPlayerBody;
bool bHop = false, bAimbot = false, bEsp = false;
float dist = 9999.f;
//uintptr_t viewMatrix = 0x7FF9E8ACEA78;
HMODULE outModuleBase = GetModuleHandle("client.dll");
uintptr_t outEntityList = ((uintptr_t)outModuleBase + 0x181D248); //update 05/29/2024
uintptr_t outViewMatrix = ((uintptr_t)outModuleBase + 0x1A06530); //update 05/29/2024
Vec3* viewAnglesAddy = (Vec3*)((uintptr_t)outModuleBase + 0x1A13688); //update 05/29/2024

namespace globalFuncts
{
	int processMemoryAddy(void* address) {
		MEMORY_BASIC_INFORMATION info;
		if (VirtualQuery(address, &info, sizeof(info)) == sizeof(info)) {
			if (info.Protect & PAGE_NOACCESS) {
				// Memory at the address is inaccessible, continue loop
				std::cout << "bytes inaccessible\n";
				return -1;
			}
			else {
				// Memory at the address is accessible, process the entity
				// Your processing code here
				return 1;
			}
		}
		else {
			std::cerr << "VirtualQuery failed." << std::endl;
			// Handle the failure if necessary
		}
	}

	int getNumPlayers()
	{
		uintptr_t modBase = (uintptr_t)GetModuleHandle("server.dll");
		uintptr_t numPlayers = ((uintptr_t)modBase + 0x13D4084); //update 05/29/2024

		return *(int*)numPlayers;
	}
}
//my global stuff end

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ent* nEntity = NULL;
ent* outLocalPlayer = NULL;
Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
ID3D11RenderTargetView* pRenderTargetView = nullptr;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

ImGuiWindow* g_pImGuiRenderWindow = nullptr;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}
		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("stro sucks");
	if (ImGui::Button("Toggle Aimbot"))
	{
		bAimbot = !bAimbot;
		std::cout << "bAimbot: " << (bAimbot ? "ON" : "OFF") << std::endl;
	}
	ImGui::End();

	//test
		//
	// Start a window for custom renders
	//
	ImGuiIO& ImGuiIO = ImGui::GetIO();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGuiIO.DisplaySize, ImGuiCond_Always);

	if (ImGui::Begin("##RENDERWINDOW", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs))
	{
		//
		// Use the current window as a render window
		//
		g_pImGuiRenderWindow = ImGui::GetCurrentWindow();

		//
		// ImGui custom rendering can be done here...
		//
		// Example:
		g_pImGuiRenderWindow->DrawList->AddText(ImVec2(0, 0), ImColor(255, 255, 255, 255), "Rendering Text");
		//

		//
		// Render everything in the drawlist on the window
		//
		g_pImGuiRenderWindow->DrawList->PushClipRectFullScreen();

		//
		// Clean up the render window
		//
		g_pImGuiRenderWindow = NULL;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	float x = 100.f;
	float y = 100.f;
	float w = 100.f;
	float h = 100.f;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 rect_min = ImVec2(x, y);
	ImVec2 rect_max = ImVec2(x + w, y + h);

	ImU32 color = ImColor(255, 255, 255, 255);
	//draw_list->AddRect(rect_min, rect_max, color);

	//ESP STUFF

	// Loop through entity list
	if (outEntityList)
	{
		for (int i = 2; i - 2 < (globalFuncts::getNumPlayers() * 2); i++)
		{
			ent* lPlayer = (ent*)outLocalPlayer;
			ent* lPlayerTest1 = (ent*)*(uintptr_t*)(*(uintptr_t*)outEntityList + 0x0);
			ent* lPlayerTest2 = (ent*)*(uintptr_t*)(*(uintptr_t*)outEntityList + 0x8);
			uint8_t* bytesOrigEnt = (uint8_t*)lPlayerTest1;
			uint8_t* bytesOrigEnt2 = (uint8_t*)lPlayerTest2;
			if (static_cast<int>(bytesOrigEnt[0]) == 0x60 && static_cast<int>(bytesOrigEnt[1]) == 0x29)
			{
				lPlayer = lPlayerTest1;
				outLocalPlayer = lPlayerTest1;
			}
			else if (static_cast<int>(bytesOrigEnt2[0]) == 0x60 && static_cast<int>(bytesOrigEnt2[1]) == 0x29)
			{
				lPlayer = lPlayerTest2;
				outLocalPlayer = lPlayerTest2;
			}
			else
				continue;

			ent* entity = (ent*)*(uintptr_t*)(*(uintptr_t*)outEntityList + 0x8 * i);
			uint8_t* bytesNewEnt = (uint8_t*)entity;
			if (globalFuncts::processMemoryAddy(bytesNewEnt) != -1) //checking to see if bytes are accessible
			{
				std::cout << "init\n";
				if (static_cast<int>(bytesNewEnt[0]) == 0x60 && static_cast<int>(bytesNewEnt[1]) == 0x29) //standard for an lPlayerPawn
				{
					Vec3 entPlayerPos = entity->pos + entity->eyeOrigin;
					Vec3 lPlayerPos = lPlayer->pos + lPlayer->eyeOrigin;
					if (entity != nullptr)
					{
						if ((entPlayerPos - lPlayerPos).hypo3() < dist && entity->health > 0 && entity->teamNum != lPlayer->teamNum && entity != lPlayer)
						{
							nEntity = entity;
							Vec3 nuEntPlayerHd = nEntity->pos + nEntity->eyeOrigin;
							nuEntPlayerHead = nuEntPlayerHd;
							Vec3 nuEntPlayerBy = nEntity->pos;
							nuEntPlayerBody = nuEntPlayerBy;

							Vec2 vFoot = {};
							Vec2 vHead = {};

							if (WorldToScreen(nuEntPlayerBy, vFoot, (float*)outViewMatrix, 1280, 720)
								&& WorldToScreen(nuEntPlayerHd, vHead, (float*)outViewMatrix, 1280, 720))
							{

								float h = (vHead.y - vFoot.y);
								float w = h / 4.f;
								float l = vFoot.x - w;
								float r = vFoot.x + w;

								draw_list->AddRect(ImVec2(l, vHead.y), ImVec2(r, vFoot.y), color, 0.f, 0, 2);
								draw_list->AddText(ImVec2(vHead.x, vHead.y), color, "Player");
							}
						}
						else if (entity->health <= 0 || entity->teamNum == lPlayer->teamNum)
						{
							continue;
						}
					}
				}
			}
		}
	}



	//ESP STUFF END


	//
	// End the current instance
	//
	ImGui::End();
	//test end


	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI HackThread(HMODULE hMod)
{

	//hook stuff
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			init_hook = true;
		}

		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);

		HMODULE moduleBase = GetModuleHandle("client.dll");
		std::cout << "Module Base: 0x" << std::uppercase << moduleBase << std::endl;
		uintptr_t entityList = ((uintptr_t)moduleBase + 0x181D248);
		std::cout << "entityList: 0x" << std::uppercase << std::hex << entityList << std::endl;
		//ent* localPlayer = (ent*)((uintptr_t)moduleBase + 0x1748FE0);
		ent* localPlayer = (ent*)*(uintptr_t*)(*(uintptr_t*)entityList + 0x8);				//two dereference!!
		std::cout << "localPlayer: 0x" << std::uppercase << std::hex << outLocalPlayer << std::dec << std::endl;
		//ent* entity = (ent*)(*(uintptr_t*)entityList + 0x18);
		//std::cout << "entity1: 0x" << std::uppercase << std::hex << entity << std::dec << std::endl;
		std::cout << "numPlayers: " << std::dec << globalFuncts::getNumPlayers() << std::endl;

		while (true)
		{
			if (GetAsyncKeyState(VK_END) & 1)
			{
				break;
			}
			if (GetAsyncKeyState(VK_NUMPAD1) & 1)
			{
				/*std::cout << "Health: " << localPlayer->health << std::endl;
				std::cout << "Team: " << (int)localPlayer->teamNum << std::endl;
				std::cout << "Eye Origin: " << localPlayer->eyeOrigin.x << ", " << localPlayer->eyeOrigin.y << ", " << localPlayer->eyeOrigin.z << std::endl;
				std::cout << "Position: " << (float)localPlayer->pos.x << ", " << (float)localPlayer->pos.y << ", " << (float)localPlayer->pos.z << std::endl;
				std::cout << "Num Players: " << getNumPlayers() << std::endl;*/
				//std::cout << viewAnglesAddy->x << ", " << viewAnglesAddy->y << ", " << viewAnglesAddy->z << std::endl;
				//std::cout << viewAnglesAddy->x << ", " << viewAnglesAddy->y << std::endl;
				bAimbot = !bAimbot;
				std::cout << "bAimbot: ON" << std::endl;
			}
			if (GetAsyncKeyState(VK_NUMPAD2) & 1)
			{
				bEsp = !bEsp;
				std::cout << "bEsp: ON" << std::endl;
			}

			if (bAimbot)
			{
				if (outLocalPlayer)
				{
					//this is where nEntity orig was
					for (int i = 2; i - 2 < (globalFuncts::getNumPlayers() * 2); i++)
					{
						ent* lPlayer = (ent*)outLocalPlayer;
						ent* lPlayerTest1 = (ent*)*(uintptr_t*)(*(uintptr_t*)outEntityList + 0x0);
						ent* lPlayerTest2 = (ent*)*(uintptr_t*)(*(uintptr_t*)outEntityList + 0x8);
						uint8_t* bytesOrigEnt = (uint8_t*)lPlayerTest1;
						uint8_t* bytesOrigEnt2 = (uint8_t*)lPlayerTest2;
						if (static_cast<int>(bytesOrigEnt[0]) == 0x60 && static_cast<int>(bytesOrigEnt[1]) == 0x29)
						{
							lPlayer = lPlayerTest1;
							outLocalPlayer = lPlayerTest1;
						}
						else if (static_cast<int>(bytesOrigEnt2[0]) == 0x60 && static_cast<int>(bytesOrigEnt2[1]) == 0x29)
						{
							lPlayer = lPlayerTest2;
							outLocalPlayer = lPlayerTest2;
						}
						else
							continue;

						ent* entity = (ent*)*(uintptr_t*)(*(uintptr_t*)entityList + 0x8 * i);
						uint8_t* bytesNewEnt = (uint8_t*)entity;


						if (bytesNewEnt != nullptr && globalFuncts::processMemoryAddy(bytesNewEnt) != -1) //checking to see if bytes are accessible
						{
							if (static_cast<int>(bytesNewEnt[0]) == 0x60 && static_cast<int>(bytesNewEnt[1]) == 0x29) //standard for an lPlayerPawn
							{
								//std::cout << "init\n";
								std::cout << "pitch: " << viewAnglesAddy->x << ", yaw: " << viewAnglesAddy->y << std::endl;

								Vec3 entPlayerPos = entity->pos + entity->eyeOrigin;
								Vec3 lPlayerPos = lPlayer->pos + lPlayer->eyeOrigin;

								if (entity != nullptr)
								{
									if ((entPlayerPos - lPlayerPos).hypo3() < dist && entity->health > 0 && entity->teamNum != lPlayer->teamNum)
									{
										nEntity = entity;
										Vec3 nuEntPlayerHd = nEntity->pos + nEntity->eyeOrigin;
										nuEntPlayerHead = nuEntPlayerHd;
										Vec3 nuEntPlayerBy = nEntity->pos;
										nuEntPlayerBody = nuEntPlayerBy;

										std::cout << "valid ent hold shift\n";
									}
									else if (entity->health <= 0 || entity->teamNum == lPlayer->teamNum)
									{
										continue;
									}
								}
								Vec3 deltaChange = (nuEntPlayerHead - lPlayerPos);
								float hyp = sqrt(deltaChange.x * deltaChange.x + deltaChange.y * deltaChange.y + deltaChange.z * deltaChange.z);
								float pitch = -asinf(deltaChange.z / hyp) * 180 / 3.141592653;
								float yaw = atan2f(deltaChange.y, deltaChange.x) * 180 / 3.141592653; //for trig functions lookup return values to make sure its in valid range

								if (GetAsyncKeyState(VK_LSHIFT) & 0x8001)
								{
									viewAnglesAddy->x = pitch;
									viewAnglesAddy->y = yaw;
								}
							}
							else continue;
						}
						else if (globalFuncts::processMemoryAddy(bytesNewEnt) == -1)
						{
							continue;
						}
						else continue;
					}
				}
				Sleep(2);
			}

			Sleep(2);
		}
	} while (!init_hook);
	//hook stuff end
	FreeLibraryAndExitThread(hMod, 0);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}