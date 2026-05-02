#include "Fonts.hpp"
#include "Logo.hpp"
#include "logoss.hpp"
#include "PrewiewModel.hpp"
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <algorithm>

void AddNotification(const std::string& message, bool isSuccess);
std::string GetPedName(class Ped& ped);
void StealOutfit(int targetPlayerID);
void EnterVehicle(int vehicleIndex);
void UnlockSelectedVehicle();
void LockSelectedVehicle();
void StealSelectedVehicle();
void CopyToClipboard(const std::string& text);
std::string GetPlayerDiscordId(int playerID);
std::string GetPlayerSteamId(int playerID);

void DrawMenu();

struct MenuStruct {
	ImVec2 Pos, Region, Spacing;
	ImVec2 WindowSize = { 950, 600 };
	float SidebarWidth = 200.0f;
	float HeaderHeight = 60.0f;
	float ContentPadding = 15.0f;
	
	ImFont* Inter;
	ImFont* InterSmaller;
	ImFont* InterSemiBold;
	ImFont* FontAwesome;
	ImFont* Arial;
	ImFont* Tahoma;
	ImFont* Verdana;
	ImFont* SegoeUI;
	ImFont* SegoeUILarge;
	ImFont* SegoeUIBold;
	ImFont* TimesNewRoman;
	ImFont* Calibri;
	ImFont* CourierNew;
	ImFont* Consolas;
	ImFont* TrebuchetMS;
	ImFontConfig Config;
	ID3D11ShaderResourceView* Logo;
	ID3D11ShaderResourceView* Chracter;
	DWORD ColorPickerFlags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
	
	int CurrentTab = 0;
	int SubTab = 0;
	int SubTab1 = 0, SubTab2 = 0, SubTab3 = 0, SubTab4 = 0, SubTab5 = 0, SubTab6 = 0, SubTab7 = 0;
	int LastTab = -1;
	float TabAlpha = 0.0f;
	
	bool ShowExitScreen = false;
	float ExitScreenAlpha = 0.0f;
	float ExitScreenTime = 0.0f;
	float MenuFadeOutAlpha = 1.0f;
	
	float BackgroundRounding = 12.f;
	float ChildRounding = 10.f;
	float PageRounding = 8.f;
	float ElementsRounding = 6.f;
	float ButtonHeight = 35.0f;
	float ButtonSpacing = 2.0f;
	float ButtonX = 15.0f;
	float SeparatorOffset = 3.0f;
	float InitialOffsetX = 0;
	const float AlphaSpeed = 0.02f;
	
	ImColor AccentColor = ImColor(235, 13, 78);
	ImColor AccentColorHover = ImColor(255, 35, 100);
	ImColor AccentColorDark = ImColor(200, 10, 65);
	
	ImColor MainColor = ImColor(235, 13, 78);
	ImColor BackgroundColor = ImColor(20, 20, 24);
	ImColor SidebarColor = ImColor(20, 20, 24);
	ImColor HeaderColor = ImColor(27, 27, 34);
	ImColor ContentColor = ImColor(30, 30, 37);
	ImColor CardColor = ImColor(34, 34, 42);
	ImColor CardHoverColor = ImColor(40, 40, 50);
	ImColor TabColor = ImColor(29, 29, 31);
	
	ImColor TextColor = ImColor(220, 220, 225);
	ImColor TextDimColor = ImColor(140, 140, 150);
	ImColor TextActiveColor = ImColor(255, 255, 255);
	ImColor TextHoverColor = ImColor(180, 180, 190);
	
	ImColor BorderColor = ImColor(47, 47, 57);
	ImColor DividerColor = ImColor(42, 42, 52);
	ImColor SeparatorColor = ImColor(52, 52, 62);
	
	ImColor ChildColor = ImColor(24, 24, 30);
	ImColor ChildCapColor = ImColor(32, 32, 40);
	ImColor ChildCapstructColor = ImColor(37, 37, 47);
	
	ImColor PageTextColor = ImColor(210, 210, 215);
	ImColor PageActiveColor = ImColor(235, 13, 78);
	ImColor PageActiveIconColor = ImColor(255, 255, 255);
	ImColor PageColor = ImColor(30, 30, 37);
	
	ImColor ElementsColor = ImColor(34, 34, 42);
	ImColor ElementsHoverColor = ImColor(44, 44, 54);
	
	ImColor ToggleOnColor = ImColor(235, 13, 78);
	ImColor ToggleOffColor = ImColor(55, 55, 65);
	ImColor ToggleKnobColor = ImColor(255, 255, 255);
	ImColor CheckboxMarkColor = ImColor(235, 13, 78);
	
	ImColor BackgroundTopColor = ImColor(23, 23, 27);
	ImColor BackgroundBottomColor = ImColor(14, 14, 17);
	
	float MenuAlpha = 0.0f;
	float TabTransition = 0.0f;
	
} Menu;


