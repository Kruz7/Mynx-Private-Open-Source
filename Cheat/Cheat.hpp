#include "GameSDK.hpp"
#include "Ped.hpp"
#include "VehicleList.hpp"
#include <random>
#include <thread>
#include <set>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <float.h>


struct KeybindEntry {
	std::string name;
	int keycode;
	bool is_pressed = false;
	float alpha = 0.0f;
	float target_alpha = 0.0f;
	float key_color_progress = 0.0f;
};
static std::vector<KeybindEntry> keybind_list;
static float keybind_overlay_alpha = 0.0f;
static ImVec2 keybind_overlay_pos = ImVec2(0, 0);
static bool keybind_overlay_dragging = false;
static bool WheelBulletActive = false;
static ImVec2 keybind_overlay_drag_offset = ImVec2(0, 0);
static float keybind_overlay_height_progress = 0.0f;

struct Notification {
	std::string message;
	bool isSuccess = true;
	float alpha = 0.0f;
	float target_alpha = 1.0f;
	float lifetime = 3.0f;
	std::chrono::steady_clock::time_point startTime;
};
static std::vector<Notification> notifications;

struct DamageLogEntry {
	std::string name;
	float amount = 0.0f;
	bool incoming = false;
	std::chrono::steady_clock::time_point timestamp;
};
static std::vector<DamageLogEntry> damageLog;
static std::mutex damageLogMutex;
static float damageLogPanelHeight = 0.0f;
static std::chrono::steady_clock::time_point lastLocalShotTime = std::chrono::steady_clock::now();
static int lastShotTargetId = -1;
static float lastShotTargetStartCombined = 0.0f;
static bool lastShotLogged = false;
static const float DAMAGE_SHOT_WINDOW_MS = 700.0f;
static const float DAMAGE_SCREEN_RADIUS = 150.0f;
static const float DAMAGE_MAX_DISTANCE = 350.0f;
static const float DAMAGE_MIN_AMOUNT = 0.5f;
static const float DAMAGE_DEDUP_MS = 150.0f;
static const float DAMAGE_TARGET_RADIUS = 150.0f;
static std::unordered_map<int, std::chrono::steady_clock::time_point> lastOutgoingLog;
static std::chrono::steady_clock::time_point lastIncomingLog = std::chrono::steady_clock::now();

Ped LocalPlayer;
uintptr_t TPModelInfo = NULL;
uintptr_t TPNavigation = NULL;
Vector3 TPPosition = Vector3(0, 0, 0);
std::vector<Ped> PedList;
Vector3 EndBulletPos;
inline bool SilentTargetActive = false;
inline Vector2 SilentTargetScreenPos = Vector2(0.f, 0.f);
inline float SilentTargetDistance = 0.f; 
inline int CurrentAimLockId = -1;
Vector3 SavedPeakPosition = Vector3(0, 0, 0);
bool PeakPositionSaved = false;

struct PedBarFix {
	int id;
	float health;
	float armor;
};

enum CarLockState : int {
	CARLOCK_NONE = 0,
	CARLOCK_UNLOCKED = 1,
	CARLOCK_LOCKED = 2,
	CARLOCK_LOCKOUT_PLAYER_ONLY = 3,
	CARLOCK_LOCKED_PLAYER_INSIDE = 4,
	CARLOCK_LOCKED_INITIALLY = 5,
	CARLOCK_FORCE_SHUT_DOORS = 6,
	CARLOCK_LOCKED_BUT_CAN_BE_DAMAGED = 7,
	CARLOCK_LOCKED_BUT_BOOT_UNLOCKED = 8,
	CARLOCK_LOCKED_NO_PASSENGERS = 9,
};

std::vector<PedBarFix> pedBarFix;

static std::map<int, std::string> pedNameCache;
static std::chrono::steady_clock::time_point lastNameCacheUpdate = std::chrono::steady_clock::now();
const int NAME_CACHE_UPDATE_INTERVAL_MS = 500; 
static std::unordered_map<int, float> pedHealthHistoryCombined;
static float lastLocalCombinedHealth = 0.0f;
static bool hasLocalHealthSnapshot = false;

void MagicBulletToggle();
void WheelBulletToggle();
void StealOutfit(int targetPlayerID);
Vector3 GetBonePosition(const Ped& ped, int boneId);
void AddNotification(const std::string& message, bool isSuccess = true);
void RenderNotifications();
void ToggleFriendAtCrosshair();
void StrafeMacro();

void AddNotification(const std::string& message, bool isSuccess) {
	Notification notif;
	notif.message = message;
	notif.isSuccess = isSuccess;
	notif.alpha = 0.0f;
	notif.target_alpha = 1.0f;
	notif.lifetime = 3.0f;
	notif.startTime = std::chrono::steady_clock::now();
	notifications.push_back(notif);
}

void CopyToClipboard(const std::string& text) {
	if (text.empty()) return;
	
	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(char));
		if (hGlobal) {
			char* pData = (char*)GlobalLock(hGlobal);
			if (pData) {
				strcpy_s(pData, text.size() + 1, text.c_str());
				GlobalUnlock(hGlobal);
				SetClipboardData(CF_TEXT, hGlobal);
			}
		}
		CloseClipboard();
	}
}

std::string GetPlayerDiscordId(int playerID) {
	if (!Offsets.CitizenPlayernamesBase) return "";
	
	uint64_t playerNames = ReadMemory<uint64_t>(Offsets.CitizenPlayernamesBase + 0x30D38);
	if (!playerNames) return "";

	uintptr_t list = ReadMemory<uintptr_t>(playerNames + 0x8);
	for (uint32_t i = 0; i < 500; ++i) {
		if (!list) break;

		if (playerID == ReadMemory<int>(list + 0x10)) {
			std::string discordStr = ReadString(list + 0x20);
			if (!discordStr.empty() && discordStr != "0") return discordStr;
			
			uint64_t discordId = ReadMemory<uint64_t>(list + 0x28);
			if (discordId != 0) return std::to_string(discordId);
			
			discordId = ReadMemory<uint64_t>(list + 0x30);
			if (discordId != 0) return std::to_string(discordId);
			
			discordStr = ReadString(list + 0x38);
			if (!discordStr.empty() && discordStr != "0") return discordStr;
		}
		list = ReadMemory<uintptr_t>(list + 0x8);
	}
	return "";
}

std::string GetPlayerSteamId(int playerID) {
	if (!Offsets.CitizenPlayernamesBase) return "";
	
	uint64_t playerNames = ReadMemory<uint64_t>(Offsets.CitizenPlayernamesBase + 0x30D38);
	if (!playerNames) return "";

	uintptr_t list = ReadMemory<uintptr_t>(playerNames + 0x8);
	for (uint32_t i = 0; i < 500; ++i) {
		if (!list) break;

		if (playerID == ReadMemory<int>(list + 0x10)) {
			std::string steamStr = ReadString(list + 0x40);
			if (!steamStr.empty() && steamStr != "0") return steamStr;
			
			uint64_t steamId = ReadMemory<uint64_t>(list + 0x48);
			if (steamId != 0) return std::to_string(steamId);
			
			steamId = ReadMemory<uint64_t>(list + 0x50);
			if (steamId != 0) return std::to_string(steamId);
			
			steamStr = ReadString(list + 0x58);
			if (!steamStr.empty() && steamStr != "0") return steamStr;
		}
		list = ReadMemory<uintptr_t>(list + 0x8);
	}
	return "";
}

void RenderNotifications() {
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImFont* pFont = Menu.SegoeUI;
	ImGui::PushFont(pFont);
	
	float yOffset = 60.0f + damageLogPanelHeight;
	float xPos = 15.0f;
	
	auto now = std::chrono::steady_clock::now();
	
	for (auto it = notifications.begin(); it != notifications.end(); ) {
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->startTime).count() / 1000.0f;

		if (elapsed > 2.5f) {
			it->target_alpha = 0.0f;
		}

		if (elapsed > it->lifetime) {
			it = notifications.erase(it);
			continue;
		}

		it->alpha = ImLerp(it->alpha, it->target_alpha, ImGui::GetIO().DeltaTime * 8.0f);
		
		if (it->alpha > 0.01f) {
			ImGui::PushFont(pFont);
			ImVec2 textSize = ImGui::CalcTextSize(it->message.c_str());
			ImGui::PopFont();

			float textPadding = 15.0f;
			float indicatorWidth = 6.0f;
			float totalWidth = indicatorWidth + textPadding + textSize.x + textPadding;
			float totalHeight = 40.0f;

			float animOffset = (1.0f - it->alpha) * -30.0f;
			
			ImVec2 boxMin = ImVec2(xPos + animOffset, yOffset);
			ImVec2 boxMax = ImVec2(xPos + totalWidth + animOffset, yOffset + totalHeight);

			ImColor bgColor = ImColor(18, 18, 22, (int)(240 * it->alpha));
			drawList->AddRectFilled(boxMin, boxMax, bgColor, 8.0f);
			
			ImColor accentColor;
			if (it->isSuccess) {
				accentColor = ImColor(60, 200, 100, (int)(255 * it->alpha));
			} else {
				accentColor = ImColor(200, 60, 60, (int)(255 * it->alpha));
			}
			
			drawList->AddRectFilled(boxMin, ImVec2(boxMin.x + indicatorWidth, boxMax.y), accentColor, 8.0f, ImDrawFlags_RoundCornersLeft);
			
			drawList->AddRect(boxMin, boxMax, ImColor(50, 50, 60, (int)(100 * it->alpha)), 8.0f, 0, 1.0f);
			
			ImColor textColor = ImColor(240, 240, 245, (int)(255 * it->alpha));
			float textY = yOffset + (totalHeight - textSize.y) / 2.0f;
			drawList->AddText(pFont, 14.0f, ImVec2(xPos + indicatorWidth + textPadding + animOffset, textY), textColor, it->message.c_str());
			
			yOffset += (totalHeight + 8.0f);
		}
		
		++it;
	}
	
	ImGui::PopFont();
}

void AddDamageLogEntry(const std::string& name, float amount, bool incoming) {
	if (amount <= DAMAGE_MIN_AMOUNT) return;
	DamageLogEntry entry{};
	entry.name = name.empty() ? (incoming ? XorString("Unknown") : XorString("Target")) : name;
	entry.amount = amount;
	entry.incoming = incoming;
	entry.timestamp = std::chrono::steady_clock::now();

	std::lock_guard<std::mutex> lock(damageLogMutex);
	damageLog.push_back(entry);
	if (damageLog.size() > 30) {
		damageLog.erase(damageLog.begin(), damageLog.begin() + (damageLog.size() - 30));
	}
}

bool IsPedLikelyTarget(const Ped& ped, const Matrix& viewMatrix) {
	if (!ped.IsPlayer() || ped.IsDead()) return false;
	if (GetDistance(ped.position, LocalPlayer.position) > DAMAGE_MAX_DISTANCE) return false;

	Vector2 headScreen;
	if (!WorldToScreen(viewMatrix, ped.boneList[Head], headScreen)) return false;
	Vector2 center = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
	float dx = center.x - headScreen.x;
	float dy = center.y - headScreen.y;
	float dist = sqrtf(dx * dx + dy * dy);
	return dist <= DAMAGE_SCREEN_RADIUS;
}

int FindCrosshairTarget(const Matrix& viewMatrix) {
	int bestId = -1;
	float bestDist = DAMAGE_TARGET_RADIUS;
	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (!ped.IsPlayer() || ped.IsDead()) continue;
		if (GetDistance(ped.position, LocalPlayer.position) > DAMAGE_MAX_DISTANCE) continue;

		Vector2 headScreen;
		if (!WorldToScreen(viewMatrix, ped.boneList[Head], headScreen)) continue;
		Vector2 center = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
		float dist = sqrtf(powf(center.x - headScreen.x, 2) + powf(center.y - headScreen.y, 2));
		if (dist <= bestDist) {
			bestDist = dist;
			bestId = ped.GetId();
		}
	}
	return bestId;
}

void RenderDamageLog() {
	damageLogPanelHeight = 0.0f;
	if (!Cheats::Misc::DamageLog) return;

	auto now = std::chrono::steady_clock::now();
	std::vector<DamageLogEntry> entries;
	{
		std::lock_guard<std::mutex> lock(damageLogMutex);
		damageLog.erase(std::remove_if(damageLog.begin(), damageLog.end(), [&](const DamageLogEntry& e) {
			return std::chrono::duration_cast<std::chrono::seconds>(now - e.timestamp).count() > 10;
		}), damageLog.end());
		entries = damageLog;
	}

	if (entries.empty()) return;

	const float width = 320.0f;
	const float rowHeight = 24.0f;
	const float headerHeight = 28.0f;
	const float padding = 10.0f;
	size_t visibleCount = std::min<size_t>(entries.size(), 10);
	damageLogPanelHeight = headerHeight + (visibleCount * rowHeight) + padding;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	ImVec2 origin = ImVec2(15.0f, 15.0f);
	ImVec2 panelMin = origin;
	ImVec2 panelMax = ImVec2(origin.x + width, origin.y + damageLogPanelHeight);

	drawList->AddRectFilled(panelMin, panelMax, ImColor(18, 18, 22, 210), 8.0f);
	drawList->AddRect(panelMin, panelMax, ImColor(45, 45, 55, 200), 8.0f);

	ImGui::PushFont(Menu.InterSmaller);
	ImColor headerColor = ImColor(200, 200, 210);
	drawList->AddText(ImVec2(origin.x + padding, origin.y + 6.0f), headerColor, XorString("Damage Log"));
	ImGui::PopFont();

	float y = origin.y + headerHeight;
	size_t rendered = 0;
	for (auto it = entries.rbegin(); it != entries.rend() && rendered < visibleCount; ++it, ++rendered) {
		float ageSec = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->timestamp).count() / 1000.0f;
		float alpha = 1.0f - (ageSec / 10.0f);
		if (alpha < 0.1f) alpha = 0.1f;

		ImColor accent = it->incoming ? ImColor(210, 70, 70) : ImColor(70, 200, 120);
		ImColor textColor = ImColor(235, 235, 240, int(255 * alpha));
		ImColor rowBg = ImColor(24, 24, 30, int(220 * alpha));

		ImVec2 rowMin(origin.x + padding, y);
		ImVec2 rowMax(origin.x + width - padding, y + rowHeight - 2.0f);
		drawList->AddRectFilled(rowMin, rowMax, rowBg, 6.0f);
		drawList->AddRect(rowMin, rowMax, ImColor(40, 40, 48, int(160 * alpha)), 6.0f);

		drawList->AddRectFilled(ImVec2(rowMin.x, rowMin.y), ImVec2(rowMin.x + 4.0f, rowMax.y), accent, 6.0f, ImDrawFlags_RoundCornersLeft);

		ImGui::PushFont(Menu.InterSmaller);
		std::string amountText = (it->incoming ? "-" : "+") + std::to_string(static_cast<int>(it->amount));
		float textY = y + (rowHeight - ImGui::CalcTextSize("A").y) / 2.0f - 1.0f;
		drawList->AddText(ImVec2(rowMin.x + 10.0f, textY), textColor, it->name.c_str());

		ImVec2 amountSize = ImGui::CalcTextSize(amountText.c_str());
		drawList->AddText(ImVec2(rowMax.x - amountSize.x - 10.0f, textY), accent, amountText.c_str());
		ImGui::PopFont();

		y += rowHeight;
	}
}

void ToggleFriendAtCrosshair() {
	if (!LocalPlayer.Update()) return;
	
	Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
	Vector2 crosshairPos = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
	
	Ped closestPed;
	float minDistance = 9999.0f;
	bool foundTarget = false;

	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (!ped.IsPlayer()) continue; 
		
		float pDistance = GetDistance(ped.position, LocalPlayer.position);
		if (pDistance >= 300.0f) continue; 

		Vector2 headScreen;
		if (!WorldToScreen(viewMatrix, ped.boneList[Head], headScreen)) continue;

		float screenDistance = abs((crosshairPos - headScreen).Length());

		if (screenDistance < 100.0f && screenDistance < minDistance) {
			minDistance = screenDistance;
			closestPed = ped;
			foundTarget = true;
		}
	}
	
	if (foundTarget) {
		int pedID = closestPed.GetId();
		std::string pedName = GetPedName(closestPed);
		
		if (pedName.empty() || pedName == "Player") {
			pedName = "Player " + std::to_string(pedID);
		}

		if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {

			friendStatus[pedID] = false;
			AddNotification(pedName + " friend listesinden cikarildi", false); 
		} else {

			friendStatus[pedID] = true;
			AddNotification(pedName + " friend olarak eklendi", true); 
		}
	}
}

std::string GetKeyName(int keycode) {
	switch (keycode) {
		case VK_LBUTTON: return "MOUSE 1";
		case VK_RBUTTON: return "MOUSE 2";
		case VK_MBUTTON: return "MOUSE 3";
		case VK_XBUTTON1: return "MOUSE 4";
		case VK_XBUTTON2: return "MOUSE 5";
		case VK_SPACE: return "SPACE";
		case VK_SHIFT: return "SHIFT";
		case VK_CONTROL: return "CTRL";
		case VK_MENU: return "ALT";
		case VK_TAB: return "TAB";
		case VK_RETURN: return "ENTER";
		case VK_ESCAPE: return "ESC";
		case VK_DELETE: return "DELETE";
		case VK_INSERT: return "INSERT";
		case VK_HOME: return "HOME";
		case VK_END: return "END";
		case VK_PRIOR: return "PAGE UP";
		case VK_NEXT: return "PAGE DOWN";
		case VK_UP: return "UP ARROW";
		case VK_DOWN: return "DOWN ARROW";
		case VK_LEFT: return "LEFT ARROW";
		case VK_RIGHT: return "RIGHT ARROW";
		case VK_F1: return "F1";
		case VK_F2: return "F2";
		case VK_F3: return "F3";
		case VK_F4: return "F4";
		case VK_F5: return "F5";
		case VK_F6: return "F6";
		case VK_F7: return "F7";
		case VK_F8: return "F8";
		case VK_F9: return "F9";
		case VK_F10: return "F10";
		case VK_F11: return "F11";
		case VK_F12: return "F12";
		default:
			if (keycode >= 'A' && keycode <= 'Z') return std::string(1, (char)keycode);
			if (keycode >= '0' && keycode <= '9') return std::string(1, (char)keycode);
			return "UNKNOWN";
	}
}