void InitializeMenu(ID3D11Device* pDevice) {
	D3DX11_IMAGE_LOAD_INFO info;
	ID3DX11ThreadPump* pump{ nullptr };

	D3DX11CreateShaderResourceViewFromMemory(pDevice, logossData, sizeof(logossData), &info, pump, &Menu.Logo, 0);
	D3DX11CreateShaderResourceViewFromMemory(pDevice, rawData, sizeof(rawData), &info, pump, &Menu.Chracter, 0);

	auto& io = ImGui::GetIO();
	auto& style = ImGui::GetStyle();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.OversampleH = 3;
	iconsConfig.OversampleV = 3;

	Menu.Inter = io.Fonts->AddFontFromMemoryTTF(Inter, sizeof(Inter), 16.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	Menu.InterSmaller = io.Fonts->AddFontFromMemoryTTF(Inter, sizeof(Inter), 13.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	Menu.FontAwesome = io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeData, FontAwesomeDataSize, 16.f, &iconsConfig, iconsRanges);
	Menu.InterSemiBold = io.Fonts->AddFontFromMemoryTTF(EspFont, sizeof(EspFont), 16.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	
	static const ImWchar turkishRanges[] = {
		0x0020, 0x00FF,
		0x0100, 0x017F,
		0x0180, 0x024F,
		0x1E00, 0x1EFF,
		0
	};
	
	Menu.Arial = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\arial.ttf"), 14.0f);
	Menu.Tahoma = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\tahoma.ttf"), 14.0f);
	Menu.Verdana = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\verdana.ttf"), 14.0f);
	Menu.SegoeUI = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\segoeui.ttf"), 15.0f, &Menu.Config, turkishRanges);
	Menu.SegoeUILarge = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\segoeui.ttf"), 20.0f, &Menu.Config, turkishRanges);
	Menu.SegoeUIBold = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\segoeuib.ttf"), 18.0f, &Menu.Config, turkishRanges);
	Menu.TimesNewRoman = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\times.ttf"), 16.0f);
	Menu.Calibri = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\calibri.ttf"), 16.0f);
	Menu.CourierNew = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\cour.ttf"), 13.0f);
	Menu.Consolas = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\consola.ttf"), 14.0f);
	Menu.TrebuchetMS = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\trebuc.ttf"), 14.0f);

	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(8, 8);
	style.WindowBorderSize = 0;
	style.ScrollbarSize = 8.f;
	style.ScrollbarRounding = 4.f;
	style.FrameRounding = 6.f;
	style.GrabRounding = 4.f;
	
	style.Colors[ImGuiCol_Button] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(Menu.AccentColorHover.Value.x, Menu.AccentColorHover.Value.y, Menu.AccentColorHover.Value.z, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(Menu.AccentColorDark.Value.x, Menu.AccentColorDark.Value.y, Menu.AccentColorDark.Value.z, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.8f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.6f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(Menu.CardHoverColor.Value.x, Menu.CardHoverColor.Value.y, Menu.CardHoverColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.4f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(Menu.AccentColorHover.Value.x, Menu.AccentColorHover.Value.y, Menu.AccentColorHover.Value.z, 1.0f);
}

#include "Items.hpp"

namespace Cheats {
	namespace MenuUtils {
		int MenuKey = VK_INSERT;
		int FriendKey = 0; 
		const char* Fonts[] = { 
			"Arial", "Tahoma", "Verdana", "Segoe UI", "Times New Roman", 
			"Calibri", "Courier New", "Consolas", "Trebuchet MS", "Inter SemiBold"
		};
		int SelectedFontIndex = 3;
		bool StreamProof = false;
		bool KeybindList = false; 
		bool WaterMark = true; 
		bool BonePreview = true;

		ImFont* GetSelectedFont() {
			switch (SelectedFontIndex) {
				case 0: return Menu.Arial;
				case 1: return Menu.Tahoma;
				case 2: return Menu.Verdana;
				case 3: return Menu.SegoeUI;
				case 4: return Menu.TimesNewRoman;
				case 5: return Menu.Calibri;
				case 6: return Menu.CourierNew;
				case 7: return Menu.Consolas;
				case 8: return Menu.TrebuchetMS;
				case 9: return Menu.InterSemiBold;
				default: return Menu.SegoeUI;
			}
		}
	}

	namespace AimAssist {
		bool OnlyVisible = false;
		bool IgnorePed = true;
		bool IgnoreDeath = true;

		namespace Aimbot {
			bool Enabled = false;
			bool DrawFov = false;
			bool RageMode = false;
			bool ClosestBone = false;
			ImColor Color = ImColor(255, 255, 255);
			const char* Type[7] = { "Head", "Neck", "Chest", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };
			int SelectedType = 0;
			int Key = 0;
			int Fov = 50;
			int Smooth = 5;
			int Distance = 300;
		}

		namespace Silent {
			bool Enabled = false;
			bool RandomTarget = false;
			bool DrawFov = false;
			bool DrawIndicator = false;
			bool DrawLine = false;
			ImColor Color = ImColor(255, 255, 255);
			ImColor LineColor = ImColor(255, 255, 255);
			const char* Type[7] = { "Head", "Neck", "Chest", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };
			int SelectedType = 2;
			bool Pslient = false;
			int Key = 0;
			int Fov = 50;
			int Smooth = 5;
			int Distance = 300;
			int MissChance = 0;
			bool MagicBullet = false;
			int MagicBulletKey = 0;
			bool ClosestBone = false;
		bool HeadShot = true; 
		bool WheelBullet = false;
		int WheelBulletKey = 0;
		const char* IndicatorTypeOptions[2] = { "Round", "Line" };
		int IndicatorType = 0;
	}

		namespace Triggerbot {
			bool Enabled = false;
			bool DrawFov = false;
			ImColor Color = ImColor(255, 255, 255);
			const char* Type[36] = { 
				"Head", "Neck", "Hip", "LeftHand", "RightHand", "LeftUpperArm", "RightUpperArm", 
				"LeftForearm", "RightForearm", "LeftFoot", "RightFoot", "LeftAnkle", "RightAnkle", 
				"LeftThigh", "RightThigh", "Spine0", "Spine1", "Spine2", "Spine3", "SpineRoot", 
				"LeftClavicle", "RightClavicle", "Spine4", "Spine5", "LeftToe", "RightToe", 
				"Jaw", "Chest", "Neck0", "Neck2", "Root", "LeftElbow", "RightElbow", "LeftKnee", "RightKnee", "Closest"
			};
			int SelectedType = 0;
			int Key = 0;
			int Fov = 50;
			int Distance = 300;
			int Delay = 50;
			int CrosshairTolerance = 10;
		}
	}

	namespace Players {
		bool OnlyVisible = false;
		bool IgnorePed = true;
		bool IgnoreDeath = true;
		int Distance = 1000;
		bool HighlightTarget = false;
		ImColor HighlightColor = ImColor(255, 140, 0, 255);

		namespace DrawSkeleton {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
			float Thickness = 1.6f;
			float OutlineThickness = 2.2f;
		}

		namespace DrawId {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawName {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawBox {
			bool Enabled = false;
			const char* Type[2]{ "2D", "Corner" };
			int SelectedType = 0;
			ImColor Color = ImColor(255, 255, 255);
			float Size = 0.75f;
			bool FillEnabled = false;
			ImColor FillColor = ImColor(0, 0, 0, 180);
			float FillOpacity = 0.35f;
			float Thickness = 1.6f;
			bool UseCustomGradient = false;
			ImColor BoxGradientTopColor = ImColor(0, 0, 255, 255);
			ImColor BoxGradientBottomColor = ImColor(255, 0, 0, 255);
			bool GradientEnabled = false;
			float GradientIntensity = 0.0f;
		}

		namespace DrawLine {
			bool Enabled = false;
			const char* Type[3]{ "Top", "Center", "Bottom" };
			int SelectedType = 0;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawDistance {
			bool Enabled = false;
			bool StyleBg = false;
			ImColor Color = ImColor(255, 255, 255);
		}

	namespace DrawHealth {
		bool Enabled = false;
		const char* Position[4] = { "Top", "Bottom", "Left", "Right" };
		int SelectedPosition = 1; 
		ImColor Color = ImColor(0, 255, 0, 255); 
		ImColor ColorLow = ImColor(255, 0, 0, 255); 
		bool GradientEnabled = false;
		ImColor GradientStart = ImColor(0, 255, 120, 255);
		ImColor GradientEnd = ImColor(255, 120, 0, 255);
	}

	namespace DrawArmor {
		bool Enabled = false;
		const char* Position[4] = { "Top", "Bottom", "Left", "Right" };
		int SelectedPosition = 2; 
		ImColor Color = ImColor(0, 150, 255, 255); 
		bool GradientEnabled = false;
		ImColor GradientStart = ImColor(0, 200, 255, 255);
		ImColor GradientEnd = ImColor(0, 120, 200, 255);
	}

		namespace DrawWeaponName {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace GenderSpectator {
			bool Enabled = false;
			ImColor MaleColor = ImColor(100, 150, 255, 255);
			ImColor FemaleColor = ImColor(255, 100, 150, 255);
		}
	}

	namespace Vehicle {
		ImColor HealthBarColor = ImColor(0, 255, 0, 255);
		ImColor DistanceColor = ImColor(255, 255, 255, 255);
		ImColor SnaplineColor = ImColor(255, 255, 255, 255);
		ImColor MarkerOuterColor = ImColor(0, 0, 0, 100);
		ImColor MarkerInnerColor = ImColor(255, 0, 0, 255);
		bool Enabled = false;
		bool DrawLocalVehicle = false;
		bool DrawEnemyVehicle = false;
		bool VehicleHealth = false;
		bool VehicleEspShowDistance = false;
		bool VehicleEspSnapline = false;
		bool VehicleMarker = false;
		int Distance = 1000;
		bool Fix = false;
		int FixKey = 0;
		bool EngineFix = false;
		int EngineFixKey = 0;
		bool VehicleGod = false;
		int VehicleGodKey = 0;
	}

	namespace Crosshairs {
		bool Enabled = false;
		int SelectedType = 0;
		const char* Type[10]{ "Type 1", "Type 2", "Type 3", "Type 4", "Type 5", "Type 6", "Type 7", "Type 8", "Type 9", "Type 10" };
		int Size = 10;
		ImColor Color = ImColor(255, 255, 255);
	}

	namespace Exploit {
		bool HealthBoost = false;
		float HealthBoostValue = 200.0f;
		int HealthBoostKey = 0x70; 
		bool ArmorBoost = false;
		float ArmorBoostValue = 20.0f;
		int ArmorBoostKey = 0x71; 
		bool InfiniteAmmo = false;
		bool NoRecoil = false;
		bool NoSpread = false;
		bool NoReload = false;
		bool NoRange = false;
		bool ReloadAmmo = false;
		int ReloadValue = 1;
		bool ServerCrash = false;
		int ServerCrashKey = 0;

		namespace Strafe {
			bool Enabled = false;
			int Key = 0; 
			float Speed = 0.5f; 
		}

		int ReloadAmmoKey = 0;
		bool GodMode = false;
		int GodModeKey = 0;
		bool NoCollision = false;
		bool SafeDamageBoost = false;
		int SafeDamageBoostKey = 0;
		int SafeDamageBoostValue = 1;
		bool InfiniteStamina = false;
		bool AntiHeadshot = false;
		bool InvisibleNoclip = false;
		bool PeakAssist = false;
		int PeakAssistKey = 0;
		bool SafeInvisible = false;
		int SafeInvisibleKey = 0;

		namespace Vehicle {
			bool Boost = false;
			float BoostValue = 1.0f;
			int BoostKey = 0;
		}
	}

	namespace Misc {
		bool DamageReduction = false;
		int DamageReductionPercent = 50;
		bool DamageLog = false;
		bool damageBoost = false;
		float damageBoostValue = 10.0f;
		int damageBoostKey = 0;
	}

	namespace NoClip {
		bool Enabled = false;
		int Speed = 10; 
		int Key = 0;
	}

	namespace FreeCam {
		bool Enabled = false;
		float Speed = 1.0f;
		int Key = 0;
		int TpKey = 0;
		bool PatchApplied = false;
	}

	namespace Teleport {
		bool TeleportWaypoint = false;
		bool CustomTeleportWaypoint = false;
		float posX = 0.0f, posY = 0.0f, posZ = 0.0f;

		struct Location {
			const char* name;
			float x, y, z;
		};
		
		Location locations[14] = {
			{"MD Hastane", 308.7889f, -592.5567f, 43.2840f},
			{"MD PVP Zone", 216.01f, -800.52f, 30.85f},
			{"Well Hastane", 312.1247f, -592.7950f, 43.2840f},
			{"Well PVP Zone", 216.01f, -800.52f, 30.85f},
			{"Non WH Hastane", 306.9377f, -595.2255f, 43.2840f},
			{"PD Yani Kiyafetci", 428.07f, -800.32f, 29.87f},
			{"SS Hastane", 336.0438f, -1436.2172f, 46.7784f},
			{"FBI", 2449.25f, -307.9f, 93.95f},
			{"Prison", 1709.14f, 2667.66f, 45.56f},
			{"Ust Kasaba", -6.72f, 6417.51f, 38.81f},
			{"Orta Kasaba", 1839.58f, 3672.61f, 34.28f},
			{"Depo Cati 1", 1211.8837f, -3113.5347f, 15.5565f},
			{"Depo Cati 2", 1212.4299f, -3325.0161f, 15.5354f},
			{"Kirmizi Otopark", 340.6054f, -1686.8359f, 52.3391f}
		};
			
		void TeleportToLocation(int locationIndex) {
			if (locationIndex >= 0 && locationIndex < 14) {
				posX = locations[locationIndex].x;
				posY = locations[locationIndex].y;
				posZ = locations[locationIndex].z;
				CustomTeleportWaypoint = true;
			}
		}
	}
}

static int SelectedItemVehicle = -1;
static char SearchBufferVehicle[128] = "";
std::vector<std::string> VehicleNames = {};

inline std::vector<int> playerIDs;
inline std::map<int, bool> friendStatus;
inline std::vector<int> newPlayerIDs;
inline std::vector<int> oldPlayerIDs;
inline std::vector<std::string> playerNames;   

inline int selectedPlayerID = -1;
inline int selectedItemPlayer = -1;
static char searchBuffer[128] = "";
inline bool Teleport = false;

inline std::vector<int> vehicleIDs;
inline std::vector<std::string> vehicleNames;
inline std::vector<std::string> vehicleModels;
inline std::vector<float> vehicleDistances;
inline std::vector<uintptr_t> vehiclePointers;
inline int selectedItemVehicle = -1;
inline bool VehicleTeleport = false;

namespace UI {
	static std::map<std::string, float> toggleAnimations;
	static std::map<std::string, float> hoverAnimations;
	static std::map<std::string, float> tabAnimations;
	static std::map<std::string, float> sliderProgressAnimations;
	static std::map<std::string, float> sliderHoverAnimations;
	static std::map<std::string, float> keybindExpandAnimations;
	static std::map<std::string, float> selectorExpandAnimations;
	static std::map<std::string, float> buttonPressAnimations;
	static std::map<std::string, bool> comboOpenStates;
	static std::map<std::string, float> comboAnimations;
	static std::map<std::string, bool> sectionOpenStates;
	
	static bool IsAnyComboOpen() {
		for (const auto& pair : comboOpenStates) {
			if (pair.second) return true;
		}
		return false;
	}
	
	bool SoftToggle(const char* label, bool* v, const char* description = nullptr) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 50);
		
		bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		bool clicked = hovered && ImGui::IsMouseClicked(0);
		
		if (clicked) *v = !*v;
		
		std::string id = label;
		float& toggleAnim = toggleAnimations[id];
		float& hoverAnim = hoverAnimations[id];
		
		toggleAnim = ImLerp(toggleAnim, *v ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		hoverAnim = ImLerp(hoverAnim, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		ImColor bgColor = ImLerp(
			ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, Menu.CardColor.Value.w),
			ImVec4(Menu.CardHoverColor.Value.x, Menu.CardHoverColor.Value.y, Menu.CardHoverColor.Value.z, Menu.CardHoverColor.Value.w),
			hoverAnim
		);
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, 8.0f);
		
		float toggleWidth = 44.0f;
		float toggleHeight = 24.0f;
		float toggleX = pos.x + size.x - toggleWidth - 15.0f;
		float toggleY = pos.y + (size.y - toggleHeight) / 2.0f;
		
		ImColor trackColor = ImLerp(
			ImVec4(Menu.ToggleOffColor.Value.x, Menu.ToggleOffColor.Value.y, Menu.ToggleOffColor.Value.z, Menu.ToggleOffColor.Value.w),
			ImVec4(Menu.ToggleOnColor.Value.x, Menu.ToggleOnColor.Value.y, Menu.ToggleOnColor.Value.z, Menu.ToggleOnColor.Value.w),
			toggleAnim
		);
		
		drawList->AddRectFilled(
			ImVec2(toggleX, toggleY),
			ImVec2(toggleX + toggleWidth, toggleY + toggleHeight),
			trackColor, toggleHeight / 2.0f
		);
		
		float knobRadius = (toggleHeight - 6) / 2.0f;
		float knobX = toggleX + 3 + knobRadius + (toggleWidth - 6 - knobRadius * 2) * toggleAnim;
		float knobY = toggleY + toggleHeight / 2.0f;
		
		if (*v) {
			for (int i = 4; i > 0; i--) {
				float glowRadius = knobRadius + i * 2.5f;
				float glowAlpha = (1.0f - (i / 4.0f)) * 0.4f;
				ImColor glowColor = ImColor(
					Menu.ToggleOnColor.Value.x,
					Menu.ToggleOnColor.Value.y,
					Menu.ToggleOnColor.Value.z,
					glowAlpha
				);
				drawList->AddCircleFilled(ImVec2(knobX, knobY), glowRadius, glowColor, 32);
			}
		}
		
		drawList->AddCircleFilled(ImVec2(knobX, knobY), knobRadius, Menu.ToggleKnobColor, 32);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 12), Menu.TextColor, label);
		ImGui::PopFont();
		
		if (description) {
			ImGui::PushFont(Menu.InterSmaller);
			drawList->AddText(ImVec2(pos.x + 15, pos.y + 30), Menu.TextDimColor, description);
			ImGui::PopFont();
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 5));
		ImGui::PopID();
		
		return clicked;
	}
	
	bool SoftToggleCompact(const char* label, bool* v) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 36);
		
		bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		bool clicked = hovered && ImGui::IsMouseClicked(0) && !IsAnyComboOpen();
		
		if (clicked) *v = !*v;
		
		std::string id = label;
		float& toggleAnim = toggleAnimations[id];
		float& hoverAnim = hoverAnimations[id];
		
		toggleAnim = ImLerp(toggleAnim, *v ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		hoverAnim = ImLerp(hoverAnim, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		float toggleWidth = 36.0f;
		float toggleHeight = 20.0f;
		float toggleX = pos.x + size.x - toggleWidth - 10.0f;
		float toggleY = pos.y + (size.y - toggleHeight) / 2.0f;
		
		ImColor trackColor = ImLerp(
			ImVec4(Menu.ToggleOffColor.Value.x, Menu.ToggleOffColor.Value.y, Menu.ToggleOffColor.Value.z, Menu.ToggleOffColor.Value.w),
			ImVec4(Menu.ToggleOnColor.Value.x, Menu.ToggleOnColor.Value.y, Menu.ToggleOnColor.Value.z, Menu.ToggleOnColor.Value.w),
			toggleAnim
		);
		
		drawList->AddRectFilled(
			ImVec2(toggleX, toggleY),
			ImVec2(toggleX + toggleWidth, toggleY + toggleHeight),
			trackColor, toggleHeight / 2.0f
		);
		
		float knobRadius = (toggleHeight - 4) / 2.0f;
		float knobX = toggleX + 2 + knobRadius + (toggleWidth - 4 - knobRadius * 2) * toggleAnim;
		float knobY = toggleY + toggleHeight / 2.0f;
		
		if (*v) {
			for (int i = 4; i > 0; i--) {
				float glowRadius = knobRadius + i * 1.8f;
				float glowAlpha = (1.0f - (i / 4.0f)) * 0.4f;
				ImColor glowColor = ImColor(
					Menu.ToggleOnColor.Value.x,
					Menu.ToggleOnColor.Value.y,
					Menu.ToggleOnColor.Value.z,
					glowAlpha
				);
				drawList->AddCircleFilled(ImVec2(knobX, knobY), glowRadius, glowColor, 32);
			}
		}
		
		drawList->AddCircleFilled(ImVec2(knobX, knobY), knobRadius, Menu.ToggleKnobColor, 32);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 10, pos.y + (size.y - 14) / 2.0f), Menu.TextColor, label);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 3));
		ImGui::PopID();
		
		return clicked;
	}
	
	void SoftSlider(const char* label, float* v, float min, float max, const char* format = "%.1f") {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 50);
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), Menu.CardColor, 8.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 8), Menu.TextColor, label);
		ImGui::PopFont();
		
		char valueStr[32];
		snprintf(valueStr, sizeof(valueStr), format, *v);
		ImVec2 valueSize = ImGui::CalcTextSize(valueStr);
		drawList->AddText(ImVec2(pos.x + size.x - valueSize.x - 15, pos.y + 8), Menu.AccentColor, valueStr);
		
		float sliderY = pos.y + 30;
		float sliderHeight = 6.0f;
		float sliderWidth = size.x - 30;
		float sliderX = pos.x + 15;
		
		std::string sliderId = std::string(label) + "_slider";
		float& progressAnim = sliderProgressAnimations[sliderId];
		float& hoverAnim = sliderHoverAnimations[sliderId];
		
		float range = max - min;
		if (range <= 0.0f) range = 1.0f;
		float targetProgress = (*v - min) / range;
		targetProgress = ImClamp(targetProgress, 0.0f, 1.0f);
		
		progressAnim = ImLerp(progressAnim, targetProgress, ImGui::GetIO().DeltaTime * 15.0f);
		
		ImGui::SetCursorScreenPos(ImVec2(sliderX, sliderY - 8));
		ImGui::InvisibleButton("##slider", ImVec2(sliderWidth, sliderHeight + 16));
		
		bool isHovered = ImGui::IsItemHovered();
		bool isActive = ImGui::IsItemActive();
		hoverAnim = ImLerp(hoverAnim, (isHovered || isActive) ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		float currentKnobRadius = 5.0f + hoverAnim * 1.0f;
		float currentSliderHeight = sliderHeight;
		float currentSliderY = sliderY;
		
		drawList->AddRectFilled(
			ImVec2(sliderX, currentSliderY),
			ImVec2(sliderX + sliderWidth, currentSliderY + currentSliderHeight),
			Menu.ToggleOffColor, currentSliderHeight / 2.0f
		);
		
		ImVec4 progressColorVec = ImVec4(
			Menu.AccentColor.Value.x + (Menu.AccentColorHover.Value.x - Menu.AccentColor.Value.x) * hoverAnim,
			Menu.AccentColor.Value.y + (Menu.AccentColorHover.Value.y - Menu.AccentColor.Value.y) * hoverAnim,
			Menu.AccentColor.Value.z + (Menu.AccentColorHover.Value.z - Menu.AccentColor.Value.z) * hoverAnim,
			Menu.AccentColor.Value.w + (Menu.AccentColorHover.Value.w - Menu.AccentColor.Value.w) * hoverAnim
		);
		ImColor progressColor = ImColor(progressColorVec);
		
		drawList->AddRectFilled(
			ImVec2(sliderX, currentSliderY),
			ImVec2(sliderX + sliderWidth * progressAnim, currentSliderY + currentSliderHeight),
			progressColor, currentSliderHeight / 2.0f
		);
		
		float knobX = sliderX + sliderWidth * progressAnim;
		float knobY = currentSliderY + currentSliderHeight / 2.0f;
		
		ImColor knobColor = ImColor(255, 255, 255, 255);
		drawList->AddCircleFilled(ImVec2(knobX, knobY), currentKnobRadius, knobColor, 32);
		
		if (isActive || isHovered) {
			if (ImGui::IsMouseDown(0)) {
				float mouseX = ImGui::GetMousePos().x;
				float newProgress = (mouseX - sliderX) / sliderWidth;
				newProgress = ImClamp(newProgress, 0.0f, 1.0f);
				*v = min + range * newProgress;
				progressAnim = newProgress;
			}
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 5));
		ImGui::PopID();
	}
	
	void SoftSliderInt(const char* label, int* v, int min, int max, const char* format = "%d") {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 50);
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), Menu.CardColor, 8.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 8), Menu.TextColor, label);
		ImGui::PopFont();
		
		char valueStr[32];
		snprintf(valueStr, sizeof(valueStr), format, *v);
		ImVec2 valueSize = ImGui::CalcTextSize(valueStr);
		drawList->AddText(ImVec2(pos.x + size.x - valueSize.x - 15, pos.y + 8), Menu.AccentColor, valueStr);
		
		float sliderY = pos.y + 30;
		float sliderHeight = 6.0f;
		float sliderWidth = size.x - 30;
		float sliderX = pos.x + 15;
		
		std::string sliderId = std::string(label) + "_sliderint";
		float& progressAnim = sliderProgressAnimations[sliderId];
		float& hoverAnim = sliderHoverAnimations[sliderId];
		
		float range = (float)(max - min);
		if (range <= 0.0f) range = 1.0f;
		float targetProgress = (float)(*v - min) / range;
		targetProgress = ImClamp(targetProgress, 0.0f, 1.0f);
		
		progressAnim = ImLerp(progressAnim, targetProgress, ImGui::GetIO().DeltaTime * 15.0f);
		
		ImGui::SetCursorScreenPos(ImVec2(sliderX, sliderY - 8));
		ImGui::InvisibleButton("##sliderint", ImVec2(sliderWidth, sliderHeight + 16));
		
		bool isHovered = ImGui::IsItemHovered();
		bool isActive = ImGui::IsItemActive();
		hoverAnim = ImLerp(hoverAnim, (isHovered || isActive) ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		float currentKnobRadius = 5.0f + hoverAnim * 1.0f;
		float currentSliderHeight = sliderHeight;
		float currentSliderY = sliderY;
		
		drawList->AddRectFilled(
			ImVec2(sliderX, currentSliderY),
			ImVec2(sliderX + sliderWidth, currentSliderY + currentSliderHeight),
			Menu.ToggleOffColor, currentSliderHeight / 2.0f
		);
		
		ImVec4 progressColorVec = ImVec4(
			Menu.AccentColor.Value.x + (Menu.AccentColorHover.Value.x - Menu.AccentColor.Value.x) * hoverAnim,
			Menu.AccentColor.Value.y + (Menu.AccentColorHover.Value.y - Menu.AccentColor.Value.y) * hoverAnim,
			Menu.AccentColor.Value.z + (Menu.AccentColorHover.Value.z - Menu.AccentColor.Value.z) * hoverAnim,
			Menu.AccentColor.Value.w + (Menu.AccentColorHover.Value.w - Menu.AccentColor.Value.w) * hoverAnim
		);
		ImColor progressColor = ImColor(progressColorVec);
		
		drawList->AddRectFilled(
			ImVec2(sliderX, currentSliderY),
			ImVec2(sliderX + sliderWidth * progressAnim, currentSliderY + currentSliderHeight),
			progressColor, currentSliderHeight / 2.0f
		);
		
		float knobX = sliderX + sliderWidth * progressAnim;
		float knobY = currentSliderY + currentSliderHeight / 2.0f;
		
		ImColor knobColor = ImColor(255, 255, 255, 255);
		drawList->AddCircleFilled(ImVec2(knobX, knobY), currentKnobRadius, knobColor, 32);
		
		if (isActive || isHovered) {
			if (ImGui::IsMouseDown(0)) {
				float mouseX = ImGui::GetMousePos().x;
				float newProgress = (mouseX - sliderX) / sliderWidth;
				newProgress = ImClamp(newProgress, 0.0f, 1.0f);
				*v = min + (int)(range * newProgress + 0.5f);
				*v = ImClamp(*v, min, max);
				progressAnim = newProgress;
			}
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 5));
		ImGui::PopID();
	}
	
	bool SoftCombo(const char* label, int* current, const char* const items[], int items_count) {
		ImGui::PushID(label);
		
		std::string id = label;
		bool& isOpen = comboOpenStates[id];
		float& openAnim = comboAnimations[id];
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 40);
		
		float comboWidth = 180.0f;
		float comboX = pos.x + size.x - comboWidth - 15;
		float comboY = pos.y + 8;
		float comboHeight = 24.0f;
		
		ImVec2 comboMin = ImVec2(comboX, comboY);
		ImVec2 comboMax = ImVec2(comboX + comboWidth, comboY + comboHeight);
		
		bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		bool clicked = hovered && ImGui::IsMouseClicked(0);
		
		if (clicked) {
			isOpen = !isOpen;
		}
		
		if (isOpen) {
			float dropdownAreaY = pos.y + size.y + 5.0f;
			float dropdownAreaHeight = items_count * 36.0f;
			ImVec2 mousePos = ImGui::GetMousePos();
			bool mouseInDropdown = mousePos.x >= comboX && mousePos.x <= comboX + comboWidth &&
			                        mousePos.y >= dropdownAreaY && mousePos.y <= dropdownAreaY + dropdownAreaHeight;
			bool mouseInCombo = ImGui::IsMouseHoveringRect(comboMin, comboMax);
			
			if (!mouseInDropdown && !mouseInCombo && ImGui::IsMouseClicked(0)) {
				isOpen = false;
			}
		}
		
		openAnim = ImLerp(openAnim, isOpen ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 15.0f);
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), 
			hovered ? Menu.CardHoverColor : Menu.CardColor, 8.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 12), Menu.TextColor, label);
		ImGui::PopFont();
		
		bool comboHovered = ImGui::IsMouseHoveringRect(comboMin, comboMax);
		
		ImColor comboBg = comboHovered ? Menu.CardHoverColor : Menu.CardColor;
		drawList->AddRectFilled(comboMin, comboMax, comboBg, 6.0f);
		drawList->AddRect(comboMin, comboMax, ImColor(255, 255, 255, 20), 6.0f, 0, 1.0f);
		
		ImGui::PushFont(Menu.InterSmaller);
		std::string currentText = items[*current];
		ImVec2 textSize = ImGui::CalcTextSize(currentText.c_str());
		drawList->AddText(ImVec2(comboX + 10, comboY + (comboHeight - textSize.y) / 2.0f), Menu.TextColor, currentText.c_str());
		
		float arrowX = comboX + comboWidth - 20.0f;
		float arrowY = comboY + comboHeight / 2.0f;
		float arrowSize = 4.0f;
		float arrowOffset = openAnim * arrowSize;
		
		ImColor arrowColor = Menu.TextColor;
		drawList->AddTriangleFilled(
			ImVec2(arrowX, arrowY - arrowSize + arrowOffset),
			ImVec2(arrowX - arrowSize, arrowY + arrowOffset),
			ImVec2(arrowX + arrowSize, arrowY + arrowOffset),
			arrowColor
		);
		ImGui::PopFont();
		
		if (openAnim > 0.01f) {
			float dropdownY = pos.y + size.y + 5.0f;
			float dropdownHeight = (items_count * 36.0f) * openAnim;
			float dropdownAlpha = openAnim;
			
			ImDrawList* foregroundDrawList = ImGui::GetForegroundDrawList();
			
			ImVec2 dropdownMin = ImVec2(comboX, dropdownY);
			ImVec2 dropdownMax = ImVec2(comboX + comboWidth, dropdownY + dropdownHeight);
			
			ImColor dropdownBg = Menu.CardColor;
			dropdownBg.Value.w *= dropdownAlpha;
			foregroundDrawList->AddRectFilled(dropdownMin, dropdownMax, dropdownBg, 6.0f);
			foregroundDrawList->AddRect(dropdownMin, dropdownMax, ImColor(255, 255, 255, (int)(20 * dropdownAlpha)), 6.0f, 0, 1.0f);
			
			for (int i = 0; i < items_count; i++) {
				float itemY = dropdownY + (i * 36.0f);
				ImVec2 itemMin = ImVec2(comboX, itemY);
				ImVec2 itemMax = ImVec2(comboX + comboWidth, itemY + 36.0f);
				
				bool itemHovered = ImGui::IsMouseHoveringRect(itemMin, itemMax) && openAnim > 0.5f;
				bool itemClicked = itemHovered && ImGui::IsMouseClicked(0);
				
				if (itemClicked) {
					*current = i;
					isOpen = false;
				}
				
				ImColor itemBg = (*current == i) ? Menu.AccentColor : (itemHovered ? Menu.CardHoverColor : ImColor(0, 0, 0, 0));
				itemBg.Value.w *= dropdownAlpha;
				if (itemBg.Value.w > 0.01f) {
					foregroundDrawList->AddRectFilled(itemMin, itemMax, itemBg, 0.0f);
				}
				
				if (*current == i) {
					ImColor checkColor = ImColor(255, 255, 255, (int)(255 * dropdownAlpha));
					foregroundDrawList->AddCircleFilled(ImVec2(comboX + 12, itemY + 18), 3.0f, checkColor, 8);
				}
				
				ImGui::PushFont(Menu.InterSmaller);
				ImVec2 itemTextSize = ImGui::CalcTextSize(items[i]);
				ImColor itemTextColor;
				if (*current == i) {
					itemTextColor = ImColor(255, 255, 255, (int)(255 * dropdownAlpha));
				} else {
					itemTextColor = ImColor(
						(int)(Menu.TextColor.Value.x * 255.0f),
						(int)(Menu.TextColor.Value.y * 255.0f),
						(int)(Menu.TextColor.Value.z * 255.0f),
						(int)(Menu.TextColor.Value.w * 255.0f * dropdownAlpha)
					);
				}
				foregroundDrawList->AddText(ImVec2(comboX + 25, itemY + (36.0f - itemTextSize.y) / 2.0f), itemTextColor, items[i]);
				ImGui::PopFont();
			}
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 5));
		ImGui::PopID();
		
		return false;
	}
	
	bool SoftHitboxSelector(const char* label, int* current, const char* const items[], int items_count) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 0);
		
		float buttonSpacing = 6.0f;
		float buttonHeight = 32.0f;
		float buttonWidth = (size.x - 30 - (buttonSpacing * (items_count - 1))) / items_count;
		float startX = pos.x + 15;
		float buttonY = pos.y + 35;
		
		float totalHeight = 50.0f + buttonHeight;
		
		bool cardHovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + totalHeight));
		
		ImColor cardBg = cardHovered ? Menu.CardHoverColor : Menu.CardColor;
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + totalHeight), cardBg, 8.0f);
		drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + totalHeight), Menu.BorderColor, 8.0f, 0, 1.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 12), Menu.TextColor, label);
		ImGui::PopFont();
		
		for (int i = 0; i < items_count; i++) {
			float btnX = startX + (i * (buttonWidth + buttonSpacing));
			ImVec2 btnMin = ImVec2(btnX, buttonY);
			ImVec2 btnMax = ImVec2(btnX + buttonWidth, buttonY + buttonHeight);
			
			bool isSelected = (*current == i);
			bool btnHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
			bool btnClicked = btnHovered && ImGui::IsMouseClicked(0) && !IsAnyComboOpen();
			
			std::string btnId = std::string(label) + "_btn_" + std::to_string(i);
			float& pressAnim = buttonPressAnimations[btnId];
			
			if (btnClicked) {
				pressAnim = 1.0f;
				*current = i;
			}
			pressAnim = ImLerp(pressAnim, 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
			
			ImColor bgColor;
			if (isSelected) {
				bgColor = Menu.AccentColor;
			} else {
				bgColor = btnHovered ? Menu.ElementsHoverColor : Menu.ElementsColor;
			}
			
			if (pressAnim > 0.01f && !isSelected) {
				float accentR = Menu.AccentColor.Value.x * 0.25f + bgColor.Value.x * 0.75f;
				float accentG = Menu.AccentColor.Value.y * 0.25f + bgColor.Value.y * 0.75f;
				float accentB = Menu.AccentColor.Value.z * 0.25f + bgColor.Value.z * 0.75f;
				bgColor = ImLerp(
					ImVec4(bgColor.Value.x, bgColor.Value.y, bgColor.Value.z, 1.0f),
					ImVec4(accentR, accentG, accentB, 1.0f),
					pressAnim
				);
			}
			
			drawList->AddRectFilled(btnMin, btnMax, bgColor, 5.0f);
			
			ImColor borderColor = isSelected ? ImColor(255, 255, 255, 80) : (btnHovered ? ImColor(255, 255, 255, 25) : ImColor(255, 255, 255, 12));
			drawList->AddRect(btnMin, btnMax, borderColor, 5.0f, 0, 1.0f);
			
			if (isSelected) {
				for (int j = 2; j > 0; j--) {
					float glowAlpha = (1.0f - (j / 2.0f)) * 0.15f;
					ImColor glowColor = ImColor(
						Menu.AccentColor.Value.x,
						Menu.AccentColor.Value.y,
						Menu.AccentColor.Value.z,
						glowAlpha
					);
					drawList->AddRectFilled(
						ImVec2(btnMin.x - j * 0.8f, btnMin.y - j * 0.4f),
						ImVec2(btnMax.x + j * 0.8f, btnMax.y + j * 0.4f),
						glowColor, 5.0f + j * 0.2f
					);
				}
			}
			
			ImGui::PushFont(Menu.InterSmaller);
			ImVec2 textSize = ImGui::CalcTextSize(items[i]);
			ImVec2 textPos = ImVec2(
				btnX + (buttonWidth - textSize.x) / 2.0f,
				buttonY + (buttonHeight - textSize.y) / 2.0f
			);
			
			ImColor itemTextColor = isSelected ? ImColor(255, 255, 255) : Menu.TextColor;
			drawList->AddText(textPos, itemTextColor, items[i]);
			ImGui::PopFont();
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + totalHeight + 5));
		ImGui::PopID();
		
		return false;
	}
	
	bool SoftPositionSelector(const char* label, int* current, const char* const items[], int items_count, bool* parentToggle = nullptr) {
		ImGui::PushID(label);
		
		std::string animKey = std::string(label);
		float& expandAnim = selectorExpandAnimations[animKey];
		
		bool shouldShow = parentToggle ? *parentToggle : true;
		float targetAnim = shouldShow ? 1.0f : 0.0f;
		expandAnim = ImLerp(expandAnim, targetAnim, ImGui::GetIO().DeltaTime * 15.0f);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 originalPos = ImGui::GetCursorScreenPos();
		float totalWidth = ImGui::GetContentRegionAvail().x;
		float totalHeight = 36.0f;
		
		float animHeight = totalHeight * expandAnim;
		float animSpacing = 3.0f * expandAnim;
		float animOffsetY = (1.0f - expandAnim) * -20.0f;
		float animAlpha = expandAnim;
		
		float finalCursorY = originalPos.y + animHeight + animSpacing;
		
		if (expandAnim < 0.01f) {
			ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
			ImGui::PopID();
			return false;
		}
		
		ImVec2 pos = originalPos;
		pos.y += animOffsetY;
		
		ImColor cardColor = Menu.CardColor;
		cardColor.Value.w *= animAlpha;
		drawList->AddRectFilled(pos, ImVec2(pos.x + totalWidth, pos.y + animHeight), cardColor, 6.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		ImColor textColor = Menu.TextColor;
		textColor.Value.w *= animAlpha;
		drawList->AddText(ImVec2(pos.x + 12, pos.y + 10), textColor, label);
		ImGui::PopFont();
		
		float buttonSpacing = 6.0f;
		float buttonWidth = 60.0f;
		float totalButtonsWidth = (buttonWidth * items_count) + (buttonSpacing * (items_count - 1));
		float startX = pos.x + totalWidth - totalButtonsWidth - 12;
		float buttonY = pos.y + 5;
		float buttonHeight = 26.0f;
		
		bool changed = false;
		
		for (int i = 0; i < items_count; i++) {
			float btnX = startX + (i * (buttonWidth + buttonSpacing));
			ImVec2 btnMin = ImVec2(btnX, buttonY);
			ImVec2 btnMax = ImVec2(btnX + buttonWidth, buttonY + buttonHeight);
			
			bool isSelected = (*current == i);
			bool btnHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
			bool btnClicked = btnHovered && ImGui::IsMouseClicked(0);
			
			std::string btnId = std::string(label) + "_btn_" + std::to_string(i);
			float& pressAnim = buttonPressAnimations[btnId];
			
			if (btnClicked) {
				pressAnim = 1.0f;
				if (!isSelected) {
					*current = i;
					changed = true;
				}
			}
			pressAnim = ImLerp(pressAnim, 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
			
			ImColor bgColor;
			if (isSelected) {
				bgColor = Menu.AccentColor;
			} else {
				bgColor = Menu.CardColor;
			}
			
			if (pressAnim > 0.01f && !isSelected) {
				float orangeR = Menu.AccentColor.Value.x * 0.2f + Menu.CardColor.Value.x * 0.8f;
				float orangeG = Menu.AccentColor.Value.y * 0.2f + Menu.CardColor.Value.y * 0.8f;
				float orangeB = Menu.AccentColor.Value.z * 0.2f + Menu.CardColor.Value.z * 0.8f;
				bgColor = ImLerp(
					ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f),
					ImVec4(orangeR, orangeG, orangeB, 1.0f),
					pressAnim
				);
			}
			
			bgColor.Value.w *= animAlpha;
			drawList->AddRectFilled(btnMin, btnMax, bgColor, 4.0f);
			
			ImColor borderColor = isSelected ? ImColor(255, 255, 255, 50) : ImColor(255, 255, 255, 20);
			borderColor.Value.w *= animAlpha;
			drawList->AddRect(btnMin, btnMax, borderColor, 4.0f, 0, 1.0f);
			
			ImGui::PushFont(Menu.InterSmaller);
			ImVec2 textSize = ImGui::CalcTextSize(items[i]);
			ImVec2 textPos = ImVec2(
				btnX + (buttonWidth - textSize.x) / 2.0f,
				buttonY + (buttonHeight - textSize.y) / 2.0f
			);
			
			ImColor itemTextColor = isSelected ? ImColor(255, 255, 255) : Menu.TextColor;
			itemTextColor.Value.w *= animAlpha;
			drawList->AddText(textPos, itemTextColor, items[i]);
			ImGui::PopFont();
		}
		
		ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
		ImGui::PopID();
		
		return changed;
	}
	
	bool SoftIndicatorTypeSelector(const char* label, int* current, const char* const items[], int items_count, bool* parentToggle = nullptr) {
		ImGui::PushID(label);
		
		std::string animKey = std::string(label);
		float& expandAnim = selectorExpandAnimations[animKey];
		
		bool shouldShow = parentToggle ? *parentToggle : true;
		float targetAnim = shouldShow ? 1.0f : 0.0f;
		expandAnim = ImLerp(expandAnim, targetAnim, ImGui::GetIO().DeltaTime * 15.0f);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 originalPos = ImGui::GetCursorScreenPos();
		float totalWidth = ImGui::GetContentRegionAvail().x;
		float totalHeight = 40.0f;
		
		float animHeight = totalHeight * expandAnim;
		float animSpacing = 3.0f * expandAnim;
		float animOffsetY = (1.0f - expandAnim) * -20.0f;
		float animAlpha = expandAnim;
		
		float finalCursorY = originalPos.y + animHeight + animSpacing;
		
		if (expandAnim < 0.01f) {
			ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
			ImGui::PopID();
			return false;
		}
		
		ImVec2 pos = originalPos;
		pos.y += animOffsetY;
		
		ImColor cardColor = Menu.CardColor;
		cardColor.Value.w *= animAlpha;
		drawList->AddRectFilled(pos, ImVec2(pos.x + totalWidth, pos.y + animHeight), cardColor, 6.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		ImColor textColor = Menu.TextColor;
		textColor.Value.w *= animAlpha;
		drawList->AddText(ImVec2(pos.x + 12, pos.y + 10), textColor, label);
		ImGui::PopFont();
		
		float buttonSpacing = 6.0f;
		float buttonWidth = 70.0f;
		float totalButtonsWidth = (buttonWidth * items_count) + (buttonSpacing * (items_count - 1));
		float startX = pos.x + totalWidth - totalButtonsWidth - 12;
		float buttonY = pos.y + 5;
		float buttonHeight = 26.0f;
		
		bool changed = false;
		
		for (int i = 0; i < items_count; i++) {
			float btnX = startX + (i * (buttonWidth + buttonSpacing));
			ImVec2 btnMin = ImVec2(btnX, buttonY);
			ImVec2 btnMax = ImVec2(btnX + buttonWidth, buttonY + buttonHeight);
			
			bool isSelected = (*current == i);
			bool btnHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
			bool btnClicked = btnHovered && ImGui::IsMouseClicked(0);
			
			std::string btnId = std::string(label) + "_btn_" + std::to_string(i);
			float& pressAnim = buttonPressAnimations[btnId];
			
			if (btnClicked) {
				pressAnim = 1.0f;
				if (!isSelected) {
					*current = i;
					changed = true;
				}
			}
			pressAnim = ImLerp(pressAnim, 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
			
			ImColor bgColor;
			if (isSelected) {
				bgColor = Menu.AccentColor;
			} else {
				bgColor = Menu.CardColor;
			}
			
			if (pressAnim > 0.01f && !isSelected) {
				float orangeR = Menu.AccentColor.Value.x * 0.2f + Menu.CardColor.Value.x * 0.8f;
				float orangeG = Menu.AccentColor.Value.y * 0.2f + Menu.CardColor.Value.y * 0.8f;
				float orangeB = Menu.AccentColor.Value.z * 0.2f + Menu.CardColor.Value.z * 0.8f;
				bgColor = ImLerp(
					ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f),
					ImVec4(orangeR, orangeG, orangeB, 1.0f),
					pressAnim
				);
			}
			
			bgColor.Value.w *= animAlpha;
			drawList->AddRectFilled(btnMin, btnMax, bgColor, 4.0f);
			
			ImColor borderColor = isSelected ? ImColor(255, 255, 255, 50) : ImColor(255, 255, 255, 20);
			borderColor.Value.w *= animAlpha;
			drawList->AddRect(btnMin, btnMax, borderColor, 4.0f, 0, 1.0f);
			
			ImGui::PushFont(Menu.InterSmaller);
			ImVec2 textSize = ImGui::CalcTextSize(items[i]);
			ImVec2 textPos = ImVec2(
				btnX + (buttonWidth - textSize.x) / 2.0f,
				buttonY + (buttonHeight - textSize.y) / 2.0f
			);
			
			ImColor itemTextColor = isSelected ? ImColor(255, 255, 255) : Menu.TextColor;
			itemTextColor.Value.w *= animAlpha;
			drawList->AddText(textPos, itemTextColor, items[i]);
			ImGui::PopFont();
		}
		
		ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
		ImGui::PopID();
		
		return changed;
	}
	
	void SoftColorEdit(const char* label, ImColor& color) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 36);
		
		bool cardHovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		
		ImColor cardBg = cardHovered ? Menu.CardHoverColor : Menu.CardColor;
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), cardBg, 6.0f);
		drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), Menu.BorderColor, 6.0f, 0, 1.0f);
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 12, pos.y + 10), Menu.TextColor, label);
		ImGui::PopFont();
		
		float colorButtonWidth = 100.0f;
		float colorButtonHeight = 26.0f;
		float colorButtonX = pos.x + size.x - colorButtonWidth - 12;
		float colorButtonY = pos.y + 5;
		
		ImVec2 colorBtnMin = ImVec2(colorButtonX, colorButtonY);
		ImVec2 colorBtnMax = ImVec2(colorButtonX + colorButtonWidth, colorButtonY + colorButtonHeight);
		
		bool colorBtnHovered = ImGui::IsMouseHoveringRect(colorBtnMin, colorBtnMax);
		bool colorBtnClicked = colorBtnHovered && ImGui::IsMouseClicked(0) && !IsAnyComboOpen();
		
		std::string btnId = std::string(label) + "_colorbtn";
		std::string popupId = std::string("##colorpicker_") + label;
		float& pressAnim = buttonPressAnimations[btnId];
		
		if (colorBtnClicked) {
			pressAnim = 1.0f;
		}
		pressAnim = ImLerp(pressAnim, 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
		
		ImColor buttonBg = colorBtnHovered ? Menu.ElementsHoverColor : Menu.ElementsColor;
		
		if (pressAnim > 0.01f) {
			float accentR = Menu.AccentColor.Value.x * 0.2f + buttonBg.Value.x * 0.8f;
			float accentG = Menu.AccentColor.Value.y * 0.2f + buttonBg.Value.y * 0.8f;
			float accentB = Menu.AccentColor.Value.z * 0.2f + buttonBg.Value.z * 0.8f;
			buttonBg = ImLerp(
				ImVec4(buttonBg.Value.x, buttonBg.Value.y, buttonBg.Value.z, 1.0f),
				ImVec4(accentR, accentG, accentB, 1.0f),
				pressAnim
			);
		}
		
		drawList->AddRectFilled(colorBtnMin, colorBtnMax, buttonBg, 4.0f);
		
		float colorBoxSize = 18.0f;
		float colorBoxX = colorButtonX + 6;
		float colorBoxY = colorButtonY + (colorButtonHeight - colorBoxSize) / 2.0f;
		
		drawList->AddRectFilled(
			ImVec2(colorBoxX, colorBoxY),
			ImVec2(colorBoxX + colorBoxSize, colorBoxY + colorBoxSize),
			color, 3.0f
		);
		drawList->AddRect(
			ImVec2(colorBoxX, colorBoxY),
			ImVec2(colorBoxX + colorBoxSize, colorBoxY + colorBoxSize),
			ImColor(255, 255, 255, 35), 3.0f, 0, 1.0f
		);
		
		ImGui::PushFont(Menu.InterSmaller);
		const char* editText = "Edit";
		ImVec2 textSize = ImGui::CalcTextSize(editText);
		float textX = colorBoxX + colorBoxSize + 6;
		float textY = colorButtonY + (colorButtonHeight - textSize.y) / 2.0f;
		ImColor textColor = colorBtnHovered ? Menu.TextActiveColor : Menu.TextColor;
		drawList->AddText(ImVec2(textX, textY), textColor, editText);
		ImGui::PopFont();
		
		ImColor borderColor = colorBtnHovered ? ImColor(255, 255, 255, 35) : ImColor(255, 255, 255, 15);
		drawList->AddRect(colorBtnMin, colorBtnMax, borderColor, 4.0f, 0, 1.0f);
		
		ImGui::SetCursorScreenPos(colorBtnMin);
		ImVec4 col = color.Value;
		if (ImGui::InvisibleButton("##colorbtn", ImVec2(colorButtonWidth, colorButtonHeight))) {
			ImGui::OpenPopup(popupId.c_str());
		}
		
		if (ImGui::BeginPopup(popupId.c_str())) {
			ImDrawList* popupDrawList = ImGui::GetWindowDrawList();
			ImVec2 popupPos = ImGui::GetWindowPos();
			ImVec2 popupSize = ImGui::GetWindowSize();
			
			ImColor popupBg = ImColor(18, 18, 22, 255);
			ImColor popupBorder = ImColor(40, 40, 48, 255);
			
			popupDrawList->AddRectFilled(popupPos, ImVec2(popupPos.x + popupSize.x, popupPos.y + popupSize.y), popupBg, 8.0f);
			popupDrawList->AddRect(popupPos, ImVec2(popupPos.x + popupSize.x, popupPos.y + popupSize.y), popupBorder, 8.0f, 0, 1.5f);
			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			
			ImGui::ColorPicker4("##picker", (float*)&col, 
				ImGuiColorEditFlags_NoSidePreview | 
				ImGuiColorEditFlags_NoSmallPreview | 
				ImGuiColorEditFlags_AlphaBar |
				ImGuiColorEditFlags_AlphaPreviewHalf |
				ImGuiColorEditFlags_DisplayRGB |
				ImGuiColorEditFlags_InputRGB |
				ImGuiColorEditFlags_PickerHueBar);
			
			color = ImColor(col);
			
			ImGui::PopStyleColor(6);
			ImGui::PopStyleVar(5);
			ImGui::EndPopup();
		}

		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 5));
		ImGui::PopID();
	}
	
	bool ListItem(const char* label, const char* sublabel, bool selected, bool isFriend = false) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float itemHeight = (sublabel && strlen(sublabel) > 0) ? 36.0f : 28.0f;
		float itemWidth = ImGui::GetContentRegionAvail().x;
		
		ImVec2 itemMin = pos;
		ImVec2 itemMax = ImVec2(pos.x + itemWidth, pos.y + itemHeight);
		
		bool hovered = ImGui::IsMouseHoveringRect(itemMin, itemMax);
		bool clicked = hovered && ImGui::IsMouseClicked(0);
		
		if (selected) {
			drawList->AddRectFilled(itemMin, itemMax, ImColor(30, 30, 36, 200), 4.0f);
			drawList->AddRectFilled(itemMin, itemMax, ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.08f), 4.0f);
			drawList->AddRect(itemMin, itemMax, ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.25f), 4.0f, 0, 0.5f);
		} else if (hovered) {
			drawList->AddRectFilled(itemMin, itemMax, ImColor(255, 255, 255, 15), 4.0f);
		}
		
		float textX = pos.x + 10.0f;
		float centerY = pos.y + (itemHeight / 2.0f);
		
		if (sublabel && strlen(sublabel) > 0) {
			ImGui::PushFont(Menu.SegoeUI);
			ImColor labelColor = selected ? Menu.TextActiveColor : Menu.TextColor;
			drawList->AddText(ImVec2(textX, centerY - 10), labelColor, label);
			ImGui::PopFont();
			
			ImGui::PushFont(Menu.InterSmaller);
			ImColor subColor = selected ? Menu.TextDimColor : Menu.TextDimColor;
			drawList->AddText(ImVec2(textX, centerY + 4), subColor, sublabel);
			ImGui::PopFont();
		} else {
			ImGui::PushFont(Menu.SegoeUI);
			ImVec2 textSize = ImGui::CalcTextSize(label);
			ImColor labelColor = selected ? Menu.TextActiveColor : Menu.TextColor;
			drawList->AddText(ImVec2(textX, centerY - textSize.y / 2.0f), labelColor, label);
			ImGui::PopFont();
		}
		
		if (isFriend) {
			ImGui::PushFont(Menu.FontAwesome);
			float iconX = itemMax.x - 24;
			ImColor heartColor = selected ? ImColor(140, 140, 150, 200) : ImColor(80, 200, 120);
			drawList->AddText(ImVec2(iconX, centerY - 7), heartColor, ICON_FA_HEART);
			ImGui::PopFont();
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + itemHeight + 2));
		ImGui::PopID();
		
		return clicked;
	}
	
	void ListHeader(const char* text, int count) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetContentRegionAvail().x;
		
		ImGui::PushFont(Menu.InterSmaller);
		drawList->AddText(ImVec2(pos.x + 8, pos.y), Menu.TextDimColor, text);
		
		char countStr[32];
		sprintf_s(countStr, "%d", count);
		ImVec2 countSize = ImGui::CalcTextSize(countStr);
		
		float badgeX = pos.x + width - countSize.x - 16;
		drawList->AddRectFilled(ImVec2(badgeX, pos.y - 2), ImVec2(badgeX + countSize.x + 10, pos.y + 16), Menu.AccentColor, 8.0f);
		drawList->AddText(ImVec2(badgeX + 5, pos.y), ImColor(255, 255, 255), countStr);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 22));
	}
	
	void SearchBox(const char* hint, char* buffer, size_t bufferSize) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetContentRegionAvail().x;
		float height = 40.0f;
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), Menu.CardColor, 10.0f);
		drawList->AddRect(pos, ImVec2(pos.x + width, pos.y + height), ImColor(50, 50, 60, 100), 10.0f);
		
		ImGui::PushFont(Menu.FontAwesome);
		drawList->AddText(ImVec2(pos.x + 14, pos.y + 11), Menu.TextDimColor, ICON_FA_MAGNIFYING_GLASS);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x + 40, pos.y + 10));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(Menu.TextColor.Value.x, Menu.TextColor.Value.y, Menu.TextColor.Value.z, 1.0f));
		ImGui::SetNextItemWidth(width - 55);
		ImGui::InputTextWithHint("##search", hint, buffer, bufferSize);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + height + 10));
	}
	
	void ExpandableSectionHeader(const char* title, const char* icon, bool* isOpen) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetContentRegionAvail().x;
		
		float headerHeight = 40.0f;
		ImVec2 headerMin = ImVec2(pos.x, pos.y);
		ImVec2 headerMax = ImVec2(pos.x + width, pos.y + headerHeight);
		
		bool hovered = ImGui::IsMouseHoveringRect(headerMin, headerMax);
		bool clicked = hovered && ImGui::IsMouseClicked(0) && !IsAnyComboOpen();
		
		if (clicked) {
			*isOpen = !*isOpen;
		}
		
		std::string id = std::string(title) + "_section";
		float& hoverAnim = hoverAnimations[id];
		hoverAnim = ImLerp(hoverAnim, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		ImColor headerBg = ImLerp(
			ImVec4(24, 24, 30, 255),
			ImVec4(30, 30, 38, 255),
			hoverAnim
		);
		drawList->AddRectFilled(headerMin, headerMax, headerBg, 8.0f);
		drawList->AddRect(headerMin, headerMax, Menu.BorderColor, 8.0f, 0, 1.0f);
		
		if (icon) {
			ImGui::PushFont(Menu.FontAwesome);
			ImVec2 iconSize = ImGui::CalcTextSize(icon);
			ImVec2 iconPos = ImVec2(pos.x + 12, pos.y + 12);
			drawList->AddText(iconPos, Menu.AccentColor, icon);
			ImGui::PopFont();
			
			ImGui::PushFont(Menu.SegoeUIBold);
			drawList->AddText(ImVec2(pos.x + 42, pos.y + 12), Menu.TextActiveColor, title);
			ImGui::PopFont();
		} else {
			ImGui::PushFont(Menu.SegoeUIBold);
			drawList->AddText(ImVec2(pos.x + 12, pos.y + 12), Menu.TextActiveColor, title);
			ImGui::PopFont();
		}
		
		float arrowX = pos.x + width - 25;
		float arrowY = pos.y + headerHeight / 2.0f;
		float arrowSize = 5.0f;
		
		ImColor arrowColor = Menu.TextColor;
		if (*isOpen) {
			drawList->AddTriangleFilled(
				ImVec2(arrowX, arrowY - arrowSize),
				ImVec2(arrowX - arrowSize, arrowY),
				ImVec2(arrowX + arrowSize, arrowY),
				arrowColor
			);
		} else {
			drawList->AddTriangleFilled(
				ImVec2(arrowX, arrowY + arrowSize),
				ImVec2(arrowX - arrowSize, arrowY),
				ImVec2(arrowX + arrowSize, arrowY),
				arrowColor
			);
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + headerHeight + 8));
	}
	
	void SectionHeader(const char* title, const char* icon = nullptr) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetContentRegionAvail().x;
		
		float headerHeight = 35.0f;
		ImVec2 headerMin = ImVec2(pos.x, pos.y);
		ImVec2 headerMax = ImVec2(pos.x + width, pos.y + headerHeight);
		
		ImColor headerBg = ImColor(24, 24, 30, 255);
		drawList->AddRectFilled(headerMin, headerMax, headerBg, 8.0f);
		
		if (icon) {
			ImGui::PushFont(Menu.FontAwesome);
			ImVec2 iconSize = ImGui::CalcTextSize(icon);
			ImVec2 iconPos = ImVec2(pos.x + 8, pos.y + 9);
			drawList->AddText(iconPos, Menu.AccentColor, icon);
			ImGui::PopFont();
			
			ImGui::PushFont(Menu.SegoeUIBold);
			drawList->AddText(ImVec2(pos.x + 38, pos.y + 9), Menu.TextActiveColor, title);
			ImGui::PopFont();
		} else {
			ImGui::PushFont(Menu.SegoeUIBold);
			drawList->AddText(ImVec2(pos.x + 12, pos.y + 9), Menu.TextActiveColor, title);
			ImGui::PopFont();
		}
		
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + headerHeight + 12));
	}
	
	void SectionDescription(const char* desc) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		
		ImGui::PushFont(Menu.InterSmaller);
		drawList->AddText(pos, Menu.TextDimColor, desc);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 20));
	}
	
	void Divider() {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float width = ImGui::GetContentRegionAvail().x;
		float offsetX = 15.0f;
		
		float lineY = pos.y + 5;
		ImVec2 lineStart = ImVec2(pos.x + offsetX, lineY);
		ImVec2 lineEnd = ImVec2(pos.x + width, lineY);
		
		ImColor lineColor = ImColor(Menu.DividerColor.Value.x, Menu.DividerColor.Value.y, Menu.DividerColor.Value.z, 0.5f);
		drawList->AddLine(lineStart, lineEnd, lineColor, 1.0f);
		
		float dotSize = 3.0f;
		float dotSpacing = 8.0f;
		float dotX = pos.x + width / 2.0f - (dotSize * 2 + dotSpacing) / 2.0f;
		for (int i = 0; i < 3; i++) {
			ImVec2 dotPos = ImVec2(dotX + i * (dotSize + dotSpacing), lineY);
			ImColor dotColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.4f);
			drawList->AddCircleFilled(dotPos, dotSize / 2.0f, dotColor, 8);
		}
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 15));
	}

	bool SidebarTab(const char* icon, const char* label, bool selected, bool isCategory = false) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 size = isCategory ? ImVec2(Menu.SidebarWidth - 20, 25) : ImVec2(Menu.SidebarWidth - 20, 38);
		
		if (isCategory) {
			ImGui::PushFont(Menu.InterSmaller);
			drawList->AddText(ImVec2(pos.x + 10, pos.y + 5), Menu.TextDimColor, label);
			ImGui::PopFont();
			ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 2));
			ImGui::PopID();
			return false;
		}
		
		bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		bool clicked = hovered && ImGui::IsMouseClicked(0);
		
		std::string id = std::string(label) + "_tab";
		float& anim = tabAnimations[id];
		anim = ImLerp(anim, selected ? 1.0f : (hovered ? 0.5f : 0.0f), ImGui::GetIO().DeltaTime * 14.0f);
		
		if (anim > 0.01f) {
			ImColor bgColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, anim * 0.08f);
			drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, 8.0f);
			
			if (selected) {
				for (int i = 2; i > 0; i--) {
					float glowAlpha = (1.0f - (i / 2.0f)) * 0.12f * anim;
					ImColor glowColor = ImColor(
						Menu.AccentColor.Value.x,
						Menu.AccentColor.Value.y,
						Menu.AccentColor.Value.z,
						glowAlpha
					);
					drawList->AddRectFilled(
						ImVec2(pos.x - i * 1.2f, pos.y - i * 0.6f),
						ImVec2(pos.x + size.x + i * 1.2f, pos.y + size.y + i * 0.6f),
						glowColor, 8.0f + i * 0.3f
					);
				}
				
				ImVec2 indicatorMin = ImVec2(pos.x + 5, pos.y + 10);
				ImVec2 indicatorMax = ImVec2(pos.x + 8, pos.y + size.y - 10);
				drawList->AddRectFilled(indicatorMin, indicatorMax, Menu.AccentColor, 2.0f);
				
				for (int i = 0; i < 2; i++) {
					float glowAlpha = (1.0f - (i / 2.0f)) * 0.15f;
					ImColor glowColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, glowAlpha);
					drawList->AddRectFilled(
						ImVec2(indicatorMin.x - i * 0.8f, indicatorMin.y - i * 0.5f),
						ImVec2(indicatorMax.x + i * 0.8f, indicatorMax.y + i * 0.5f),
						glowColor, 2.0f + i * 0.3f
					);
				}
			} else if (hovered && anim > 0.2f) {
				for (int i = 1; i > 0; i--) {
					float glowAlpha = (1.0f - (i / 1.0f)) * 0.08f * anim;
					ImColor glowColor = ImColor(
						Menu.AccentColor.Value.x,
						Menu.AccentColor.Value.y,
						Menu.AccentColor.Value.z,
						glowAlpha
					);
					drawList->AddRectFilled(
						ImVec2(pos.x - i * 1.0f, pos.y - i * 0.5f),
						ImVec2(pos.x + size.x + i * 1.0f, pos.y + size.y + i * 0.5f),
						glowColor, 8.0f + i * 0.3f
					);
				}
			}
		}
		
		ImColor textColor = ImLerp(
			ImVec4(Menu.TextDimColor.Value.x, Menu.TextDimColor.Value.y, Menu.TextDimColor.Value.z, Menu.TextDimColor.Value.w),
			ImVec4(Menu.TextActiveColor.Value.x, Menu.TextActiveColor.Value.y, Menu.TextActiveColor.Value.z, Menu.TextActiveColor.Value.w),
			selected ? 1.0f : anim
		);
		
		ImColor iconColor = selected ? Menu.AccentColor : textColor;
		
		ImGui::PushFont(Menu.FontAwesome);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 11), iconColor, icon);
		ImGui::PopFont();
		
		ImGui::PushFont(Menu.SegoeUI);
		drawList->AddText(ImVec2(pos.x + 40, pos.y + 10), textColor, label);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y + 2));
		ImGui::PopID();
		
		return clicked;
	}

	static std::map<std::string, bool> keybindActive;
	
	bool SoftKeybind(const char* label, int* key, int* mode = nullptr, bool* parentToggle = nullptr) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 originalPos = ImGui::GetCursorScreenPos();
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 40);
		
		std::string id = label;
		bool& isActive = keybindActive[id];
		float& hoverAnim = hoverAnimations[id];
		float& expandAnim = keybindExpandAnimations[id];
		
		bool shouldShow = (parentToggle == nullptr || *parentToggle);
		float targetExpand = shouldShow ? 1.0f : 0.0f;
		expandAnim = ImLerp(expandAnim, targetExpand, ImGui::GetIO().DeltaTime * 15.0f);
		
		float animOffsetY = (1.0f - expandAnim) * -20.0f;
		float animAlpha = expandAnim;
		float animHeight = size.y * expandAnim;
		float animSpacing = 5.0f * expandAnim;
		
		float finalCursorY = originalPos.y + animHeight + animSpacing;
		
		if (expandAnim < 0.01f) {
			ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
			ImGui::PopID();
			return false;
		}
		
		ImVec2 pos = originalPos;
		pos.y += animOffsetY;
		
		char keyName[64] = "None";
		if (*key != 0 && !isActive) {
			if (*key >= 1 && *key <= 6) {
				const char* mouseKeys[] = { "", "Mouse1", "Mouse2", "Cancel", "Mouse3", "Mouse4", "Mouse5" };
				strcpy_s(keyName, mouseKeys[*key]);
			} else {
				strcpy_s(keyName, ImGui::GetKeyName(CustomImGui::VirtualKeyToImGuiKey(*key)));
			}
		} else if (isActive) {
			strcpy_s(keyName, "Press a key...");
		}
		
		ImVec2 keySize = ImGui::CalcTextSize(keyName);
		float keyBoxWidth = ImMax(keySize.x + 20, 80.0f);
		float keyBoxX = pos.x + size.x - keyBoxWidth - 15;
		float keyBoxY = pos.y + 8;
		
		bool hovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + animHeight));
		hoverAnim = ImLerp(hoverAnim, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 14.0f);
		
		ImVec4 bgColorVec = ImVec4(
			Menu.CardColor.Value.x + (Menu.CardHoverColor.Value.x - Menu.CardColor.Value.x) * hoverAnim,
			Menu.CardColor.Value.y + (Menu.CardHoverColor.Value.y - Menu.CardColor.Value.y) * hoverAnim,
			Menu.CardColor.Value.z + (Menu.CardHoverColor.Value.z - Menu.CardColor.Value.z) * hoverAnim,
			(Menu.CardColor.Value.w + (Menu.CardHoverColor.Value.w - Menu.CardColor.Value.w) * hoverAnim) * animAlpha
		);
		ImColor bgColor = ImColor(bgColorVec);
		
		if (isActive) {
			bgColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.12f * animAlpha);
		}
		
		drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + animHeight), bgColor, 8.0f);
		
		if (isActive) {
			ImColor borderColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.3f * animAlpha);
			drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + animHeight), borderColor, 8.0f, 0, 1.0f);
		}
		
		ImGui::PushFont(Menu.SegoeUI);
		ImColor labelColor = ImColor(Menu.TextColor.Value.x, Menu.TextColor.Value.y, Menu.TextColor.Value.z, Menu.TextColor.Value.w * animAlpha);
		drawList->AddText(ImVec2(pos.x + 15, pos.y + 12), labelColor, label);
		ImGui::PopFont();
		
		ImColor keyBgColor = isActive ? ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.2f * animAlpha) : ImColor(Menu.ToggleOffColor.Value.x, Menu.ToggleOffColor.Value.y, Menu.ToggleOffColor.Value.z, Menu.ToggleOffColor.Value.w * animAlpha);
		drawList->AddRectFilled(
			ImVec2(keyBoxX, keyBoxY),
			ImVec2(keyBoxX + keyBoxWidth, keyBoxY + 24),
			keyBgColor, 6.0f
		);
		
		ImColor keyTextColor = isActive ? ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, animAlpha) : ImColor(Menu.TextActiveColor.Value.x, Menu.TextActiveColor.Value.y, Menu.TextActiveColor.Value.z, Menu.TextActiveColor.Value.w * animAlpha);
		ImVec2 textPos = ImVec2(keyBoxX + (keyBoxWidth - keySize.x) / 2, keyBoxY + 4);
		drawList->AddText(textPos, keyTextColor, keyName);
		
		ImGui::SetCursorScreenPos(pos);
		if (ImGui::InvisibleButton("##keybind_btn", ImVec2(size.x, animHeight)) && !IsAnyComboOpen()) {
			if (!isActive) {
				isActive = true;
			}
		}
		
		if (isActive) {
			for (int vk = 0x01; vk <= 0xFE; vk++) {
				if (vk == VK_LBUTTON) continue;
				
				if (GetAsyncKeyState(vk) & 0x8000) {
					if (vk == VK_ESCAPE) {
						*key = 0;
					} else {
						*key = vk;
					}
					isActive = false;
					break;
				}
			}
		}
		
		ImGui::SetCursorScreenPos(ImVec2(originalPos.x, finalCursorY));
		ImGui::PopID();
		
		return isActive;
	}
	
	bool ActionButton(const char* label, const ImVec2& size = ImVec2(0, 0)) {
		ImGui::PushID(label);
		
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 buttonSize = size.x > 0 ? size : ImVec2(ImGui::GetContentRegionAvail().x, 36.0f);
		
		ImGui::SetCursorScreenPos(pos);
		ImGui::InvisibleButton("##action_btn", buttonSize);
		
		bool clicked = ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !IsAnyComboOpen();
		
		std::string id = label;
		float& pressAnim = buttonPressAnimations[id];
		
		// Basınca animasyonu başlat, sonra yavaşça kaybolsun
		if (clicked) {
			pressAnim = 1.0f;
		}
		pressAnim = ImLerp(pressAnim, 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
		
		// Keybind ile aynı arka plan rengi (normal durum)
		float cardR = Menu.CardColor.Value.x;
		float cardG = Menu.CardColor.Value.y;
		float cardB = Menu.CardColor.Value.z;
		
		// Hafif turuncu renk (basınca) - accent color'dan karıştır
		float orangeR = Menu.AccentColor.Value.x * 0.25f + cardR * 0.75f;
		float orangeG = Menu.AccentColor.Value.y * 0.25f + cardG * 0.75f;
		float orangeB = Menu.AccentColor.Value.z * 0.25f + cardB * 0.75f;
		
		// Animasyonlu arka plan rengi
		ImColor bgColor = ImLerp(
			ImVec4(cardR, cardG, cardB, 1.0f),
			ImVec4(orangeR, orangeG, orangeB, 1.0f),
			pressAnim
		);
		
		// Buton arka planı
		drawList->AddRectFilled(pos, ImVec2(pos.x + buttonSize.x, pos.y + buttonSize.y), bgColor, 6.0f);
		
		// Metin (beyaz)
		ImGui::PushFont(Menu.SegoeUI);
		ImVec2 textSize = ImGui::CalcTextSize(label);
		ImVec2 textPos = ImVec2(
			pos.x + (buttonSize.x - textSize.x) / 2.0f,
			pos.y + (buttonSize.y - textSize.y) / 2.0f
		);
		
		ImColor textColor = ImColor(255, 255, 255, 255);
		drawList->AddText(textPos, textColor, label);
		ImGui::PopFont();
		
		ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + buttonSize.y + 3));
		
		ImGui::PopID();
		
		return clicked;
	}
}