void UpdateKeybindList() {
	std::vector<KeybindEntry> new_keybinds;

	auto add_keybind = [&](const std::string& name, int keycode) {
		if (keycode != 0) {
			KeybindEntry kb;
			kb.name = name;
			kb.keycode = keycode;
			kb.is_pressed = (GetAsyncKeyState(keycode) & 0x8000) != 0;
			kb.target_alpha = 1.0f;

			auto it = std::find_if(keybind_list.begin(), keybind_list.end(),
				[&](const KeybindEntry& k) { return k.name == kb.name; });

			if (it != keybind_list.end()) {
				kb.alpha = it->alpha;
				kb.key_color_progress = it->key_color_progress;
			}

			new_keybinds.push_back(kb);
		}
	};

	if (Cheats::AimAssist::Aimbot::Enabled && Cheats::AimAssist::Aimbot::Key != 0)
		add_keybind("Aimbot", Cheats::AimAssist::Aimbot::Key);
	
	if (Cheats::AimAssist::Silent::Enabled && Cheats::AimAssist::Silent::Key != 0)
		add_keybind("Silent Aim", Cheats::AimAssist::Silent::Key);
	
	if (Cheats::NoClip::Enabled && Cheats::NoClip::Key != 0)
		add_keybind("NoClip", Cheats::NoClip::Key);
	
	if (Cheats::Exploit::HealthBoost && Cheats::Exploit::HealthBoostKey != 0)
		add_keybind("Health Boost", Cheats::Exploit::HealthBoostKey);
	
	if (Cheats::Exploit::ArmorBoost && Cheats::Exploit::ArmorBoostKey != 0)
		add_keybind("Armor Boost", Cheats::Exploit::ArmorBoostKey);
	
	if (Cheats::AimAssist::Silent::MagicBullet && Cheats::AimAssist::Silent::MagicBulletKey != 0)
		add_keybind("Magic Bullet", Cheats::AimAssist::Silent::MagicBulletKey);
	
	if (Cheats::AimAssist::Silent::WheelBullet && Cheats::AimAssist::Silent::WheelBulletKey != 0)
		add_keybind("Wheel Bullet", Cheats::AimAssist::Silent::WheelBulletKey);
	
	if (Cheats::Exploit::ServerCrash && Cheats::Exploit::ServerCrashKey != 0)
		add_keybind("Manuel crash", Cheats::Exploit::ServerCrashKey);
	
	if (Cheats::Vehicle::Fix && Cheats::Vehicle::FixKey != 0)
		add_keybind("Vehicle Fix", Cheats::Vehicle::FixKey);
	
	if (Cheats::MenuUtils::FriendKey != 0)
		add_keybind("Friend Toggle", Cheats::MenuUtils::FriendKey);
	
	if (Cheats::Exploit::Strafe::Enabled && Cheats::Exploit::Strafe::Key != 0)
		add_keybind("Strafe Lag", Cheats::Exploit::Strafe::Key);
	
	if (Cheats::Exploit::PeakAssist && Cheats::Exploit::PeakAssistKey != 0)
		add_keybind("Peak Assist", Cheats::Exploit::PeakAssistKey);
	
	if (Cheats::Exploit::SafeInvisible && Cheats::Exploit::SafeInvisibleKey != 0)
		add_keybind("Safe Invisible", Cheats::Exploit::SafeInvisibleKey);

	if (Cheats::FreeCam::Key != 0)
		add_keybind("Free Cam", Cheats::FreeCam::Key);
	
	if (Cheats::FreeCam::Enabled && Cheats::FreeCam::TpKey != 0)
		add_keybind("FreeCam TP", Cheats::FreeCam::TpKey);

	keybind_list = new_keybinds;

	for (auto& kb : keybind_list) {
		kb.alpha = ImLerp(kb.alpha, kb.target_alpha, ImGui::GetIO().DeltaTime * 8.0f);
		float target_color = kb.is_pressed ? 1.0f : 0.0f;
		kb.key_color_progress = ImLerp(kb.key_color_progress, target_color, ImGui::GetIO().DeltaTime * 12.0f);
	}
}

void RenderKeybindOverlay() {
	if (!Cheats::MenuUtils::KeybindList)
		return;

	UpdateKeybindList();

	int keybind_count = static_cast<int>(keybind_list.size());

	float target_overlay_alpha = 1.0f;
	keybind_overlay_alpha = ImLerp(keybind_overlay_alpha, target_overlay_alpha, ImGui::GetIO().DeltaTime * 6.0f);

	float target_height_progress = (keybind_count > 0) ? 1.0f : 0.0f;
	keybind_overlay_height_progress = ImLerp(keybind_overlay_height_progress, target_height_progress, ImGui::GetIO().DeltaTime * 8.0f);

	if (keybind_overlay_alpha < 0.01f) return;

	if (keybind_overlay_pos.x == 0 && keybind_overlay_pos.y == 0) {
		keybind_overlay_pos = ImVec2(ImGui::GetIO().DisplaySize.x - 220, 80);
	}

	float item_height = 18.0f; 
	float title_height = 28.0f; 
	float padding_top = 8.0f;
	float padding_bottom = 8.0f; 
	float padding_horizontal = 10.0f;
	float window_height = title_height + padding_top + (keybind_count * item_height * keybind_overlay_height_progress) + padding_bottom;
	float window_width = 180.0f; 

	ImGui::SetNextWindowPos(keybind_overlay_pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, keybind_overlay_alpha);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2)); 

	ImGui::Begin("##KeybindOverlay", nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 windowSize = ImGui::GetWindowSize();

	ImVec2 bgMin = windowPos;
	ImVec2 bgMax = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);
	
	ImColor bgColor = ImColor(18, 18, 22, (int)(220 * keybind_overlay_alpha));
	drawList->AddRectFilled(bgMin, bgMax, bgColor, 6.0f);

	ImGui::SetCursorPos(ImVec2(padding_horizontal, padding_top));
	
	ImGui::PushFont(Menu.SegoeUI);
	char title[64];
	snprintf(title, sizeof(title), "Keybinds (%d)", keybind_count);
	ImVec2 titlePos = ImVec2(windowPos.x + padding_horizontal, windowPos.y + padding_top + 2);
	ImColor titleColor = ImColor(Menu.TextColor.Value.x, Menu.TextColor.Value.y, Menu.TextColor.Value.z, keybind_overlay_alpha);
	drawList->AddText(titlePos, titleColor, title);
	ImGui::PopFont();

	if (ImGui::IsWindowHovered() && (GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) {
		if (!keybind_overlay_dragging) {
			keybind_overlay_dragging = true;
			keybind_overlay_drag_offset = ImVec2(
				ImGui::GetMousePos().x - keybind_overlay_pos.x,
				ImGui::GetMousePos().y - keybind_overlay_pos.y
			);
		}
	}

	if (keybind_overlay_dragging) {
		if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
			keybind_overlay_pos = ImVec2(
				ImGui::GetMousePos().x - keybind_overlay_drag_offset.x,
				ImGui::GetMousePos().y - keybind_overlay_drag_offset.y
			);
		}
		else {
			keybind_overlay_dragging = false;
		}
	}

	ImGui::SetCursorPos(ImVec2(padding_horizontal, title_height + 2));

	ImVec2 content_start = ImGui::GetCursorScreenPos();
	float content_item_height = item_height; 
	float total_content_height = keybind_list.size() * content_item_height;
	float clipped_height = total_content_height * keybind_overlay_height_progress;
	
	ImVec2 clip_min = content_start;
	ImVec2 clip_max = ImVec2(content_start.x + windowSize.x - padding_horizontal * 2, content_start.y + clipped_height);
	ImGui::PushClipRect(clip_min, clip_max, true);

	ImGui::PushFont(Menu.InterSmaller);
	for (const auto& kb : keybind_list) {
		if (kb.alpha > 0.01f) {
			ImVec2 itemPos = ImGui::GetCursorScreenPos();
			
			ImColor text_color = ImColor(Menu.TextDimColor.Value.x, Menu.TextDimColor.Value.y, Menu.TextDimColor.Value.z, Menu.TextDimColor.Value.w * kb.alpha * keybind_overlay_alpha);
			drawList->AddText(ImVec2(itemPos.x, itemPos.y + 2), text_color, kb.name.c_str());

			std::string key_name = GetKeyName(kb.keycode);
			ImVec2 keyNameSize = ImGui::CalcTextSize(key_name.c_str());
			float keyX = itemPos.x + windowSize.x - padding_horizontal * 2 - keyNameSize.x;
			
			ImColor keyTextColor = ImColor(Menu.AccentColor.Value.x, Menu.AccentColor.Value.y, Menu.AccentColor.Value.z, kb.alpha * keybind_overlay_alpha);
			drawList->AddText(ImVec2(keyX, itemPos.y + 2), keyTextColor, key_name.c_str());

			ImGui::SetCursorScreenPos(ImVec2(itemPos.x, itemPos.y + item_height));
		}
	}
	ImGui::PopFont();
	ImGui::PopClipRect();

	keybind_overlay_pos = ImGui::GetWindowPos();

	ImGui::End();
	ImGui::PopStyleVar(5); 
}

struct BoneInfo {
	int boneId;
	float priority; 
};

static const BoneInfo targetBones[] = {

	{Head, 1.0f},
	{Jaw, 1.1f},
	{Neck, 1.2f},
	{Neck0, 1.3f},
	{Neck2, 1.4f},

	{Chest, 2.0f},
	{Spine2, 2.1f},
	{Spine1, 2.2f},
	{Spine0, 2.3f},
	{Spine3, 2.4f},
	{Spine4, 2.5f},
	{Spine5, 2.6f},
	{SpineRoot, 2.7f},

	{Hip, 3.0f},
	{Root, 3.1f},

	{LeftClavicle, 4.0f},
	{RightClavicle, 4.0f},
	{LeftUpperArm, 4.1f},
	{RightUpperArm, 4.1f},

	{LeftElbow, 4.2f},
	{RightElbow, 4.2f},
	{LeftForearm, 4.3f},
	{RightForearm, 4.3f},
	{LeftHand, 4.4f},
	{RightHand, 4.4f},

	{LeftThigh, 5.0f},
	{RightThigh, 5.0f},
	
	
	{LeftKnee, 5.1f},
	{RightKnee, 5.1f},
	{LeftAnkle, 5.2f},
	{RightAnkle, 5.2f},
	{LeftFoot, 5.3f},
	{RightFoot, 5.3f},
	{LeftToe, 5.4f},
	{RightToe, 5.4f}
};

Vector3 FindClosestBoneInFOV(const Ped& ped, float fov, const Matrix& viewMatrix) {
	Vector3 bestBonePos = Vector3(0, 0, 0);
	float bestScreenDistance = FLT_MAX;
	bool foundValidBone = false;

	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);

	bool headShotEnabled = Cheats::AimAssist::Silent::HeadShot;

	for (const auto& boneInfo : targetBones) {
		if (boneInfo.boneId < 0 || boneInfo.boneId >= 35) continue;


		if (!headShotEnabled && boneInfo.boneId == Head) {
			continue; 
		}
		
		Vector3 bonePos = ped.boneList[boneInfo.boneId];

		if (Vec3Empty(bonePos))
			continue;

		Vector2 screenPos;
		if (!WorldToScreen(viewMatrix, bonePos, screenPos))
			continue;

		float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));

		bool isLookingUp = (screenPos.y < screenCenter.y);


		float adjustedFov = fov;
		if (!headShotEnabled) {

			if (boneInfo.boneId == Jaw || boneInfo.boneId == Neck || 
			    boneInfo.boneId == Neck0 || boneInfo.boneId == Neck2) {

				if (isLookingUp) {

					adjustedFov = fov * 3.0f;
				} else {

					adjustedFov = fov * 2.0f;
				}
			} else {

				if (isLookingUp) {
					adjustedFov = fov * 2.0f; 
				} else {
					adjustedFov = fov * 1.5f; 
				}
			}
		} else {

			if (isLookingUp) {
				adjustedFov = fov * 1.5f; 
			} else {
				adjustedFov = fov * 1.2f; 
			}
		}

		if (distanceFromCenter > adjustedFov)
			continue;

		float adjustedDistance = distanceFromCenter;
		if (!headShotEnabled) {

			if (boneInfo.boneId == Jaw || boneInfo.boneId == Neck || 
			    boneInfo.boneId == Neck0 || boneInfo.boneId == Neck2) {

				adjustedDistance = distanceFromCenter * 0.8f;
			}
		}



		if (!foundValidBone || adjustedDistance < bestScreenDistance) {
			bestBonePos = bonePos;
			bestScreenDistance = adjustedDistance;
			foundValidBone = true;
		}
	}

	if (!foundValidBone) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}


	if (!headShotEnabled && !Vec3Empty(ped.boneList[Head])) {
		float headDistance = GetDistance(bestBonePos, ped.boneList[Head]);
		if (headDistance < 0.1f) { 

			return Vector3(0.0f, 0.0f, 0.0f);
		}
	}

	return bestBonePos;
}

void TeleportObject(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3 Position, Vector3 VisualPosition, bool Stop);

Vector3 FindClosestBoneInFOVForAimbot(const Ped& ped, float fov, const Matrix& viewMatrix) {
	Vector3 bestBonePos = Vector3(0, 0, 0);
	float bestScreenDistance = FLT_MAX;
	bool foundValidBone = false;

	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);

	for (const auto& boneInfo : targetBones) {
		if (boneInfo.boneId < 0 || boneInfo.boneId >= 35) continue;
		
		Vector3 bonePos = ped.boneList[boneInfo.boneId];

		if (Vec3Empty(bonePos))
			continue;

		Vector2 screenPos;
		if (!WorldToScreen(viewMatrix, bonePos, screenPos))
			continue;

		float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));

		if (distanceFromCenter > fov)
			continue;

		if (!foundValidBone || distanceFromCenter < bestScreenDistance) {
			bestBonePos = bonePos;
			bestScreenDistance = distanceFromCenter;
			foundValidBone = true;
		}
	}

	if (!foundValidBone) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	return bestBonePos;
}

Vector3 FindRandomBoneInFOV(const Ped& ped, float fov, const Matrix& viewMatrix) {
	std::vector<Vector3> bonesInFOV;
	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
	
	bool headShotEnabled = Cheats::AimAssist::Silent::HeadShot;
	
	for (const auto& boneInfo : targetBones) {
		if (boneInfo.boneId < 0 || boneInfo.boneId >= 35) continue;
		
		if (!headShotEnabled && boneInfo.boneId == Head) {
			continue;
		}
		
		Vector3 bonePos = ped.boneList[boneInfo.boneId];
		if (Vec3Empty(bonePos))
			continue;
		
		Vector2 screenPos;
		if (!WorldToScreen(viewMatrix, bonePos, screenPos))
			continue;
		
		float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));
		
		// Tam FOV kontrolü (adjustedFov kullanmadan)
		if (distanceFromCenter <= fov) {
			bonesInFOV.push_back(bonePos);
		}
	}
	
	if (bonesInFOV.empty()) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	
	// Rastgele bir kemik seç
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, (int)bonesInFOV.size() - 1);
	
	return bonesInFOV[dis(gen)];
}

Vector3 FindClosestWheelInFOV(float fov, const Matrix& viewMatrix) {
	Vector3 bestWheelPos = Vector3(0, 0, 0);
	float bestScreenDistance = FLT_MAX;
	bool foundValidWheel = false;

	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);

	uintptr_t vehicleInterface = ReadMemory<DWORD64>(Game.ReplayInterface + 0x10);
	if (!vehicleInterface) return Vector3(0, 0, 0);
	
	uintptr_t vehicleList = ReadMemory<DWORD64>(vehicleInterface + 0x180);
	if (!vehicleList) return Vector3(0, 0, 0);
	
	int vehicleListCount = ReadMemory<int>(vehicleInterface + 0x188);
	if (vehicleListCount > 300 || vehicleListCount <= 0) return Vector3(0, 0, 0);
	
	Vector3 localPos = LocalPlayer.position;
	
	for (int i = 0; i < vehicleListCount; ++i) {
		uintptr_t vehicle = ReadMemory<uintptr_t>(vehicleList + (i * 0x10));
		if (!vehicle) continue;
		
		Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
		if (Vec3Empty(vehiclePos)) continue;

		float distance = GetDistance(vehiclePos, localPos);
		if (distance > (float)Cheats::AimAssist::Silent::Distance) continue;

		Matrix vehicleMatrix = ReadMemory<Matrix>(vehicle + 0x60);





		Vector3 wheelOffsetsLocal[4] = {
			Vector3(-1.0f, 1.5f, -0.55f),  
			Vector3(1.0f, 1.5f, -0.55f),   
			Vector3(-1.0f, -1.5f, -0.55f), 
			Vector3(1.0f, -1.5f, -0.55f)   
		};

		for (int j = 0; j < 4; j++) {

			Vector3 offset = wheelOffsetsLocal[j];
			Vector3 wheelLocalPos;
			wheelLocalPos.x = offset.x * vehicleMatrix._11 + offset.y * vehicleMatrix._21 + offset.z * vehicleMatrix._31;
			wheelLocalPos.y = offset.x * vehicleMatrix._12 + offset.y * vehicleMatrix._22 + offset.z * vehicleMatrix._32;
			wheelLocalPos.z = offset.x * vehicleMatrix._13 + offset.y * vehicleMatrix._23 + offset.z * vehicleMatrix._33;

			Vector3 wheelWorldPos = Vector3(
				vehiclePos.x + wheelLocalPos.x,
				vehiclePos.y + wheelLocalPos.y,
				vehiclePos.z + wheelLocalPos.z
			);

			Vector2 screenPos;
			if (!WorldToScreen(viewMatrix, wheelWorldPos, screenPos))
				continue;

			float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));

			if (distanceFromCenter > fov)
				continue;

			if (!foundValidWheel || distanceFromCenter < bestScreenDistance) {
				bestWheelPos = wheelWorldPos;
				bestScreenDistance = distanceFromCenter;
				foundValidWheel = true;
			}
		}
	}
	
	return bestWheelPos;
}

Vector3 FindClosestBone(const Ped& ped, const Matrix& viewMatrix) {
	return FindClosestBoneInFOV(ped, 9999.0f, viewMatrix);
}

std::string GetPedName(Ped& ped) {
	std::string pedName = XorString("Player");
	if (!Offsets.CitizenPlayernamesBase) {
		return pedName;
	}

	uint64_t playerNames = ReadMemory<uint64_t>(Offsets.CitizenPlayernamesBase + 0x30D38);
	if (!playerNames) {
		return pedName;
	}

	int playerId = ped.GetId();
	uintptr_t list = ReadMemory<uintptr_t>(playerNames + 0x8);
	for (uint32_t i = 0; i < maxPlayerCount; ++i) {
		if (!list) {
			break;
		}

		if (playerId == ReadMemory<int>(list + 0x10)) {
			std::vector<uintptr_t> nameOffsets = { 0x18, 0x20, 0x28, 0x30, 0x38, 0x40 };
			
			for (uintptr_t offset : nameOffsets) {
				uintptr_t namePtr = ReadMemory<uintptr_t>(list + offset);
				if (namePtr) {
					std::string testName = ReadString(namePtr);
					if (!testName.empty() && testName != "0" && testName != "Player" && testName.length() > 0 && testName.length() < 64) {
						bool isValid = true;
						for (size_t k = 0; k < testName.length(); ++k) {
							unsigned char c = testName[k];
							if (c == 0 || (c < 32 && c != 9 && c != 10 && c != 13)) {
								if (k == 0 || c != 0) {
									isValid = false;
									break;
								}
							}
						}
						
						if (isValid) {
							pedName = testName;
							break;
						}
					}
				}
				
				std::string testName = ReadString(list + offset);
				if (!testName.empty() && testName != "0" && testName != "Player" && testName.length() > 0 && testName.length() < 64) {
					bool isValid = true;
					for (size_t k = 0; k < testName.length(); ++k) {
						unsigned char c = testName[k];
						if (c == 0 || (c < 32 && c != 9 && c != 10 && c != 13)) {
							if (k == 0 || c != 0) {
								isValid = false;
								break;
							}
						}
					}
					
					if (isValid && pedName == XorString("Player")) {
						pedName = testName;
						break;
					}
				}
			}

			if (pedName == XorString("Player") || pedName.empty()) {
				uintptr_t namePtr = ReadMemory<uintptr_t>(list + 0x18);
				if (namePtr) {
					pedName = ReadString(namePtr);
				}
				if (pedName.empty() || pedName == "Player" || pedName == "0") {
					pedName = ReadString(list + 0x18);
				}
			}

			std::string fixedName = "";
			for (size_t j = 0; j < pedName.length(); ++j) {
				unsigned char c = pedName[j];
				if (c == 0) break;
				
				if (c == 0xC4) { 
					if (j + 1 < pedName.length()) {
						unsigned char next = pedName[j + 1];
						if (next == 0x9E) fixedName += "Ğ";
						else if (next == 0x9F) fixedName += "ğ";
						else if (next == 0xB0) fixedName += "İ";
						else if (next == 0xB1) fixedName += "ı";
						else if (next == 0x9E) fixedName += "Ş";
						else if (next == 0x9F) fixedName += "ş";
						else fixedName += c;
						j++; 
					} else {
						fixedName += c;
					}
				}
				else if (c == 0xC3) { 
					if (j + 1 < pedName.length()) {
						unsigned char next = pedName[j + 1];
						if (next == 0x96) fixedName += "Ö";
						else if (next == 0xB6) fixedName += "ö";
						else if (next == 0x9C) fixedName += "Ü";
						else if (next == 0xBC) fixedName += "ü";
						else if (next == 0x87) fixedName += "Ç";
						else if (next == 0xA7) fixedName += "ç";
						else fixedName += c;
						j++; 
					} else {
						fixedName += c;
					}
				}
				else if (c >= 32 && c <= 126) { 
					fixedName += c;
				}
				else if (c == 0xC4 && j + 1 < pedName.length() && pedName[j + 1] == 0x9E) {
					fixedName += "Ş";
					j++;
				}
				else if (c == 0xC4 && j + 1 < pedName.length() && pedName[j + 1] == 0x9F) {
					fixedName += "ş";
					j++;
				}
				else if (c > 127) {
					fixedName += c;
				}
			}
			pedName = fixedName.empty() ? pedName : fixedName;
			
			if (pedName.empty() || pedName == XorString("Player") || pedName == "0") {
				pedName = "Player " + std::to_string(playerId);
			}
			break;
		}
		list = ReadMemory<uintptr_t>(list + 0x8);
	}
	return pedName;
}

void UpdatePeds() {
	while (!exitLoop) {
		std::vector<Ped> updatedPedList;
		Game.World = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
		LocalPlayer.pointer = ReadMemory<uintptr_t>(Game.World + Offsets.LocalPlayer);
		
		if (Game.World == 0) {
			Sleep(100);
			continue;
		}
		
		if (LocalPlayer.pointer == 0) {
			Sleep(100);
			continue;
		}

		if (!LocalPlayer.Update()) {
			Sleep(50);
			continue;
		}

		Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
		Game.ReplayInterface = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
		Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);

		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
			lastLocalShotTime = std::chrono::steady_clock::now();
			lastShotTargetId = FindCrosshairTarget(viewMatrix);
			lastShotLogged = false;
			if (lastShotTargetId != -1) {
				auto itStart = pedHealthHistoryCombined.find(lastShotTargetId);
				lastShotTargetStartCombined = (itStart != pedHealthHistoryCombined.end()) ? itStart->second : 0.0f;
			} else {
				lastShotTargetStartCombined = 0.0f;
			}
		} else {
			auto msSinceShot = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastLocalShotTime).count();
			if (msSinceShot > DAMAGE_SHOT_WINDOW_MS) {
				lastShotTargetId = -1;
				lastShotLogged = false;
			}
		}

		float currentLocalHealth = LocalPlayer.health + LocalPlayer.armor;
		std::set<int> seenPeds;

		uintptr_t entityListPtr = ReadMemory<uintptr_t>(Game.ReplayInterface + 0x18);
		uintptr_t entityList = ReadMemory<uintptr_t>(entityListPtr + 0x100);

		for (int i = 0; i < maxPlayerCount; i++) {
			Ped ped;
			uintptr_t player = ReadMemory<uintptr_t>(entityList + (i * 0x10));
			if (player == LocalPlayer.pointer) {
				continue;
			}
			else if (!ped.GetPlayer(player)) {
				continue;
			}
			else if (!ped.Update()) {
				continue;
			}

			int pedId = ped.GetId();
			if (ped.IsPlayer()) {
				seenPeds.insert(pedId);
				float combined = ped.health + ped.armor;
				auto it = pedHealthHistoryCombined.find(pedId);
				// Outgoing hit logging disabled to prevent random spam; only keep history.
				if (it != pedHealthHistoryCombined.end()) {
					(void)it->second; // keep branch for future use without side effects
				}
				pedHealthHistoryCombined[pedId] = combined;
			}
			updatedPedList.push_back(ped);
		}

		if (hasLocalHealthSnapshot) {
			float delta = lastLocalCombinedHealth - currentLocalHealth;
			if (delta > DAMAGE_MIN_AMOUNT) {
				std::string attackerName = XorString("Unknown");
				float closestDistance = DAMAGE_MAX_DISTANCE;
				for (auto& ped : updatedPedList) {
					if (!ped.IsPlayer()) continue;
					if (!IsPedLikelyTarget(ped, viewMatrix)) continue;
					float distance = GetDistance(ped.position, LocalPlayer.position);
					if (distance < closestDistance) {
						closestDistance = distance;
						attackerName = GetPedName(ped);
						if (attackerName.empty() || attackerName == "Player") {
							attackerName = XorString("Player ") + std::to_string(ped.GetId());
						}
					}
				}

				// Damage reduction: heal back a portion of the taken damage.
				if (Cheats::Misc::DamageReduction && Cheats::Misc::DamageReductionPercent > 0) {
					float reduceRatio = static_cast<float>(Cheats::Misc::DamageReductionPercent) / 100.0f;
					if (reduceRatio < 0.0f) reduceRatio = 0.0f;
					if (reduceRatio > 1.0f) reduceRatio = 1.0f;
					float healAmount = delta * reduceRatio;
					if (healAmount > 0.0f) {
						float curHealth = ReadMemory<float>(LocalPlayer.pointer + Offsets.Health);
						float curArmor = ReadMemory<float>(LocalPlayer.pointer + Offsets.Armor);
						float currentCombined = curHealth + curArmor;
						float targetCombined = currentCombined + healAmount;
						if (targetCombined > lastLocalCombinedHealth) {
							targetCombined = lastLocalCombinedHealth;
						}
						float need = targetCombined - currentCombined;
						if (need > 0.0f) {
							float newHealth = curHealth + need;
							WriteMemory<float>(LocalPlayer.pointer + Offsets.Health, newHealth);
						}
					}
				}

				auto now = std::chrono::steady_clock::now();
				auto msSince = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastIncomingLog).count();
				if (msSince >= DAMAGE_DEDUP_MS) {
					AddDamageLogEntry(attackerName, delta, true);
					lastIncomingLog = now;
				}
			}
		}
		lastLocalCombinedHealth = currentLocalHealth;
		hasLocalHealthSnapshot = true;

		for (auto it = pedHealthHistoryCombined.begin(); it != pedHealthHistoryCombined.end(); ) {
			if (seenPeds.find(it->first) == seenPeds.end()) {
				it = pedHealthHistoryCombined.erase(it);
			} else {
				++it;
			}
		}

		PedList = updatedPedList;
		Sleep(loopDelay);
	}
}

namespace Draw {
	void Always() {
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		if (Cheats::AimAssist::Aimbot::DrawFov) {
			drawList->AddCircle(ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f), Cheats::AimAssist::Aimbot::Fov, Cheats::AimAssist::Aimbot::Color, 100, 1.0f);
		}