void TabAimbot() {
	ImGui::BeginChild("##aimbotcontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Aimbot", ICON_FA_CROSSHAIRS);
	
	UI::SoftToggleCompact("Aimbot", &Cheats::AimAssist::Aimbot::Enabled);
	UI::SoftKeybind("Aimbot Key", &Cheats::AimAssist::Aimbot::Key, nullptr, &Cheats::AimAssist::Aimbot::Enabled);
	
	UI::SoftToggleCompact("Only Visible", &Cheats::AimAssist::OnlyVisible);
	UI::SoftToggleCompact("Ignore Ped", &Cheats::AimAssist::IgnorePed);
	UI::SoftToggleCompact("Ignore Death", &Cheats::AimAssist::IgnoreDeath);
	UI::SoftToggleCompact("Draw Fov", &Cheats::AimAssist::Aimbot::DrawFov);
	UI::SoftToggleCompact("Rage Mode", &Cheats::AimAssist::Aimbot::RageMode);
	UI::SoftToggleCompact("Closest Bone", &Cheats::AimAssist::Aimbot::ClosestBone);
	
	UI::Divider();
	
	UI::SectionHeader("Settings", ICON_FA_SLIDERS);
	
	UI::SoftSliderInt("Fov", &Cheats::AimAssist::Aimbot::Fov, 1, 400, "%d");
	UI::SoftSliderInt("Smooth", &Cheats::AimAssist::Aimbot::Smooth, 1, 50, "%d");
	UI::SoftSliderInt("Distance", &Cheats::AimAssist::Aimbot::Distance, 1, 1000, "%dm");
	if (!Cheats::AimAssist::Aimbot::ClosestBone) {
	UI::SoftHitboxSelector("Hitbox", &Cheats::AimAssist::Aimbot::SelectedType, Cheats::AimAssist::Aimbot::Type, 7);
	}
	
	ImGui::EndChild();
}

void TabSilent() {
	ImGui::BeginChild("##silentcontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Silent Aim", ICON_FA_GHOST);
	
	UI::SoftToggleCompact("Silent", &Cheats::AimAssist::Silent::Enabled);
	UI::SoftKeybind("Silent Key", &Cheats::AimAssist::Silent::Key, nullptr, &Cheats::AimAssist::Silent::Enabled);
	
	if (UI::SoftToggleCompact("Random Target", &Cheats::AimAssist::Silent::RandomTarget)) {
		if (Cheats::AimAssist::Silent::RandomTarget) {
			Cheats::AimAssist::Silent::ClosestBone = false;
		}
	}
	UI::SoftToggleCompact("Only Visible", &Cheats::AimAssist::OnlyVisible);
	UI::SoftToggleCompact("Ignore Ped", &Cheats::AimAssist::IgnorePed);
	UI::SoftToggleCompact("Ignore Death", &Cheats::AimAssist::IgnoreDeath);
	UI::SoftToggleCompact("Draw Fov", &Cheats::AimAssist::Silent::DrawFov);
	UI::SoftToggleCompact("Indicator", &Cheats::AimAssist::Silent::DrawIndicator);
	UI::SoftIndicatorTypeSelector("Indicator Type", &Cheats::AimAssist::Silent::IndicatorType, Cheats::AimAssist::Silent::IndicatorTypeOptions, 2, &Cheats::AimAssist::Silent::DrawIndicator);
	if (UI::SoftToggleCompact("Closest Bone", &Cheats::AimAssist::Silent::ClosestBone)) {
		if (Cheats::AimAssist::Silent::ClosestBone) {
			Cheats::AimAssist::Silent::RandomTarget = false;
		}
	}
	
	UI::SoftToggleCompact("Magic Bullet", &Cheats::AimAssist::Silent::MagicBullet);
	UI::SoftKeybind("Magic Bullet Key", &Cheats::AimAssist::Silent::MagicBulletKey, nullptr, &Cheats::AimAssist::Silent::MagicBullet);
	
	UI::SoftToggleCompact("Wheel Bullet", &Cheats::AimAssist::Silent::WheelBullet);
	UI::SoftKeybind("Wheel Bullet Key", &Cheats::AimAssist::Silent::WheelBulletKey, nullptr, &Cheats::AimAssist::Silent::WheelBullet);
	
	UI::Divider();
	
	UI::SectionHeader("Settings", ICON_FA_WAND_MAGIC_SPARKLES);
	
	UI::SoftSliderInt("Fov", &Cheats::AimAssist::Silent::Fov, 1, 400, "%d");
	UI::SoftSliderInt("Distance", &Cheats::AimAssist::Silent::Distance, 1, 1000, "%dm");
	UI::SoftSliderInt("Miss Chance", &Cheats::AimAssist::Silent::MissChance, 0, 100, "%d%%");
	if (!Cheats::AimAssist::Silent::RandomTarget && !Cheats::AimAssist::Silent::ClosestBone) {
	UI::SoftHitboxSelector("Hitbox", &Cheats::AimAssist::Silent::SelectedType, Cheats::AimAssist::Silent::Type, 7);
	}
	
	ImGui::EndChild();
}