		if (Cheats::AimAssist::Silent::DrawFov) {
			drawList->AddCircle(ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f), Cheats::AimAssist::Silent::Fov, Cheats::AimAssist::Silent::Color, 100, 1.0f);
		}

	}

	void Esp() {
		ImFont* pFont = Cheats::MenuUtils::GetSelectedFont();
		ImGui::PushFont(pFont);
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
		if (!LocalPlayer.Update()) {
			return;
		}

		for (auto& ped : PedList) {
			if (!ped.Update()) {
				continue;
			}

			float pDistance = GetDistance(ped.position, LocalPlayer.position);
			if (pDistance > Cheats::Players::Distance) {
				continue;
			}
			if (Cheats::Players::IgnorePed && !ped.IsPlayer()) {
				continue;
			}

			if (Cheats::Players::IgnoreDeath && ped.IsDead()) {
				continue;
			}


			int pedID = ped.GetId();
			bool isLockedTarget = Cheats::Players::HighlightTarget && pedID == CurrentAimLockId;
			ImColor targetColor = Cheats::Players::HighlightColor;
			Vector2 pBase{}, pHead{}, pNeck{}, pLeftFoot{}, pRightFoot{};
			if (!WorldToScreen(viewMatrix, ped.position, pBase) ||
				!WorldToScreen(viewMatrix, ped.boneList[Head], pHead) ||
				!WorldToScreen(viewMatrix, ped.boneList[Neck], pNeck) ||
				!WorldToScreen(viewMatrix, ped.boneList[LeftFoot], pLeftFoot) ||
				!WorldToScreen(viewMatrix, ped.boneList[RightFoot], pRightFoot)) {
				continue;
			}

			float HeadToNeck = pNeck.y - pHead.y;
			float pTop = pHead.y - (HeadToNeck * 2.5f);
			float pBottom = (pLeftFoot.y > pRightFoot.y ? pLeftFoot.y : pRightFoot.y) * 1.001f;
			float pHeight = pBottom - pTop;
			float pWidth = pHeight / 3.5f;
			float bScale = pWidth / 1.2f;
			float reducedWidth = pWidth * Cheats::Players::DrawBox::Size;
			ImVec2 pos(pBase.x, pBottom + 2.0f);
			float spacing = 2.0f;
			float baseSize = 15.0f;
			float scaledSize = baseSize;
			if (pDistance > 100.0f) {
				scaledSize = baseSize * (100.0f / pDistance);
				if (scaledSize < 8.0f) {
					scaledSize = 8.0f;
				}
			}

			if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {
				ImColor friendsColor = ImColor(255, 255, 0);
				DrawOutlinedText(drawList, pFont, XorString("Friend"), ImVec2(pBase.x, pTop - 15) + ImVec2(1, 1), 15.0f, ImColor(0, 0, 0, 150), true);
				DrawOutlinedText(drawList, pFont, XorString("Friend"), ImVec2(pBase.x, pTop - 15), 15.0f, friendsColor, true);
				continue;
			}

			if (Cheats::Players::DrawSkeleton::Enabled) {
				ImColor color = isLockedTarget ? targetColor : Cheats::Players::DrawSkeleton::Color;

				// force a proper two-piece arm: shoulder sits between neck and elbow
				auto synthShoulder = [&](const Vector3& neck, const Vector3& elbow) {
					return neck + ((elbow - neck) * 0.5f);
				};
				Vector3 leftShoulder = synthShoulder(ped.boneList[Neck], ped.boneList[LeftElbow]);
				Vector3 rightShoulder = synthShoulder(ped.boneList[Neck], ped.boneList[RightElbow]);

				std::vector<std::pair<Vector3, Vector3>> skeletonSegments = {
					{ ped.boneList[Head], ped.boneList[Neck] },
					{ ped.boneList[Neck], ped.boneList[Hip] },
					{ ped.boneList[Hip], ped.boneList[LeftFoot] },
					{ ped.boneList[Hip], ped.boneList[RightFoot] },
					{ ped.boneList[Neck], leftShoulder },
					{ leftShoulder, ped.boneList[LeftElbow] },
					{ ped.boneList[LeftElbow], ped.boneList[LeftHand] },
					{ ped.boneList[Neck], rightShoulder },
					{ rightShoulder, ped.boneList[RightElbow] },
					{ ped.boneList[RightElbow], ped.boneList[RightHand] }
				};

				for (const auto& seg : skeletonSegments) {
					Vector2 ScreenB1, ScreenB2;
					if (Vec3Empty(seg.first) || Vec3Empty(seg.second)) {
						continue;
					}
					if (!WorldToScreen(viewMatrix, seg.first, ScreenB1) || !WorldToScreen(viewMatrix, seg.second, ScreenB2)) {
						continue;
					}
					const float outlineThickness = Cheats::Players::DrawSkeleton::OutlineThickness;
					const float skeletonThickness = Cheats::Players::DrawSkeleton::Thickness;
					if (outlineThickness > 0.0f) {
						DrawLine(drawList, ImVec2(ScreenB1.x, ScreenB1.y), ImVec2(ScreenB2.x, ScreenB2.y), ImColor(0, 0, 0, 255), outlineThickness);
					}
					DrawLine(drawList, ImVec2(ScreenB1.x, ScreenB1.y), ImVec2(ScreenB2.x, ScreenB2.y), color, skeletonThickness);
				}
			}

			if (Cheats::Players::DrawBox::Enabled) {
				ImColor color = Cheats::Players::DrawBox::Color;
				ImVec4 mainColor = color.Value;

				auto DrawBoxWithGradient = [&](bool isCornerBox) {
					if (Cheats::Players::DrawBox::GradientEnabled) {
						ImColor topColor, bottomColor;
						if (Cheats::Players::DrawBox::UseCustomGradient) {
							topColor = Cheats::Players::DrawBox::BoxGradientTopColor;
							topColor.Value.w *= Cheats::Players::DrawBox::GradientIntensity;
							bottomColor = Cheats::Players::DrawBox::BoxGradientBottomColor;
							bottomColor.Value.w *= Cheats::Players::DrawBox::GradientIntensity;
						}
						else {
							float intensity = Cheats::Players::DrawBox::GradientIntensity;
							topColor = ImColor(mainColor.x * 0.6f, mainColor.y * 0.6f, mainColor.z * 0.6f, 0.25f * intensity);
							bottomColor = ImColor(mainColor.x * 0.1f, mainColor.y * 0.1f, mainColor.z * 0.1f, 0.35f * intensity);
						}

						drawList->AddRectFilledMultiColor(ImVec2(pBase.x - reducedWidth + 1, pTop + 1), ImVec2(pBase.x + reducedWidth - 1, pBottom - 1), ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(bottomColor), ImGui::ColorConvertFloat4ToU32(bottomColor));
					}

					if (Cheats::Players::DrawBox::FillEnabled) {
						ImColor fill = Cheats::Players::DrawBox::FillColor;
						fill.Value.w *= Cheats::Players::DrawBox::FillOpacity;
						drawList->AddRectFilled(ImVec2(pBase.x - reducedWidth + 1, pTop + 1), ImVec2(pBase.x + reducedWidth - 1, pBottom - 1), fill);
					}

					ImVec2 boxMin = ImVec2(pBase.x - reducedWidth, pTop);
					ImVec2 boxMax = ImVec2(pBase.x + reducedWidth, pBottom);
					const float borderThickness = Cheats::Players::DrawBox::Thickness;

					if (!isCornerBox) {
						ImVec2 outlineMin = ImVec2(boxMin.x - 1.0f, boxMin.y - 1.0f);
						ImVec2 outlineMax = ImVec2(boxMax.x + 1.0f, boxMax.y + 1.0f);
						drawList->AddRect(outlineMin, outlineMax, ImColor(0, 0, 0, 200), 0.0f, 0, borderThickness);
						drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, borderThickness);
					}
					else {
						float cornerLen = (bScale > 6.0f ? bScale : 6.0f);
						ImColor outlineColor = ImColor(0, 0, 0, 200);
						auto drawCorner = [&](ImVec2 origin, ImVec2 dirH, ImVec2 dirV) {
							ImVec2 hEnd = ImVec2(origin.x + dirH.x * cornerLen, origin.y + dirH.y * cornerLen);
							ImVec2 vEnd = ImVec2(origin.x + dirV.x * cornerLen, origin.y + dirV.y * cornerLen);

							drawList->AddLine(origin, hEnd, outlineColor, borderThickness + 0.8f);
							drawList->AddLine(origin, vEnd, outlineColor, borderThickness + 0.8f);

							drawList->AddLine(origin, hEnd, color, borderThickness);
							drawList->AddLine(origin, vEnd, color, borderThickness);
						};

						drawCorner(boxMin, ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f));
						drawCorner(ImVec2(boxMax.x, boxMin.y), ImVec2(-1.0f, 0.0f), ImVec2(0.0f, 1.0f));
						drawCorner(ImVec2(boxMin.x, boxMax.y), ImVec2(1.0f, 0.0f), ImVec2(0.0f, -1.0f));
						drawCorner(boxMax, ImVec2(-1.0f, 0.0f), ImVec2(0.0f, -1.0f));
					}
					};

				if (Cheats::Players::DrawBox::SelectedType == 0) {
					DrawBoxWithGradient(false);
				}
				else if (Cheats::Players::DrawBox::SelectedType == 1) {
					DrawBoxWithGradient(true);
				}
			}

			if (Cheats::Players::DrawLine::Enabled) {
				ImColor color = Cheats::Players::DrawLine::Color;
				ImVec2 startPos, endPos;
				ImGuiIO& io = ImGui::GetIO();
				if (Cheats::Players::DrawLine::SelectedType == 0) {
					startPos.x = pBase.x + reducedWidth;
					startPos.y = pTop;
					endPos.x = Game.lpRect.right / 2.f;
					endPos.y = 0;
				}
				if (Cheats::Players::DrawLine::SelectedType == 1) {
					startPos.x = pBase.x + reducedWidth;
					startPos.y = pTop;
					endPos.x = io.DisplaySize.x / 2.0f;
					endPos.y = io.DisplaySize.y / 2.0f;
				}
				if (Cheats::Players::DrawLine::SelectedType == 2) {
					startPos.x = pBase.x;
					startPos.y = pBottom;
					endPos.x = io.DisplaySize.x / 2.0f;
					endPos.y = io.DisplaySize.y;
				}
				DrawLineOutline(startPos, endPos, color, 1.f);
			}

			if (Cheats::Players::DrawId::Enabled) {
				ImColor color = Cheats::Players::DrawId::Color;

				std::string pedId = std::to_string(pedID);
				if (!pedId.empty()) {
					ImVec2 idPos;
					idPos.x = pBase.x;
					idPos.y = pTop - 27.0f;
					std::string displayText = pedId;
					DrawOutlinedText(drawList, pFont, displayText, idPos + ImVec2(1, 1), scaledSize, ImColor(0, 0, 0, 150), true);
					DrawOutlinedText(drawList, pFont, displayText, idPos, scaledSize, color, true);
				}
			}

			if (Cheats::Players::DrawName::Enabled) {
				ImColor color = Cheats::Players::DrawName::Color;

				std::string pedName;
				if (!ped.IsPlayer()) {
					pedName = "NPC";
				}
				else {
					auto cacheIt = pedNameCache.find(pedID);
					if (cacheIt != pedNameCache.end() && !cacheIt->second.empty() && cacheIt->second != "Player") {
						pedName = cacheIt->second;
					}
					else {
						pedName = GetPedName(ped);
						if (pedName.empty() || pedName == "Player" || pedName == "0") {
							bool found = false;
							for (size_t i = 0; i < playerIDs.size() && i < playerNames.size(); i++) {
								if (playerIDs[i] == pedID) {
									pedName = playerNames[i];
									if (pedName.empty() || pedName == "Player" || pedName == "0") {
										pedName = "Player " + std::to_string(pedID);
									}
									pedNameCache[pedID] = pedName;
									found = true;
									break;
								}
							}
							if (!found) {
								pedName = "Player " + std::to_string(pedID);
								pedNameCache[pedID] = pedName;
							}
						}
						else {
							pedNameCache[pedID] = pedName;
						}
					}
				}

				if (!pedName.empty()) {
					ImVec2 idPos;
					idPos.x = pBase.x;
					idPos.y = pTop - 15.0f;
					DrawOutlinedText(drawList, pFont, pedName.c_str(), idPos + ImVec2(1, 1), scaledSize, ImColor(0, 0, 0, 150), true);
					DrawOutlinedText(drawList, pFont, pedName.c_str(), idPos, scaledSize, color, true);
				}
			}

			if (Cheats::Players::DrawHealth::Enabled) {
				float Health = ped.health;
				float MaxHealth = ReadMemory<float>(ped.pointer + Offsets.MaxHealth);
				if (MaxHealth <= 0.0f || MaxHealth > 1000.0f) {
					MaxHealth = 200.0f;
				}

				float healthRatio = Health / MaxHealth;
				if (healthRatio < 0.0f) healthRatio = 0.0f;
				if (healthRatio > 1.0f) healthRatio = 1.0f;

				ImColor healthColor;
				if (Health > MaxHealth * 0.66f) {
					healthColor = ImColor(0, 255, 0);
				} else if (Health > MaxHealth * 0.33f) {
					healthColor = ImColor(255, 255, 0);
				} else {
					healthColor = ImColor(255, 0, 0);
				}
				
				float barWidth, barHeight;
				ImVec2 barPos, barEnd;

				if (pDistance < 90.0f) {
					barHeight = 4.0f;
				} else {
					barHeight = pHeight * 0.05f;
					if (barHeight < 2.0f) barHeight = 2.0f;
					if (barHeight > 8.0f) barHeight = 8.0f;
				}

				switch (Cheats::Players::DrawHealth::SelectedPosition) {
					case 0: 
						barWidth = reducedWidth * 2.0f;
						barPos = ImVec2(pBase.x - reducedWidth, pTop - barHeight - 2);
						barEnd = ImVec2(pBase.x + reducedWidth, pTop - 2);
						break;
					case 1: 
						barWidth = reducedWidth * 2.0f;
						barPos = ImVec2(pBase.x - reducedWidth, pos.y + 2);
						barEnd = ImVec2(pBase.x + reducedWidth, pos.y + barHeight + 2);
						pos.y += barHeight + 2;
						break;
					case 2: 
						barWidth = 4.0f; 
						barHeight = pHeight; 
						barPos = ImVec2(pBase.x - reducedWidth - 4, pTop); 
						barEnd = ImVec2(pBase.x - reducedWidth, pBottom); 
						break;
					case 3: 
						barWidth = 4.0f; 
						barHeight = pHeight; 
						barPos = ImVec2(pBase.x + reducedWidth, pTop); 
						barEnd = ImVec2(pBase.x + reducedWidth + 4, pBottom); 
						break;
				}

				if (Cheats::Players::DrawHealth::SelectedPosition < 2) { 

					drawList->AddRectFilled(barPos, barEnd, ImColor(0, 0, 0, 150));

					float filledWidth = barWidth * healthRatio;
					if (healthRatio > 0.0f) {
						ImVec2 filledEnd = ImVec2(barPos.x + filledWidth, barPos.y + barHeight);
						if (Cheats::Players::DrawHealth::GradientEnabled) {
							ImU32 gradStart = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawHealth::GradientStart);
							ImU32 gradEnd = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawHealth::GradientEnd);
							drawList->AddRectFilledMultiColor(barPos, filledEnd, gradStart, gradEnd, gradEnd, gradStart);
						} else {
							drawList->AddRectFilled(barPos, filledEnd, healthColor);
						}
					}

					drawList->AddRect(barPos, barEnd, ImColor(0, 0, 0, 200), 0.0f, 0, 1.0f);
				} else { 

					drawList->AddRectFilled(barPos, barEnd, ImColor(0, 0, 0, 150));

					float filledHeight = barHeight * healthRatio;
					if (healthRatio > 0.0f) {
						ImVec2 filledStart = ImVec2(barPos.x, barPos.y + barHeight - filledHeight);
						if (Cheats::Players::DrawHealth::GradientEnabled) {
							ImU32 gradStart = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawHealth::GradientStart);
							ImU32 gradEnd = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawHealth::GradientEnd);
							drawList->AddRectFilledMultiColor(filledStart, barEnd, gradStart, gradStart, gradEnd, gradEnd);
						} else {
							drawList->AddRectFilled(filledStart, barEnd, healthColor);
						}
					}

					drawList->AddRect(barPos, barEnd, ImColor(0, 0, 0, 200), 0.0f, 0, 1.0f);
				}
			}

			if (Cheats::Players::DrawArmor::Enabled) {
				float Armor = ped.armor;
				const float MaxArmor = 100.0f;

				float armorRatio = Armor / MaxArmor;
				if (armorRatio < 0.0f) armorRatio = 0.0f;
				if (armorRatio > 1.0f) armorRatio = 1.0f;
				
				float barWidth, barHeight;
				ImVec2 barPos, barEnd;

				if (pDistance < 90.0f) {
					barHeight = 4.0f;
				} else {
					barHeight = pHeight * 0.05f;
					if (barHeight < 2.0f) barHeight = 2.0f;
					if (barHeight > 8.0f) barHeight = 8.0f;
				}

				float topOffset = 2.0f;
				if (Cheats::Players::DrawArmor::SelectedPosition == 0 && Cheats::Players::DrawHealth::Enabled && Cheats::Players::DrawHealth::SelectedPosition == 0) {
					topOffset = barHeight + 4.0f;
				}

				float bottomOffset = 2.0f;
				if (Cheats::Players::DrawArmor::SelectedPosition == 1 && Cheats::Players::DrawHealth::Enabled && Cheats::Players::DrawHealth::SelectedPosition == 1) {
					bottomOffset = barHeight + 4.0f;
				}

				float leftOffset = 4.0f;
				if (Cheats::Players::DrawArmor::SelectedPosition == 2 && Cheats::Players::DrawHealth::Enabled && Cheats::Players::DrawHealth::SelectedPosition == 2) {
					leftOffset = 8.0f;
				}

				float rightOffset = 4.0f;
				if (Cheats::Players::DrawArmor::SelectedPosition == 3 && Cheats::Players::DrawHealth::Enabled && Cheats::Players::DrawHealth::SelectedPosition == 3) {
					rightOffset = 8.0f;
				}

				switch (Cheats::Players::DrawArmor::SelectedPosition) {
					case 0: 
						barWidth = reducedWidth * 2.0f;
						barPos = ImVec2(pBase.x - reducedWidth, pTop - barHeight - topOffset);
						barEnd = ImVec2(pBase.x + reducedWidth, pTop - topOffset);
						break;
					case 1: 
						barWidth = reducedWidth * 2.0f;
						barPos = ImVec2(pBase.x - reducedWidth, pos.y + bottomOffset);
						barEnd = ImVec2(pBase.x + reducedWidth, pos.y + barHeight + bottomOffset);
						pos.y += barHeight + 2;
						break;
					case 2: 
						barWidth = 4.0f; 
						barHeight = pHeight; 
						barPos = ImVec2(pBase.x - reducedWidth - leftOffset, pTop); 
						barEnd = ImVec2(pBase.x - reducedWidth - leftOffset + 4, pBottom); 
						break;
					case 3: 
						barWidth = 4.0f; 
						barHeight = pHeight; 
						barPos = ImVec2(pBase.x + reducedWidth + rightOffset - 4, pTop); 
						barEnd = ImVec2(pBase.x + reducedWidth + rightOffset, pBottom); 
						break;
				}

				ImU32 armorColor = Cheats::Players::DrawArmor::Color;

				if (Cheats::Players::DrawArmor::SelectedPosition < 2) { 

					drawList->AddRectFilled(barPos, barEnd, ImColor(0, 0, 0, 150));

					if (armorRatio > 0.0f) {
						float filledWidth = barWidth * armorRatio;
						ImVec2 filledEnd = ImVec2(barPos.x + filledWidth, barPos.y + barHeight);
						if (Cheats::Players::DrawArmor::GradientEnabled) {
							ImU32 gradStart = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawArmor::GradientStart);
							ImU32 gradEnd = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawArmor::GradientEnd);
							drawList->AddRectFilledMultiColor(barPos, filledEnd, gradStart, gradEnd, gradEnd, gradStart);
						} else {
							drawList->AddRectFilled(barPos, filledEnd, armorColor);
						}
					}

					drawList->AddRect(barPos, barEnd, ImColor(0, 0, 0, 200), 0.0f, 0, 1.0f);
				} else { 

					drawList->AddRectFilled(barPos, barEnd, ImColor(0, 0, 0, 150));

					if (armorRatio > 0.0f) {
						float filledHeight = barHeight * armorRatio;
						ImVec2 filledStart = ImVec2(barPos.x, barPos.y + barHeight - filledHeight);
						if (Cheats::Players::DrawArmor::GradientEnabled) {
							ImU32 gradStart = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawArmor::GradientStart);
							ImU32 gradEnd = ImGui::ColorConvertFloat4ToU32(Cheats::Players::DrawArmor::GradientEnd);
							drawList->AddRectFilledMultiColor(filledStart, barEnd, gradStart, gradStart, gradEnd, gradEnd);
						} else {
							drawList->AddRectFilled(filledStart, barEnd, armorColor);
						}
					}

					drawList->AddRect(barPos, barEnd, ImColor(0, 0, 0, 200), 0.0f, 0, 1.0f);
				}
			}

			if (Cheats::Players::DrawWeaponName::Enabled) {
				ImColor color = Cheats::Players::DrawWeaponName::Color;
				std::string weaponName = ped.GetWeaponName();
				if (!weaponName.empty())
				{
					ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(scaledSize, FLT_MAX, 0.0f, weaponName.c_str());
					DrawOutlinedText(drawList, pFont, weaponName.c_str(), pos + ImVec2(1, 1), scaledSize, ImColor(0, 0, 0, 150), true);
					DrawOutlinedText(drawList, pFont, weaponName.c_str(), pos, scaledSize, color, true);
					pos.y += textSize.y + 1.0f;
				}
			}

			if (Cheats::Players::DrawDistance::Enabled) {
				ImColor color = Cheats::Players::DrawDistance::Color;
				std::string dataText = "[" + std::to_string((int)pDistance) + "m]";

				ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(scaledSize, FLT_MAX, 0.0f, dataText.c_str());
				DrawOutlinedText(drawList, pFont, dataText, pos + ImVec2(1, 1), scaledSize, IM_COL32(0, 0, 0, 150), true);
				DrawOutlinedText(drawList, pFont, dataText, pos, scaledSize, color, true);
				pos.y += textSize.y + 1.0f;
			}

			if (Cheats::Players::GenderSpectator::Enabled) {
				uint32_t modelHash = ped.GetModelHash();
				ImColor genderColor = ImColor(255, 255, 255, 255);
				const char* genderLabel = nullptr;
				
				if (modelHash == 0x9C9EFFD8) {
					genderColor = Cheats::Players::GenderSpectator::FemaleColor;
					genderLabel = XorString("Female");
				}
				else if (modelHash == 0x705E61F2) {
					genderColor = Cheats::Players::GenderSpectator::MaleColor;
					genderLabel = XorString("Male");
				}
				
				if (genderLabel) {
					float genderX = pBase.x + reducedWidth + 5.0f;
					float genderY = pTop + (pHeight / 2.0f) - (scaledSize / 2.0f);
					ImVec2 genderPos = ImVec2(genderX, genderY);
					DrawOutlinedText(drawList, pFont, genderLabel, genderPos + ImVec2(1, 1), scaledSize, ImColor(0, 0, 0, 150), false);
					DrawOutlinedText(drawList, pFont, genderLabel, genderPos, scaledSize, genderColor, false);
				}
			}
		}

		if (Cheats::AimAssist::Silent::DrawIndicator && SilentTargetActive) {
			if (Cheats::AimAssist::Silent::IndicatorType == 0) {
			float indicatorSize = 4.0f; 
			
			if (SilentTargetDistance > 0.0f) {
				if (SilentTargetDistance > 100.0f) {
					indicatorSize = indicatorSize * (100.0f / SilentTargetDistance);
					if (indicatorSize < 3.0f) {
						indicatorSize = 3.0f; 
					}
				} else {
					float scaleFactor = 1.0f + ((100.0f - SilentTargetDistance) / 200.0f); 
					indicatorSize = indicatorSize * scaleFactor;
					if (indicatorSize > 10.0f) {
						indicatorSize = 10.0f; 
					}
				}
			}

			drawList->AddCircle(ImVec2(SilentTargetScreenPos.x, SilentTargetScreenPos.y), indicatorSize, Cheats::AimAssist::Silent::Color, 32, 2.0f);
			} else if (Cheats::AimAssist::Silent::IndicatorType == 1) {
				Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
				ImVec2 startPos = ImVec2(screenCenter.x, screenCenter.y);
				ImVec2 endPos = ImVec2(SilentTargetScreenPos.x, SilentTargetScreenPos.y);
				drawList->AddLine(startPos, endPos, Cheats::AimAssist::Silent::Color, 2.0f);
			}
		}

		if (Cheats::Exploit::PeakAssist && PeakPositionSaved && !(SavedPeakPosition.x == 0 && SavedPeakPosition.y == 0 && SavedPeakPosition.z == 0)) {
			Vector2 peakScreenPos;
			if (WorldToScreen(viewMatrix, SavedPeakPosition, peakScreenPos)) {
				ImVec2 iconPos = ImVec2(peakScreenPos.x, peakScreenPos.y);
				
				float outerSize = 14.0f;
				float innerSize = 6.0f;
				float lineLength = 8.0f;
				
				ImColor mainColor = ImColor(0, 255, 100, 255);
				ImColor outlineColor = ImColor(0, 0, 0, 200);
				
				drawList->AddLine(ImVec2(iconPos.x - lineLength, iconPos.y), ImVec2(iconPos.x - innerSize, iconPos.y), outlineColor, 4.0f);
				drawList->AddLine(ImVec2(iconPos.x + innerSize, iconPos.y), ImVec2(iconPos.x + lineLength, iconPos.y), outlineColor, 4.0f);
				drawList->AddLine(ImVec2(iconPos.x, iconPos.y - lineLength), ImVec2(iconPos.x, iconPos.y - innerSize), outlineColor, 4.0f);
				drawList->AddLine(ImVec2(iconPos.x, iconPos.y + innerSize), ImVec2(iconPos.x, iconPos.y + lineLength), outlineColor, 4.0f);

				drawList->AddLine(ImVec2(iconPos.x - lineLength, iconPos.y), ImVec2(iconPos.x - innerSize, iconPos.y), mainColor, 2.0f);
				drawList->AddLine(ImVec2(iconPos.x + innerSize, iconPos.y), ImVec2(iconPos.x + lineLength, iconPos.y), mainColor, 2.0f);
				drawList->AddLine(ImVec2(iconPos.x, iconPos.y - lineLength), ImVec2(iconPos.x, iconPos.y - innerSize), mainColor, 2.0f);
				drawList->AddLine(ImVec2(iconPos.x, iconPos.y + innerSize), ImVec2(iconPos.x, iconPos.y + lineLength), mainColor, 2.0f);
				
				drawList->AddCircle(iconPos, outerSize, outlineColor, 32, 3.0f);
				drawList->AddCircle(iconPos, outerSize, mainColor, 32, 1.5f);
				
				drawList->AddCircleFilled(iconPos, 3.0f, mainColor, 16);
			}
		}

		ImGui::PopFont();
	}

	void Vehicle() {
		if (Cheats::Vehicle::Enabled) {
			ImDrawList* drawList = ImGui::GetBackgroundDrawList();

			if (!LocalPlayer.Update()) {
				return;
			}

			uintptr_t replayInterface = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			uintptr_t vehicleInterface = ReadMemory<DWORD64>(replayInterface + 0x10);
			if (!vehicleInterface) {
				return;
			}

			uintptr_t vehicleList = ReadMemory<DWORD64>(vehicleInterface + 0x180);
			int vehicleListCount = ReadMemory<int>(vehicleInterface + 0x188);
			if (vehicleListCount <= 0 || vehicleListCount > 500) {
				return;
			}
			for (int i = 0; i < vehicleListCount; ++i) {
				uintptr_t vehicle = ReadMemory<uintptr_t>(vehicleList + (i * 0x10));
				if (!vehicle) {
					continue;
				}

				float VehicleHealthh = ReadMemory<float>(vehicle + Offsets.Health);
				float vehicleMaxHealth = 1000;

				uintptr_t viewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
				if (!viewPort) {
					continue;
				}

				Vector3 getCordLocal = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
				Matrix viewMatrix = ReadMemory<Matrix>(viewPort + 0x24C);
				Vector3 getCorrdinateVehicle = ReadMemory<Vector3>(vehicle + 0x90);
				Vector2 vehicleLocation;
				if (!WorldToScreen(viewMatrix, getCorrdinateVehicle, vehicleLocation)) {
					continue;
				}

				Vector3 distanceCalculation = getCordLocal - getCorrdinateVehicle;
				double Distance = sqrtf(distanceCalculation.x * distanceCalculation.x + distanceCalculation.y * distanceCalculation.y + distanceCalculation.z * distanceCalculation.z);

				if (Distance <= Cheats::Vehicle::Distance) {
					if ((int)Distance == 0 && !Cheats::Vehicle::DrawLocalVehicle) {
						continue;
					}

					if ((int)Distance >= 1 && !Cheats::Vehicle::DrawEnemyVehicle) {
						continue;
					}

					if (vehicleLocation.x == 0 || vehicleLocation.y == 0) {
						continue;
					}

					if (Cheats::Vehicle::VehicleHealth) {
						float healthPercentage = VehicleHealthh / vehicleMaxHealth;
						float barWidth = 50.0f;
						float barHeight = 5.0f;
						float filledWidth = barWidth * healthPercentage;
						ImColor healthColor = ImColor((int)(Cheats::Vehicle::HealthBarColor.Value.x * 255), (int)(Cheats::Vehicle::HealthBarColor.Value.y * 255), (int)(Cheats::Vehicle::HealthBarColor.Value.z * 255), (int)(Cheats::Vehicle::HealthBarColor.Value.w * 255));
						ImColor backgroundColor = ImColor(0, 0, 0, 255);
						ImVec2 barPosition(vehicleLocation.x - barWidth / 2, vehicleLocation.y - 20);
						ImVec2 barFilled(barPosition.x + filledWidth, barPosition.y + barHeight);
						drawList->AddRectFilled(barPosition, ImVec2(barPosition.x + barWidth, barPosition.y + barHeight), backgroundColor);
						drawList->AddRectFilled(barPosition, barFilled, healthColor);
					}

					if (Cheats::Vehicle::VehicleEspShowDistance) {
						ImFont* pFonts = Cheats::MenuUtils::GetSelectedFont();
						ImGui::PushFont(pFonts);
						std::string vehicleDistanceText = std::to_string((int)Distance) + XorString(" m");
						DrawOutlinedText(drawList, pFonts, vehicleDistanceText, ImVec2(vehicleLocation.x, vehicleLocation.y + 20), 14.f, Cheats::Vehicle::DistanceColor, true);
						ImGui::PopFont();
					}

					if (Cheats::Vehicle::VehicleEspSnapline) {
						drawList->AddLine(ImVec2(GetSystemMetrics(SM_CXSCREEN) / 2, 0), ImVec2(vehicleLocation.x, vehicleLocation.y), Cheats::Vehicle::SnaplineColor, 1.f);
					}

					if (Cheats::Vehicle::VehicleMarker) {
						drawList->AddCircle(ImVec2(vehicleLocation.x, vehicleLocation.y + 45), 4, Cheats::Vehicle::MarkerOuterColor, 100, 3.0f);
						drawList->AddCircle(ImVec2(vehicleLocation.x, vehicleLocation.y + 45), 2, Cheats::Vehicle::MarkerInnerColor, 100, 3.0f);
					}
				}
			}
		}
	}
}

static std::vector<uint8_t> originalBypassBytes;
static bool bypassBytesSaved = false;