void TabVisuals() {
	ImGui::BeginChild("##visualscontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Visual Features", ICON_FA_PALETTE);
	
	UI::SoftToggleCompact("Skeleton", &Cheats::Players::DrawSkeleton::Enabled);
	if (Cheats::Players::DrawSkeleton::Enabled) {
		UI::SoftSlider("Skeleton Thickness", &Cheats::Players::DrawSkeleton::Thickness, 0.5f, 5.0f, "%.1f");
		UI::SoftSlider("Skeleton Outline", &Cheats::Players::DrawSkeleton::OutlineThickness, 0.0f, 6.0f, "%.1f");
	}
	UI::SoftToggleCompact("Id", &Cheats::Players::DrawId::Enabled);
	UI::SoftToggleCompact("Name", &Cheats::Players::DrawName::Enabled);
	
	UI::SoftToggleCompact("Box", &Cheats::Players::DrawBox::Enabled);
	UI::SoftPositionSelector("Box Type", &Cheats::Players::DrawBox::SelectedType, Cheats::Players::DrawBox::Type, 2, &Cheats::Players::DrawBox::Enabled);
	if (Cheats::Players::DrawBox::Enabled) {
		UI::SoftToggleCompact("Box Fill", &Cheats::Players::DrawBox::FillEnabled);
		UI::SoftSlider("Fill Opacity", &Cheats::Players::DrawBox::FillOpacity, 0.0f, 1.0f, "%.2f");
		UI::SoftSlider("Box Thickness", &Cheats::Players::DrawBox::Thickness, 0.5f, 4.0f, "%.1f");
	}
	
	UI::SoftToggleCompact("Line", &Cheats::Players::DrawLine::Enabled);
	UI::SoftPositionSelector("Line Type", &Cheats::Players::DrawLine::SelectedType, Cheats::Players::DrawLine::Type, 3, &Cheats::Players::DrawLine::Enabled);
	
	UI::SoftToggleCompact("Distance", &Cheats::Players::DrawDistance::Enabled);
	
	UI::SoftToggleCompact("Health", &Cheats::Players::DrawHealth::Enabled);
	UI::SoftPositionSelector("Health Bar", &Cheats::Players::DrawHealth::SelectedPosition, Cheats::Players::DrawHealth::Position, 4, &Cheats::Players::DrawHealth::Enabled);
	if (Cheats::Players::DrawHealth::Enabled) {
		UI::SoftToggleCompact("Health Gradient", &Cheats::Players::DrawHealth::GradientEnabled);
	}
	
	UI::SoftToggleCompact("Armor", &Cheats::Players::DrawArmor::Enabled);
	UI::SoftPositionSelector("Armor Bar", &Cheats::Players::DrawArmor::SelectedPosition, Cheats::Players::DrawArmor::Position, 4, &Cheats::Players::DrawArmor::Enabled);
	if (Cheats::Players::DrawArmor::Enabled) {
		UI::SoftToggleCompact("Armor Gradient", &Cheats::Players::DrawArmor::GradientEnabled);
	}
	
	UI::SoftToggleCompact("Weapon Name", &Cheats::Players::DrawWeaponName::Enabled);
	
	UI::SoftToggleCompact("Gender Spectator", &Cheats::Players::GenderSpectator::Enabled);

	UI::Divider();

	UI::SectionHeader("Targeting", ICON_FA_CROSSHAIRS);
	UI::SoftToggleCompact("Highlight Locked Target", &Cheats::Players::HighlightTarget);
	
	UI::Divider();
	
	UI::SoftSliderInt("Players Distance", &Cheats::Players::Distance, 1, 5000, "%dm");
	
	UI::Divider();
	
	UI::SectionHeader("Filters", ICON_FA_SHIELD);
	UI::SoftToggleCompact("Only Visible", &Cheats::Players::OnlyVisible);
	UI::SoftToggleCompact("Ignore Ped", &Cheats::Players::IgnorePed);
	UI::SoftToggleCompact("Ignore Death", &Cheats::Players::IgnoreDeath);
	
	ImGui::EndChild();
}

void TabPlayers() {
	ImGui::BeginChild("##playerscontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	static char searchBuf[128] = "";
	UI::SearchBox("Search players...", searchBuf, sizeof(searchBuf));
	
	std::vector<int> filtered;
	for (int i = 0; i < (int)playerIDs.size(); i++) {
		std::string searchText = std::to_string(playerIDs[i]) + " " + playerNames[i];
		if (strlen(searchBuf) > 0 && searchText.find(searchBuf) == std::string::npos) continue;
		filtered.push_back(i);
	}
	
	UI::ListHeader("Online Players", (int)filtered.size());
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
	ImGui::BeginChild("##playerlist", ImVec2(ImGui::GetContentRegionAvail().x, 260), true, ImGuiWindowFlags_NoScrollbar);
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	
	for (int idx : filtered) {
		int realID = playerIDs[idx];
		std::string name = playerNames[idx];
		char idStr[32];
		sprintf_s(idStr, "ID: %d", realID);
		
		if (UI::ListItem(name.c_str(), idStr, selectedPlayerID == realID, friendStatus[realID])) {
			selectedPlayerID = realID;
			selectedItemPlayer = idx;
		}
	}
	
	if (filtered.empty()) {
		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2 - 50, 100));
		ImGui::PushFont(Menu.InterSmaller);
		ImGui::TextColored(ImVec4(Menu.TextDimColor.Value.x, Menu.TextDimColor.Value.y, Menu.TextDimColor.Value.z, 1.0f), "No players found");
		ImGui::PopFont();
	}
	
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	
	UI::SectionHeader("Actions", ICON_FA_GEAR);
	
	if (UI::ActionButton("Teleport the player")) {
		if (selectedPlayerID >= 0) {
			Teleport = true;
		}
	}
	
	std::string friendButtonText = "Add friend";
	if (selectedPlayerID >= 0 && friendStatus.find(selectedPlayerID) != friendStatus.end() && friendStatus[selectedPlayerID]) {
		friendButtonText = "Unfriend";
	}
	
	if (UI::ActionButton(friendButtonText.c_str())) {
		if (selectedPlayerID >= 0) {
			if (friendStatus[selectedPlayerID]) {
				friendStatus[selectedPlayerID] = false;
				AddNotification("Removed from friend list", false);
			} else {
				friendStatus[selectedPlayerID] = true;
				AddNotification("Added to friend list", true);
			}
		}
	}
	
	if (UI::ActionButton("Copy name")) {
		if (selectedPlayerID >= 0 && selectedItemPlayer >= 0 && selectedItemPlayer < (int)playerNames.size()) {
			std::string playerName = playerNames[selectedItemPlayer];
			if (!playerName.empty()) {
				CopyToClipboard(playerName);
				AddNotification("Name copied: " + playerName, true);
			} else {
				AddNotification("Name not found", false);
			}
		}
	}
	
	ImGui::EndChild();
}

void TabVehicles() {
	ImGui::BeginChild("##vehiclescontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Vehicle ESP", ICON_FA_CAR);
	
	UI::SoftToggleCompact("Vehicle Esp", &Cheats::Vehicle::Enabled);
	UI::SoftToggleCompact("Local Vehicle", &Cheats::Vehicle::DrawLocalVehicle);
	UI::SoftToggleCompact("Enemy Vehicle", &Cheats::Vehicle::DrawEnemyVehicle);
	UI::SoftToggleCompact("Vehicle Distance", &Cheats::Vehicle::VehicleEspShowDistance);
	UI::SoftToggleCompact("Vehicle Marker", &Cheats::Vehicle::VehicleMarker);
	UI::SoftToggleCompact("Vehicle Snapline", &Cheats::Vehicle::VehicleEspSnapline);
	UI::SoftToggleCompact("Vehicle Health", &Cheats::Vehicle::VehicleHealth);
	
	UI::SoftToggleCompact("Vehicle Fix", &Cheats::Vehicle::Fix);
	if (Cheats::Vehicle::Fix) {
		UI::SoftKeybind("Fix Key", &Cheats::Vehicle::FixKey);
	}
	
	UI::Divider();
	
	UI::SoftSliderInt("Draw Distance", &Cheats::Vehicle::Distance, 30, 5000, "%dm");
	
	UI::Divider();
	
	UI::SectionHeader("Vehicle Boost", ICON_FA_BOLT);
	
	UI::SoftToggleCompact("Vehicle Boost", &Cheats::Exploit::Vehicle::Boost);
	if (Cheats::Exploit::Vehicle::Boost) {
		UI::SoftSlider("Boost Value", &Cheats::Exploit::Vehicle::BoostValue, 1.0f, 10.0f, "%.1f");
	}
	UI::SoftKeybind("Boost Key", &Cheats::Exploit::Vehicle::BoostKey, nullptr, &Cheats::Exploit::Vehicle::Boost);
	
	ImGui::EndChild();
}