bool ApplyMemoryBypass() {
	uintptr_t address = Offsets.GameBase + 0x0D8A404;
	
	if (!bypassBytesSaved) {
		originalBypassBytes.resize(64);
		SIZE_T bytesRead = 0;
		if (!ReadProcessMemory(Game.hProcess, (LPCVOID)address, &originalBypassBytes[0], 64, &bytesRead)) {
			return false;
		}
		bypassBytesSaved = true;
	}
	
	uint8_t nopPatch[] = {
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
		0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	};
	
	DWORD oldProtect;
	if (!VirtualProtectEx(Game.hProcess, (LPVOID)address, sizeof(nopPatch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}
	
	SIZE_T bytesWritten;
	if (!WriteProcessMemory(Game.hProcess, (LPVOID)address, nopPatch, sizeof(nopPatch), &bytesWritten)) {
		VirtualProtectEx(Game.hProcess, (LPVOID)address, sizeof(nopPatch), oldProtect, NULL);
		return false;
	}
	
	VirtualProtectEx(Game.hProcess, (LPVOID)address, sizeof(nopPatch), oldProtect, NULL);
	return true;
}

bool RestoreMemoryBypass() {
	if (!bypassBytesSaved || originalBypassBytes.empty()) {
		return false;
	}
	
	uintptr_t address = Offsets.GameBase + 0x0D8A404;
	
	DWORD oldProtect;
	if (!VirtualProtectEx(Game.hProcess, (LPVOID)address, originalBypassBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}
	
	SIZE_T bytesWritten;
	if (!WriteProcessMemory(Game.hProcess, (LPVOID)address, &originalBypassBytes[0], originalBypassBytes.size(), &bytesWritten)) {
		VirtualProtectEx(Game.hProcess, (LPVOID)address, originalBypassBytes.size(), oldProtect, NULL);
		return false;
	}
	
	VirtualProtectEx(Game.hProcess, (LPVOID)address, originalBypassBytes.size(), oldProtect, NULL);
	return true;
}

void RestoreSilent() {
	std::vector<uint8_t> ReWriteTable =
	{
		0xF3, 0x41, 0x0F, 0x10, 0x19,
		0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04,
		0xF3, 0x41, 0x0F, 0x10, 0x51, 0x08
	};

	WriteBytes(Offsets.GameBase + Offsets.Silent, &ReWriteTable[0], ReWriteTable.size());
	
	std::vector<uint8_t> AngleReWriteTable =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	WriteBytes(Offsets.GameBase + 0x34E, &AngleReWriteTable[0], AngleReWriteTable.size());
	
	RestoreMemoryBypass();
}

void ApplySilent() {
	ApplyMemoryBypass();
	
	static uint64_t HandleBulletAddress = Offsets.GameBase + Offsets.Silent;
	static uint64_t AllocPtr = Offsets.GameBase + 0x34E;

	auto CalculateRelativeOffset = [](uint64_t CurrentAddress, uint64_t TargetAddress, int Offset = 5) {
		intptr_t RelativeOffset = static_cast<intptr_t>(TargetAddress - (CurrentAddress + Offset));
		return static_cast<uint32_t>(RelativeOffset);
		};

	union
	{
		float f;
		uint32_t i;
	} EndPosX, EndPosY, EndPosZ;

	EndPosX.f = EndBulletPos.x;
	EndPosY.f = EndBulletPos.y;
	EndPosZ.f = EndBulletPos.z;

	{
		std::vector<uint8_t> ReWriteTable =
		{
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint32_t JmpOffset = CalculateRelativeOffset(HandleBulletAddress, AllocPtr);
		ReWriteTable[1] = static_cast<uint8_t>(JmpOffset & 0xFF);
		ReWriteTable[2] = static_cast<uint8_t>((JmpOffset >> 8) & 0xFF);
		ReWriteTable[3] = static_cast<uint8_t>((JmpOffset >> 16) & 0xFF);
		ReWriteTable[4] = static_cast<uint8_t>((JmpOffset >> 24) & 0xFF);
		WriteBytes(HandleBulletAddress, &ReWriteTable[0], ReWriteTable.size());
	}

	{
		uintptr_t currentAddress = (uintptr_t)AllocPtr;
		uintptr_t targetAddress = (uintptr_t)(HandleBulletAddress);
		intptr_t relativeOffset = static_cast<intptr_t>(targetAddress - (currentAddress + 28));
		uint32_t jmpOffset = static_cast<uint32_t>(relativeOffset);

		std::vector<uint8_t> ReWriteTable =
		{
			0x41, 0xC7, 0x01, static_cast<uint8_t>(EndPosX.i), static_cast<uint8_t>(EndPosX.i >> 8), static_cast<uint8_t>(EndPosX.i >> 16), static_cast<uint8_t>(EndPosX.i >> 24),
			0x41, 0xC7, 0x41, 0x04, static_cast<uint8_t>(EndPosY.i), static_cast<uint8_t>(EndPosY.i >> 8), static_cast<uint8_t>(EndPosY.i >> 16), static_cast<uint8_t>(EndPosY.i >> 24),
			0x41, 0xC7, 0x41, 0x08, static_cast<uint8_t>(EndPosZ.i), static_cast<uint8_t>(EndPosZ.i >> 8), static_cast<uint8_t>(EndPosZ.i >> 16), static_cast<uint8_t>(EndPosZ.i >> 24),
			0xF3, 0x41, 0x0F, 0x10, 0x19,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		ReWriteTable[29] = static_cast<uint8_t>(jmpOffset & 0xFF);
		ReWriteTable[30] = static_cast<uint8_t>((jmpOffset >> 8) & 0xFF);
		ReWriteTable[31] = static_cast<uint8_t>((jmpOffset >> 16) & 0xFF);
		ReWriteTable[32] = static_cast<uint8_t>((jmpOffset >> 24) & 0xFF);
		WriteBytes(AllocPtr, &ReWriteTable[0], ReWriteTable.size());
	}
}

Ped FindBestTarget(int aimFov) {
	Ped bestTarget;
	float minFov = 9999.f;
	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
	
	if (!Game.ViewPort)
		Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
	if (!Game.ViewPort)
		return Ped{};
	Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
	
	for (auto& ped : PedList) {
		if (!LocalPlayer.Update())
			break;

		if (!ped.Update())
			continue;

		float pDistance = GetDistance(ped.position, LocalPlayer.position);
		if (pDistance <= 0.f || pDistance >= Cheats::AimAssist::Aimbot::Distance)
			continue;

		if (Cheats::AimAssist::OnlyVisible && !ped.IsVisible())
			continue;

		if (Cheats::AimAssist::IgnorePed && !ped.IsPlayer())
			continue;

		if (Cheats::AimAssist::IgnoreDeath && ped.IsDead())
			continue;

		int pedID = ped.GetId();
		if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID])
			continue;

		Vector2 screenPosition;
		Vector3 targetBonePos = Vector3(0, 0, 0);
		float fov = 9999.f;
		
		if (Cheats::AimAssist::Aimbot::ClosestBone) {
			targetBonePos = FindClosestBoneInFOVForAimbot(ped, (float)aimFov, viewMatrix);
			if (Vec3Empty(targetBonePos))
			continue;
			if (!WorldToScreen(viewMatrix, targetBonePos, screenPosition))
				continue;
			fov = abs((screenCenter - screenPosition).Length());
		} else {
		int boneIndex = Cheats::AimAssist::Aimbot::SelectedType;
		if (boneIndex >= 0 && boneIndex < 35) {
			if (Vec3Empty(ped.boneList[boneIndex]))
				continue;
			if (!WorldToScreen(viewMatrix, ped.boneList[boneIndex], screenPosition))
				continue;
		} else {
			if (Vec3Empty(ped.boneList[Head]))
				continue;
			if (!WorldToScreen(viewMatrix, ped.boneList[Head], screenPosition))
				continue;
		}
			fov = abs((screenCenter - screenPosition).Length());
		}
		
		if (fov < aimFov) {
			if (fov < minFov) {
				bestTarget = ped;
				minFov = fov;
				if (Cheats::AimAssist::Aimbot::ClosestBone) {
					bestTarget.boneList[35] = targetBonePos;
				}
			}
		}
	}
	if (minFov == 9999.f)
		return Ped{};
	return bestTarget;
}

Ped FindBestTargetForSilent(int aimFov) {
	Ped bestTarget;
	float minFov = 9999.f;
	Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
	
	if (!Game.ViewPort)
		Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
	if (!Game.ViewPort)
		return Ped{};
	Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
	
	for (auto& ped : PedList) {
		if (!LocalPlayer.Update())
			break;

		if (!ped.Update())
			continue;

		float pDistance = GetDistance(ped.position, LocalPlayer.position);
		if (pDistance <= 0.f || pDistance >= Cheats::AimAssist::Aimbot::Distance)
			continue;

		if (Cheats::AimAssist::OnlyVisible && !ped.IsVisible())
			continue;

		if (Cheats::AimAssist::IgnorePed && !ped.IsPlayer())
			continue;

		if (Cheats::AimAssist::IgnoreDeath && ped.IsDead())
			continue;

		int pedID = ped.GetId();
		if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID])
			continue;

		// FOV içindeki en yakın kemiği bul (adjustedFov kullanmadan, tam FOV ile)
		Vector3 bestBonePos = Vector3(0, 0, 0);
		float bestScreenDistance = FLT_MAX;
		bool foundValidBone = false;
		
		bool headShotEnabled = Cheats::AimAssist::Silent::HeadShot;
		
		for (const auto& boneInfo : targetBones) {
			if (boneInfo.boneId < 0 || boneInfo.boneId >= 35) continue;
			
			if (!headShotEnabled && boneInfo.boneId == Head) {
				continue;
			}
			
			Vector3 bonePos = ped.boneList[boneInfo.boneId];
			if (Vec3Empty(bonePos))
				continue;
			
			Vector2 screenPos;
			if (!WorldToScreen(viewMatrix, bonePos, screenPos))
				continue;
			
			float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));
			
			// Tam FOV kontrolü (adjustedFov kullanmadan)
			if (distanceFromCenter > aimFov)
				continue;
			
			if (!foundValidBone || distanceFromCenter < bestScreenDistance) {
				bestBonePos = bonePos;
				bestScreenDistance = distanceFromCenter;
				foundValidBone = true;
			}
		}
		
		if (!foundValidBone)
			continue;
		
		// Ekran merkezine olan mesafeyi hesapla
		float fov = bestScreenDistance;
		if (fov < minFov) {
			bestTarget = ped;
			minFov = fov;
		}
	}
	
	if (minFov == 9999.f)
		return Ped{};
	return bestTarget;
}

void SetAim() {
	while (!exitLoop) {
		CurrentAimLockId = -1;
		SilentTargetActive = false;
		if (Cheats::AimAssist::Aimbot::Enabled) {
			Ped target = FindBestTarget(Cheats::AimAssist::Aimbot::Fov);
			if (Vec3Empty(target.position))
				goto silent_part;
			if (GetAsyncKeyState(Cheats::AimAssist::Aimbot::Key) & 0x8000) {
				CurrentAimLockId = target.GetId();
					uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
					if (!camera)
						goto silent_part;
					Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);
					Vector3 cameraPosition = ReadMemory<Vector3>(camera + 0x60);
					Vector3 angle;
				Vector3 targetBonePos = Vector3(0, 0, 0);

				if (Cheats::AimAssist::Aimbot::ClosestBone) {
					if (!Game.ViewPort)
						Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
					if (Game.ViewPort) {
						Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
						targetBonePos = FindClosestBoneInFOVForAimbot(target, (float)Cheats::AimAssist::Aimbot::Fov, viewMatrix);
						if (Vec3Empty(targetBonePos)) {
							goto silent_part;
						}
						angle = CalcAngle(cameraPosition, targetBonePos);
					} else {
						goto silent_part;
					}
				} else {
					int boneIndex = Cheats::AimAssist::Aimbot::SelectedType;
					if (boneIndex >= 0 && boneIndex < 35) {
						if (Vec3Empty(target.boneList[boneIndex])) {
							goto silent_part;
						}
						angle = CalcAngle(cameraPosition, target.boneList[boneIndex]);
					} else {
						if (Vec3Empty(target.boneList[Head])) {
							goto silent_part;
						}
						angle = CalcAngle(cameraPosition, target.boneList[Head]);
					}
					}
					NormalizeAngles(angle);
					Vector3 delta = angle - viewAngle;
					NormalizeAngles(delta);

					if (Cheats::AimAssist::Aimbot::RageMode) {
						Vector3 writeAngle = viewAngle + delta;

						static float shakeTime = 0.0f;
						shakeTime += 0.1f;
						float shakeIntensity = 0.02f;
						Vector3 shake;
						shake.x = sin(shakeTime * 15.0f) * shakeIntensity;
						shake.y = cos(shakeTime * 12.0f) * shakeIntensity;
						shake.z = sin(shakeTime * 18.0f) * shakeIntensity * 0.5f;
						
						writeAngle = writeAngle + shake;
						NormalizeAngles(writeAngle);
						
						if (!Vec3Empty(writeAngle)) {
							WriteMemory<Vector3>(camera + 0x3D0, writeAngle);
						}
					} else {
						Vector3 writeAngle = viewAngle + (Cheats::AimAssist::Aimbot::Smooth ? delta / Cheats::AimAssist::Aimbot::Smooth : delta);
						NormalizeAngles(writeAngle);
						if (!Vec3Empty(writeAngle)) {
							WriteMemory<Vector3>(camera + 0x3D0, writeAngle);
					}
				}
			}
		}

silent_part:
		if (Cheats::AimAssist::Silent::Enabled) {

			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_int_distribution<> dis(1, 100);

			static bool SilentAplied = false;
			bool CanSilent = false;

			if (Cheats::AimAssist::Silent::WheelBullet && WheelBulletActive) {
				uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
				if (!camera) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f;
					Sleep(loopDelay);
					continue;
				}
				if (!Game.ViewPort)
					Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
				if (!Game.ViewPort) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f;
					Sleep(loopDelay);
					continue;
				}
				Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
				
				Vector3 wheelPos = FindClosestWheelInFOV((float)Cheats::AimAssist::Silent::Fov, viewMatrix);
				if (Vec3Empty(wheelPos)) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f;
					Sleep(loopDelay);
					continue;
				}
				
				if (!(GetAsyncKeyState(Cheats::AimAssist::Silent::Key) & 0x8000) || !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f;
					Sleep(loopDelay);
					continue;
				}
				
				if (Cheats::AimAssist::Silent::MissChance > 0) {
					int randomValue = dis(gen);
					if (randomValue <= Cheats::AimAssist::Silent::MissChance) {
						if (SilentAplied) {
							RestoreSilent();
							SilentAplied = false;
						}
						SilentTargetActive = false;
						SilentTargetDistance = 0.f;
						Sleep(loopDelay);
						continue;
					}
					CanSilent = true;
				}
				else {
					CanSilent = true;
				}
				
				if (CanSilent) {
					Vector3 SlientType = wheelPos;
					EndBulletPos = SlientType;
					Vector2 silentScreen;
					if (WorldToScreen(viewMatrix, SlientType, silentScreen)) {
						SilentTargetScreenPos = silentScreen;
						SilentTargetDistance = 0.f;
						SilentTargetActive = true;
					}
					
					if (Cheats::AimAssist::Silent::Pslient) {
						HANDLE h1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
						if (h1) { WaitForSingleObject(h1, 100); CloseHandle(h1); }
						
						HANDLE h2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
						if (h2) { WaitForSingleObject(h2, 100); CloseHandle(h2); }
						
						HANDLE h3 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
						if (h3) { WaitForSingleObject(h3, 100); CloseHandle(h3); }
					}
					else {
						HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
						if (h) { WaitForSingleObject(h, 50); CloseHandle(h); }
					}
					SilentAplied = true;
				}
				Sleep(loopDelay);
				continue;
			}

			Ped target = FindBestTargetForSilent(Cheats::AimAssist::Silent::Fov);
			if (Vec3Empty(target.position)) {
				if (SilentAplied) {
					RestoreSilent();
					SilentAplied = false;
				}
				SilentTargetActive = false;
				SilentTargetDistance = 0.f;
				Sleep(loopDelay);
				continue;
			}
			Vector3 SlientType;
			uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
			if (!camera) {
				if (SilentAplied) {
					RestoreSilent();
					SilentAplied = false;
				}
				SilentTargetActive = false;
				SilentTargetDistance = 0.f;
				Sleep(loopDelay);
				continue;
			}
			Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);
			Vector3 cameraPosition = ReadMemory<Vector3>(camera + 0x60);
			if (!Game.ViewPort)
				Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			if (!Game.ViewPort) {
				if (SilentAplied) {
					RestoreSilent();
					SilentAplied = false;
				}
				SilentTargetActive = false;
				SilentTargetDistance = 0.f;
				Sleep(loopDelay);
				continue;
			}
			Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);

			// FOV içindeki en yakın kemiği bul (target bulunurken zaten kontrol edildi)
			Vector3 closestBoneInFOV = Vector3(0, 0, 0);
			bool foundBoneInFOV = false;
			
			if (Cheats::AimAssist::Silent::ClosestBone) {
				// ClosestBone aktifse, FOV içindeki en yakın kemiği bul
				closestBoneInFOV = FindClosestBoneInFOV(target, (float)Cheats::AimAssist::Silent::Fov, viewMatrix);
				foundBoneInFOV = !Vec3Empty(closestBoneInFOV);
			} else if (Cheats::AimAssist::Silent::RandomTarget) {
				// RandomTarget aktifse, FOV içindeki rastgele bir kemiği bul
				closestBoneInFOV = FindRandomBoneInFOV(target, (float)Cheats::AimAssist::Silent::Fov, viewMatrix);
				foundBoneInFOV = !Vec3Empty(closestBoneInFOV);
			} else {
				// ClosestBone ve RandomTarget aktif değilse, seçili kemiğin FOV içinde olup olmadığını kontrol et
				Vector2 screenCenter = Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
				Vector3 boneToCheck = Vector3(0, 0, 0);
				
				int boneIndex = Cheats::AimAssist::Silent::SelectedType;
				if (boneIndex >= 0 && boneIndex < 35) {
					boneToCheck = target.boneList[boneIndex];
				} else {
					boneToCheck = target.boneList[Head];
				}
				
				if (!Vec3Empty(boneToCheck)) {
					Vector2 screenPos;
					if (WorldToScreen(viewMatrix, boneToCheck, screenPos)) {
						float distanceFromCenter = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));
						// Sadece seçili kemik FOV içindeyse kullan
						if (distanceFromCenter <= Cheats::AimAssist::Silent::Fov) {
							closestBoneInFOV = boneToCheck;
							foundBoneInFOV = true;
						}
					}
				}
			}
			
			if (!foundBoneInFOV) {
				if (SilentAplied) {
					RestoreSilent();
					SilentAplied = false;
				}
				SilentTargetActive = false;
				SilentTargetDistance = 0.f; 
				Sleep(loopDelay);
				continue;
			}

			{
				int pedID = target.GetId();
				if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f; 
				}
				else if (!(GetAsyncKeyState(Cheats::AimAssist::Silent::Key) & 0x8000)) {
					if (SilentAplied) {
						RestoreSilent();
						SilentAplied = false;
					}
					SilentTargetActive = false;
					SilentTargetDistance = 0.f; 
				}
				else {

					if (Cheats::AimAssist::Silent::MissChance > 0) {
						int randomValue = dis(gen);
						if (randomValue <= Cheats::AimAssist::Silent::MissChance) {

							if (SilentAplied) {
								RestoreSilent();
								SilentAplied = false;
							}
							SilentTargetActive = false;
							SilentTargetDistance = 0.f; 
						}
						else {
							CanSilent = true;
						}
					}
					else {
						CanSilent = true;
					}

					if (CanSilent) {
						// FOV içindeki en yakın kemiği kullan (zaten bulundu)
						CurrentAimLockId = pedID;
						SlientType = closestBoneInFOV;
						
						// ClosestBone aktifse zaten closestBoneInFOV kullanılıyor
						// RandomTarget veya SelectedType için de closestBoneInFOV kullanılıyor (FOV içindeki en yakın kemik)

						if (Vec3Empty(SlientType)) {
							if (SilentAplied) {
								RestoreSilent();
								SilentAplied = false;
							}
							SilentTargetActive = false;
							SilentTargetDistance = 0.f; 
						}
						else {

							EndBulletPos = SlientType;
							Vector2 silentScreen;
							if (WorldToScreen(viewMatrix, SlientType, silentScreen)) {
								SilentTargetScreenPos = silentScreen;

								SilentTargetDistance = GetDistance(target.position, LocalPlayer.position);
								SilentTargetActive = true;
							}

							if (Cheats::AimAssist::Silent::Pslient) {
								HANDLE h1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
								if (h1) { WaitForSingleObject(h1, 100); CloseHandle(h1); }
								
								HANDLE h2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
								if (h2) { WaitForSingleObject(h2, 100); CloseHandle(h2); }
								
								HANDLE h3 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
								if (h3) { WaitForSingleObject(h3, 100); CloseHandle(h3); }
							}
							else {
								HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
								if (h) { WaitForSingleObject(h, 100); CloseHandle(h); }
							}
							SilentAplied = true;
						}
					}
				}
			}
		}
		else {
			SilentTargetActive = false;
			SilentTargetDistance = 0.f; 
		}
		Sleep(loopDelay);
	}
}


void TeleportObject(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3 Position, Vector3 VisualPosition, bool Stop) {
	float BackupMagic = 0.f;
	if (Stop) {
		BackupMagic = ReadMemory<float>(ModelInfo + 0x2C);
		WriteMemory(ModelInfo + 0x2C, 0.f);
	}

	WriteMemory(Object + 0x90, VisualPosition);
	WriteMemory(Navigation + 0x50, Position);
	if (Stop) {
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
		WriteMemory(ModelInfo + 0x2C, BackupMagic);
	}
}

Vector3 GetPositionByID(int targetID) {
	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (ped.GetId() == targetID) {
			return ped.position;
		}
	}
	return Vector3(0, 0, 0); 
}

void AddVehicleList() {
	while (true) {
		if (!LocalPlayer.Update()) return;

		std::vector<int> newVehicleIDs;
		std::vector<std::string> newVehicleNames;
		std::vector<std::string> newVehicleModels;
		std::vector<float> newVehicleDistances;
		std::vector<uintptr_t> newVehiclePointers;
		
		uintptr_t replayInterface = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
		uintptr_t vehicleInterface = ReadMemory<DWORD64>(replayInterface + 0x10);
		if (!vehicleInterface) {
			vehicleIDs.clear();
			vehicleNames.clear();
			vehicleModels.clear();
			vehicleDistances.clear();
			vehiclePointers.clear();
			Sleep(1000);
			continue;
		}
		
		uintptr_t vehicleList = ReadMemory<DWORD64>(vehicleInterface + 0x180);
		if (!vehicleList) {
			vehicleIDs.clear();
			vehicleNames.clear();
			vehicleModels.clear();
			vehicleDistances.clear();
			vehiclePointers.clear();
			Sleep(1000);
			continue;
		}
		
		int vehicleListCount = ReadMemory<int>(vehicleInterface + 0x188);
		if (vehicleListCount > 300 || vehicleListCount <= 0) {
			vehicleIDs.clear();
			vehicleNames.clear();
			vehicleModels.clear();
			vehicleDistances.clear();
			vehiclePointers.clear();
			Sleep(1000);
			continue;
		}
		
		Vector3 localPos = LocalPlayer.position;
		
		for (int i = 0; i < vehicleListCount; ++i) {
			uintptr_t vehicle = ReadMemory<uintptr_t>(vehicleList + (i * 0x10));
			if (!vehicle) continue;
			
			Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
			if (Vec3Empty(vehiclePos)) continue;
			
			float distance = GetDistance(vehiclePos, localPos);
			if (distance > (float)Cheats::Vehicle::Distance) continue;
			
			DWORD vehicleModelHash = ReadMemory<DWORD>(vehicle + 0x18);
			if (vehicleModelHash == 0 || vehicleModelHash == 0xFFFFFFFF) continue;
			
			newVehiclePointers.push_back(vehicle);
			newVehicleIDs.push_back(i);
			newVehicleDistances.push_back(distance);
			
			char modelHashStr[64];
			sprintf_s(modelHashStr, sizeof(modelHashStr), "0x%X", vehicleModelHash);
			newVehicleModels.push_back(modelHashStr);

			std::string vehicleNameStr = "Vehicle";
			uintptr_t modelInfo = ReadMemory<uintptr_t>(vehicle + 0x20);
			if (modelInfo && modelInfo != 0xFFFFFFFFFFFFFFFF) {
				std::string modelName = ReadString(modelInfo + 0x298);
				if (!modelName.empty() && modelName.length() <= 24) {
					vehicleNameStr = modelName;
				}
				else {

					modelName = ReadString(modelInfo + 0x2A0);
					if (!modelName.empty() && modelName.length() <= 24) {
						vehicleNameStr = modelName;
					}
					else {
						modelName = ReadString(modelInfo + 0x290);
						if (!modelName.empty() && modelName.length() <= 24) {
							vehicleNameStr = modelName;
						}
					}
				}
			}

			if (vehicleNameStr == "Vehicle" || vehicleNameStr.empty()) {
				char hashStr[64];
				sprintf_s(hashStr, sizeof(hashStr), "0x%X", vehicleModelHash);
				vehicleNameStr = hashStr;
			}
			
			char vehicleName[128];
			sprintf_s(vehicleName, sizeof(vehicleName), "%s [%.1fm]", vehicleNameStr.c_str(), distance);
			newVehicleNames.push_back(vehicleName);
		}
		
		int lastSelectedID = (selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehicleIDs.size()) ? vehicleIDs[selectedItemVehicle] : -1;
		vehicleIDs = std::move(newVehicleIDs);
		vehicleNames = std::move(newVehicleNames);
		vehicleModels = std::move(newVehicleModels);
		vehicleDistances = std::move(newVehicleDistances);
		vehiclePointers = std::move(newVehiclePointers);
		
		if (lastSelectedID != -1) {
			auto it = std::find(vehicleIDs.begin(), vehicleIDs.end(), lastSelectedID);
			selectedItemVehicle = (it != vehicleIDs.end()) ? std::distance(vehicleIDs.begin(), it) : 0;
		}
		else {
			selectedItemVehicle = -1;
		}
		
		if (VehicleTeleport && selectedItemVehicle >= 0 && selectedItemVehicle < (int)vehiclePointers.size()) {
			uintptr_t vehicle = vehiclePointers[selectedItemVehicle];
			if (vehicle) {
				Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
				if (!Vec3Empty(vehiclePos)) {
					uint64_t Object = (uint64_t)LocalPlayer.pointer;
					if (Object) {
						uint64_t Navigation = ReadMemory<uint64_t>(Object + 0x30);
						uint64_t ModelInfo = ReadMemory<uint64_t>(Object + 0x20);
						if (Navigation && ModelInfo) {
							TeleportObject(Object, Navigation, ModelInfo, vehiclePos, vehiclePos, true);
							AddNotification("Teleported to vehicle!", true);
						}
					}
				}
			}
			VehicleTeleport = false;
		}
		
		Sleep(500);
	}
}

bool ApplyVehicleDoorLock(uintptr_t vehicle, CarLockState state) {
	if (!vehicle || Offsets.VehicleDoorsLock == 0) {
		return false;
	}

	int lockValue = static_cast<int>(state);
	WriteMemory<int>(vehicle + Offsets.VehicleDoorsLock, lockValue);

	uintptr_t vehicleNetObject = ReadMemory<uintptr_t>(vehicle + 0xD0);
	if (vehicleNetObject && vehicleNetObject != 0xFFFFFFFFFFFFFFFF) {
		WriteMemory<int>(vehicleNetObject + Offsets.VehicleDoorsLock, lockValue);
	}

	return true;
}

void SetSelectedVehicleLock(CarLockState state, const std::string& actionLabel) {
	if (Offsets.VehicleDoorsLock == 0) {
		AddNotification("Vehicle door lock offset not found!", false);
		return;
	}

	if (selectedItemVehicle < 0 || selectedItemVehicle >= (int)vehiclePointers.size()) {
		AddNotification("No vehicle selected!", false);
		return;
	}

	uintptr_t vehicle = vehiclePointers[selectedItemVehicle];
	if (!vehicle) {
		AddNotification("Vehicle pointer is invalid!", false);
		return;
	}

	ApplyVehicleDoorLock(vehicle, state);
	AddNotification(actionLabel, state == CARLOCK_UNLOCKED);
}

void UnlockSelectedVehicle() {
	SetSelectedVehicleLock(CARLOCK_UNLOCKED, "Car unlocked");
}

void LockSelectedVehicle() {
	SetSelectedVehicleLock(CARLOCK_LOCKED, "Car locked");
}

void StealSelectedVehicle() {
	if (Offsets.VehicleDoorsLock == 0 || Offsets.VehicleDriver == 0) {
		AddNotification("Vehicle offsets missing!", false);
		return;
	}

	if (!LocalPlayer.Update()) {
		AddNotification("Local player not found!", false);
		return;
	}

	if (selectedItemVehicle < 0 || selectedItemVehicle >= (int)vehiclePointers.size()) {
		AddNotification("No vehicle selected!", false);
		return;
	}

	uintptr_t vehicle = vehiclePointers[selectedItemVehicle];
	if (!vehicle) {
		AddNotification("Vehicle pointer is invalid!", false);
		return;
	}

	uintptr_t localPed = LocalPlayer.pointer;
	uintptr_t currentVehicle = ReadMemory<uintptr_t>(localPed + Offsets.Vehicle);

	// Require being in the selected car; we only grab keys, no teleport/seat forcing.
	if (currentVehicle != vehicle) {
		AddNotification("Enter the selected vehicle first to steal keys.", false);
		return;
	}

	uintptr_t vehicleNetObject = ReadMemory<uintptr_t>(vehicle + 0xD0);

	// Unlock doors and assert our driver pointer only if we are already driving.
	ApplyVehicleDoorLock(vehicle, CARLOCK_UNLOCKED);
	WriteProcessMemory(Game.hProcess, (LPVOID)(vehicle + Offsets.VehicleDriver), &localPed, sizeof(uintptr_t), nullptr);
	if (vehicleNetObject && vehicleNetObject != 0xFFFFFFFFFFFFFFFF) {
		WriteProcessMemory(Game.hProcess, (LPVOID)(vehicleNetObject + Offsets.VehicleDriver), &localPed, sizeof(uintptr_t), nullptr);
	}

	AddNotification("Keys stolen for current vehicle!", true);
}

void EnterVehicle(int vehicleIndex) {
	if (vehicleIndex < 0 || vehicleIndex >= (int)vehiclePointers.size()) {
		AddNotification("Invalid vehicle selection!", false);
		return;
	}
	
	uintptr_t vehicle = vehiclePointers[vehicleIndex];
	if (!vehicle) {
		AddNotification("Vehicle pointer is invalid!", false);
		return;
	}
	
	if (!LocalPlayer.Update()) {
		AddNotification("Local player not found!", false);
		return;
	}
	
	uintptr_t Object = LocalPlayer.pointer;
	if (!Object) {
		AddNotification("Local player object is invalid!", false);
		return;
	}

	Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
	if (Vec3Empty(vehiclePos)) {
		AddNotification("Failed to get vehicle position!", false);
		return;
	}

	uintptr_t vehicleNetObject = ReadMemory<uintptr_t>(vehicle + 0xD0);
	uintptr_t playerNetObject = ReadMemory<uintptr_t>(Object + 0xD0);

	ApplyVehicleDoorLock(vehicle, CARLOCK_UNLOCKED);


	if (Offsets.Vehicle != 0) {
		WriteProcessMemory(Game.hProcess, (LPVOID)(Object + Offsets.Vehicle), &vehicle, sizeof(uintptr_t), nullptr);
	}

	if (Offsets.VehicleDriver != 0) {
		WriteProcessMemory(Game.hProcess, (LPVOID)(vehicle + Offsets.VehicleDriver), &Object, sizeof(uintptr_t), nullptr);

		if (vehicleNetObject && vehicleNetObject != 0xFFFFFFFFFFFFFFFF) {
			WriteProcessMemory(Game.hProcess, (LPVOID)(vehicleNetObject + Offsets.VehicleDriver), &Object, sizeof(uintptr_t), nullptr);
		}
	}

	Sleep(50);

	Vector3 insidePos = Vector3(vehiclePos.x, vehiclePos.y, vehiclePos.z + 0.5f);
	
	uintptr_t Navigation = ReadMemory<uintptr_t>(Object + 0x30);
	uintptr_t ModelInfo = ReadMemory<uintptr_t>(Object + 0x20);
	
	if (!Navigation || !ModelInfo) {
		AddNotification("Failed to get player navigation/model info!", false);
		return;
	}

	TeleportObject(Object, Navigation, ModelInfo, insidePos, insidePos, true);

	for (int i = 0; i < 20; i++) {
		Sleep(100);

		Vector3 currentVehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
		if (!Vec3Empty(currentVehiclePos)) {
			Vector3 newInsidePos = Vector3(currentVehiclePos.x, currentVehiclePos.y, currentVehiclePos.z + 0.5f);
			TeleportObject(Object, Navigation, ModelInfo, newInsidePos, newInsidePos, true);
		}

		if (Offsets.Vehicle != 0) {
			WriteProcessMemory(Game.hProcess, (LPVOID)(Object + Offsets.Vehicle), &vehicle, sizeof(uintptr_t), nullptr);
		}
		
		if (Offsets.VehicleDriver != 0) {
			WriteProcessMemory(Game.hProcess, (LPVOID)(vehicle + Offsets.VehicleDriver), &Object, sizeof(uintptr_t), nullptr);

			if (vehicleNetObject && vehicleNetObject != 0xFFFFFFFFFFFFFFFF) {
				WriteProcessMemory(Game.hProcess, (LPVOID)(vehicleNetObject + Offsets.VehicleDriver), &Object, sizeof(uintptr_t), nullptr);
			}
		}

		ApplyVehicleDoorLock(vehicle, CARLOCK_UNLOCKED);

		uintptr_t currentVehicle = ReadMemory<uintptr_t>(Object + Offsets.Vehicle);
		if (currentVehicle == vehicle) {

			Sleep(100);
			currentVehicle = ReadMemory<uintptr_t>(Object + Offsets.Vehicle);
			if (currentVehicle == vehicle) {
				break;
			}
		}
	}
	
	std::string vehicleName = vehicleNames[vehicleIndex];
	AddNotification("Entered vehicle: " + vehicleName, true);
}

void StealOutfit(int targetPlayerID) {
	if (!LocalPlayer.Update()) {
		AddNotification("Local player not found!", false);
		return;
	}
	
	Ped targetPed;
	bool foundTarget = false;
	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (ped.GetId() == targetPlayerID) {
			targetPed = ped;
			foundTarget = true;
			break;
		}
	}
	
	if (!foundTarget || !targetPed.pointer) {
		AddNotification("Target player not found!", false);
		return;
	}
	
	uintptr_t localPed = LocalPlayer.pointer;
	uintptr_t targetPedPtr = targetPed.pointer;
	
	if (!localPed || !targetPedPtr) {
		AddNotification("Failed to get ped pointers!", false);
		return;
	}
	
	try {

		DWORD targetModelHash = ReadMemory<DWORD>(targetPedPtr + 0x18);
		if (targetModelHash != 0 && targetModelHash != 0xFFFFFFFF) {
			WriteMemory<DWORD>(localPed + 0x18, targetModelHash);
			Sleep(100);
		}


		int componentOffsets[] = { 0x2B88, 0x2B90, 0x2B98, 0x2BA0, 0x2BA8, 0x2BB0 };
		
		for (int offsetIdx = 0; offsetIdx < 6; offsetIdx++) {
			uintptr_t componentBase = componentOffsets[offsetIdx];

			uintptr_t targetComponentPtr = ReadMemory<uintptr_t>(targetPedPtr + componentBase);
			uintptr_t localComponentPtr = ReadMemory<uintptr_t>(localPed + componentBase);
			
			if (targetComponentPtr != 0 && targetComponentPtr != 0xFFFFFFFFFFFFFFFF && 
			    localComponentPtr != 0 && localComponentPtr != 0xFFFFFFFFFFFFFFFF) {

				for (int i = 0; i < 12; i++) {

					int drawable = ReadMemory<int>(targetComponentPtr + (i * 0x10) + 0x0);
					int texture = ReadMemory<int>(targetComponentPtr + (i * 0x10) + 0x4);
					int palette = ReadMemory<int>(targetComponentPtr + (i * 0x10) + 0x8);
					
					if (drawable >= 0 && drawable < 200) {
						WriteMemory<int>(localComponentPtr + (i * 0x10) + 0x0, drawable);
						WriteMemory<int>(localComponentPtr + (i * 0x10) + 0x4, texture);
						WriteMemory<int>(localComponentPtr + (i * 0x10) + 0x8, palette);
					}
				}
			}
			else {

				for (int i = 0; i < 12; i++) {

					int drawable = ReadMemory<int>(targetPedPtr + componentBase + (i * 0x10) + 0x0);
					int texture = ReadMemory<int>(targetPedPtr + componentBase + (i * 0x10) + 0x4);
					int palette = ReadMemory<int>(targetPedPtr + componentBase + (i * 0x10) + 0x8);
					
					if (drawable >= 0 && drawable < 200) {
						WriteMemory<int>(localPed + componentBase + (i * 0x10) + 0x0, drawable);
						WriteMemory<int>(localPed + componentBase + (i * 0x10) + 0x4, texture);
						WriteMemory<int>(localPed + componentBase + (i * 0x10) + 0x8, palette);
					}
				}
			}

			for (int j = 0; j < 12; j++) {
				int drawable2 = ReadMemory<int>(targetPedPtr + componentBase + (j * 0xC) + 0x0);
				if (drawable2 >= 0 && drawable2 < 200) {
					int texture2 = ReadMemory<int>(targetPedPtr + componentBase + (j * 0xC) + 0x4);
					int palette2 = ReadMemory<int>(targetPedPtr + componentBase + (j * 0xC) + 0x8);
					
					WriteMemory<int>(localPed + componentBase + (j * 0xC) + 0x0, drawable2);
					WriteMemory<int>(localPed + componentBase + (j * 0xC) + 0x4, texture2);
					WriteMemory<int>(localPed + componentBase + (j * 0xC) + 0x8, palette2);
				}
			}
		}

		uintptr_t localModelInfo = ReadMemory<uintptr_t>(localPed + 0x20);
		uintptr_t targetModelInfo = ReadMemory<uintptr_t>(targetPedPtr + 0x20);
		
		if (localModelInfo != 0 && targetModelInfo != 0) {

			for (int i = 0; i < 12; i++) {
				int drawable = ReadMemory<int>(targetModelInfo + 0x2B88 + (i * 0x10) + 0x0);
				if (drawable >= 0 && drawable < 200) {
					int texture = ReadMemory<int>(targetModelInfo + 0x2B88 + (i * 0x10) + 0x4);
					int palette = ReadMemory<int>(targetModelInfo + 0x2B88 + (i * 0x10) + 0x8);
					
					WriteMemory<int>(localModelInfo + 0x2B88 + (i * 0x10) + 0x0, drawable);
					WriteMemory<int>(localModelInfo + 0x2B88 + (i * 0x10) + 0x4, texture);
					WriteMemory<int>(localModelInfo + 0x2B88 + (i * 0x10) + 0x8, palette);
				}
			}
		}

		DWORD pedState = ReadMemory<DWORD>(localPed + 0xE50);
		WriteMemory<DWORD>(localPed + 0xE50, pedState | 0x1);
		Sleep(50);
		WriteMemory<DWORD>(localPed + 0xE50, pedState);

		BYTE visibilityFlag = ReadMemory<BYTE>(localPed + Offsets.VisibleFlag);
		WriteMemory<BYTE>(localPed + Offsets.VisibleFlag, visibilityFlag | 0x1);
		Sleep(50);
		WriteMemory<BYTE>(localPed + Offsets.VisibleFlag, visibilityFlag);
		
		std::string targetName = GetPedName(targetPed);
		if (targetName.empty()) {
			targetName = "Player " + std::to_string(targetPlayerID);
		}
		
		AddNotification("Outfit stolen from " + targetName + "!", true);
	}
	catch (...) {
		AddNotification("Failed to steal outfit!", false);
	}
}

void AddPlayerList() {
	while (true) {
		if (!LocalPlayer.Update()) return;

		std::vector<int> oldPlayerIDs = playerIDs;
		std::vector<int> newPlayerIDs;
		std::vector<std::string> newPlayerNames;
		std::set<int> addedIDs;
		newPlayerIDs.reserve(PedList.size());
		newPlayerNames.reserve(PedList.size());

		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastNameCacheUpdate).count();
		bool shouldUpdateCache = (elapsed >= NAME_CACHE_UPDATE_INTERVAL_MS);

		for (auto& ped : PedList) {
			if (!ped.Update()) continue;
			if (Cheats::Players::IgnorePed && !ped.IsPlayer()) continue;

			float distance = GetDistance(ped.position, LocalPlayer.position);

			if (distance > (float)Cheats::Players::Distance) continue;

			int pedID = ped.GetId();
			
			if (addedIDs.count(pedID) > 0) continue;
			addedIDs.insert(pedID);
			
			std::string pedName;

			if (shouldUpdateCache) {
				pedName = GetPedName(ped);
				if (pedName.empty() || pedName == "Player") {
					pedName = "Player " + std::to_string(pedID);
				}
				pedNameCache[pedID] = pedName; 
			}
			else {

				auto cacheIt = pedNameCache.find(pedID);
				if (cacheIt != pedNameCache.end()) {
					pedName = cacheIt->second;
				}
				else {

					pedName = GetPedName(ped);
					if (pedName.empty() || pedName == "Player") {
						pedName = "Player " + std::to_string(pedID);
					}
					pedNameCache[pedID] = pedName; 
				}
			}

			newPlayerIDs.push_back(pedID);
			newPlayerNames.push_back(pedName);

			if (friendStatus.find(pedID) == friendStatus.end()) {
				friendStatus[pedID] = false;
			}
		}

		if (shouldUpdateCache) {
			lastNameCacheUpdate = now;
		}

		if (shouldUpdateCache) {
			std::map<int, std::string> newCache;
			for (int id : newPlayerIDs) {
				if (pedNameCache.find(id) != pedNameCache.end()) {
					newCache[id] = pedNameCache[id];
				}
			}
			pedNameCache = std::move(newCache);
		}

		int lastSelectedID = (selectedItemPlayer >= 0 && selectedItemPlayer < (int)playerIDs.size()) ? playerIDs[selectedItemPlayer] : -1;
		playerIDs = std::move(newPlayerIDs);
		playerNames = std::move(newPlayerNames);

		if (lastSelectedID != -1) {
			auto it = std::find(playerIDs.begin(), playerIDs.end(), lastSelectedID);
			selectedItemPlayer = (it != playerIDs.end()) ? std::distance(playerIDs.begin(), it) : 0;
		}
		else {
			selectedItemPlayer = -1;
		}

		if (Teleport && selectedPlayerID >= 0) {
			Vector3 pos = GetPositionByID(selectedPlayerID);
			if (!(pos.x == 0.f && pos.y == 0.f && pos.z == 0.f)) {
				uint64_t Object = (uint64_t)LocalPlayer.pointer;
				if (Object) {
					uint64_t Navigation = ReadMemory<uint64_t>(Object + 0x30);
					uint64_t ModelInfo = ReadMemory<uint64_t>(Object + 0x20);

					if (Navigation && ModelInfo) {
						Vector3 TeleportPos = Vector3(pos.x, pos.y, pos.z);
						TeleportObject(Object, Navigation, ModelInfo, TeleportPos, TeleportPos, true);
					}
				}
			}
			Teleport = false;
		}
		Sleep(loopDelay);
	}
}