void TabVehicleList() {
	ImGui::BeginChild("##vehiclelistcontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::ListHeader("Nearby Vehicles", (int)vehicleIDs.size());
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
	ImGui::BeginChild("##vehiclelist", ImVec2(ImGui::GetContentRegionAvail().x, 340), true, ImGuiWindowFlags_NoScrollbar);
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	
	for (int i = 0; i < (int)vehicleIDs.size(); i++) {
		std::string name = vehicleNames[i];
		char distStr[32] = "";
		if (i < (int)vehicleDistances.size()) {
			sprintf_s(distStr, "%.0fm away", vehicleDistances[i]);
		}
		
		if (UI::ListItem(name.c_str(), distStr, selectedItemVehicle == i)) {
			selectedItemVehicle = i;
		}
	}
	
	if (vehicleIDs.empty()) {
		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2 - 50, 140));
		ImGui::PushFont(Menu.InterSmaller);
		ImGui::TextColored(ImVec4(Menu.TextDimColor.Value.x, Menu.TextDimColor.Value.y, Menu.TextDimColor.Value.z, 1.0f), "No vehicles nearby");
		ImGui::PopFont();
	}
	
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	
	UI::SectionHeader("Actions", ICON_FA_GEAR);
	
	if (UI::ActionButton("Teleport")) {
		if (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			VehicleTeleport = true;
		}
	}

	if (UI::ActionButton("Unlock Car")) {
		if (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			UnlockSelectedVehicle();
		}
	}

	if (UI::ActionButton("Lock Car")) {
		if (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			LockSelectedVehicle();
		}
	}

	if (UI::ActionButton("Steal Car")) {
		if (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			StealSelectedVehicle();
		}
	}
	
	if (UI::ActionButton("Enter Vehicle")) {
		if (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			EnterVehicle(selectedItemVehicle);
		}
	}
	
	ImGui::EndChild();
}

void TabWorld() {
	ImGui::BeginChild("##worldcontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::ListHeader("Teleport Locations", 14);
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(Menu.CardColor.Value.x, Menu.CardColor.Value.y, Menu.CardColor.Value.z, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
	ImGui::BeginChild("##locations", ImVec2(ImGui::GetContentRegionAvail().x, 260), true, ImGuiWindowFlags_NoScrollbar);
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	
	static int selectedLoc = -1;
	for (int i = 0; i < 14; i++) {
		if (UI::ListItem(Cheats::Teleport::locations[i].name, nullptr, selectedLoc == i)) {
			selectedLoc = i;
		}
	}
	
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	
	UI::SectionHeader("Actions", ICON_FA_GEAR);
	
	float availWidth = ImGui::GetContentRegionAvail().x;
	float buttonSpacing = 10.0f;
	float buttonWidth = (availWidth - buttonSpacing) / 2.0f;
	
	ImVec2 button1Pos = ImGui::GetCursorScreenPos();
	if (UI::ActionButton("Teleport to Location", ImVec2(buttonWidth, 36.0f))) {
		if (selectedLoc >= 0) {
			Cheats::Teleport::TeleportToLocation(selectedLoc);
		}
	}
	
	ImGui::SetCursorScreenPos(ImVec2(button1Pos.x + buttonWidth + buttonSpacing, button1Pos.y));
	
	if (UI::ActionButton("Teleport to Waypoint", ImVec2(buttonWidth, 36.0f))) {
		Cheats::Teleport::TeleportWaypoint = true;
	}
	
	ImGui::SetCursorScreenPos(ImVec2(button1Pos.x, button1Pos.y + 36.0f + 5));
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
	UI::Divider();
	
	UI::SectionHeader("No Clip", ICON_FA_PLANE);
	
	UI::SoftToggleCompact("No Clip", &Cheats::NoClip::Enabled);
	if (Cheats::NoClip::Enabled) {
		UI::SoftSliderInt("No Clip Speed", &Cheats::NoClip::Speed, 1, 50, "%d");
	}
	UI::SoftKeybind("No Clip Key", &Cheats::NoClip::Key, nullptr, &Cheats::NoClip::Enabled);
	
	UI::Divider();
	
	UI::SectionHeader("Free Cam", ICON_FA_VIDEO);
	
	UI::SoftToggleCompact("Free Cam", &Cheats::FreeCam::Enabled);
	if (Cheats::FreeCam::Enabled) {
		UI::SoftSlider("Free Cam Speed", &Cheats::FreeCam::Speed, 0.1f, 10.0f, "%.1f");
	}
	UI::SoftKeybind("Free Cam Key", &Cheats::FreeCam::Key, nullptr, &Cheats::FreeCam::Enabled);
	UI::SoftKeybind("Free Cam TP Key", &Cheats::FreeCam::TpKey, nullptr, &Cheats::FreeCam::Enabled);
	
	ImGui::EndChild();
}

void TabMisc() {
	ImGui::BeginChild("##misccontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Player", ICON_FA_HEART);
	
	UI::SoftToggleCompact("Health Boost", &Cheats::Exploit::HealthBoost);
	if (Cheats::Exploit::HealthBoost) {
		UI::SoftSlider("Health Value", &Cheats::Exploit::HealthBoostValue, 100.0f, 500.0f, "%.0f");
	}
	UI::SoftKeybind("Health Boost Key", &Cheats::Exploit::HealthBoostKey, nullptr, &Cheats::Exploit::HealthBoost);
	
	UI::SoftToggleCompact("Armor Boost", &Cheats::Exploit::ArmorBoost);
	if (Cheats::Exploit::ArmorBoost) {
		UI::SoftSlider("Armor Value", &Cheats::Exploit::ArmorBoostValue, 10.0f, 100.0f, "%.0f");
	}
	UI::SoftKeybind("Armor Boost Key", &Cheats::Exploit::ArmorBoostKey, nullptr, &Cheats::Exploit::ArmorBoost);
	
	UI::SoftToggleCompact("Damage Reduction", &Cheats::Misc::DamageReduction);
	if (Cheats::Misc::DamageReduction) {
		UI::SoftSliderInt("Reduce Incoming %", &Cheats::Misc::DamageReductionPercent, 0, 100, "%d%%");
	}
	
	UI::SoftToggleCompact("Infinite Stamina", &Cheats::Exploit::InfiniteStamina);
	UI::SoftToggleCompact("Safe Godmode", &Cheats::Exploit::AntiHeadshot);
	
	UI::SoftToggleCompact("Peak Assist", &Cheats::Exploit::PeakAssist);
	UI::SoftKeybind("Peak Assist Key", &Cheats::Exploit::PeakAssistKey, nullptr, &Cheats::Exploit::PeakAssist);
	
	UI::SoftToggleCompact("Safe Invisible", &Cheats::Exploit::SafeInvisible);
	UI::SoftKeybind("Safe Invisible Key", &Cheats::Exploit::SafeInvisibleKey, nullptr, &Cheats::Exploit::SafeInvisible);
	
	UI::Divider();
	
	UI::SectionHeader("Weapon", ICON_FA_SKULL);
	
	UI::SoftToggleCompact("Infinite Ammo", &Cheats::Exploit::InfiniteAmmo);
	UI::SoftToggleCompact("No Recoil", &Cheats::Exploit::NoRecoil);
	UI::SoftToggleCompact("No Spread", &Cheats::Exploit::NoSpread);
	UI::SoftToggleCompact("No Reload", &Cheats::Exploit::NoReload);
	UI::SoftToggleCompact("Reload Ammo", &Cheats::Exploit::ReloadAmmo);
	
	UI::SoftToggleCompact("Damage Log", &Cheats::Misc::DamageLog);
	
	UI::SoftToggleCompact("Damage Boost", &Cheats::Misc::damageBoost);
	if (Cheats::Misc::damageBoost) {
		UI::SoftSlider("Damage Value", &Cheats::Misc::damageBoostValue, 1.0f, 100.0f, "%.1f");
	}
	UI::SoftKeybind("Damage Boost Key", &Cheats::Misc::damageBoostKey, nullptr, &Cheats::Misc::damageBoost);
	
	UI::Divider();
	
	UI::SectionHeader("Strafe", ICON_FA_FORWARD);
	
	UI::SoftToggleCompact("Strafe Lag", &Cheats::Exploit::Strafe::Enabled);
	if (Cheats::Exploit::Strafe::Enabled) {
		UI::SoftSlider("Strafe Speed", &Cheats::Exploit::Strafe::Speed, 0.1f, 5.0f, "%.2f");
	}
	UI::SoftKeybind("Strafe Key", &Cheats::Exploit::Strafe::Key, nullptr, &Cheats::Exploit::Strafe::Enabled);
	
	ImGui::EndChild();
}

void TabColors() {
	ImGui::BeginChild("##colorscontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Player ESP", ICON_FA_PALETTE);
	
	UI::SoftColorEdit("Skeleton", Cheats::Players::DrawSkeleton::Color);
	UI::SoftColorEdit("ID", Cheats::Players::DrawId::Color);
	UI::SoftColorEdit("Name", Cheats::Players::DrawName::Color);
	UI::SoftColorEdit("Box", Cheats::Players::DrawBox::Color);
	UI::SoftColorEdit("Box Fill", Cheats::Players::DrawBox::FillColor);
	UI::SoftColorEdit("Line", Cheats::Players::DrawLine::Color);
	UI::SoftColorEdit("Player Distance", Cheats::Players::DrawDistance::Color);
	UI::SoftColorEdit("Player Health", Cheats::Players::DrawHealth::Color);
	UI::SoftColorEdit("Health Low", Cheats::Players::DrawHealth::ColorLow);
	UI::SoftColorEdit("Health Grad Start", Cheats::Players::DrawHealth::GradientStart);
	UI::SoftColorEdit("Health Grad End", Cheats::Players::DrawHealth::GradientEnd);
	UI::SoftColorEdit("Armor", Cheats::Players::DrawArmor::Color);
	UI::SoftColorEdit("Armor Grad Start", Cheats::Players::DrawArmor::GradientStart);
	UI::SoftColorEdit("Armor Grad End", Cheats::Players::DrawArmor::GradientEnd);
	UI::SoftColorEdit("Weapon Name", Cheats::Players::DrawWeaponName::Color);
	UI::SoftColorEdit("Male", Cheats::Players::GenderSpectator::MaleColor);
	UI::SoftColorEdit("Female", Cheats::Players::GenderSpectator::FemaleColor);
	UI::SoftColorEdit("Target Lock", Cheats::Players::HighlightColor);
	
	UI::Divider();
	
	UI::SectionHeader("Vehicle ESP", ICON_FA_CAR);
	
	UI::SoftColorEdit("Vehicle Health", Cheats::Vehicle::HealthBarColor);
	UI::SoftColorEdit("Vehicle Distance", Cheats::Vehicle::DistanceColor);
	UI::SoftColorEdit("Snapline", Cheats::Vehicle::SnaplineColor);
	UI::SoftColorEdit("Marker Outer", Cheats::Vehicle::MarkerOuterColor);
	UI::SoftColorEdit("Marker Inner", Cheats::Vehicle::MarkerInnerColor);
	
	UI::Divider();
	
	UI::SectionHeader("Aim Assist", ICON_FA_CROSSHAIRS);
	
	UI::SoftColorEdit("Aimbot FOV", Cheats::AimAssist::Aimbot::Color);
	UI::SoftColorEdit("Silent FOV", Cheats::AimAssist::Silent::Color);
	UI::SoftColorEdit("Silent Line", Cheats::AimAssist::Silent::LineColor);
	
	UI::Divider();
	
	UI::SectionHeader("Other", ICON_FA_PAINTBRUSH);
	
	UI::SoftColorEdit("Crosshair", Cheats::Crosshairs::Color);
	
	ImGui::EndChild();
}

static const char* kBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string Base64Encode(const std::string& input) {
	std::string out;
	int val = 0, valb = -6;
	for (unsigned char c : input) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back(kBase64Chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back(kBase64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}

inline std::string Base64Decode(const std::string& input) {
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) T[kBase64Chars[i]] = i;
	std::string out;
	int val = 0, valb = -8;
	for (unsigned char c : input) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}

inline void ApplyConfigKV(const std::string& key, const std::string& value) {
	auto toBool = [](const std::string& v) { return v == "1" || v == "true" || v == "True"; };
	if (key == "strafe_enabled") Cheats::Exploit::Strafe::Enabled = toBool(value);
	else if (key == "strafe_speed") Cheats::Exploit::Strafe::Speed = std::stof(value);
	else if (key == "strafe_key") Cheats::Exploit::Strafe::Key = std::stoi(value);
	else if (key == "silent_enabled") Cheats::AimAssist::Silent::Enabled = toBool(value);
	else if (key == "silent_key") Cheats::AimAssist::Silent::Key = std::stoi(value);
	else if (key == "silent_fov") Cheats::AimAssist::Silent::Fov = std::stoi(value);
	else if (key == "aim_enabled") Cheats::AimAssist::Aimbot::Enabled = toBool(value);
	else if (key == "aim_key") Cheats::AimAssist::Aimbot::Key = std::stoi(value);
	else if (key == "aim_fov") Cheats::AimAssist::Aimbot::Fov = std::stoi(value);
	else if (key == "box_enabled") Cheats::Players::DrawBox::Enabled = toBool(value);
	else if (key == "box_type") Cheats::Players::DrawBox::SelectedType = std::stoi(value);
	else if (key == "box_thickness") Cheats::Players::DrawBox::Thickness = std::stof(value);
	else if (key == "box_fill") Cheats::Players::DrawBox::FillEnabled = toBool(value);
	else if (key == "box_fill_opacity") Cheats::Players::DrawBox::FillOpacity = std::stof(value);
	else if (key == "skeleton_enabled") Cheats::Players::DrawSkeleton::Enabled = toBool(value);
	else if (key == "health_enabled") Cheats::Players::DrawHealth::Enabled = toBool(value);
	else if (key == "health_pos") Cheats::Players::DrawHealth::SelectedPosition = std::stoi(value);
	else if (key == "armor_enabled") Cheats::Players::DrawArmor::Enabled = toBool(value);
	else if (key == "armor_pos") Cheats::Players::DrawArmor::SelectedPosition = std::stoi(value);
	else if (key.rfind("color_", 0) == 0) {
		std::istringstream ss(value);
		std::string r, g, b, a;
		if (std::getline(ss, r, ',') && std::getline(ss, g, ',') && std::getline(ss, b, ',') && std::getline(ss, a, ',')) {
			ImColor c((int)std::stoi(r), (int)std::stoi(g), (int)std::stoi(b), (int)std::stoi(a));
			if (key == "color_box") Cheats::Players::DrawBox::Color = c;
			else if (key == "color_boxfill") Cheats::Players::DrawBox::FillColor = c;
			else if (key == "color_skeleton") Cheats::Players::DrawSkeleton::Color = c;
			else if (key == "color_health") Cheats::Players::DrawHealth::Color = c;
			else if (key == "color_armor") Cheats::Players::DrawArmor::Color = c;
		}
	}
}

inline std::string ExportConfigPlain() {
	std::ostringstream ss;
	auto addBool = [&](const char* k, bool v) { ss << k << "=" << (v ? "1" : "0") << "\n"; };
	auto addInt = [&](const char* k, int v) { ss << k << "=" << v << "\n"; };
	auto addFloat = [&](const char* k, float v) { ss << k << "=" << v << "\n"; };
	auto addColor = [&](const char* k, const ImColor& c) {
		int r = (int)(c.Value.x * 255.0f);
		int g = (int)(c.Value.y * 255.0f);
		int b = (int)(c.Value.z * 255.0f);
		int a = (int)(c.Value.w * 255.0f);
		ss << k << "=" << r << "," << g << "," << b << "," << a << "\n";
	};

	addBool("strafe_enabled", Cheats::Exploit::Strafe::Enabled);
	addFloat("strafe_speed", Cheats::Exploit::Strafe::Speed);
	addInt("strafe_key", Cheats::Exploit::Strafe::Key);
	addBool("silent_enabled", Cheats::AimAssist::Silent::Enabled);
	addInt("silent_key", Cheats::AimAssist::Silent::Key);
	addInt("silent_fov", Cheats::AimAssist::Silent::Fov);
	addBool("aim_enabled", Cheats::AimAssist::Aimbot::Enabled);
	addInt("aim_key", Cheats::AimAssist::Aimbot::Key);
	addInt("aim_fov", Cheats::AimAssist::Aimbot::Fov);
	addBool("box_enabled", Cheats::Players::DrawBox::Enabled);
	addInt("box_type", Cheats::Players::DrawBox::SelectedType);
	addFloat("box_thickness", Cheats::Players::DrawBox::Thickness);
	addBool("box_fill", Cheats::Players::DrawBox::FillEnabled);
	addFloat("box_fill_opacity", Cheats::Players::DrawBox::FillOpacity);
	addBool("skeleton_enabled", Cheats::Players::DrawSkeleton::Enabled);
	addBool("health_enabled", Cheats::Players::DrawHealth::Enabled);
	addInt("health_pos", Cheats::Players::DrawHealth::SelectedPosition);
	addBool("armor_enabled", Cheats::Players::DrawArmor::Enabled);
	addInt("armor_pos", Cheats::Players::DrawArmor::SelectedPosition);

	addColor("color_box", Cheats::Players::DrawBox::Color);
	addColor("color_boxfill", Cheats::Players::DrawBox::FillColor);
	addColor("color_skeleton", Cheats::Players::DrawSkeleton::Color);
	addColor("color_health", Cheats::Players::DrawHealth::Color);
	addColor("color_armor", Cheats::Players::DrawArmor::Color);

	return ss.str();
}

inline void ImportConfigPlain(const std::string& data) {
	std::istringstream ss(data);
	std::string line;
	while (std::getline(ss, line)) {
		auto pos = line.find('=');
		if (pos == std::string::npos) continue;
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		ApplyConfigKV(key, value);
	}
}

void TabSettings() {
	ImGui::BeginChild("##settingscontent", ImVec2(0, 0), false);
	ImGui::SetCursorPos(ImVec2(15, 15));
	
	UI::SectionHeader("Main Settings", ICON_FA_CIRCLE_INFO);
	
	UI::SoftToggleCompact("Keybind List", &Cheats::MenuUtils::KeybindList);
	UI::SoftToggleCompact("Water Mark", &Cheats::MenuUtils::WaterMark);
	
	UI::Divider();
	
	UI::SoftSliderInt("Max Player Count", &maxPlayerCount, 0, 5000, "%d");
	
	UI::SoftCombo("ESP Font", &Cheats::MenuUtils::SelectedFontIndex, Cheats::MenuUtils::Fonts, 10);
	
	UI::Divider();
	
	UI::SectionHeader("Config (Base64)", ICON_FA_FILE_CODE);
	static char configBuffer[4096] = "";
	ImGui::InputTextMultiline("##configbuf", configBuffer, sizeof(configBuffer), ImVec2(-1, 120));
	if (ImGui::Button("Export to Base64")) {
		std::string plain = ExportConfigPlain();
		std::string b64 = Base64Encode(plain);
		strcpy_s(configBuffer, sizeof(configBuffer), b64.c_str());
		CopyToClipboard(b64);
		AddNotification("Config exported to clipboard (base64).", true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Import from Base64")) {
		std::string decoded = Base64Decode(configBuffer);
		if (!decoded.empty()) {
			ImportConfigPlain(decoded);
			AddNotification("Config imported.", true);
		} else {
			AddNotification("Config import failed.", false);
		}
	}
	
	UI::Divider();
	
	UI::SectionHeader("Font Preview", ICON_FA_PAINTBRUSH);
	
	ImGui::PushFont(Cheats::MenuUtils::GetSelectedFont());
	ImGui::Text("Sample Text - 123 ABC abc");
	ImGui::PopFont();
	
	UI::Divider();
	
	UI::SectionHeader("Keybinds", ICON_FA_HAND_POINTER);
	
	UI::SoftKeybind("Menu Key", &Cheats::MenuUtils::MenuKey);
	UI::SoftKeybind("Friend Key", &Cheats::MenuUtils::FriendKey);
	
	UI::Divider();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
	
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float width = ImGui::GetContentRegionAvail().x;
	float buttonWidth = 220.0f;
	float buttonHeight = 45.0f;
	float buttonX = pos.x + (width - buttonWidth) / 2.0f;
	float buttonY = pos.y;
	
	ImVec2 buttonMin = ImVec2(buttonX, buttonY);
	ImVec2 buttonMax = ImVec2(buttonX + buttonWidth, buttonY + buttonHeight);
	
	bool hovered = ImGui::IsMouseHoveringRect(buttonMin, buttonMax);
	bool clicked = hovered && ImGui::IsMouseClicked(0);
	
	static float exitHoverAnim = 0.0f;
	exitHoverAnim = ImLerp(exitHoverAnim, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 15.0f);
	
	ImColor buttonBg = ImLerp(
		ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.15f),
		ImVec4(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, 0.25f),
		exitHoverAnim
	);
	
	drawList->AddRectFilled(buttonMin, buttonMax, buttonBg, 8.0f);
	
	ImGui::PushFont(Menu.SegoeUIBold);
	ImVec2 textSize = ImGui::CalcTextSize("Exit");
	ImVec2 textPos = ImVec2(buttonX + (buttonWidth - textSize.x) / 2.0f, buttonY + (buttonHeight - textSize.y) / 2.0f);
	drawList->AddText(textPos, Menu.AccentColor, "Exit");
	ImGui::PopFont();
	
	if (clicked) {
		exit(0);
	}
	
	ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + buttonHeight + 10));
	
	ImGui::EndChild();
}

void DrawSidebar() {
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();
	
	ImVec2 sidebarMin = ImVec2(pos.x, pos.y + 1);
	ImVec2 sidebarMax = ImVec2(pos.x + Menu.SidebarWidth, pos.y + Menu.WindowSize.y - 1);
	
	drawList->AddRectFilled(
		sidebarMin,
		sidebarMax,
		Menu.SidebarColor,
		0.0f
	);
	
	for (int i = 0; i < 3; i++) {
		float gradientY = pos.y + (Menu.WindowSize.y / 3.0f) * i;
		float gradientAlpha = 0.03f - (i * 0.008f);
		ImColor gradientColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, gradientAlpha);
		drawList->AddRectFilled(
			ImVec2(pos.x, gradientY),
			ImVec2(pos.x + Menu.SidebarWidth, gradientY + Menu.WindowSize.y / 3.0f),
			gradientColor,
			0.0f
		);
	}
	
	drawList->AddLine(
		ImVec2(pos.x + Menu.SidebarWidth, pos.y + 15),
		ImVec2(pos.x + Menu.SidebarWidth, pos.y + Menu.WindowSize.y - 15),
		ImColor(Menu.BorderColor.Value.x, Menu.BorderColor.Value.y, Menu.BorderColor.Value.z, 0.4f), 1.0f
	);
	
	ImGui::SetCursorPos(ImVec2(15, 20));
	if (Menu.Logo) {
		ImGui::Image((ImTextureID)Menu.Logo, ImVec2(38, 38));
	}
	
	ImGui::SetCursorPos(ImVec2(60, 22));
	ImGui::PushFont(Menu.SegoeUIBold);
	ImGui::TextColored(Menu.AccentColor, "Nevers");
	ImGui::PopFont();
	
	ImGui::SetCursorPos(ImVec2(60, 42));
	ImGui::PushFont(Menu.InterSmaller);
	ImGui::TextColored(Menu.TextDimColor, "Always Win,Always Dominate");
	ImGui::PopFont();
	
	ImGui::SetCursorPos(ImVec2(10, 72));
	
	if (UI::SidebarTab(ICON_FA_CROSSHAIRS, "Aimbot", Menu.CurrentTab == 0)) Menu.CurrentTab = 0;
	if (UI::SidebarTab(ICON_FA_BULLSEYE, "Silent", Menu.CurrentTab == 1)) Menu.CurrentTab = 1;
	
	UI::SidebarTab("", "Players & Vision", false, true);
	if (UI::SidebarTab(ICON_FA_EYE, "Visuals", Menu.CurrentTab == 2)) Menu.CurrentTab = 2;
	if (UI::SidebarTab(ICON_FA_USERS, "Players", Menu.CurrentTab == 3)) Menu.CurrentTab = 3;
	if (UI::SidebarTab(ICON_FA_CAR, "Vehicles", Menu.CurrentTab == 4)) Menu.CurrentTab = 4;
	if (UI::SidebarTab(ICON_FA_LIST, "Vehicle List", Menu.CurrentTab == 5)) Menu.CurrentTab = 5;
	if (UI::SidebarTab(ICON_FA_GLOBE, "World", Menu.CurrentTab == 6)) Menu.CurrentTab = 6;
	
	UI::SidebarTab("", "Utilities & Settings", false, true);
	if (UI::SidebarTab(ICON_FA_WRENCH, "Misc", Menu.CurrentTab == 7)) Menu.CurrentTab = 7;
	if (UI::SidebarTab(ICON_FA_PALETTE, "Colors", Menu.CurrentTab == 8)) Menu.CurrentTab = 8;
	if (UI::SidebarTab(ICON_FA_GEAR, "Settings", Menu.CurrentTab == 9)) Menu.CurrentTab = 9;
	
	ImGui::SetCursorPos(ImVec2(15, Menu.WindowSize.y - 50));
	ImGui::PushFont(Menu.InterSmaller);
	
	ImGui::SetCursorPos(ImVec2(15, Menu.WindowSize.y - 30));
	ImGui::TextColored(Menu.TextDimColor, "v2.0.0");
	ImGui::PopFont();
}