void NoClip(Vector3 CameraPos) {
	if (!LocalPlayer.pointer)
		return;

	if (!TPModelInfo)
		return;

	static float MagicValue = 0.0f;
	static bool Restoring = false;
	Vector3 NewPosition = TPPosition;

	if (!Cheats::NoClip::Enabled) {
		if (Restoring) {
			WriteMemory<float>(TPModelInfo + 0x2C, MagicValue);
		}
		Restoring = false;
		return;
	}

	if (!Restoring) {
		MagicValue = ReadMemory<float>(TPModelInfo + 0x2C);
		WriteMemory<float>(TPModelInfo + 0x2C, 0.f);
		Restoring = true;
	}

	auto Speed = static_cast<float>(Cheats::NoClip::Speed) * 0.01f; 

	if (GetAsyncKeyState(VK_SHIFT)) {
		Speed *= 2.0f; 
	}

	uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
	Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);

	float pitch = viewAngle.x * (3.14159f / 180.0f);
	float yaw = viewAngle.y * (3.14159f / 180.0f);

	Vector3 Forward;
	Forward.x = cos(pitch) * cos(yaw);
	Forward.y = cos(pitch) * sin(yaw);
	Forward.z = sin(pitch);

	Vector3 Up(0, 0, 1);
	Vector3 Right = Forward.Cross(Up);
	Right.Normalize();

	if (GetAsyncKeyState('W')) {
		NewPosition.x += Forward.x * Speed;
		NewPosition.y += Forward.y * Speed;
		NewPosition.z += Forward.z * Speed;
	}

	if (GetAsyncKeyState('S')) {
		NewPosition.x -= Forward.x * Speed;
		NewPosition.y -= Forward.y * Speed;
		NewPosition.z -= Forward.z * Speed;
	}

	if (GetAsyncKeyState('A')) {
		NewPosition.x -= Right.x * Speed;
		NewPosition.y -= Right.y * Speed;
		NewPosition.z -= Right.z * Speed;
	}

	if (GetAsyncKeyState('D')) {
		NewPosition.x += Right.x * Speed;
		NewPosition.y += Right.y * Speed;
		NewPosition.z += Right.z * Speed;
	}

	if (GetAsyncKeyState(VK_SPACE)) {
		NewPosition.z += Speed;
	}

	if (GetAsyncKeyState(VK_LCONTROL)) {
		NewPosition.z -= (Speed);
	}

	TeleportObject(LocalPlayer.pointer, TPNavigation, 0, NewPosition, NewPosition, false);
}

void FreeCam() {
	if (!LocalPlayer.pointer)
		return;

	if (!TPModelInfo)
		return;

	static bool freeCamKeyPressed = false;
	static bool freeCamTpKeyPressed = false;
	static Vector3 freeCamPosition = Vector3(0, 0, 0);
	static Vector3 savedPlayerPosition = Vector3(0, 0, 0);
	static float savedMagicValue = 0.0f;
	static bool freeCamInitialized = false;

	if (Cheats::FreeCam::Key != 0) {
		bool keyPressed = (GetAsyncKeyState(Cheats::FreeCam::Key) & 0x8000) != 0;
		if (keyPressed && !freeCamKeyPressed) {
			freeCamKeyPressed = true;
			Cheats::FreeCam::Enabled = !Cheats::FreeCam::Enabled;
			
			if (Cheats::FreeCam::Enabled) {
				freeCamInitialized = false;
				AddNotification("Free Cam enabled!", true);
			} else {
				AddNotification("Free Cam disabled!", false);
			}
		} else if (!keyPressed) {
			freeCamKeyPressed = false;
		}
	}

	if (Cheats::FreeCam::Enabled && Cheats::FreeCam::TpKey != 0) {
		bool tpKeyPressed = (GetAsyncKeyState(Cheats::FreeCam::TpKey) & 0x8000) != 0;
		if (tpKeyPressed && !freeCamTpKeyPressed) {
			freeCamTpKeyPressed = true;
			
			savedPlayerPosition = freeCamPosition;
			
			uintptr_t navigation = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x30);
			if (navigation) {
				WriteMemory<Vector3>(LocalPlayer.pointer + 0x90, freeCamPosition);
				WriteMemory<Vector3>(navigation + 0x50, freeCamPosition);
			}
			
			AddNotification("Teleported to camera position!", true);
		} else if (!tpKeyPressed) {
			freeCamTpKeyPressed = false;
		}
	}

	if (Cheats::FreeCam::Enabled) {
		if (!Cheats::FreeCam::PatchApplied) {
			savedMagicValue = ReadMemory<float>(TPModelInfo + 0x2C);
			WriteMemory<float>(TPModelInfo + 0x2C, 0.f);
			WriteMemory<float>(TPModelInfo + 0x20, 0.f);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch, 0x100FF39090909090);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch + 0xB, 0x100FF39090909090);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch + 0x16, 0x100FF39090909090);
			Cheats::FreeCam::PatchApplied = true;
		}

		uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
		if (!camera) return;

		Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);
		Vector3 cameraPos = ReadMemory<Vector3>(camera + 0x60);

		if (!freeCamInitialized) {
			freeCamPosition = cameraPos;
			savedPlayerPosition = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
			freeCamInitialized = true;
		}

		uintptr_t navigation = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x30);
		if (navigation) {
			WriteMemory<Vector3>(LocalPlayer.pointer + 0x90, savedPlayerPosition);
			WriteMemory<Vector3>(navigation + 0x50, savedPlayerPosition);
		}

		float camSpeed = Cheats::FreeCam::Speed * 0.5f;
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			camSpeed *= 3.0f;
		}

		float pitch = viewAngle.x * (3.14159f / 180.0f);
		float yaw = viewAngle.y * (3.14159f / 180.0f);

		Vector3 Forward;
		Forward.x = cos(pitch) * cos(yaw);
		Forward.y = cos(pitch) * sin(yaw);
		Forward.z = sin(pitch);

		Vector3 Up(0, 0, 1);
		Vector3 Right = Forward.Cross(Up);
		Right.Normalize();

		if (GetAsyncKeyState('W') & 0x8000) {
			freeCamPosition.x += Forward.x * camSpeed;
			freeCamPosition.y += Forward.y * camSpeed;
			freeCamPosition.z += Forward.z * camSpeed;
		}

		if (GetAsyncKeyState('S') & 0x8000) {
			freeCamPosition.x -= Forward.x * camSpeed;
			freeCamPosition.y -= Forward.y * camSpeed;
			freeCamPosition.z -= Forward.z * camSpeed;
		}

		if (GetAsyncKeyState('A') & 0x8000) {
			freeCamPosition.x -= Right.x * camSpeed;
			freeCamPosition.y -= Right.y * camSpeed;
		}

		if (GetAsyncKeyState('D') & 0x8000) {
			freeCamPosition.x += Right.x * camSpeed;
			freeCamPosition.y += Right.y * camSpeed;
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			freeCamPosition.z += camSpeed;
		}

		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) {
			freeCamPosition.z -= camSpeed;
		}

		WriteMemory<Vector3>(camera + 0x60, freeCamPosition);
	}
	else {
		if (Cheats::FreeCam::PatchApplied) {
			WriteMemory<float>(TPModelInfo + 0x2C, savedMagicValue);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch, 0x100FF36047110FF3);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch + 0xB, 0x100FF3644F110FF3);
			WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreeCamPatch + 0x16, 0x110FF36847110FF3);
			Cheats::FreeCam::PatchApplied = false;
			freeCamInitialized = false;
		}
	}
}

void MapTeleport() {
	static const char* locations[] = {
		"MD Hastane", "MD PVP Zone", "Well Hastane", "Well PVP Zone", "Non WH Hastane",
		"PD Yanı Kıyafetci", "SS Hastane", "FBI", "Prison", "Ust Kasaba", "Orta Kasaba",
		"Depo Cati 1", "Depo Cati 2", "Kırmızı Otopark"
	};

	static const Vector3 teleportLocations[] = {
		{ 308.7889f, -592.5567f, 43.2840f },
		{ 216.01f, -800.52f, 30.85f },
		{ 312.1247f, -592.7950f, 43.2840f },
		{ 216.01f, -800.52f, 30.85f },
		{ 306.9377f, -595.2255f, 43.2840f },
		{ 428.07f, -800.32f, 29.87f },
		{ 336.0438f, -1436.2172f, 46.7784f },
		{ 2449.25f, -307.9f, 93.95f },
		{ 1709.14f, 2667.66f, 45.56f },
		{ -6.72f, 6417.51f, 38.81f },
		{ 1839.58f, 3672.61f, 34.28f },
		{ 1211.8837f, -3113.5347f, 15.5565f },
		{ 1212.4299f, -3325.0161f, 15.5354f },
		{ 340.6054f, -1686.8359f, 52.3391f }
	};

	static int selected_location = -1;
	for (int i = 0; i < IM_ARRAYSIZE(locations); i++) {
		if (CustomImGui::SelectTable(locations[i], selected_location == i)) {
			selected_location = i;
		}
		if (selected_location == i && ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
			if (selected_location >= 0 && selected_location < IM_ARRAYSIZE(teleportLocations)) {
				Vector3 currentPos = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
				uintptr_t navig = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x30);
				Vector3 targetPos = teleportLocations[selected_location];
				float distanceThreshold = 2.0f;
				float distance = sqrtf(powf(currentPos.x - targetPos.x, 2) +
					powf(currentPos.y - targetPos.y, 2) +
					powf(currentPos.z - targetPos.z, 2));

				if (distance <= distanceThreshold) {
				}
				else {
					WriteMemory<Vector3>(navig + 0x30, Vector3(0, 0, 0));
					WriteMemory<Vector3>(LocalPlayer.pointer + 0x90, targetPos);
					WriteMemory<Vector3>(navig + 0x50, targetPos);
				}
			}
		}
	}
}

void TeleportToWaypoint(bool toogle = false) {
	if (toogle) {
		if (Cheats::Teleport::posX == 0.0f && Cheats::Teleport::posY == 0.0f) {
			return;
		}
		uint64_t Object = NULL;
		uint64_t Navigation = NULL;
		uint64_t ModelInfo = NULL;
		if (!Object || !Navigation || !ModelInfo) {
			Object = (uint64_t)LocalPlayer.pointer;
			Navigation = ReadMemory<uint64_t>(LocalPlayer.pointer + 0x30);
			ModelInfo = ReadMemory<uint64_t>(LocalPlayer.pointer + 0x20);
		}
		Vector3 TeleportPos = Vector3(Cheats::Teleport::posX, Cheats::Teleport::posY, Cheats::Teleport::posZ);
		TeleportObject(Object, Navigation, ModelInfo, TeleportPos, TeleportPos, true);

	}
	else {
		for (int i = 0; i < 2000; i++) {
			uint64_t Blip = ReadMemory<uint64_t>(Offsets.GameBase + Offsets.Waypoint + (i * 8));
			if (!Blip)
				continue;
			int BlipIcon = ReadMemory<int>(Blip + 0x40);
			int BlipColor = ReadMemory<int>(Blip + 0x48);
			if ((BlipColor != 84) || (BlipIcon != 8))
				continue;
			Vector2 WaypointPos = ReadMemory<Vector2>(Blip + 0x10);
			if (WaypointPos.x != 0 && WaypointPos.y != 0) {
				uint64_t Object = NULL;
				uint64_t Navigation = NULL;
				uint64_t ModelInfo = NULL;
				if (!Object || !Navigation || !ModelInfo) {
					Object = (uint64_t)LocalPlayer.pointer;
					Navigation = ReadMemory<uint64_t>(LocalPlayer.pointer + 0x30);
					ModelInfo = ReadMemory<uint64_t>(LocalPlayer.pointer + 0x20);
				}
				Vector3 TeleportPos = Vector3(WaypointPos.x, WaypointPos.y, -210.f);
				TeleportObject(Object, Navigation, ModelInfo, TeleportPos, TeleportPos, true);
			}
		}
	}
}

void Exploits() {
	while (!exitLoop) {
		if (LocalPlayer.pointer == 0) {
			Sleep(100);
			continue;
		}
		
		if (!LocalPlayer.Update()) {
			Sleep(100);
			continue;
		}
		
		uintptr_t weaponManager = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.WeaponManager);
		if (!weaponManager) {
			Sleep(loopDelay);
			continue;
		}
		uintptr_t weaponinfo = ReadMemory<uintptr_t>(weaponManager + 0x20);
		if (!weaponinfo) {
			Sleep(loopDelay);
			continue;
		}

		TPModelInfo = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x20);
		TPPosition = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
		TPNavigation = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x30);

		uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.BlipList);
		uintptr_t camera2 = ReadMemory<uintptr_t>(camera + 0x3C0);
		Vector3 camerapos = ReadMemory<Vector3>(camera2 + 0x40);
		NoClip(camerapos);
		FreeCam();

		if (Cheats::Exploit::HealthBoost) {
			if (GetAsyncKeyState(Cheats::Exploit::HealthBoostKey) & 0x8000) {
				WriteMemory<float>(LocalPlayer.pointer + 0x280, static_cast<float>(Cheats::Exploit::HealthBoostValue));
			}
		}

		if (Cheats::Exploit::ArmorBoost) {
			if (GetAsyncKeyState(Cheats::Exploit::ArmorBoostKey) & 0x8000) {
				float ArmorCheck = ReadMemory<float>(LocalPlayer.pointer + Offsets.Armor);
				if (ArmorCheck < 100) {
					float newArmorValue = ArmorCheck + static_cast<float>(Cheats::Exploit::ArmorBoostValue);
					if (newArmorValue > 100) {
						newArmorValue = 100;
					}
					WriteMemory<float>(LocalPlayer.pointer + Offsets.Armor, newArmorValue);
				}
			}
		}

		if (Cheats::Exploit::InfiniteAmmo) {
			uintptr_t AmmoInfo = ReadMemory<uintptr_t>(weaponinfo + 0x60);
			if (!AmmoInfo) goto after_weapon_writes;
			uintptr_t AmmoCount = ReadMemory<uintptr_t>(AmmoInfo + 0x8);
			if (!AmmoCount) goto after_weapon_writes;
			uintptr_t AmmoCount2 = ReadMemory<uintptr_t>(AmmoCount + 0x0);
			if (AmmoCount2)
				WriteMemory<float>(AmmoCount2 + 0x18, 30);
		}

		if (Cheats::Exploit::NoRecoil) {
			WriteMemory<float>(weaponinfo + 0x2F4, 0.f);
		}

		if (Cheats::Exploit::NoSpread) {
			WriteMemory<float>(weaponinfo + 0x84, 0.0f);
		}

		if (Cheats::Exploit::NoReload) {
			WriteMemory<float>(weaponinfo + 0x134, 1000);
		}

		if (Cheats::Exploit::NoRange) {
			WriteMemory<float>(weaponinfo + 0x28C, 1000.f);
		}

		static bool damageBoostKeyPressed = false;
		static bool damageBoostActive = false;
		static uint32_t originalDamageValue = 1;
		static bool originalDamageSaved = false;
		static uintptr_t lastWeaponInfo = 0;
		static bool wasDamageBoostUsed = false;

		if (weaponinfo != lastWeaponInfo) {
			if (lastWeaponInfo != 0 && wasDamageBoostUsed && originalDamageSaved) {
				WriteMemory<uint32_t>(lastWeaponInfo + 0x120, originalDamageValue);
			}
			originalDamageSaved = false;
			wasDamageBoostUsed = false;
			lastWeaponInfo = weaponinfo;
			damageBoostActive = false;
		}

		if (Cheats::Misc::damageBoost && Cheats::Misc::damageBoostKey != 0) {
			bool keyPressed = (GetAsyncKeyState(Cheats::Misc::damageBoostKey) & 0x8000) != 0;
			
			if (keyPressed && !damageBoostKeyPressed) {
				damageBoostKeyPressed = true;
				damageBoostActive = !damageBoostActive;
				
				if (damageBoostActive) {
					if (!originalDamageSaved) {
						originalDamageValue = ReadMemory<uint32_t>(weaponinfo + 0x120);
						if (originalDamageValue == 0) originalDamageValue = 1;
						originalDamageSaved = true;
					}
					wasDamageBoostUsed = true;
					AddNotification("Damage Boost enabled!", true);
				} else {
					AddNotification("Damage Boost disabled!", false);
				}
			} else if (!keyPressed) {
				damageBoostKeyPressed = false;
			}

			if (damageBoostActive && wasDamageBoostUsed) {
				WriteMemory<uint32_t>(weaponinfo + 0x120, static_cast<uint32_t>(Cheats::Misc::damageBoostValue));
			} else if (!damageBoostActive && wasDamageBoostUsed && originalDamageSaved) {
				WriteMemory<uint32_t>(weaponinfo + 0x120, originalDamageValue);
			}
		} else if (Cheats::Misc::damageBoost && Cheats::Misc::damageBoostKey == 0) {
			if (!originalDamageSaved) {
				originalDamageValue = ReadMemory<uint32_t>(weaponinfo + 0x120);
				if (originalDamageValue == 0) originalDamageValue = 1;
				originalDamageSaved = true;
			}
			wasDamageBoostUsed = true;
			WriteMemory<uint32_t>(weaponinfo + 0x120, static_cast<uint32_t>(Cheats::Misc::damageBoostValue));
			damageBoostActive = true;
		} else {
			if (wasDamageBoostUsed && originalDamageSaved) {
				WriteMemory<uint32_t>(weaponinfo + 0x120, originalDamageValue);
				wasDamageBoostUsed = false;
				damageBoostActive = false;
			}
		}

after_weapon_writes:
		if (Cheats::Exploit::ReloadAmmo) {


			uintptr_t AmmoInfo = ReadMemory<uintptr_t>(weaponinfo + 0x60);
			if (!AmmoInfo) goto after_reload;
			uintptr_t AmmoCount = ReadMemory<uintptr_t>(AmmoInfo + 0x8);
			if (!AmmoCount) goto after_reload;
			uintptr_t AmmoCount2 = ReadMemory<uintptr_t>(AmmoCount + 0x0);
			if (AmmoCount2) {
				int currentAmmo = ReadMemory<int>(AmmoCount2 + 0x18);
				if (currentAmmo < Cheats::Exploit::ReloadValue) {
					WriteMemory<float>(AmmoCount2 + 0x18, static_cast<float>(Cheats::Exploit::ReloadValue));
				}
			}
		}

after_reload:

		if (Cheats::Teleport::TeleportWaypoint) {
			TeleportToWaypoint(false);
			Cheats::Teleport::TeleportWaypoint = false;
		}
		if (Cheats::Teleport::CustomTeleportWaypoint) {
			TeleportToWaypoint(true);
			Cheats::Teleport::CustomTeleportWaypoint = false;
		}

		if (Cheats::Vehicle::Fix) {
			if (GetAsyncKeyState(Cheats::Vehicle::FixKey) & 0x8000) {
				uintptr_t vehiclePtr = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.Vehicle);
				if (vehiclePtr) {
					WriteMemory<uint8_t>(vehiclePtr + 0x972, 0x17);
				}
			}
		}

		if (Cheats::Exploit::InfiniteStamina) {
			uintptr_t playerinfo = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x10C8);
			if (playerinfo) {
				// Keep stamina topped by force-writing high values each tick.
				const float staminaFull = 1000.0f;
				WriteMemory<float>(playerinfo + 0x0CF4, staminaFull); // current stamina
				WriteMemory<float>(playerinfo + 0x0CF8, staminaFull); // max/reserve
			}
		}

		if (Cheats::Exploit::AntiHeadshot) {
			WriteMemory<uint8_t>(LocalPlayer.pointer + 0x189, 1); 
		}

		if (Cheats::Exploit::InvisibleNoclip) {
			WriteMemory<BYTE>(LocalPlayer.pointer + 0x2C, 0x01); 
		}

		static bool safeInvisibleKeyPressed = false;
		static bool safeInvisibleActive = false;
		static BYTE originalVisibilityValue = 0x00;
		static BYTE originalVisibleFlag = 0x00;
		static BYTE originalNetVisibleFlag = 0x00;
		static DWORD originalPedState = 0;
		static bool originalValueSaved = false;
		
		if (Cheats::Exploit::SafeInvisible && Cheats::Exploit::SafeInvisibleKey != 0) {

			if (!originalValueSaved && LocalPlayer.pointer != 0) {
				originalVisibilityValue = ReadMemory<BYTE>(LocalPlayer.pointer + 0x2C);
				originalVisibleFlag = ReadMemory<BYTE>(LocalPlayer.pointer + Offsets.VisibleFlag);
				originalPedState = ReadMemory<DWORD>(LocalPlayer.pointer + 0xE50);
				uintptr_t playerNetObject = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0xD0);
				if (playerNetObject && playerNetObject != 0xFFFFFFFFFFFFFFFF) {
					originalNetVisibleFlag = ReadMemory<BYTE>(playerNetObject + Offsets.VisibleFlag);
				}
				originalValueSaved = true;
			}

			auto writeVisibility = [&](BYTE value2c, BYTE flagValue, BYTE netFlagValue, DWORD pedStateValue) {
				WriteMemory<BYTE>(LocalPlayer.pointer + 0x2C, value2c);
				WriteMemory<BYTE>(LocalPlayer.pointer + Offsets.VisibleFlag, flagValue);
				WriteMemory<DWORD>(LocalPlayer.pointer + 0xE50, pedStateValue);

				uintptr_t playerNetObject = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0xD0);
				if (playerNetObject && playerNetObject != 0xFFFFFFFFFFFFFFFF) {
					WriteMemory<BYTE>(playerNetObject + Offsets.VisibleFlag, netFlagValue);
					WriteMemory<BYTE>(playerNetObject + 0x2C, value2c);
					WriteMemory<DWORD>(playerNetObject + 0xE50, pedStateValue);
				}
			};
			
			if (GetAsyncKeyState(Cheats::Exploit::SafeInvisibleKey) & 0x8000) {
				if (!safeInvisibleKeyPressed) {
					safeInvisibleKeyPressed = true;
					safeInvisibleActive = !safeInvisibleActive;
					
					if (safeInvisibleActive) {

						writeVisibility(0x01, 0x01, 0x01, originalPedState | 0x1);
						AddNotification("Safe Invisible enabled!", true);
					} else {

						if (originalValueSaved) {
							writeVisibility(originalVisibilityValue, originalVisibleFlag, originalNetVisibleFlag, originalPedState);
						} else {
							writeVisibility(0x00, 0x00, 0x00, originalPedState);
						}
						AddNotification("Safe Invisible disabled!", false);
					}
				}
			} else {
				safeInvisibleKeyPressed = false;
			}

			if (safeInvisibleActive) {
				writeVisibility(0x01, 0x01, 0x01, originalPedState | 0x1);
			}
		} else {

			if (safeInvisibleActive && originalValueSaved) {
				WriteMemory<BYTE>(LocalPlayer.pointer + 0x2C, originalVisibilityValue);
				WriteMemory<BYTE>(LocalPlayer.pointer + Offsets.VisibleFlag, originalVisibleFlag);
				WriteMemory<DWORD>(LocalPlayer.pointer + 0xE50, originalPedState);
				uintptr_t playerNetObject = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0xD0);
				if (playerNetObject && playerNetObject != 0xFFFFFFFFFFFFFFFF) {
					WriteMemory<BYTE>(playerNetObject + Offsets.VisibleFlag, originalNetVisibleFlag);
					WriteMemory<BYTE>(playerNetObject + 0x2C, originalVisibilityValue);
					WriteMemory<DWORD>(playerNetObject + 0xE50, originalPedState);
				}
				safeInvisibleActive = false;
			} else if (safeInvisibleActive) {
				WriteMemory<BYTE>(LocalPlayer.pointer + 0x2C, 0x00);
				WriteMemory<BYTE>(LocalPlayer.pointer + Offsets.VisibleFlag, 0x00);
				WriteMemory<DWORD>(LocalPlayer.pointer + 0xE50, originalPedState);
				uintptr_t playerNetObject = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0xD0);
				if (playerNetObject && playerNetObject != 0xFFFFFFFFFFFFFFFF) {
					WriteMemory<BYTE>(playerNetObject + Offsets.VisibleFlag, 0x00);
					WriteMemory<BYTE>(playerNetObject + 0x2C, 0x00);
					WriteMemory<DWORD>(playerNetObject + 0xE50, originalPedState);
				}
				safeInvisibleActive = false;
			}

			originalValueSaved = false;
			originalVisibilityValue = 0x00;
			originalVisibleFlag = 0x00;
			originalNetVisibleFlag = 0x00;
			originalPedState = 0;
		}

		static bool vehicleBoostKeyPressed = false;
		static bool vehicleBoostActive = false;
		static float originalBoostValue = 1.0f;
		static uintptr_t lastVehiclePtr = 0;

		if (Cheats::Exploit::Vehicle::Boost && Cheats::Exploit::Vehicle::BoostKey != 0) {
			uintptr_t vehiclePtr = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.Vehicle);
			if (vehiclePtr) {
				uintptr_t handling = ReadMemory<uintptr_t>(vehiclePtr + 0x960);
				if (handling) {
					if (vehiclePtr != lastVehiclePtr) {
						if (lastVehiclePtr != 0 && vehicleBoostActive) {
							uintptr_t lastHandling = ReadMemory<uintptr_t>(lastVehiclePtr + 0x960);
							if (lastHandling) {
								WriteMemory<float>(lastHandling + 0x4C, originalBoostValue);
							}
						}
						originalBoostValue = ReadMemory<float>(handling + 0x4C);
						lastVehiclePtr = vehiclePtr;
					}

					bool keyPressed = (GetAsyncKeyState(Cheats::Exploit::Vehicle::BoostKey) & 0x8000) != 0;
					
					if (keyPressed && !vehicleBoostKeyPressed) {
						vehicleBoostKeyPressed = true;
						vehicleBoostActive = !vehicleBoostActive;
						
						if (vehicleBoostActive) {
							AddNotification("Vehicle Boost enabled!", true);
						} else {
							AddNotification("Vehicle Boost disabled!", false);
						}
					} else if (!keyPressed) {
						vehicleBoostKeyPressed = false;
					}

					if (vehicleBoostActive) {
						WriteMemory<float>(handling + 0x4C, Cheats::Exploit::Vehicle::BoostValue);
					} else {
						WriteMemory<float>(handling + 0x4C, originalBoostValue);
					}
				}
			} else {
				if (lastVehiclePtr != 0 && vehicleBoostActive) {
					uintptr_t lastHandling = ReadMemory<uintptr_t>(lastVehiclePtr + 0x960);
					if (lastHandling) {
						WriteMemory<float>(lastHandling + 0x4C, originalBoostValue);
					}
					lastVehiclePtr = 0;
				}
			}
		} else if (Cheats::Exploit::Vehicle::Boost && Cheats::Exploit::Vehicle::BoostKey == 0) {
			uintptr_t vehiclePtr = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.Vehicle);
			if (vehiclePtr) {
				uintptr_t handling = ReadMemory<uintptr_t>(vehiclePtr + 0x960);
				if (handling) {
					WriteMemory<float>(handling + 0x4C, Cheats::Exploit::Vehicle::BoostValue);
				}
			}
		} else {
			if (vehicleBoostActive && lastVehiclePtr != 0) {
				uintptr_t handling = ReadMemory<uintptr_t>(lastVehiclePtr + 0x960);
				if (handling) {
					WriteMemory<float>(handling + 0x4C, originalBoostValue);
				}
				vehicleBoostActive = false;
				lastVehiclePtr = 0;
			}
		}

		static bool peakAssistKeyPressed = false;
		
		if (Cheats::Exploit::PeakAssist && Cheats::Exploit::PeakAssistKey != 0) {
			if (GetAsyncKeyState(Cheats::Exploit::PeakAssistKey) & 0x8000) {
				if (!peakAssistKeyPressed) {
					peakAssistKeyPressed = true;
					
					if (!PeakPositionSaved) {

						SavedPeakPosition = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
						PeakPositionSaved = true;
						AddNotification("Peak position saved!", true);
					} else {

						if (!(SavedPeakPosition.x == 0 && SavedPeakPosition.y == 0 && SavedPeakPosition.z == 0)) {
							uint64_t Object = (uint64_t)LocalPlayer.pointer;
							if (Object) {
								uint64_t Navigation = ReadMemory<uint64_t>(Object + 0x30);
								uint64_t ModelInfo = ReadMemory<uint64_t>(Object + 0x20);
								
								if (Navigation && ModelInfo) {
									TeleportObject(Object, Navigation, ModelInfo, SavedPeakPosition, SavedPeakPosition, true);
									AddNotification("Teleported to saved position!", true);
									PeakPositionSaved = false; 
									SavedPeakPosition = Vector3(0, 0, 0);
								}
							}
						}
					}
				}
			} else {
				peakAssistKeyPressed = false;
			}
		}

		static bool friendKeyPressed = false;
		if (Cheats::MenuUtils::FriendKey != 0) {
			if (GetAsyncKeyState(Cheats::MenuUtils::FriendKey) & 0x8000) {
				if (!friendKeyPressed) {
					friendKeyPressed = true;
					ToggleFriendAtCrosshair();
				}
			} else {
				friendKeyPressed = false;
			}
		}

		MagicBulletToggle();

		WheelBulletToggle();

		if (Cheats::Exploit::ServerCrash && Cheats::Exploit::ServerCrashKey != 0) {
			static bool serverCrashKeyPressed = false;
			static bool serverCrashActive = false;
			bool keyPressed = (GetAsyncKeyState(Cheats::Exploit::ServerCrashKey) & 0x8000) != 0;
			
			if (keyPressed && !serverCrashKeyPressed) {
				serverCrashKeyPressed = true;
				
				if (!serverCrashActive) {

					serverCrashActive = true;
					AddNotification("Manuel crash activated! Press again to execute.", true);
				} else {

					serverCrashActive = false;

					int crashCount = 0;
					for (auto& ped : PedList) {
						if (!ped.Update()) continue;
						if (ped.IsDead()) continue;
						if (!ped.IsPlayer()) continue;

						int pedID = ped.GetId();
						if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {
							continue; 
						}

						float distance = GetDistance(ped.position, LocalPlayer.position);
						if (distance > 100.0f) continue; 

						uintptr_t targetPed = ped.pointer;
						if (!targetPed) continue;


					auto crashThread = [targetPed]() {
						for (int i = 0; i < 5000; i++) {

							WriteMemory<int>(targetPed + 0xE50, rand() % 0x7FFFFFFF); 
							WriteMemory<int>(targetPed + 0xE48, rand() % 0x7FFFFFFF); 
							WriteMemory<int>(targetPed + 0xE4C, rand() % 0x7FFFFFFF); 
							WriteMemory<int>(targetPed + 0xE54, rand() % 0x7FFFFFFF); 

							WriteMemory<DWORD64>(targetPed + 0x2C8, (DWORD64)((rand() << 32) | rand()));
							WriteMemory<DWORD64>(targetPed + 0x2CC, (DWORD64)((rand() << 32) | rand()));
							WriteMemory<DWORD64>(targetPed + 0x2D0, (DWORD64)((rand() << 32) | rand()));

							WriteMemory<DWORD64>(targetPed + 0x2D8, (DWORD64)((rand() << 32) | rand()));
							WriteMemory<DWORD64>(targetPed + 0x2DC, (DWORD64)((rand() << 32) | rand()));

							WriteMemory<int>(targetPed + 0x188, rand() % 0xFFFFFFFF);
							WriteMemory<int>(targetPed + 0x18C, rand() % 0xFFFFFFFF);

							WriteMemory<DWORD>(targetPed + 0x18, (DWORD)(rand() % 0xFFFFFFFF));

							uintptr_t netEntity = ReadMemory<uintptr_t>(targetPed + 0xD0);
							if (netEntity) {
								WriteMemory<int>(netEntity + 0x44, rand() % 0xFFFFFFFF);
								WriteMemory<int>(netEntity + 0x48, rand() % 0xFFFFFFFF);
								WriteMemory<DWORD64>(netEntity + 0x58, (DWORD64)((rand() << 32) | rand()));
							}
						}
					};

					for (int t = 0; t < 10; t++) {
						std::thread(crashThread).detach();
					}

					uintptr_t targetNavigation = ReadMemory<uintptr_t>(targetPed + 0x30);
					uintptr_t targetModelInfo = ReadMemory<uintptr_t>(targetPed + 0x20);
					
					if (targetNavigation && targetModelInfo) {
						for (int i = 0; i < 3000; i++) {

							Vector3 crashPos = Vector3(
								(float)(rand() % 20000 - 10000),
								(float)(rand() % 20000 - 10000),
								(float)(rand() % 20000 - 10000)
							);

							TeleportObject(targetPed, targetNavigation, targetModelInfo, crashPos, crashPos, false);

							WriteMemory<Vector3>(targetPed + 0x90, crashPos);
							WriteMemory<Vector3>(targetNavigation + 0x50, crashPos);

							WriteMemory<float>(targetPed + 0x70, (float)(rand() % 360));
							WriteMemory<float>(targetPed + 0x74, (float)(rand() % 360));
							WriteMemory<float>(targetPed + 0x78, (float)(rand() % 360));

							WriteMemory<float>(targetPed + Offsets.Health, (float)(rand() % 1000000));
							WriteMemory<float>(targetPed + Offsets.Armor, (float)(rand() % 1000000));
							WriteMemory<float>(targetPed + Offsets.MaxHealth, (float)(rand() % 1000000));

							Matrix corruptMatrix;
							for (int m = 0; m < 16; m++) {
								((float*)&corruptMatrix)[m] = (float)(rand() % 20000 - 10000);
							}
							WriteMemory<Matrix>(targetPed + 0x60, corruptMatrix);

							WriteMemory<int>(targetPed + 0xE50, rand() % 0x7FFFFFFF);
							WriteMemory<int>(targetPed + 0xE48, rand() % 0x7FFFFFFF);
						}
					}

					uintptr_t targetVehicle = ReadMemory<uintptr_t>(targetPed + Offsets.Vehicle);
					if (targetVehicle) {
						for (int i = 0; i < 2000; i++) {
							Vector3 vehicleCrashPos = Vector3(
								(float)(rand() % 20000 - 10000),
								(float)(rand() % 20000 - 10000),
								(float)(rand() % 20000 - 10000)
							);
							WriteMemory<Vector3>(targetVehicle + 0x90, vehicleCrashPos);
							WriteMemory<float>(targetVehicle + Offsets.Health, (float)(rand() % 1000000));

							WriteMemory<int>(targetVehicle + 0xE50, rand() % 0x7FFFFFFF);
							WriteMemory<int>(targetVehicle + 0xE48, rand() % 0x7FFFFFFF);
						}
					}
					
						crashCount++;
					}
					
					if (crashCount > 0) {
						AddNotification("Manuel crash: " + std::to_string(crashCount) + " player(s) targeted!", true);
					} else {
						AddNotification("No players found nearby!", false);
					}
				}
			} else if (!keyPressed) {
				serverCrashKeyPressed = false;
			}
		}

		Sleep(loopDelay);
	}
}

void StrafeMacro() {
	while (!exitLoop) {
		if (Cheats::Exploit::Strafe::Enabled) {
			bool keyOk = (Cheats::Exploit::Strafe::Key == 0) || ((GetAsyncKeyState(Cheats::Exploit::Strafe::Key) & 0x8000) != 0);
			if (keyOk && TPModelInfo) {
				bool moving = (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState('D') & 0x8000);
				float speed = 0.0f;
				if (moving) {
					speed = static_cast<float>(Cheats::Exploit::Strafe::Speed) / 10.0f;
					if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
						speed *= 2.0f;
					}
				}
				WriteMemory<float>(TPModelInfo + 0x2C, speed);
			}
		}
		
		Sleep(1);
	}
}

Vector3 GetBonePosition(const Ped& ped, int boneId) {
	if (boneId >= 0 && boneId < 35) {
		return ped.boneList[boneId];
	}	
	return Vector3(0, 0, 0);
}

void MagicBulletToggle() {
	if (!Cheats::AimAssist::Silent::MagicBullet) return;
	if (!LocalPlayer.Update()) return;
	if (Cheats::AimAssist::Silent::MagicBulletKey == 0) return;

	static bool magicKeyPressed = false;
	static bool magicActive = false;

	bool keyPressed = (GetAsyncKeyState(Cheats::AimAssist::Silent::MagicBulletKey) & 0x8000) != 0;

	if (keyPressed && !magicKeyPressed) {
		magicKeyPressed = true;
		magicActive = !magicActive;
		
		if (magicActive) {
			AddNotification("Magic Bullet enabled!", true);
		} else {
			AddNotification("Magic Bullet disabled!", false);
		}
	} else if (!keyPressed) {
		magicKeyPressed = false;
	}

	if (!magicActive) return;

	if (!(GetAsyncKeyState(Cheats::AimAssist::Silent::Key) & 0x8000)) return;

	Ped target = FindBestTarget(Cheats::AimAssist::Silent::Fov);
	if (Vec3Empty(target.boneList[Head])) return;

	int pedID = target.GetId();
	if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) return;

	Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
	Vector3 bonePos;
	
	if (Cheats::AimAssist::Silent::ClosestBone) {
		bonePos = FindClosestBoneInFOV(target, (float)Cheats::AimAssist::Silent::Fov, viewMatrix);
	} else {
		int boneIndex = Cheats::AimAssist::Silent::SelectedType;
		if (boneIndex >= 0 && boneIndex < 35) {
			bonePos = target.boneList[boneIndex];
		} else {
			bonePos = target.boneList[Head];
		}
	}

	if (Vec3Empty(bonePos)) return;

	uintptr_t weaponManager = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.WeaponManager);
	if (!weaponManager) return;
	
	uintptr_t weaponObg = ReadMemory<uintptr_t>(weaponManager + 0x78);
	if (!weaponObg) return;
	
	uintptr_t CWeapon = ReadMemory<uintptr_t>(weaponObg + 0x320);
	if (!CWeapon) return;

	Vector3 heading = ReadMemory<Vector3>(target.pointer + 0x70);

	float distanceAhead = 5.f;
	Vector3 forwardVector = { 0.0f, 0.0f, distanceAhead };
	Vector3 spawnPosition = bonePos + heading + forwardVector;


	WriteMemory<Vector3>(CWeapon + 0x20, spawnPosition);

	EndBulletPos = spawnPosition;
}

void WheelBulletToggle() {
	if (!Cheats::AimAssist::Silent::WheelBullet) {
		WheelBulletActive = false;
		return;
	}
	if (!LocalPlayer.Update()) return;
	if (Cheats::AimAssist::Silent::WheelBulletKey == 0) {
		WheelBulletActive = false;
		return;
	}

	static bool wheelKeyPressed = false;

	bool keyPressed = (GetAsyncKeyState(Cheats::AimAssist::Silent::WheelBulletKey) & 0x8000) != 0;

	if (keyPressed && !wheelKeyPressed) {
		wheelKeyPressed = true;
		WheelBulletActive = !WheelBulletActive;
		
		if (WheelBulletActive) {
			AddNotification("Wheel Bullet enabled!", true);
		} else {
			AddNotification("Wheel Bullet disabled!", false);
		}
	} else if (!keyPressed) {
		wheelKeyPressed = false;
	}
}