void DrawContent() {
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();
	
	float contentX = Menu.SidebarWidth + 15;
	float contentY = 15;
	float contentWidth = Menu.WindowSize.x - Menu.SidebarWidth - 30;
	float contentHeight = Menu.WindowSize.y - 30;
	
	ImVec2 contentMin = ImVec2(windowPos.x + contentX - 5, windowPos.y + contentY - 5);
	ImVec2 contentMax = ImVec2(windowPos.x + contentX + contentWidth + 5, windowPos.y + contentY + contentHeight + 5);
	
	float contentAlpha = Menu.TabTransition * 0.6f + 0.3f;
	ImColor contentBg = ImColor(26, 26, 32, (int)(180 * contentAlpha));
	drawList->AddRectFilled(contentMin, contentMax, contentBg, 10.0f);
	ImColor borderColor = ImLerp(
		ImVec4(Menu.BorderColor.Value.x, Menu.BorderColor.Value.y, Menu.BorderColor.Value.z, 0.2f),
		ImVec4(Menu.BorderColor.Value.x, Menu.BorderColor.Value.y, Menu.BorderColor.Value.z, 0.5f),
		Menu.TabTransition
	);
	drawList->AddRect(contentMin, contentMax, ImColor(borderColor), 10.0f, 0, 1.0f);
	
	ImGui::SetCursorPos(ImVec2(contentX, contentY));
	
	ImGui::BeginChild("##content", ImVec2(contentWidth, contentHeight), false);
	
	if (Menu.LastTab != Menu.CurrentTab) {
		Menu.TabTransition = 0.0f;
		Menu.LastTab = Menu.CurrentTab;
	}
	
	Menu.TabTransition = ImLerp(Menu.TabTransition, 1.0f, ImGui::GetIO().DeltaTime * 6.0f);
	
	float fadeIn = Menu.TabTransition * Menu.TabTransition * (3.0f - 2.0f * Menu.TabTransition);
	
	if (Menu.TabTransition > 0.01f) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fadeIn);
		
		switch (Menu.CurrentTab) {
			case 0: TabAimbot(); break;
			case 1: TabSilent(); break;
			case 2: TabVisuals(); break;
			case 3: TabPlayers(); break;
			case 4: TabVehicles(); break;
			case 5: TabVehicleList(); break;
			case 6: TabWorld(); break;
			case 7: TabMisc(); break;
			case 8: TabColors(); break;
			case 9: TabSettings(); break;
		}
		
		ImGui::PopStyleVar();
	}
	
	ImGui::EndChild();
}

void DrawMenu() {
	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	const float screen_w = float(screen_rect.right - screen_rect.left);
	const float screen_h = float(screen_rect.bottom - screen_rect.top);

	float x = (screen_w - Menu.WindowSize.x) / 2.0f;
	float y = (screen_h - Menu.WindowSize.y) / 2.0f;

	Menu.MenuAlpha = ImLerp(Menu.MenuAlpha, 1.0f, ImGui::GetIO().DeltaTime * 5.0f);

	float currentMenuAlpha = Menu.MenuAlpha;

	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(Menu.WindowSize);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, currentMenuAlpha);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(Menu.BackgroundColor.Value.x, Menu.BackgroundColor.Value.y, Menu.BackgroundColor.Value.z, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	
	ImGui::Begin("##mainmenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();
	
	drawList->AddRect(pos, ImVec2(pos.x + Menu.WindowSize.x, pos.y + Menu.WindowSize.y), 
		Menu.BorderColor, 12.0f, 0, 1.0f);

	DrawSidebar();
	DrawContent();
	
	ImGui::End();
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(3);
	
}

void RenderWatermark() {
	if (!Cheats::MenuUtils::WaterMark) return;
	
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	char timeStr[32];
	sprintf_s(timeStr, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	
	ImGui::PushFont(Menu.InterSmaller);
	ImVec2 mynxSize = ImGui::CalcTextSize("Mynx");
	ImVec2 timeSize = ImGui::CalcTextSize(timeStr);
	ImGui::PopFont();
	
	float screenWidth = ImGui::GetIO().DisplaySize.x;
	float spacing = 8.0f;
	float padding = 12.0f;
	float totalWidth = mynxSize.x + spacing + timeSize.x + padding * 2;
	float boxHeight = 26.0f;
	float boxX = (screenWidth - totalWidth) / 2.0f;
	float boxY = 12.0f;
	
	ImVec2 boxMin = ImVec2(boxX, boxY);
	ImVec2 boxMax = ImVec2(boxX + totalWidth, boxY + boxHeight);
	
	ImColor bgColor = ImColor(18, 18, 22, 220);
	drawList->AddRectFilled(boxMin, boxMax, bgColor, 6.0f);
	
	float textY = boxY + (boxHeight - mynxSize.y) / 2.0f;
	
	ImGui::PushFont(Menu.InterSmaller);
	
	float xOffset = boxX + padding;
	drawList->AddText(ImVec2(xOffset, textY), Menu.AccentColor, "Mynx");
	xOffset += mynxSize.x + spacing;
	
	ImColor timeColor = ImColor(140, 140, 150, 200);
	drawList->AddText(ImVec2(xOffset, textY), timeColor, timeStr);
	
	ImGui::PopFont();
}
