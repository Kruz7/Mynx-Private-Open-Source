#include "custom_widgets.hpp"
#pragma comment(lib, "Winmm.lib")
#include <cmath>

#include "font_defines.h"


#include "Sounds.hpp"


namespace custom
{
	float GetAnimOffset()
	{
		if (c::anim_offset_side == 0)
			return -ImGui::GetWindowSize().x;
		else
			return ImGui::GetWindowSize().x;
	}

	int rotation_start_index;
	void ImRotateStart()
	{
		rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
	}

	ImVec2 ImRotationCenter()
	{
		ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);

		const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

		return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
	}

	void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
	{
		float s = sin(rad), c = cos(rad);
		center = ImRotate(center, s, c) - center;

		auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
	}



	const char* keys[] =
	{
		"-",
		"Mouse 1",
		"Mouse 2",
		"CN",
		"Mouse 3",
		"Mouse 4",
		"Mouse 5",
		"-",
		"Back",
		"Tab",
		"-",
		"-",
		"CLR",
		"Enter",
		"-",
		"-",
		"Shift",
		"CTL",
		"Menu",
		"Pause",
		"Caps Lock",
		"KAN",
		"-",
		"JUN",
		"FIN",
		"KAN",
		"-",
		"Escape",
		"CON",
		"NCO",
		"ACC",
		"MAD",
		"Space",
		"PGU",
		"PGD",
		"End",
		"Home",
		"Left",
		"Up",
		"Right",
		"Down",
		"SEL",
		"PRI",
		"EXE",
		"PRI",
		"INS",
		"Delete",
		"HEL",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"WIN",
		"WIN",
		"APP",
		"-",
		"SLE",
		"Numpad 0",
		"Numpad 1",
		"Numpad 2",
		"Numpad 3",
		"Numpad 4",
		"Numpad 5",
		"Numpad 6",
		"Numpad 7",
		"Numpad 8",
		"Numpad 9",
		"MUL",
		"ADD",
		"SEP",
		"MIN",
		"Delete",
		"DIV",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"F16",
		"F17",
		"F18",
		"F19",
		"F20",
		"F21",
		"F22",
		"F23",
		"F24",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"NUM",
		"SCR",
		"EQU",
		"MAS",
		"TOY",
		"OYA",
		"OYA",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"Shift",
		"Shift",
		"Ctrl",
		"Ctrl",
		"Alt",
		"Alt"
	};

#include <string>

	struct item_bg_state
	{
		ImVec4 background, stroke;
	};

	struct description_state
	{
		ImVec4 hint_rect_col, hint_text_col;
		bool is_hinted;
	};
	void ItemGrab(ImGuiID id, ImRect bb, bool hovered)
	{
		static std::map<ImGuiID, item_bg_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, item_bg_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.background = ImLerp(it_anim->second.background,
			hovered ? ImColor(1.f, 1.f, 1.f, 1.f) : ImColor(0.75f, 0.75f, 0.75f, 1.f),
			c::anim::speed);

		it_anim->second.stroke = ImLerp(it_anim->second.stroke,
			hovered ? ImColor(0.f, 0.f, 0.f, 0.25f) : ImColor(0.f, 0.f, 0.f, 0.15f),
			c::anim::speed);

		ImDrawList* draw = ImGui::GetWindowDrawList();

		// Отрисовка фона и рамки

		draw->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.background), c::elements::rounding);
		draw->AddRect(bb.Min, bb.Max, GetColorU32(it_anim->second.stroke), c::elements::rounding);

		// === РЁБРА (3 вертикальные полоски) ===
		const float padding_x = 3.0f;
		const float padding_y = 2.0f;
		const float line_width = 2.0f;
		const float spacing = 3.0f;

		const float total_width = (3 * line_width) + (2 * spacing); // 3 линии + 2 пробела
		const float start_x = bb.Min.x + (bb.GetWidth() - total_width) / 2.0f;
		const float top = bb.Min.y + padding_y;
		const float bottom = bb.Max.y - padding_y;
		const ImU32 line_color = ImColor(0.f, 0.f, 0.f, 0.35f);

		for (int i = 0; i < 3; ++i)
		{
			float x = start_x + i * (line_width + spacing);
			draw->AddRectFilled(
				ImVec2(x, top),
				ImVec2(x + line_width, bottom),
				line_color,
				1.0f
			);
		}
	}



	void ItemDescription(const char* text, ImGuiID id, ImRect bb, bool hovered, ImVec2 pos, ImVec4 description_col)
	{
		static std::map<ImGuiID, description_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, description_state() });
			it_anim = anim.find(id);
		}


		if (hovered) {
			if (!it_anim->second.is_hinted)
			{
				static DWORD dwTickStart = GetTickCount();
				if (GetTickCount() - dwTickStart > 800)
				{
					it_anim->second.is_hinted = true;
					dwTickStart = GetTickCount();
				}
			}
		}
		else
			it_anim->second.is_hinted = false;


		it_anim->second.hint_rect_col = ImLerp(it_anim->second.hint_rect_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(window_bg_color, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(window_bg_color, 0.f), GetAnimSpeed() / 3);
		it_anim->second.hint_text_col = ImLerp(it_anim->second.hint_text_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(label::default, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(label::default, 0.f), GetAnimSpeed() / 3);

		ImGui::PushClipRect(pos, bb.Max, true);


		PushFont(font::description_font);
		ImRect descrtiption_bb(pos, pos + CalcTextSize(text));

		// Render SV Square
		const int vtx_idx_0 = GetWindowDrawList()->VtxBuffer.Size;
		GetWindowDrawList()->AddText(descrtiption_bb.Min, GetColorU32(description_col), text);
		const int vtx_idx_1 = GetWindowDrawList()->VtxBuffer.Size;
		ShadeVertsLinearColorGradientSetAlpha(GetWindowDrawList(), vtx_idx_0, vtx_idx_1, bb.Min, bb.Max, GetColorU32(description_col), utils::GetColorWithAlpha(description_col, 0.f));

		if (hovered) {
			if (!it_anim->second.is_hinted)
			{
				static DWORD dwTickStart = GetTickCount();
				if (GetTickCount() - dwTickStart > 800)
				{
					it_anim->second.is_hinted = true;
					dwTickStart = GetTickCount();
				}
			}
		}
		else
			it_anim->second.is_hinted = false;


		it_anim->second.hint_rect_col = ImLerp(it_anim->second.hint_rect_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(window_bg_color, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(window_bg_color, 0.f), GetAnimSpeed());
		it_anim->second.hint_text_col = ImLerp(it_anim->second.hint_text_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(label::default, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(label::default, 0.f), GetAnimSpeed());


		ImGui::PopClipRect();

		for (int i = 0; i < 3; i++)
			GetForegroundDrawList()->AddShadowRect(descrtiption_bb.Min - ImVec2(4, 4), descrtiption_bb.Max + ImVec2(4, 4), GetColorU32(it_anim->second.hint_rect_col), 35.f, ImVec2(0, 0), 0, 2.f);

		GetForegroundDrawList()->AddRectFilled(descrtiption_bb.Min - ImVec2(4, 4), descrtiption_bb.Max + ImVec2(4, 4), GetColorU32(it_anim->second.hint_rect_col), 2.f);
		GetForegroundDrawList()->AddText(descrtiption_bb.Min, GetColorU32(it_anim->second.hint_text_col), text);

		PopFont();


	}


	enum KeybindStatus {
		KEYBIND_NONE = 0,
		KEYBIND_WAITING,
		KEYBIND_ASSIGNED
	};

	struct key_state {
		ImVec4 background, text, icon_col;
		ImVec4 text_color, description_col;
		float alpha = 0.f;
		float size_x = 0.f;
		int status = KEYBIND_NONE;
		const char* key_name;
		bool clicked_to_waiting = false; // Флаг для предотвращения моментального захвата Mouse1


		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	struct child_state
	{
		bool active = true;
		std::string name2;

		ImVec2 label_size;
		ImVec2 desc_size;
		ImVec2 total_text_size;

		float check_delta;
		float size_y;
		float bg_alpha;
	};

	bool ChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags flags, const char* icon)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* parent_window = g.CurrentWindow;
		flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
		flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);
		if (parent_window->DC.IsSameLine)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 30);

		static std::map<ImGuiID, child_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, child_state() });
			it_anim = anim.find(id);
		}


		ImVec2 content_avail = GetContentRegionAvail();
		ImVec2 size = ImFloor(ImVec2(size_arg.x, it_anim->second.size_y)) + ImVec2(0, cap ? 30 : 0);

		int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0);

		if (size.x <= 0.0f)
			size.x = ImMax(content_avail.x + size.x, 4.0f);
		if (size.y <= 0.0f)
			size.y = ImMax((content_avail.y) + it_anim->second.size_y, 4.0f);

		float target_size = it_anim->second.active ? size_arg.y : 26;
		it_anim->second.size_y = ImLerp(it_anim->second.size_y, target_size, GetAnimSpeed());

		SetNextWindowPos(parent_window->DC.CursorPos + ImVec2(0, cap ? 55 : 0));
		SetNextWindowSize(size - ImVec2(0, cap ? 55 : 0));
		GetWindowDrawList()->AddRectFilled(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size + ImVec2(0, 2), GetColorU32(c::child::background), c::child::rounding);
		
		GetWindowDrawList()->AddRectFilled(parent_window->DC.CursorPos, parent_window->DC.CursorPos + ImVec2(size.x, 55), c::anim::default, c::child::rounding);

		it_anim->second.check_delta = ImLerp(it_anim->second.check_delta, it_anim->second.active ? 1.f : 0.f, GetAnimSpeed());

		ImRect cap_bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + ImVec2(size.x, 55));
		ImRect check_bb(ImVec2(cap_bb.Max.x - 50, cap_bb.GetCenter().y - 10), ImVec2(ImVec2(cap_bb.Max.x - 20, cap_bb.GetCenter().y + 10)));

		GetWindowDrawList()->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32(c::window_bg_color.Value), 36);
		fade_rect_filled(GetWindowDrawList(), check_bb.Min, check_bb.Max, utils::GetColorWithAlpha(c::anim::active, it_anim->second.check_delta), utils::GetColorWithAlpha(c::dark_color, it_anim->second.check_delta), fade_direction::diagonally, 36.f, 0, true);
		//GetWindowDrawList()->AddRectFilled(check_bb.Min, check_bb.Max, utils::GetColorWithAlpha(c::anim::active, it_anim->second.check_delta), 36);
		GetWindowDrawList()->AddRect(check_bb.Min, check_bb.Max, GetColorU32(c::stroke_color.Value), 36);

		GetWindowDrawList()->AddShadowCircle(ImVec2(check_bb.Min.x + 10 + ((check_bb.GetSize().x - 20) * it_anim->second.check_delta), check_bb.GetCenter().y), 5.f, ImColor(0.f, 0.f, 0.f, 0.6f), 15.f, ImVec2(0, 0), 0, 36);
		GetWindowDrawList()->AddCircleFilled(ImVec2(check_bb.Min.x + 10 + ((check_bb.GetSize().x - 20) * it_anim->second.check_delta), check_bb.GetCenter().y), 5.f, ImColor(0.9f, 0.9f, 0.9f, 0.5f + (0.5f * it_anim->second.check_delta)), 36);

		GetWindowDrawList()->AddRect(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size + ImVec2(0, 2), GetColorU32(c::child::stroke), c::child::rounding);

		if (check_bb.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
			it_anim->second.active = !it_anim->second.active;

		std::string s = name;
		std::string label, description;
		size_t p = s.find(':');
		if (p != std::string::npos)
		{
			label = s.substr(0, p);
			description = s.substr(p + 1);
		}
		else
		{
			label = s;
		}
		auto trim = [](std::string& str) {
			size_t b = str.find_first_not_of(' '), e = str.find_last_not_of(' ');
			if (b == std::string::npos) str.clear();
			else str = str.substr(b, e - b + 1);
			};
		trim(label);
		trim(description);

		PushFont(font::inter_bold);
		GetWindowDrawList()->AddText(parent_window->DC.CursorPos + ImVec2(GetStyle().WindowPadding) - ImVec2(-42.5f, 5), utils::GetColorWithAlpha(c::label::active, ImGui::GetStyle().Alpha), label.c_str());
		PopFont();



		if (!description.empty())
			GetWindowDrawList()->AddText(parent_window->DC.CursorPos + ImVec2(GetStyle().WindowPadding) + ImVec2(42.5f, 13), utils::GetColorWithAlpha(c::label::default, ImGui::GetStyle().Alpha), description.c_str());

		PushFont(font::inter_semibold);

		fade_text(GetWindowDrawList(), icon, utils::center_text(parent_window->DC.CursorPos, parent_window->DC.CursorPos + ImVec2(55, 55), icon) + ImVec2(0, 3.f), c::anim::active, c::dark_color, 0);
		//GetWindowDrawList()->AddText(utils::center_text(parent_window->DC.CursorPos, parent_window->DC.CursorPos + ImVec2(55, 55), icon) + ImVec2(0, 3.f), utils::GetColorWithAlpha(c::anim::active, ImGui::GetStyle().Alpha), icon);
		PopFont();



		const char* temp_window_name;

		if (name)
			ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
		else
			ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);
		bool ret = Begin(temp_window_name, NULL, flags | ImGuiWindowFlags_NoBackground);
		ImGuiWindow* child_window = g.CurrentWindow;
		child_window->ChildId = id;
		child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;
		if (child_window->BeginCount == 1)
			parent_window->DC.CursorPos = child_window->Pos;
		ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
		if (g.ActiveId == temp_id_for_activation)
			ClearActiveID();

		if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
		{
			FocusWindow(child_window);
			NavInitWindow(child_window, false);
			SetActiveID(temp_id_for_activation, child_window);
			g.ActiveIdSource = g.NavInputSource;
		}
		return ret;
	}

	bool Child(const char* str_id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags extra_flags, bool space, const char* icon)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (cap) {
			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13, 13));
			PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(13, 9));
		}

		if (space)
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, 30));

		return ChildEx(str_id, window->GetID(str_id), size_arg, cap, extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding, icon);
	}

	bool ChildID(ImGuiID id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags extra_flags)
	{
		IM_ASSERT(id != 0);
		return ChildEx(NULL, id, size_arg, cap, extra_flags);
	}

	void EndChild()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		PopStyleVar(2);

		IM_ASSERT(g.WithinEndChild == false);
		IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);

		g.WithinEndChild = true;
		if (window->BeginCount > 1)
		{
			End();
		}
		else
		{
			ImVec2 sz = window->Size;

			if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) sz.x = ImMax(4.0f, sz.x);
			if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y)) sz.y = ImMax(4.0f, sz.y);

			End();

			ImGuiWindow* parent_window = g.CurrentWindow;
			ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
			ItemSize(sz);
			if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavWindowHasScrollY) && !(window->Flags & ImGuiWindowFlags_NavFlattened))
			{
				ItemAdd(bb, window->ChildId);
			}
			else
			{
				ItemAdd(bb, 0);

				if (window->Flags & ImGuiWindowFlags_NavFlattened) parent_window->DC.NavLayersActiveMaskNext |= window->DC.NavLayersActiveMaskNext;
			}
			if (g.HoveredWindow == window) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
		}
		g.WithinEndChild = false;
		g.LogLinePosY = -FLT_MAX;
	}


	void BeginGroup()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		g.GroupStack.resize(g.GroupStack.Size + 1);
		ImGuiGroupData& group_data = g.GroupStack.back();
		group_data.WindowID = window->ID;
		group_data.BackupCursorPos = window->DC.CursorPos;
		group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
		group_data.BackupIndent = window->DC.Indent;
		group_data.BackupGroupOffset = window->DC.GroupOffset;
		group_data.BackupCurrLineSize = window->DC.CurrLineSize;
		group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
		group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
		group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
		group_data.BackupActiveIdPreviousFrameIsAlive = g.ActiveIdPreviousFrameIsAlive;
		group_data.EmitItem = true;

		window->DC.GroupOffset.x = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
		window->DC.Indent = window->DC.GroupOffset;
		window->DC.CursorMaxPos = window->DC.CursorPos;
		window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);
		if (g.LogEnabled) g.LogLinePosY = -FLT_MAX;
	}

	void EndGroup()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		IM_ASSERT(g.GroupStack.Size > 0);

		ImGuiGroupData& group_data = g.GroupStack.back();
		IM_ASSERT(group_data.WindowID == window->ID);

		if (window->DC.IsSetPos) ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

		ImRect group_bb(group_data.BackupCursorPos, ImMax(window->DC.CursorMaxPos, group_data.BackupCursorPos));

		window->DC.CursorPos = group_data.BackupCursorPos;
		window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
		window->DC.Indent = group_data.BackupIndent;
		window->DC.GroupOffset = group_data.BackupGroupOffset;
		window->DC.CurrLineSize = group_data.BackupCurrLineSize;
		window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
		if (g.LogEnabled) g.LogLinePosY = -FLT_MAX;

		if (!group_data.EmitItem)
		{
			g.GroupStack.pop_back();
			return;
		}

		window->DC.CurrLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrLineTextBaseOffset);
		ItemSize(group_bb.GetSize());
		ItemAdd(group_bb, 0, NULL, ImGuiItemFlags_NoTabStop);

		const bool group_contains_curr_active_id = (group_data.BackupActiveIdIsAlive != g.ActiveId) && (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
		const bool group_contains_prev_active_id = (group_data.BackupActiveIdPreviousFrameIsAlive == false) && (g.ActiveIdPreviousFrameIsAlive == true);
		if (group_contains_curr_active_id) g.LastItemData.ID = g.ActiveId;
		else if (group_contains_prev_active_id) g.LastItemData.ID = g.ActiveIdPreviousFrame;
		g.LastItemData.Rect = group_bb;

		const bool group_contains_curr_hovered_id = (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
		if (group_contains_curr_hovered_id) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;

		if (group_contains_curr_active_id && g.ActiveIdHasBeenEditedThisFrame) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDeactivated;
		if (group_contains_prev_active_id && g.ActiveId != g.ActiveIdPreviousFrame) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Deactivated;

		g.GroupStack.pop_back();
	}




	void Separator_line()
	{
		GetWindowDrawList()->AddRectFilled(GetCursorScreenPos(), GetCursorScreenPos() + ImVec2(GetContentRegionMax().x - GetStyle().WindowPadding.x, 1), GetColorU32(c::separator));
		Spacing();
	}

	void SeparatorEx(ImGuiSeparatorFlags flags, float thickness)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return;

		ImGuiContext& g = *GImGui;
		IM_ASSERT(ImIsPowerOfTwo(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)));
		IM_ASSERT(thickness > 0.0f);

		if (flags & ImGuiSeparatorFlags_Vertical)
		{
			float y1 = window->DC.CursorPos.y;
			float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
			const ImRect bb(ImVec2(window->DC.CursorPos.x, y1 + (GetStyle().ItemSpacing.y / 2)), ImVec2(window->DC.CursorPos.x + thickness, y2 - (GetStyle().ItemSpacing.y / 2)));


			ItemSize(ImVec2(thickness, 0.0f));
			if (!ItemAdd(bb, 0)) return;

			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::child::background));

			ImGui::SameLine();
		}
		else if (flags & ImGuiSeparatorFlags_Horizontal)
		{
			float x1 = window->Pos.x;
			float x2 = window->Pos.x + window->Size.x;

			if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID) x1 += window->DC.Indent.x;

			if (ImGuiTable* table = g.CurrentTable)
			{
				x1 = table->Columns[table->CurrentColumn].MinX;
				x2 = table->Columns[table->CurrentColumn].MaxX;
			}

			ImGuiOldColumns* columns = (flags & ImGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
			if (columns) PushColumnsBackground();

			const float thickness_for_layout = (thickness == 1.0f) ? 0.0f : thickness;
			const ImRect bb(ImVec2(x1 + GetStyle().WindowPadding.x, window->DC.CursorPos.y), ImVec2(x2 - GetStyle().WindowPadding.x, window->DC.CursorPos.y + thickness));

			ItemSize(ImVec2(0.0f, thickness_for_layout));

			if (ItemAdd(bb, 0))
			{
				window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::separator));
			}
			if (columns)
			{
				PopColumnsBackground();
				columns->LineMinY = window->DC.CursorPos.y;
			}
		}
	}

	void Separator()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems) return;

		ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
		flags |= ImGuiSeparatorFlags_SpanAllColumns;
		SeparatorEx(flags, 1.0f);
	}

	struct theme_state
	{
		ImVec4 background;
		float smooth_swap, alpha_line, line_size;
	};

	bool ThemeButton(const char* id_theme, bool dark, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(id_theme);
		const ImVec2 label_size = CalcTextSize(id_theme, NULL, true), pos = window->DC.CursorPos;

		ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

		const ImRect bb(pos, pos + size);

		ItemSize(size, 0.f);
		if (!ItemAdd(bb, id)) return false;

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

		static std::map<ImGuiID, theme_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, theme_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.background = ImLerp(it_anim->second.background, dark || hovered ? c::page::background_active : c::page::background, g.IO.DeltaTime * 6.f);

		it_anim->second.alpha_line = ImLerp(it_anim->second.alpha_line, dark ? 1.f : 0.f, g.IO.DeltaTime * 6.f);
		it_anim->second.line_size = ImLerp(it_anim->second.line_size, dark ? (size_arg.x / 4) : (size_arg.x / 2), g.IO.DeltaTime * 6.f);

		it_anim->second.smooth_swap = ImLerp(it_anim->second.smooth_swap, dark ? 26.f : 0, g.IO.DeltaTime * 12.f);

		GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.background), c::page::rounding);

		PushClipRect(bb.Min, bb.Max, true);

		PushFont(font::icomoon_page);
		GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size_arg.x - CalcTextSize("k").x) / 2, bb.Max.y - CalcTextSize("k").y - (size.y - CalcTextSize("k").y) / 2 + it_anim->second.smooth_swap), GetColorU32(c::anim::active.Value), "k");
		GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size_arg.x - CalcTextSize("a").x) / 2, bb.Max.y - CalcTextSize("a").y - (size.y - CalcTextSize("a").y) / 2 - 25 + it_anim->second.smooth_swap), GetColorU32(c::anim::active.Value), "a");
		PopFont();

		PopClipRect();

		return pressed;
	}

	struct rotated_text_state
	{
		float hover_anim, scale_offset;
		ImVec4 text;


		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool rotated_text(const char* label, bool active)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, rotated_text_state> anim;
		auto& state = anim[id];

		const ImVec2 pos = window->DC.CursorPos + ImVec2(state.total_offset, 0);
		ImVec2 size = ImVec2(20, 20);



		const ImRect bb(pos, pos + size);
		ItemSize(size, 0.f);
		ItemAdd(bb, id);

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);

		float dt = g.IO.DeltaTime;


		state.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {

			state.total_offset = GetAnimOffset();
			state.total_alpha = 0.f;
			state.timer = 0.f;
		}
		state.timer += ImGui::GetIO().DeltaTime;
		float relative = (bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((state.timer - delay) / 0.3f, 0.f, 1.f);
		state.total_offset = ImLerp(state.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		state.total_alpha = ImLerp(state.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);


		state.text = easing->easing_in_out<ImVec4>(id * 1, active, c::label::default, c::label::active, dt * 20.f, interpolation_type::back);

		state.scale_offset = easing->easing_in_out<float>(id * 2, active || hovered, 0.8f, 1.f, dt * 20.f, interpolation_type::back);

		state.hover_anim = ImLerp(state.hover_anim, hovered || active ? IM_PI : IM_PI / 2, dt * 9.f);

		ImRotateStart();
		ImGui::GetWindowDrawList()->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * state.scale_offset, bb.GetCenter() - (CalcTextSize(label) * state.scale_offset) / 2 + ImVec2(0, 1), utils::GetColorWithAlpha(state.text, state.total_alpha), label);
		ImRotateEnd(state.hover_anim);

		return pressed;
	}


	struct button_state
	{
		float t;
		float press_anim;
		float hover_anim;
		float gradient_alpha;
		ImVec4 background, text;

		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool InActiveButton(const char* label, const ImVec2& size_arg)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, button_state> anim;
		auto& state = anim[id];

		const ImVec2 pos = window->DC.CursorPos;
		ImVec2      size = size_arg;
		ImRect      bb(pos, pos + size);
		ItemSize(size, 0.f);
		ItemAdd(bb, id);

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);
		float dt = g.IO.DeltaTime;

		// Анимация текста (цвет) при hover/hold
		state.text = easing->easing_in_out<ImVec4>(
			id * 2,
			hovered || held,
			c::label::default,
			c::label::active,
			dt * 20.f,
			interpolation_type::back
		);

		// Анимация нажатия (scale)
		state.press_anim = easing->easing_out<float>(
			id * 3,
			0.f,
			pressed ? 1.f : 0.f,
			dt * 30.f,
			interpolation_type::expo
		);

		// Анимация hover (градиент)
		state.hover_anim = easing->easing_in_out<float>(
			id * 4,
			pressed ? 1.2f : (hovered ? 1.f : 0.f),
			0.45f,
			1.f,
			dt * 23.f,
			interpolation_type::back
		);
		state.gradient_alpha = ImLerp(state.gradient_alpha, hovered ? 1.f : 0.f, dt * 12.f);

		ImDrawList* dl = GetWindowDrawList();
		ImVec2      center = pos + size * 0.5f;

		// Подсчет scale для эффекта нажатия
		float    scale = 1.f - state.press_anim * 0.08f;
		ImVec2   anim_size = size * scale;
		ImVec2   box_min = center - anim_size * 0.5f;
		ImVec2   box_max = center + anim_size * 0.5f;

		// Появление/исчезновение кнопки при смене вкладок
		state.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35)
		{
			state.total_offset = GetAnimOffset();
			state.total_alpha = 0.f;
			state.timer = 0.f;
		}
		state.timer += dt;
		float relative = (center.y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((state.timer - delay) / 0.3f, 0.f, 1.f);
		state.total_offset = ImLerp(state.total_offset, 0.f, dt * t * 4);
		state.total_alpha = ImLerp(state.total_alpha, ImGui::GetStyle().Alpha, dt * t * 3);

		dl->AddRectFilled(
			box_min,
			box_max,
			utils::GetColorWithAlpha(c::anim::default, 0.5f),
			c::page::rounding
		);

		// Градиент при hover (опционально оставляем)
		if (state.hover_anim > 0.001f)
		{
			ImVec2 grad_size = anim_size * state.hover_anim;
			ImVec2 grad_offset = grad_size * 0.5f;
			ImVec2 grad_min = center - grad_offset;
			ImVec2 grad_max = center + grad_offset;

			fade_rect_filled(
				dl,
				grad_min,
				grad_max,
				utils::GetColorWithAlpha(c::anim::active.Value, state.gradient_alpha),
				utils::GetColorWithAlpha(c::dark_color.Value, state.gradient_alpha),
				fade_direction::horizontally,
				c::elements::rounding,
				0,
				true
			);
		}

		// Текст кнопки: анимированный цвет
		PushClipRect(bb.Min, bb.Max, true);
		ImVec2 text_size = CalcTextSize(label);
		ImVec2 text_pos = center - text_size * 0.5f;
		dl->AddText(text_pos, ImGui::GetColorU32(state.text), label);
		PopClipRect();

		// Эффект частиц на нажатие
		if (pressed)
		{
			for (int i = 0; i < particle::PARTICLE_COUT; ++i)
				particle::AddParticle(center, ImVec2(3, 3) / 10, 0);
		}

		return pressed;
	}


	bool Button(const char* label, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, button_state> anim;
		auto& state = anim[id];

		const ImVec2 pos = window->DC.CursorPos + ImVec2(state.total_offset, 0.f);
		ImVec2 size = size_arg;

		const ImRect bb(pos, pos + size);
		ItemSize(size, 0.f);
		ItemAdd(bb, id);

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);


		float dt = g.IO.DeltaTime;


		state.text = easing->easing_in_out<ImVec4>(id * 2, hovered || held, c::label::default, c::label::active, dt * 20.f, interpolation_type::back);

		// FIX: bool → float
		state.press_anim = easing->easing_out<float>(id * 3, 0.f, pressed ? 1.f : 0.f, dt * 30.f, interpolation_type::expo);

		// Hover anim (0 → 1)

		state.hover_anim = easing->easing_in_out<float>(id * 4, pressed ? 1.2f : hovered ? 1.f : 0.f, 0.45f, 1.f, dt * 23.f, interpolation_type::back);

		state.gradient_alpha = ImLerp(state.gradient_alpha, hovered ? 1.f : 0.f, dt * 12.f);

		ImDrawList* draw = GetWindowDrawList();
		ImVec2 center = pos + size * 0.5f;

		float scale = 1.f - state.press_anim * 0.08f;
		ImVec2 anim_size = size * scale;
		ImVec2 box_min = center - anim_size * 0.5f;
		ImVec2 box_max = center + anim_size * 0.5f;

		state.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35)
		{
			state.total_offset = GetAnimOffset();
			state.total_alpha = 0.f;
			state.timer = 0.f;
		}
		state.timer += ImGui::GetIO().DeltaTime;
		float relative = (center.y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((state.timer - delay) / 0.3f, 0.f, 1.f);
		state.total_offset = ImLerp(state.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		state.total_alpha = ImLerp(state.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);


		PushStyleVar(ImGuiStyleVar_Alpha, state.total_alpha); {


			draw->AddRectFilled(box_min, box_max, GetColorU32(c::anim::default.Value), c::page::rounding);

			// FIX: Увеличение градиента по X и Y
			if (state.hover_anim > 0.001f)
			{
				ImVec2 gradient_size = anim_size * state.hover_anim;
				ImVec2 gradient_offset = gradient_size * 0.5f;
				ImVec2 grad_min = center - gradient_offset;
				ImVec2 grad_max = center + gradient_offset;

				fade_rect_filled(
					draw,
					grad_min,
					grad_max,
					utils::GetColorWithAlpha(c::anim::active.Value, state.gradient_alpha),
					utils::GetColorWithAlpha(c::dark_color.Value, state.gradient_alpha),
					fade_direction::horizontally,
					c::elements::rounding,
					0,
					true);
			}

			PushClipRect(bb.Min, bb.Max, true);
			ImVec2 text_size = CalcTextSize(label);
			ImVec2 text_pos = center - text_size * 0.5f;
			draw->AddText(text_pos, GetColorU32(state.text), label);
			PopClipRect();

			if (pressed)
			{
				for (int i = 0; i < particle::PARTICLE_COUT; ++i)
					particle::AddParticle(center, ImVec2(3, 3) / 10, 0);
			}
		}ImGui::PopStyleVar();

		//particle::RenderEffects(draw, size, ImGui::GetTime());

		return pressed;
	}


	struct tab_state {
		ImVec4 text_col, icon_col, dark_col;
		ImVec4 frame_col;
		ImVec4 line_col;
		bool is_want = true;
		float icon_size;
		ImFont* icon_font;
		float text_offset;
	};

	bool Tab(const char* label, const char* icon, int* v, int number) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(ImGui::GetContentRegionAvail().x, 45));


		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, tab_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end()) {
			anim.insert({ id, tab_state() });
			it_anim = anim.find(id);
		}

		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) {
			it_anim->second.is_want = true;
		}

		if (it_anim->second.is_want) {

			//*v = number;

			it_anim->second.is_want = false;
		}
		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, *v == number ? utils::GetColorWithAlpha(c::anim::active, 0.3f) : hovered ? utils::GetColorWithAlpha(c::anim::active, 0.15f) : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it_anim->second.dark_col = ImLerp(it_anim->second.dark_col, *v == number ? utils::GetColorWithAlpha(c::dark_color, 0.05f) : utils::GetColorWithAlpha(c::dark_color, 0.f), GetAnimSpeed());

		it_anim->second.text_col = ImLerp(it_anim->second.text_col, *v == number ? c::text::label::active : hovered ? c::text::label::hovered : c::label::default, GetAnimSpeed());

		it_anim->second.text_offset = ImLerp(it_anim->second.text_offset, *v == number ? -7.f : 5.f, GetAnimSpeed());
		it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, *v == number ? c::anim::active : hovered ? utils::GetColorWithAlpha(c::anim::active, 0.8f) : utils::GetColorWithAlpha(c::anim::active, 0.5f), GetAnimSpeed());

		//it_anim->second.icon_size = ImLerp(it_anim->second.icon_size, hovered ? font::regular_l->FontSize : font::regular_l->FontSize - 10.f, ImGui::GetIO().DeltaTime * 10.f);

		it_anim->second.icon_size = easing->easing_in_out<float>(id * 15, (*v == number), font::regular_l->FontSize - 10.f, font::regular_l->FontSize, GetIO().DeltaTime * 15.f, interpolation_type::back);



		ImVec2 font_sz_offset(it_anim->second.icon_size - font::regular_l->FontSize, it_anim->second.icon_size - font::regular_l->FontSize);



		PushFont(font::regular_m);
		fade_text(window->DrawList, icon, ImVec2(total_bb.Min.x + 10, utils::center_text(total_bb.Min, total_bb.Max, icon).y), c::anim::active, c::dark_color, 0);
		//window->DrawList->AddText(ImVec2(total_bb.Min.x + 10, utils::center_text(total_bb.Min, total_bb.Max, icon).y), c::anim::active, icon);
		PopFont();



		window->DrawList->AddRectFilledMultiColor(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), utils::GetColorWithAlpha(c::dark_color, 0.f), utils::GetColorWithAlpha(c::dark_color, 0.f), GetColorU32(it_anim->second.frame_col), 4.f);
		//fade_rect_filled(window->DrawList, total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.dark_col), GetColorU32(it_anim->second.frame_col), fade_direction::vertically, 5.f, 0m)
		//window->DrawList->AddShadowRect(ImVec2(total_bb.Min.x, total_bb.Min.y ), total_bb.Max, GetColorU32(it_anim->second.frame_col), 25.f, ImVec2(0,0), ImDrawFlags_ShadowCutOutShapeBackground);
		//window->DrawList->AddRectFilled(ImVec2(total_bb.Min.x, total_bb.Max.y - 2), total_bb.Max, GetColorU32(it_anim->second.frame_col));

		window->DrawList->AddText(ImVec2(total_bb.Min.x + 40.f, utils::center_text(total_bb.Min, total_bb.Max, label).y), GetColorU32(it_anim->second.text_col), label);

		return pressed;
	}


	bool SquareTab(const char* label, const char* icon, int* v, int number) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(82, ImGui::GetContentRegionAvail().y - 8));


		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, tab_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end()) {
			anim.insert({ id, tab_state() });
			it_anim = anim.find(id);
		}

		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) {
			it_anim->second.is_want = true;
		}

		if (it_anim->second.is_want) {

			//*v = number;

			it_anim->second.is_want = false;
		}
		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, *v == number ? utils::GetColorWithAlpha(c::anim::active, 0.3f) : hovered ? utils::GetColorWithAlpha(c::anim::active, 0.15f) : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it_anim->second.dark_col = ImLerp(it_anim->second.dark_col, *v == number ? utils::GetColorWithAlpha(c::dark_color, 0.05f) : utils::GetColorWithAlpha(c::dark_color, 0.f), GetAnimSpeed());

		it_anim->second.text_col = ImLerp(it_anim->second.text_col, *v == number ? utils::GetColorWithAlpha(c::anim::active, 0.f) : hovered ? c::text::label::hovered : c::label::default, GetAnimSpeed());

		it_anim->second.text_offset = ImLerp(it_anim->second.text_offset, *v == number ? -7.f : 5.f, GetAnimSpeed());
		it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, *v == number ? c::anim::active : hovered ? utils::GetColorWithAlpha(c::anim::active, 0.8f) : utils::GetColorWithAlpha(c::anim::active, 0.5f), GetAnimSpeed());

		//it_anim->second.icon_size = ImLerp(it_anim->second.icon_size, hovered ? font::regular_l->FontSize : font::regular_l->FontSize - 10.f, ImGui::GetIO().DeltaTime * 10.f);

		it_anim->second.icon_size = easing->easing_in_out<float>(id * 15, (*v == number), font::regular_l->FontSize - 10.f, font::regular_l->FontSize, GetIO().DeltaTime * 15.f, interpolation_type::back);


		ImVec2 font_sz_offset(it_anim->second.icon_size - font::regular_l->FontSize, it_anim->second.icon_size - font::regular_l->FontSize);


		PushFont(font::regular_l);
		window->DrawList->AddText(font::regular_l, it_anim->second.icon_size,  utils::center_text(total_bb.Min, total_bb.Max, icon) - ImVec2(0, it_anim->second.text_offset) - font_sz_offset / 2, GetColorU32(it_anim->second.icon_col), icon);
		PopFont();


		window->DrawList->AddRectFilledMultiColor(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(it_anim->second.frame_col, 0.f), utils::GetColorWithAlpha(it_anim->second.frame_col, 0.f), GetColorU32(it_anim->second.frame_col), GetColorU32(it_anim->second.frame_col), 5);
		//fade_rect_filled(window->DrawList, total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.dark_col), GetColorU32(it_anim->second.frame_col), fade_direction::vertically, 5.f, 0m)
		//window->DrawList->AddShadowRect(ImVec2(total_bb.Min.x, total_bb.Min.y ), total_bb.Max, GetColorU32(it_anim->second.frame_col), 25.f, ImVec2(0,0), ImDrawFlags_ShadowCutOutShapeBackground);
		
		window->DrawList->AddText(utils::center_text(total_bb.Min, total_bb.Max, label) + ImVec2(0, 20),
			GetColorU32(it_anim->second.text_col),
			label);

		return pressed;
	}

	struct subtab_state {
		ImVec4 text_col;
		ImVec4 frame_col;
		ImVec4 line_col;
	};

	bool SubTab(const char* label, int* v, int number, ImColor color)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(20 + CalcTextSize(label).x, 25));


		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, subtab_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, subtab_state() });
			it_anim = anim.find(id);
		}

		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
		{
			*v = number;
		}

		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, *v == number ? color : utils::GetColorWithAlpha(color, 0.f), GetAnimSpeed());
		it_anim->second.text_col = ImLerp(it_anim->second.text_col, *v == number ? color : c::label::active, GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), style.FrameRounding);

		window->DrawList->AddRect(total_bb.Min, total_bb.Max, color, style.FrameRounding);

		window->DrawList->AddText(utils::center_text(total_bb.Min, total_bb.Max, label), c::label::active, label);

		return pressed;
	}

	struct arrow_state {
		ImVec4 arrow_col;
		ImVec4 frame_col;
	};

	bool ArrowButton(const char* label, ImGuiDir dir)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(20, 20));

		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, arrow_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, arrow_state() });
			it_anim = anim.find(id);
		}

		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, hovered ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it_anim->second.arrow_col = ImLerp(it_anim->second.arrow_col, hovered ? utils::GetDarkColor(c::anim::active) : c::anim::active, GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), c::elements::rounding);
		window->DrawList->AddRect(total_bb.Min, total_bb.Max, stroke_color, c::elements::rounding);

		PushFont(font::icomoon_page);
		window->DrawList->AddText(utils::center_text(total_bb.Min, total_bb.Max, dir == ImGuiDir_Right ? "r" : dir == ImGuiDir_Left ? "l" : dir == ImGuiDir_Up ? "u" : "d"), GetColorU32(it_anim->second.arrow_col), dir == ImGuiDir_Right ? "r" : dir == ImGuiDir_Left ? "l" : dir == ImGuiDir_Up ? "u" : "d");
		PopFont();

		return IsItemClicked();
	}
	
	struct check_state
	{
		float t;
		float check_t;
		float bg_anim;
		float gradient_anim;
		float hover_anim;
		ImVec4 bg_color;
		ImVec4 text_color;
		ImVec4 check_color;
		ImRect check_bb;
		bool hover_flags = false;

		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};


	bool Checkbox(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if(window->SkipItems && tab_want_to_change) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(label);

		const float box_size = 20.0f;
		const float spacing = 28.0f;
		const ImVec2 label_size = ImGui::CalcTextSize(label);
		const ImVec2 pos = window->DC.CursorPos;

		static std::map<ImGuiID, check_state> anim;
		auto& state = anim[id];

		const ImRect total_bb(pos + ImVec2(state.total_offset, 0), pos + ImVec2(state.total_offset, 0)  + ImVec2(box_size + spacing / 2 + label_size.x, box_size));
		ItemSize(total_bb, 0.0f);
		ItemAdd(total_bb, id);

		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);


		float dt = ImGui::GetIO().DeltaTime;

		state.t = ImClamp(state.t + ((*v ? 1.f : 0.f) - state.t) * dt * 36.f, 0.f, 1.f);
		state.check_t = *v
			? easing->easing_out<float>(id * 13, 0.0f, 1.0f, dt * 40.f, interpolation_type::back)
			: easing->easing_in<float>(id * 13, 1.0f, 0.0f, dt * 40.f, interpolation_type::back);
		state.bg_color = easing->easing_in_out<ImVec4>(id * 2, *v, c::anim::default, c::anim::active, dt * 30.f, interpolation_type::back);
		state.text_color = ImLerp(state.text_color, held ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());

		state.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			state.total_offset = GetAnimOffset();
			state.total_alpha = 0.f;
			state.timer = 0.f;
		}
		
		state.timer += ImGui::GetIO().DeltaTime;
		float relative = (total_bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((state.timer - delay) / 0.3f, 0.f, 1.f);
		state.total_offset = ImLerp(state.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);

		state.total_alpha = ImLerp(state.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);

		PushStyleVar(ImGuiStyleVar_Alpha, state.total_alpha);
		{
			state.check_color = easing->easing_in_out<ImVec4>(id * 4, *v, ImVec4(1, 1, 1, 0.0f), ImVec4(1, 1, 1, 1.0f), dt * 30.f, interpolation_type::back);
			state.gradient_anim = easing->easing_in_out<float>(id * 15, *v, 0.0f, 1.0f, dt * 40.f, interpolation_type::back);

			if (state.timer > 900.f && state.is_visible)
				state.can_view = true;

			// Анимация размера квадрата (hover работает только если вкл)
			state.bg_anim = *v ? easing->easing_in_out<float>(id * 6, hovered, 0.0f, 1.0f, dt * 20.f, interpolation_type::back) : 0.0f;

			ImDrawList* draw = ImGui::GetWindowDrawList();
			ImVec2 center = total_bb.Min + ImVec2(box_size / 2, box_size / 2);

			float hover_add = state.bg_anim * 4.0f;
			float animated_size = box_size + hover_add;
			ImVec2 box_min = center - ImVec2(animated_size / 2, animated_size / 2);
			ImVec2 box_max = center + ImVec2(animated_size / 2, animated_size / 2);

			// Основной квадрат
			draw->AddRectFilled(box_min, box_max, ImGui::GetColorU32(c::anim::default.Value), c::elements::rounding);

			// Градиент если включён
			if (state.gradient_anim > 0.001f)
			{
				float gradient_size = animated_size * state.gradient_anim;
				ImVec2 gradient_offset = ImVec2(gradient_size * 0.5f, gradient_size * 0.5f);
				ImVec2 grad_min = center - gradient_offset;
				ImVec2 grad_max = center + gradient_offset;
				state.check_bb = ImRect(grad_min, grad_max);

				fade_rect_filled(draw, grad_min, grad_max,
					ImGui::GetColorU32(c::anim::active.Value),
					ImGui::GetColorU32(c::dark_color.Value),
					fade_direction::diagonally,
					c::elements::rounding, 0, true);

				if (!hovered)
					state.hover_flags = false;

				if (!state.hover_flags && hovered)
				{

					for (int i = 0; i < particle::PARTICLE_COUT / 2; ++i)
						particle::AddParticle(center, ImVec2(3, 3), 0);

					state.hover_flags = true;
				}
			}

			// Галочка
			if (state.check_t > 0.001f)
			{
				float eased = ImClamp(state.check_t, 0.f, 1.f);
				float check_size = animated_size * 0.5f * eased;
				ImVec2 check_offset = ImVec2(check_size * 0.5f, check_size * 0.5f);
				ImGui::RenderCheckMark(draw, center - check_offset, ImGui::GetColorU32(state.check_color), check_size);
			}

			// Текст — ВСЕГДА анимируем по hover/active
			draw->AddText(ImVec2(total_bb.Min.x + spacing, utils::center_text(box_min, box_max, label).y),
				ImGui::GetColorU32(state.text_color), label);

			if (IsItemClicked())
			{
				*v = !(*v);
				for (int i = 0; i < particle::PARTICLE_COUT; ++i)
					particle::AddParticle(center, ImVec2(3, 3) / 10, 0);

				MarkItemEdited(id);
			}
		}ImGui::PopStyleVar();
		return pressed;
	}


	struct checkboxclicked_state
	{
		ImVec4 text_color_offset;
		ImVec4 rect_color;
		ImVec4 circle_color;
		float circle_offset;
	};

	bool CheckboxClicked(const char* label, bool* v)
	{
		ImGuiWindow* window = GetCurrentWindow();
		std::string name = label;

		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, checkboxclicked_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, checkboxclicked_state() });
			it_anim = anim.find(id);
		}

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		ImRect total_bb(pos, pos + ImVec2(75 + CalcTextSize(label).x, 50));

		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);


		bool hovered, held;
		bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) *v = !(*v);

		it_anim->second.circle_offset = ImLerp(it_anim->second.circle_offset, *v ? 20.f : 0.f, GetAnimSpeed());
		it_anim->second.rect_color = ImLerp(it_anim->second.rect_color, *v ? c::anim::active : ImColor(0.1f, 0.1f, 0.1f, 0.5f), GetAnimSpeed());
		it_anim->second.circle_color = ImLerp(it_anim->second.circle_color, *v ? ImColor(1.f, 1.f, 1.f, 1.f) : ImColor(0.6f, 0.6f, 0.6f, 1.f), GetAnimSpeed());
		it_anim->second.text_color_offset = ImLerp(it_anim->second.text_color_offset, *v ? ImColor(0.f, 0.f, 0.f, 0.0f) : ImColor(0.f, 0.f, 0.f, 0.5f), GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Max - ImVec2(60, 35), total_bb.Max - ImVec2(20, 15), GetColorU32(it_anim->second.rect_color), 25);
		window->DrawList->AddRect(total_bb.Max - ImVec2(60, 35), total_bb.Max - ImVec2(20, 15), second_color, 25, 0, 1.5f);
		window->DrawList->AddCircleFilled(total_bb.Max - ImVec2(50 - it_anim->second.circle_offset, 25), 7.f, GetColorU32(it_anim->second.circle_color), 60);

		window->DrawList->AddText(ImVec2(total_bb.Min.x + 5.f, utils::center_text(total_bb.Min, total_bb.Max, label).y), c::label::active, label);
		return pressed;
	}

	static float CalcMaxPopupHeightFromItemCount(int items_count)
	{
		ImGuiContext& g = *GImGui;
		if (items_count <= 0)
			return FLT_MAX;
		return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
	}

	struct begin_state
	{
		ImVec4 background, text, icon_col;
		float open, alpha, combo_size = 0.f, shadow_opticaly;
		bool opened_combo = false, hovered = false;
		float arrow_roll;
		
		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool BeginCombo(const char* label, const char* preview_value, int val, bool multi, ImGuiComboFlags flags)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();

		g.NextWindowData.ClearFlags();
		if (window->SkipItems) return false;

		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, begin_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, begin_state() });
			it_anim = anim.find(id);
		}

		const ImVec2 pos = window->DC.CursorPos + ImVec2(it_anim->second.total_offset, 0);

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float w = GetContentRegionAvail().x;

		const ImRect total_bb(pos, pos + ImVec2(w, 28));

		float preview_value_size = CalcTextSize(preview_value).x;

		const ImRect bb_box(total_bb.Max - ImVec2(preview_value_size + 55, total_bb.GetHeight()), total_bb.Max);

		float box_offset = bb_box.Max.x - ImGui::GetCurrentWindow()->Size.x / 2 - 10;

		const ImRect bb(ImVec2(total_bb.Min.x + CalcTextSize(label).x + 10 < box_offset ? box_offset : total_bb.Min.x + CalcTextSize(label).x + 10, bb_box.Min.y), bb_box.Max);

		ItemSize(total_bb, 0.f);

		ItemAdd(total_bb, id, &bb);

		bool hovered, held, pressed = ButtonBehavior(total_bb, id, &hovered, &held);


		static ImDrawList* child_dl;
		static ImVec2 tooltip_pos;

		it_anim->second.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			it_anim->second.total_offset = GetAnimOffset();
			it_anim->second.total_alpha = 0.f;
			it_anim->second.timer = 0.f;
		}
		it_anim->second.timer += ImGui::GetIO().DeltaTime;
		float relative = (total_bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((it_anim->second.timer - delay) / 0.3f, 0.f, 1.f);
		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		it_anim->second.total_alpha = ImLerp(it_anim->second.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);


		if (hovered && g.IO.MouseClicked[0] || it_anim->second.opened_combo && g.IO.MouseClicked[0] && !it_anim->second.hovered)
		{
			it_anim->second.opened_combo = !it_anim->second.opened_combo;

			if (it_anim->second.opened_combo) {
				tooltip_pos = g.IO.MousePos;
			}
		}
		it_anim->second.arrow_roll = ImLerp(it_anim->second.arrow_roll, it_anim->second.opened_combo ? -1.f : 1.f, g.IO.DeltaTime * 6.f);
		it_anim->second.text = ImLerp(it_anim->second.text, it_anim->second.opened_combo ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());
		it_anim->second.background = ImLerp(it_anim->second.background, it_anim->second.opened_combo ? c::second_color : c::elements::background, g.IO.DeltaTime * 6.f);
		//it_anim->second.combo_size = ImLerp(it_anim->second.combo_size, it_anim->second.opened_combo ? (val * 32) + 25 : 0.f, g.IO.DeltaTime * 12.f);

		it_anim->second.combo_size = easing->easing_in_out<float>(id * 12, !it_anim->second.opened_combo, (val * 32) + 25, 0.2f, g.IO.DeltaTime * 15.f, interpolation_type::back);
		it_anim->second.alpha = easing->easing_in_out<float>(id * 13, !it_anim->second.opened_combo, 1.f, 0.f, g.IO.DeltaTime * 15.f, interpolation_type::back);

		it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, it_anim->second.opened_combo ? c::anim::active : hovered ? c::description::hovered : c::description::default, GetAnimSpeed());
		
		PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.total_alpha);
		GetWindowDrawList()->AddText(ImVec2(total_bb.Min.x, utils::center_text(total_bb.Min, total_bb.Max, label).y), GetColorU32(it_anim->second.text), label);
		GetWindowDrawList()->AddRectFilled(bb_box.Min, bb_box.Max, c::anim::default, c::elements::rounding);

		PushClipRect(bb.Min, bb.Max, true);
		GetWindowDrawList()->AddText(ImVec2(bb_box.Min.x + 10.f, utils::center_text(total_bb.Min, total_bb.Max, preview_value).y), utils::GetColorWithAlpha(c::label::active, style.Alpha), preview_value);
		PopClipRect();

		GetWindowDrawList()->AddRectFilled(ImVec2(bb.Max.x - 11.f - CalcTextSize(ICON_DOWN_LINE).x, bb.Min.y), ImVec2(bb.Max.x - 10.f - CalcTextSize(ICON_DOWN_LINE).x, bb.Max.y), utils::GetColorWithAlpha(c::text::label::default, 0.1f), 0.f);

		ImRotateStart();
		GetWindowDrawList()->AddText(ImVec2(bb.Max.x - 5.f - CalcTextSize(ICON_DOWN_FILL).x, bb.GetCenter().y - CalcTextSize(ICON_DOWN_FILL).y / 2), GetColorU32(it_anim->second.icon_col), ICON_DOWN_FILL);
		ImRotateEnd(1.57f * it_anim->second.arrow_roll);

		PopStyleVar();

		ImVec2 viewport_size = ImGui::GetMainViewport()->Size;
		ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;

		// Проверяем видимость прямоугольника
		if (!IsRectVisible(bb.Min, bb.Max + ImVec2(0, 2))) {
			it_anim->second.opened_combo = false;
			it_anim->second.combo_size = 0.f;
		}

		if (!it_anim->second.opened_combo && it_anim->second.combo_size < 1.f)
			return false;

		ImVec2 window_size = ImVec2(bb.GetWidth(), it_anim->second.combo_size);
		ImVec2 window_pos = tooltip_pos;

		if (window_pos.x + window_size.x > viewport_pos.x + viewport_size.x) {
			window_pos.x = (viewport_pos.x + viewport_size.x) - window_size.x; // Сдвигаем влево
		}

		// Сдвиг по оси Y, если окно выходит за нижнюю границу
		if (window_pos.y + window_size.y > viewport_pos.y + viewport_size.y) {
			window_pos.y = (viewport_pos.y + viewport_size.y) - window_size.y; // Сдвигаем вверх
		}

		// Сдвиг по оси Y, если окно выходит за верхнюю границу
		if (window_pos.y < viewport_pos.y) {
			window_pos.y = viewport_pos.y; // Сдвигаем вниз
		}

		// Устанавливаем позицию перед открытием окна
		SetNextWindowPos(window_pos, ImGuiCond_Always);

		// Устанавливаем размер окна
		ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollWithMouse;

		PushStyleColor(ImGuiCol_WindowBg, utils::ImColorToImVec4(c::background_color));
		PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.f, 0.f, 0.f, 1.f));
		PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.alpha);
		PushStyleVar(ImGuiStyleVar_WindowRounding, c::bg::rounding);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		
		ImGui::SetNextWindowFocus();
		// Открываем окно
		bool ret = Begin(label, NULL, window_flags);

		PushClipRect(ImVec2(0, 0), ImVec2(4000, 4000), false);
		ImGui::GetWindowDrawList()->AddShadowRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(0.f, 0.f, 0.f, it_anim->second.alpha / 1.5f), 65.f, ImVec2(0, 0), 0, c::elements::rounding);
		PopClipRect();

		PopStyleVar(3);
		PopStyleColor(2);

		ImGui::GetForegroundDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), GetColorU32(c::child::stroke), 1.f);

		it_anim->second.hovered = IsWindowHovered();

		if (multi && it_anim->second.hovered && g.IO.MouseClicked[0]) it_anim->second.opened_combo = false;

		return true;
	}

	void EndCombo()
	{
		End();
		PopStyleVar();
	}

	void MultiCombo(const char* label, bool variable[], const char* labels[], int count)
	{
		ImGuiContext& g = *GImGui;

		std::string preview = "None";

		for (auto i = 0, j = 0; i < count; i++)
		{
			if (variable[i])
			{
				if (j)
					preview += (", ") + (std::string)labels[i];
				else
					preview = labels[i];

				j++;
			}
		}

		if (BeginCombo(label, preview.c_str(), count))
		{
			ImGui::SetWindowFocus();
			for (auto i = 0; i < count; i++)
			{
				PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
				PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
				custom::Selectable(labels[i], &variable[i], ImGuiSelectableFlags_DontClosePopups);
				PopStyleVar(2);
			}
			End();
		}

		preview = ("None");
	}

	bool BeginComboPreview()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		if (window->SkipItems || !(g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible)) return false;

		IM_ASSERT(g.LastItemData.Rect.Min.x == preview_data->PreviewRect.Min.x && g.LastItemData.Rect.Min.y == preview_data->PreviewRect.Min.y);

		if (!window->ClipRect.Overlaps(preview_data->PreviewRect)) return false;

		preview_data->BackupCursorPos = window->DC.CursorPos;
		preview_data->BackupCursorMaxPos = window->DC.CursorMaxPos;
		preview_data->BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
		preview_data->BackupPrevLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
		preview_data->BackupLayout = window->DC.LayoutType;
		window->DC.CursorPos = preview_data->PreviewRect.Min + g.Style.FramePadding;
		window->DC.CursorMaxPos = window->DC.CursorPos;
		window->DC.LayoutType = ImGuiLayoutType_Horizontal;
		window->DC.IsSameLine = false;
		PushClipRect(preview_data->PreviewRect.Min, preview_data->PreviewRect.Max, true);

		return true;
	}

	void EndComboPreview()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		ImDrawList* draw_list = window->DrawList;
		if (window->DC.CursorMaxPos.x < preview_data->PreviewRect.Max.x && window->DC.CursorMaxPos.y < preview_data->PreviewRect.Max.y)
			if (draw_list->CmdBuffer.Size > 1)
			{
				draw_list->_CmdHeader.ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 2].ClipRect;
				draw_list->_TryMergeDrawCmds();
			}
		PopClipRect();
		window->DC.CursorPos = preview_data->BackupCursorPos;
		window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, preview_data->BackupCursorMaxPos);
		window->DC.CursorPosPrevLine = preview_data->BackupCursorPosPrevLine;
		window->DC.PrevLineTextBaseOffset = preview_data->BackupPrevLineTextBaseOffset;
		window->DC.LayoutType = preview_data->BackupLayout;
		window->DC.IsSameLine = false;
		preview_data->PreviewRect = ImRect();
	}

	static const char* Items_ArrayGetter(void* data, int idx)
	{
		const char* const* items = (const char* const*)data;
		return items[idx];
	}

	static const char* Items_SingleStringGetter(void* data, int idx)
	{
		const char* items_separated_by_zeros = (const char*)data;
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			if (idx == items_count)
				break;
			p += strlen(p) + 1;
			items_count++;
		}
		return *p ? p : NULL;
	}

	bool Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items)
	{
		ImGuiContext& g = *GImGui;

		const char* preview_value = NULL;
		if (*current_item >= 0 && *current_item < items_count)
			preview_value = getter(user_data, *current_item);

		if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
			SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

		if (!BeginCombo(label, preview_value, items_count, ImGuiComboFlags_None)) return false;

		bool value_changed = false;
		PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
		for (int i = 0; i < items_count; i++)
		{
			const char* item_text = getter(user_data, i);
			if (item_text == NULL)
				item_text = "*Unknown item*";

			PushID(i);
			const bool item_selected = (i == *current_item);
			if (custom::Selectable(item_text, item_selected) && *current_item != i)
			{
				value_changed = true;
				*current_item = i;
			}
			if (item_selected)
				SetItemDefaultFocus();
			PopID();
		}
		PopStyleVar();

		EndCombo();

		if (value_changed)
			MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

	bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
	{
		const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
		return value_changed;
	}

	bool Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
	{
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			p += strlen(p) + 1;
			items_count++;
		}
		bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
		return value_changed;
	}



	struct select_state
	{
		ImVec4 text, background, stroke;
		float circle_radius, text_offset;
		float total_offset;
	};

	bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGuiID id = window->GetID(label);
		ImVec2 label_size = CalcTextSize(label, NULL, true);
		ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
		ImVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;

		static std::map<ImGuiID, select_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, select_state() });
			it_anim = anim.find(id);
		}

		ItemSize(size, 0.0f);

		const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
		const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
		const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
		if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth)) size.x = ImMax(label_size.x, max_x - min_x);

		const ImVec2 text_min = pos;
		const ImVec2 text_max(min_x + size.x, pos.y + size.y);

		ImRect bb(min_x + it_anim->second.total_offset, pos.y, text_max.x + it_anim->second.total_offset, text_max.y);
		if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
		{
			const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
			const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
			bb.Min.x -= spacing_L;
			bb.Min.y -= spacing_U;
			bb.Max.x += (spacing_x - spacing_L);
			bb.Max.y += (spacing_y - spacing_U);
		}

		const float backup_clip_rect_min_x = window->ClipRect.Min.x;
		const float backup_clip_rect_max_x = window->ClipRect.Max.x;
		if (span_all_columns)
		{
			window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
			window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
		}

		const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
		const bool item_add = ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
		if (span_all_columns)
		{
			window->ClipRect.Min.x = backup_clip_rect_min_x;
			window->ClipRect.Max.x = backup_clip_rect_max_x;
		}

		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, item_add ? 0.f : 150.f, c::anim::speed / 1.5f);


		if (!item_add) return false;

		const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
		if (disabled_item && !disabled_global) BeginDisabled();

		if (span_all_columns && window->DC.CurrentColumns) PushColumnsBackground();
		else if (span_all_columns && g.CurrentTable) TablePushBackgroundChannel();

		ImGuiButtonFlags button_flags = 0;
		if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
		if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
		if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
		if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
		if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
		if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

		const bool was_selected = selected;
		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

		if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
			if (g.NavJustMovedToId == id)  selected = pressed = true;

		// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
		if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
		{
			if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
			{
				SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb)); // (bb == NavRect)
				g.NavDisableHighlight = true;
			}
		}
		if (pressed) MarkItemEdited(id);

		if (selected != was_selected)  g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;


		if (g.NavId == id) RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

		if (span_all_columns && window->DC.CurrentColumns) PopColumnsBackground();
		else if (span_all_columns && g.CurrentTable) TablePopBackgroundChannel();


		it_anim->second.text = ImLerp(it_anim->second.text, selected ? c::label::active : c::label::default, GetAnimSpeed());
		it_anim->second.circle_radius = ImLerp(it_anim->second.circle_radius, selected ? style.Alpha * 3.f : 0.f, GetAnimSpeed());
		it_anim->second.text_offset = ImLerp(it_anim->second.text_offset, selected ? 20.f : 4.5f, GetAnimSpeed());
		it_anim->second.background = ImLerp(it_anim->second.background, selected ? c::background_color : utils::GetColorWithAlpha(c::background_color, 0.f), GetAnimSpeed());
		it_anim->second.stroke = ImLerp(it_anim->second.stroke, selected ? c::child::stroke : utils::GetColorWithAlpha(c::child::stroke, 0.f), GetAnimSpeed());


		//window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(it_anim->second.stroke), c::elements::rounding);

		window->DrawList->AddCircleFilled(ImVec2(bb.Min.x + 9, bb.GetCenter().y), it_anim->second.circle_radius, c::anim::active);

		window->DrawList->AddText(ImVec2(bb.Min.x + it_anim->second.text_offset, utils::center_text(bb.Min, bb.Max, label).y), utils::GetColorWithAlpha(it_anim->second.text, style.Alpha), label);

		if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup)) CloseCurrentPopup();

		if (disabled_item && !disabled_global) EndDisabled();

		return pressed;
	}

	bool Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
	{
		if (Selectable(label, *p_selected, flags, size_arg))
		{
			*p_selected = !*p_selected;
			return true;
		}
		return false;
	}

	static void ColorEditRestoreH(const float* col, float* H)
	{
		ImGuiContext& g = *GImGui;
		IM_ASSERT(g.ColorEditCurrentID != 0);
		if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
			return;
		*H = g.ColorEditSavedHue;
	}

	static void ColorEditRestoreHS(const float* col, float* H, float* S, float* V)
	{
		ImGuiContext& g = *GImGui;
		IM_ASSERT(g.ColorEditCurrentID != 0);
		if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0))) return;

		if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
			*H = g.ColorEditSavedHue;

		if (*V == 0.0f) *S = g.ColorEditSavedSat;
	}


	struct edit_state
	{
		ImVec4 text;
		ImVec4 icon;
		ImVec4 text_color, description_col;

		float menu_alpha = 0.f;
		bool enabled_popup;


		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool ColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float square_sz = GetFrameHeight();
		const float w_full = CalcItemWidth();
		const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
		const float w_inputs = w_full - w_button;
		const char* label_display_end = FindRenderedTextEnd(label);
		g.NextItemData.ClearFlags();

		BeginGroup();
		PushID(label);
		const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = window->IDStack.back();

		// If we're not showing any slider there's no point in doing any HSV conversions
		const ImGuiColorEditFlags flags_untouched = flags;
		if (flags & ImGuiColorEditFlags_NoInputs)
			flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

		// Context menu: display and modify options (before defaults are applied)
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ColorEditOptionsPopup(col, flags);

		// Read stored options
		if (!(flags & ImGuiColorEditFlags_DisplayMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
		if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
		if (!(flags & ImGuiColorEditFlags_PickerMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
		if (!(flags & ImGuiColorEditFlags_InputMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
		flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_)); // Check that only 1 is selected
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));   // Check that only 1 is selected

		const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
		const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
		const int components = alpha ? 4 : 3;

		// Convert to the formats we need
		float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
		if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
		else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
		{
			// Hue is lost when converting from grayscale rgb (saturation=0). Restore it.
			ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
			ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
		}
		int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

		bool value_changed = false;
		bool value_changed_as_float = false;

		const ImVec2 pos = window->DC.CursorPos;
		const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
		window->DC.CursorPos.x = pos.x + inputs_offset_x;

		if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
		{
			// RGB/HSV 0..255 Sliders
			const float w_item_one = ImMax(1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
			const float w_item_last = ImMax(1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

			const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
			static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
			static const char* fmt_table_int[3][4] =
			{
				{   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
				{ "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
				{ "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
			};
			static const char* fmt_table_float[3][4] =
			{
				{   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
				{ "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
				{ "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
			};
			const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;

			for (int n = 0; n < components; n++)
			{
				if (n > 0)
					SameLine(0, style.ItemInnerSpacing.x);
				SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

				// FIXME: When ImGuiColorEditFlags_HDR flag is passed HS values snap in weird ways when SV values go below 0.
				if (flags & ImGuiColorEditFlags_Float)
				{
					value_changed |= DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
					value_changed_as_float |= value_changed;
				}
				else
				{
					value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
				}
				if (!(flags & ImGuiColorEditFlags_NoOptions))
					OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
			}
		}
		else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
		{

			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
		}

		// RGB Hexadecimal Input
		char buf[64];
		ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));

		const float width = GetContentRegionAvail().x;

		static std::map<ImGuiID, edit_state> anim;
		auto it_anim = anim.find(ImGui::GetID(label));

		if (it_anim == anim.end())
		{
			anim.insert({ ImGui::GetID(label), edit_state() });
			it_anim = anim.find(ImGui::GetID(label));
		}

		const ImRect rect(window->DC.CursorPos + ImVec2(it_anim->second.total_offset, 0), window->DC.CursorPos + ImVec2(width + it_anim->second.total_offset, 60));

		ImRect combo_bb(rect.Min + ImVec2(0, 26), rect.Max);



		it_anim->second.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			it_anim->second.total_offset = GetAnimOffset();
			it_anim->second.total_alpha = 0.f;
			it_anim->second.timer = 0.f;
		}

		it_anim->second.timer += ImGui::GetIO().DeltaTime;
		float relative = (rect.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((it_anim->second.timer - delay) / 0.3f, 0.f, 1.f);
		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		it_anim->second.total_alpha = ImLerp(it_anim->second.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);

		it_anim->second.menu_alpha = ImLerp(it_anim->second.menu_alpha, it_anim->second.enabled_popup ? 1.f : 0.f, anim::speed);

		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
		ItemSize(rect, 0.0f);
		ItemAdd(rect, GetID(label));


		bool hovered, held;
		bool pressed = ButtonBehavior(rect, GetID(label), &hovered, &held);



		ImGuiWindow* picker_active_window = NULL;
		if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
		{

			PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.menu_alpha);
			{

				const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
				window->DC.CursorPos = ImVec2(pos.x, pos.y);


				if (pressed && it_anim->second.menu_alpha < 0.1f)
				{
					if (!(flags & ImGuiColorEditFlags_NoPicker))
					{
						g.ColorPickerRef = col_v4;
						it_anim->second.enabled_popup = true;
						//SetNextWindowPos(ImVec2(combo_bb.Min.x, combo_bb.Max.y));
					}
				}
				if (!(flags & ImGuiColorEditFlags_NoOptions))
					OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

				PushStyleVar(ImGuiStyleVar_PopupRounding, c::child::rounding);
				PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

				static bool active_popup;

				if (it_anim->second.menu_alpha > 0.05f) {

					ImGuiWindow* wnd = ImGui::FindWindowByName("ESP");

					SetNextWindowPos(ImVec2(combo_bb.Min.x + (wnd != nullptr ? 250.f : wnd != nullptr ? -20.f : 0.f), combo_bb.Max.y));
					
					if (Begin("picker", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysUseWindowPadding))
					{
						ImRect bb = ImGui::GetCurrentWindow()->Rect();

						ImGui::PushClipRect(ImVec2(0, 0), ImGui::GetMainViewport()->Size, false);
						ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::background_color.Value), c::elements::rounding);
						ImGui::GetWindowDrawList()->AddRect(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, 0.05f), c::elements::rounding);
						ImGui::PopClipRect();

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !GetCurrentWindow()->Rect().Contains(ImGui::GetMousePos()))
							it_anim->second.enabled_popup = false;

						if (g.CurrentWindow->BeginCount == 1)
						{
							picker_active_window = g.CurrentWindow;

							ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
							ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
							SetNextItemWidth(square_sz * 10.0f); // Use 256 + bar sizes?
							value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
						}
						End();
					}
				}

				PopStyleVar(2);

			}ImGui::PopStyleVar();

		}

		if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
		{
				SameLine(0.0f, style.ItemInnerSpacing.x);
				window->DC.CursorPos.x = pos.x - w_button + ((flags & ImGuiColorEditFlags_NoInputs) ? w_button : w_full);

				const ImVec2 check_offset = ImVec2(3.f, 3.f);

				ImRect check_bb(rect.Max - ImVec2(rect.GetSize().y, rect.GetSize().y) + check_offset, rect.Max - check_offset);


				//ItemBackground(GetID(label), rect, rect.Contains(ImGui::GetMousePos()));

				it_anim->second.text = ImLerp(it_anim->second.text, ImGui::IsPopupOpen("picker") ? c::text::label::active : IsMouseHoveringRect(rect.Min, rect.Max, true) ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());

				it_anim->second.icon = ImLerp(it_anim->second.icon, ImGui::IsPopupOpen("picker") ? c::anim::active : IsMouseHoveringRect(rect.Min, rect.Max, true) ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());

				it_anim->second.text_color = ImLerp(it_anim->second.text_color, ImGui::IsPopupOpen("picker") ? c::text::label::active : rect.Contains(ImGui::GetMousePos()) ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());
				it_anim->second.description_col = ImLerp(it_anim->second.description_col, ImGui::IsPopupOpen("picker") ? c::text::description::active : rect.Contains(ImGui::GetMousePos()) ? c::text::description::hovered : c::text::description::default, GetAnimSpeed());

				ImColor color_rgb = col_v4;



				GetWindowDrawList()->AddRectFilled(combo_bb.Min, combo_bb.Max, c::anim::default, c::elements::rounding);
				GetWindowDrawList()->AddRectFilled(combo_bb.Min + ImVec2(8, 8), combo_bb.Min + ImVec2(combo_bb.GetSize().y, combo_bb.GetSize().y) - ImVec2(8, 8), color_rgb, 360);

				char hex[10];
				int r = static_cast<int>(col[0] * 255.0f);
				int g = static_cast<int>(col[1] * 255.0f);
				int b = static_cast<int>(col[2] * 255.0f);

				ImFormatString(hex, sizeof(hex), "#%02X%02X%02X", ImClamp(r, 0, 255), ImClamp(g, 0, 255), ImClamp(b, 0, 255));

				PushFont(font::description_font);
				GetWindowDrawList()->AddText(ImVec2(combo_bb.Min.x + 5 + combo_bb.GetSize().y, utils::center_text(combo_bb.Min, combo_bb.Max, hex).y), c::label::active, hex);
				GetWindowDrawList()->AddText(ImVec2(combo_bb.Max.x - combo_bb.GetSize().y, utils::center_text(combo_bb.Min, combo_bb.Max, ICON_PALETTE_LINE).y) + ImVec2(6, 3.5f), c::label::default, ICON_PALETTE_LINE);
				PopFont();

				ImGui::PushClipRect(rect.Min, rect.Max, true);
				GetWindowDrawList()->AddText(rect.Min, GetColorU32(it_anim->second.text_color), label);
				ImGui::PopClipRect();

			ImGui::SetCursorScreenPos(ImVec2(rect.Min.x, rect.Max.y));
		}

		// Convert back
		if (value_changed && picker_active_window == NULL)
		{
			if (!value_changed_as_float)
				for (int n = 0; n < 4; n++)
					f[n] = i[n] / 255.0f;
			if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
			{
				g.ColorEditSavedHue = f[0];
				g.ColorEditSavedSat = f[1];
				ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
				g.ColorEditSavedID = g.ColorEditCurrentID;
				g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
			}
			if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
				ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

			col[0] = f[0];
			col[1] = f[1];
			col[2] = f[2];
			if (alpha)
				col[3] = f[3];
		}

		if (set_current_color_edit_id)
			g.ColorEditCurrentID = 0;
		PopID();
		EndGroup();

		// Drag and Drop Target
		// NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
		if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
		{
			bool accepted_drag_drop = false;
			if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
			{
				memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512 //-V1086
				value_changed = accepted_drag_drop = true;
			}
			if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
			{
				memcpy((float*)col, payload->Data, sizeof(float) * components);
				value_changed = accepted_drag_drop = true;
			}

			// Drag-drop payloads are always RGB
			if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
				ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
			EndDragDropTarget();
		}

		// When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
		if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
			g.LastItemData.ID = g.ActiveId;

		if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
			MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}


	// Helper for ColorPicker4()
	static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha)
	{
		ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32(255, 255, 255, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32(255, 255, 255, alpha8));
	}

	struct picker_state
	{
		float hue_bar;
		float alpha_bar;
		float circle;
		ImVec2 circle_move;
	};

	bool ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImDrawList* draw_list = window->DrawList;
		ImGuiStyle& style = g.Style;
		ImGuiIO& io = g.IO;

		const float width = CalcItemWidth();
		g.NextItemData.ClearFlags();

		PushID(label);
		BeginGroup();

		if (!(flags & ImGuiColorEditFlags_NoSidePreview))
			flags |= ImGuiColorEditFlags_NoSmallPreview;

		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ColorPickerOptionsPopup(col, flags);

		// Read stored options
		if (!(flags & ImGuiColorEditFlags_PickerMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
		if (!(flags & ImGuiColorEditFlags_InputMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_)); // Check that only 1 is selected
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));  // Check that only 1 is selected
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

		// Setup
		int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
		bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
		ImVec2 picker_pos = window->DC.CursorPos;
		ImVec2 bar_pos = window->DC.CursorPos + ImVec2(0, 133);
		float square_sz = GetFrameHeight();
		float bars_width = 209.f; // Arbitrary smallish width of Hue/Alpha picking bars
		float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)) + 0; // Saturation/Value picking box
		float sv_bar_size = 20; // Saturation/Value picking box
		float bar0_pos_x = GetWindowPos().x + style.WindowPadding.x;
		float bar1_pos_x = bar0_pos_x;
		float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

		float backup_initial_col[4];
		memcpy(backup_initial_col, col, components * sizeof(float));

		float H = col[0], S = col[1], V = col[2];
		float R = col[0], G = col[1], B = col[2];
		if (flags & ImGuiColorEditFlags_InputRGB)
		{
			// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
			ColorConvertRGBtoHSV(R, G, B, H, S, V);
			ColorEditRestoreHS(col, &H, &S, &V);
		}
		else if (flags & ImGuiColorEditFlags_InputHSV)
		{
			ColorConvertHSVtoRGB(H, S, V, R, G, B);
		}

		bool value_changed = false, value_changed_h = false, value_changed_sv = false;

		PushItemFlag(ImGuiItemFlags_NoNav, true);

		// SV rectangle logic
		InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size - 80));
		if (IsItemActive())
		{
			S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
			V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 80));

			// Greatly reduces hue jitter and reset to 0 when hue == 255 and color is rapidly modified using SV square.
			if (g.ColorEditSavedColor == ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
				H = g.ColorEditSavedHue;
			value_changed = value_changed_sv = true;
		}

		// Hue bar logic
		SetCursorScreenPos(ImVec2(bar0_pos_x, bar_pos.y));
		InvisibleButton("hue", ImVec2(bars_width, sv_bar_size));
		if (IsItemActive())
		{
			H = 1.f - ImSaturate((io.MousePos.x - bar_pos.x) / (bars_width - 1));
			value_changed = value_changed_h = true;
		}

		// Alpha bar logic
		if (alpha_bar)
		{
			SetCursorScreenPos(ImVec2(bar1_pos_x, bar_pos.y + 16));
			InvisibleButton("alpha", ImVec2(bars_width, sv_bar_size));
			if (IsItemActive())
			{
				col[3] = ImSaturate((io.MousePos.x - bar_pos.x) / (bars_width - 1));
				value_changed = true;
			}
		}
		PopItemFlag(); // ImGuiItemFlags_NoNav

		// Convert back color to RGB
		if (value_changed_h || value_changed_sv)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
				g.ColorEditSavedHue = H;
				g.ColorEditSavedSat = S;
				g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
			}

			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				col[0] = H;
				col[1] = S;
				col[2] = V;
			}
		}

		// R,G,B and H,S,V slider color editor
		bool value_changed_fix_hue_wrap = false;

		if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
		{
			// Try to cancel hue wrap (after ColorEdit4 call), if any
			float new_H, new_S, new_V;
			ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
			if (new_H <= 0 && H > 0)
			{
				if (new_V <= 0 && V != new_V)
					ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
				else if (new_S <= 0)
					ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
			}
		}

		if (value_changed)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				R = col[0];
				G = col[1];
				B = col[2];
				ColorConvertRGBtoHSV(R, G, B, H, S, V);
				ColorEditRestoreHS(col, &H, &S, &V);   // Fix local Hue as display below will use it immediately.
			}
			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				H = col[0];
				S = col[1];
				V = col[2];
				ColorConvertHSVtoRGB(H, S, V, R, G, B);
			}
		}
		ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!

		const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
		const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
		const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
		const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
		const ImU32 col_hues[7] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(0,0,255,style_alpha8),IM_COL32(0,255,255,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(255,0,0,style_alpha8) };

		ImVec4 hue_color_f(1, 1, 1, style.Alpha); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
		ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);

		ImVec2 sv_cursor_pos;

		// Render SV Square
		const int vtx_idx_0 = draw_list->VtxBuffer.Size;
		draw_list->AddRectFilled(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size - 2 - 80), col_white, 4.0f);
		const int vtx_idx_1 = draw_list->VtxBuffer.Size;
		ShadeVertsLinearColorGradientKeepAlpha(draw_list, vtx_idx_0, vtx_idx_1, picker_pos, picker_pos + ImVec2(sv_picker_size, 0.0f), col_white, hue_color32);

		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size - 80), 0, 0, col_black, col_black, 4.f);

		sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * (sv_picker_size - 80)), picker_pos.y + 2, picker_pos.y + sv_picker_size - 80);

		static std::map<ImGuiID, picker_state> anim;
		auto it_anim = anim.find(ImGui::GetID(label));

		if (it_anim == anim.end())
		{
			anim.insert({ ImGui::GetID(label), picker_state() });
			it_anim = anim.find(ImGui::GetID(label));
		}

		for (int i = 0; i < 6; ++i)
			GetForegroundDrawList()->AddRectFilledMultiColor(ImVec2(picker_pos.x + i * (bars_width / 6) - (i == 5 ? 1 : 0), picker_pos.y + 139), ImVec2(picker_pos.x + (i + 1) * (bars_width / 6) + (i == 0 ? 1 : 0), picker_pos.y + 132 + sv_bar_size - 7), col_hues[i], col_hues[i + 1], col_hues[i + 1], col_hues[i], 10.f, i == 0 ? ImDrawFlags_RoundCornersLeft : i == 5 ? ImDrawFlags_RoundCornersRight : ImDrawFlags_RoundCornersNone);

		float bar0_line_x = IM_ROUND(bar_pos.x + (1.f - H) * bars_width);

		bar0_line_x = ImClamp(bar0_line_x, picker_pos.x + 3.f, picker_pos.x + 204.f);

		it_anim->second.hue_bar = ImLerp(it_anim->second.hue_bar, bar0_line_x - bar_pos.x, g.IO.DeltaTime * 24.f);

		GetForegroundDrawList()->AddCircleFilled(ImVec2(it_anim->second.hue_bar + bar_pos.x, bar_pos.y + 9), 6.5f, ImColor(255, 255, 255, 255), 30.f);

		it_anim->second.circle_move = ImLerp(it_anim->second.circle_move, sv_cursor_pos - bar_pos, g.IO.DeltaTime * 24.f);
		it_anim->second.circle = ImLerp(it_anim->second.circle, value_changed_sv ? 4.f : 7.f, g.IO.DeltaTime * 24.f);

		GetForegroundDrawList()->AddCircle(it_anim->second.circle_move + bar_pos + ImVec2(0, 1), it_anim->second.circle, ImColor(255, 255, 255, 255), 32);

		if (alpha_bar)
		{
			float alpha = ImSaturate(col[3]);
			ImRect bar1_bb(bar1_pos_x, bar_pos.y + 20, bar1_pos_x + bars_width, bar_pos.y + 20 + sv_bar_size);

			draw_list->AddRectFilledMultiColor(picker_pos + ImVec2(0, 161), picker_pos + ImVec2(bars_width, 147 + sv_bar_size), col_black, user_col32_striped_of_alpha, user_col32_striped_of_alpha, col_black, 10.f);

			float bar1_line_x = IM_ROUND(bar_pos.x + alpha * bars_width);

			bar1_line_x = ImClamp(bar1_line_x, bar_pos.x, picker_pos.x + 200.f);
			it_anim->second.alpha_bar = ImLerp(it_anim->second.alpha_bar, bar1_line_x - bar_pos.x + 5.f, g.IO.DeltaTime * 24.f);
			GetForegroundDrawList()->AddCircleFilled(ImVec2(it_anim->second.alpha_bar + bar_pos.x, bar1_bb.Min.y + 11.0f), 6.5f, ImColor(255, 255, 255, 255), 100.f);
		}

		EndGroup();

		ImVec4 preset_colors[] = {
			ImVec4(244 / 255.f, 99 / 255.f, 17 / 255.f, 1.f),
			ImVec4(237 / 255.f, 28 / 255.f, 128 / 255.f, 1.f),
			ImVec4(112 / 255.f, 81 / 255.f, 204 / 255.f, 1.f),
			ImVec4(178 / 255.f, 99 / 255.f, 217 / 255.f, 1.f),
			ImVec4(230 / 255.f, 111 / 255.f, 176 / 255.f, 1.f),
			ImVec4(255 / 255.f, 202 / 255.f, 58 / 255.f, 1.f),

			ImVec4(217 / 255.f, 83 / 255.f, 79 / 255.f, 1.f),
			ImVec4(255 / 255.f, 107 / 255.f, 107 / 255.f, 1.f),
			ImVec4(67 / 255.f, 197 / 255.f, 94 / 255.f, 1.f),
			ImVec4(77 / 255.f, 130 / 255.f, 255 / 255.f, 1.f),
			ImVec4(128 / 255.f, 0 / 255.f, 128 / 255.f, 1.f),
			ImVec4(255 / 255.f, 182 / 255.f, 193 / 255.f, 1.f)
		};

		ImVec2 preset_start = ImGui::GetCursorScreenPos();
		float spacing = 12.f;
		ImVec2 box_size = ImVec2(24.f, 24.f);
		int preset_id = 0;

		for (int row = 0; row < 2; row++)
		{
			for (int col_idx = 0; col_idx < 6 && preset_id < IM_ARRAYSIZE(preset_colors); col_idx++, preset_id++)
			{
				ImVec2 box_pos = preset_start + ImVec2(col_idx * (box_size.x + spacing), row * (box_size.y + spacing));
				ImRect box_bb(box_pos, box_pos + box_size);

				static std::map<int, float> anim;
				anim[preset_id] = ImLerp(anim[preset_id], box_bb.Contains(ImGui::GetMousePos()) ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 10.f);

				ImGui::GetWindowDrawList()->AddRectFilled(
					box_bb.Min, box_bb.Max,
					ImColor(preset_colors[preset_id]),
					6.f
				);

				bool is_active =
					(fabsf(col[0] - preset_colors[preset_id].x) < 0.01f &&
						fabsf(col[1] - preset_colors[preset_id].y) < 0.01f &&
						fabsf(col[2] - preset_colors[preset_id].z) < 0.01f);

				if (anim[preset_id] > 0.01f || is_active)
				{
					float alpha = is_active ? 1.f : anim[preset_id];
					ImColor border = ImColor(255, 255, 255, int(255 * alpha));
					ImGui::GetWindowDrawList()->AddRect(box_bb.Min, box_bb.Max, border, 6.f, 0, 2.f);
				}

				if (box_bb.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
				{
					col[0] = preset_colors[preset_id].x;
					col[1] = preset_colors[preset_id].y;
					col[2] = preset_colors[preset_id].z;
					col[3] = preset_colors[preset_id].w;
				}
			}
		}

		ImGui::Dummy(ImVec2(5.f, box_size.y * 2.f + spacing + 5));

		// --- HEX INPUT ---
		ImGui::SetCursorScreenPos(preset_start + ImVec2(0, box_size.y * 2.f + spacing * 2.f));

		// Текст
		ImGui::TextColored(ImColor(180, 180, 180), "Custom:");

		ImGui::SameLine();
		ImGui::PushItemWidth(55.f);

		static char hex_input[8];
		snprintf(hex_input, sizeof(hex_input), "%02X%02X%02X",
			IM_F32_TO_INT8_SAT(col[0]),
			IM_F32_TO_INT8_SAT(col[1]),
			IM_F32_TO_INT8_SAT(col[2])
		);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

		if (ImGui::InputTextEx("Here color", NULL, hex_input, sizeof(hex_input), ImVec2(ImGui::GetContentRegionAvail().x - 2, 22), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase, NULL, NULL))
		{
			unsigned int hex_color = 0;
			if (sscanf(hex_input, "%06X", &hex_color) == 1)
			{
				col[0] = ((hex_color >> 16) & 0xFF) / 255.f;
				col[1] = ((hex_color >> 8) & 0xFF) / 255.f;
				col[2] = (hex_color & 0xFF) / 255.f;
			}
		}

		ImGui::PopItemWidth();

		if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0) value_changed = false;
		//if (value_changed) MarkItemEdited(g.LastItemData.ID);

		PopID();
		return value_changed;
	}


	bool ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(desc_id);
		const float default_size = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const float width = GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x;
		const ImRect rect(pos, pos + ImVec2(width, 19));

		const ImRect clickable(rect.Min + ImVec2(width - 25, 0), rect.Max - ImVec2(7, 0));

		ItemSize(ImRect(rect.Min, rect.Max - ImVec2(0, 0)));
		if (!ItemAdd(rect, id)) return false;

		bool hovered, held, pressed = ButtonBehavior(rect, id, &hovered, &held);

		if (flags & ImGuiColorEditFlags_NoAlpha) flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);

		ImVec4 col_rgb = col;
		if (flags & ImGuiColorEditFlags_InputHSV) ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);

		GetWindowDrawList()->AddRectFilled(clickable.Min, clickable.Max, GetColorU32(col_rgb), c::elements::rounding);

		RenderColorRectWithAlphaCheckerboard(window->DrawList, clickable.Min, clickable.Max, GetColorU32(col_rgb), ImMin(36, 29) / 2.99f, ImVec2(0.f, 0.f), c::elements::rounding);


		return pressed;
	}

	struct knob_state {
		float plus_float;
		int plus_int;
		ImVec4 background, circle, text;
		float slow_anim, circle_anim;
		float position;
	};


	bool KnobScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = GetContentRegionMax().x - style.WindowPadding.x;

		const ImVec2 label_size = CalcTextSize(label, NULL, true);

		const ImRect frame_bb(window->DC.CursorPos + ImVec2(0, 0), window->DC.CursorPos + ImVec2(w, 32));

		const ImRect slider_bb(window->DC.CursorPos + ImVec2(w - 30, 0), window->DC.CursorPos + ImVec2(w, 100));

		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ItemSize(ImRect(total_bb.Min, total_bb.Max - ImVec2(0, 0)));

		if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0)) return false;

		if (format == NULL) format = DataTypeGetInfo(data_type)->PrintFmt;

		bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags), held, pressed = ButtonBehavior(frame_bb, id, &hovered, &held, NULL);

		ImRect grab_bb;

		static std::map<ImGuiID, knob_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, knob_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.circle_anim = ImLerp(it_anim->second.circle_anim, IsItemActive() ? 11.f : 10.f, g.IO.DeltaTime * 6.f);

		if ((flags & ImGuiSliderFlags_Integer) == 0) {
			it_anim->second.plus_float = ImLerp(it_anim->second.plus_float, *(float*)p_data <= *(float*)p_max && hovered && GetAsyncKeyState(VK_OEM_PLUS) & 0x01 ? *(float*)p_data += 0.05f : *(float*)p_data >= *(float*)p_min && hovered && GetAsyncKeyState(VK_OEM_MINUS) & 0x01 ? *(float*)p_data -= 0.05f : 0, g.IO.DeltaTime * 6.f);
			if (*(float*)p_data > *(float*)p_max) *(float*)p_data = *(float*)p_max;
			if (*(float*)p_data < *(float*)p_min) *(float*)p_data = *(float*)p_min;
		}
		else
		{
			it_anim->second.plus_int = ImLerp(it_anim->second.plus_int, *(int*)p_data <= *(int*)p_max && hovered && GetAsyncKeyState(VK_OEM_PLUS) & 0x01 ? *(int*)p_data += 1 : *(int*)p_data >= *(int*)p_min && hovered && GetAsyncKeyState(VK_OEM_MINUS) & 0x01 ? *(int*)p_data -= 1 : 0, g.IO.DeltaTime * 6.f);
			if (*(int*)p_data > *(int*)p_max) *(int*)p_data = *(int*)p_max;
			if (*(int*)p_data < *(int*)p_min) *(int*)p_data = *(int*)p_min;
		}

		it_anim->second.text = ImLerp(it_anim->second.text, g.ActiveId == id ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::default, g.IO.DeltaTime * 6.f);

		const bool value_changed = DragBehavior(id, data_type, p_data, 0.f, p_min, p_max, format, NULL);


		if (value_changed) MarkItemEdited(id);

		char value_buf[64];
		const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

		float radius = 10.f;
		float thickness = 3.f;

		it_anim->second.position = ImLerp(it_anim->second.position, *static_cast<float*>(p_data) / *reinterpret_cast<const float*>(p_max) * 6.25f, ImGui::GetIO().DeltaTime * 18.f);

		GetWindowDrawList()->PathClear();
		GetWindowDrawList()->PathArcTo(ImVec2(frame_bb.Max.x + radius - 22.f, frame_bb.Min.y + (32 / 2)), radius, 0.f, 2.f * IM_PI, 40.f);
		GetWindowDrawList()->PathStroke(GetColorU32(c::elements::background), 0, thickness);

		GetWindowDrawList()->PathClear();
		GetWindowDrawList()->PathArcTo(ImVec2(frame_bb.Max.x + radius - 22.f, frame_bb.Min.y + (32 / 2)), radius, IM_PI * 1.5f, IM_PI * 1.5f + it_anim->second.position, 40.f);
		GetWindowDrawList()->PathStroke(GetColorU32(c::anim::active.Value), 0, thickness);

		GetWindowDrawList()->AddCircleFilled(ImVec2(frame_bb.Max.x + radius - 22.f + ImCos(IM_PI * 1.5f + it_anim->second.position) * radius, frame_bb.Min.y + (32 / 2) + ImSin(IM_PI * 1.5f + it_anim->second.position) * radius), 2.f, c::label::active);

		GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - (40 + CalcTextSize(value_buf).x), frame_bb.Min.y + (32 - CalcTextSize(value_buf).y) / 2), c::text::label::default, value_buf);

		GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - w, frame_bb.Min.y + (32 - CalcTextSize(value_buf).y) / 2), GetColorU32(it_anim->second.text), label);

		return value_changed;
	}

	bool KnobFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
	{
		return KnobScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool KnobInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
	{
		return KnobScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags | ImGuiSliderFlags_Integer);
	}

	struct slider_state {
		ImVec4 background, circle, text_color;
		float position, slow, circle_radius;
		ImVec4 minus_col, plus_col;
		bool disabled;
		bool held_mode;
		float grab_anim;


		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		std::string label_str = label;
		std::string arrows_str[2] = { label_str + "left", label_str + "right" };

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = GetContentRegionAvail().x;

		static std::map<ImGuiID, slider_state> anim;
		auto it_anim = anim.find(id);
		if (it_anim == anim.end())
		{
			anim.insert({ id, slider_state() });
			it_anim = anim.find(id);
		}

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos + ImVec2(it_anim->second.total_offset, 0), pos + ImVec2(w, 44) + ImVec2(it_anim->second.total_offset, 0));
		const ImRect item_bb(total_bb.Min, total_bb.Max);
		const ImRect slider_bb(total_bb.Min + ImVec2(25, 15 + label_size.y - 2), total_bb.Max - ImVec2(25, 5));
		const ImRect minus_bb(slider_bb.Min - ImVec2(30, 5), ImVec2(slider_bb.Min.x, slider_bb.Max.y));
		const ImRect plus_bb(slider_bb.Max - ImVec2(0, slider_bb.GetSize().y + 5), slider_bb.Max + ImVec2(30, 0));
		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;

		BeginDisabled(it_anim->second.disabled);
		ItemSize(total_bb);
		ItemAdd(total_bb, id, &total_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0);
		if (format == NULL) format = DataTypeGetInfo(data_type)->PrintFmt;

		bool hovered = ItemHoverable(total_bb, id, g.LastItemData.InFlags), held, pressed = ButtonBehavior(total_bb, id, &hovered, &held, NULL);

		ImRect grab_bb;
		const bool value_changed = SliderBehavior(ImRect(slider_bb.Min - ImVec2(1.5f, 0), slider_bb.Max + ImVec2(10, 0)), id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
		if (value_changed) MarkItemEdited(id);

		char value_buf[64];
		const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

		it_anim->second.circle_radius = ImLerp(it_anim->second.circle_radius, held ? 5.f : hovered ? 4.5f : 4.f, GetAnimSpeed());
		it_anim->second.slow = ImLerp(it_anim->second.slow, grab_bb.Min.x - (slider_bb.Min.x), g.IO.DeltaTime * 25.f);
		it_anim->second.text_color = ImLerp(it_anim->second.text_color, held ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());

		it_anim->second.minus_col = ImLerp(it_anim->second.minus_col, ImGui::IsMouseHoveringRect(minus_bb.Min, minus_bb.Max) && ImGui::IsMouseDown(0) ? c::text::label::active : ImGui::IsMouseHoveringRect(minus_bb.Min, minus_bb.Max) ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());
		it_anim->second.plus_col = ImLerp(it_anim->second.plus_col, ImGui::IsMouseHoveringRect(plus_bb.Min, plus_bb.Max) && ImGui::IsMouseDown(0) ? c::text::label::active : ImGui::IsMouseHoveringRect(plus_bb.Min, plus_bb.Max) ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());

		it_anim->second.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			it_anim->second.total_offset = GetAnimOffset();
			it_anim->second.total_alpha = 0.f;
			it_anim->second.timer = 0.f;
		}

		float relative = (total_bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((it_anim->second.timer - delay) / 0.3f, 0.f, 1.f);

		it_anim->second.total_alpha = ImLerp(it_anim->second.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);

		PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.total_alpha);
		it_anim->second.timer += ImGui::GetIO().DeltaTime;
		
		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		

		GetWindowDrawList()->AddRectFilled(slider_bb.Min, slider_bb.Max, utils::GetColorWithAlpha(c::anim::default, style.Alpha), c::elements::rounding);
		fade_rect_filled(GetWindowDrawList(), slider_bb.Min, ImVec2(it_anim->second.slow + (slider_bb.Min.x + 6), slider_bb.Max.y), ImGui::GetColorU32(c::anim::active.Value), ImGui::GetColorU32(c::dark_color.Value), fade_direction::horizontally, c::elements::rounding, 0, true);

		ImVec2 grab_center(ImVec2(ImClamp(it_anim->second.slow + (slider_bb.Min.x + 6) - slider_bb.GetSize().y / 2, slider_bb.Min.x + 8, slider_bb.Max.x - 7), slider_bb.GetCenter().y));

		float dt = g.IO.DeltaTime;
		bool active_or_hovered = hovered || g.ActiveId == id;
		
		it_anim->second.grab_anim = easing->easing_in_out<float>(id * 55, active_or_hovered, 0.0f, 1.0f, dt * 20.f, interpolation_type::back);

		float extra = it_anim->second.grab_anim * 1.50f;
		ImVec2 grab_min = grab_center - ImVec2(9 + extra, 7 + extra);
		ImVec2 grab_max = grab_center + ImVec2(9 + extra, 7 + extra);

		//GetWindowDrawList()->AddShadowRect(grab_min, grab_max, ImColor(0.f, 0.f, 0.f, 1.f), 35.f, ImVec2(0, 0), ImDrawFlags_RoundCornersAll, c::elements::rounding);
		ItemGrab(id, ImRect(grab_min, grab_max), active_or_hovered);

		it_anim->second.disabled = ImGui::IsMouseHoveringRect(minus_bb.Min, minus_bb.Max) || ImGui::IsMouseHoveringRect(plus_bb.Min, plus_bb.Max);

		static DWORD dwTickStart = GetTickCount();

		if (it_anim->second.held_mode)
		{
			if (ImGui::IsMouseHoveringRect(minus_bb.Min, minus_bb.Max) && ImGui::IsMouseDown(0) && *(int*)p_data > *(int*)p_min)
				*(int*)p_data -= 1;

			if (ImGui::IsMouseHoveringRect(plus_bb.Min, plus_bb.Max) && ImGui::IsMouseDown(0) && *(int*)p_data < *(int*)p_max)
				*(int*)p_data += 1;
		}
		else
		{
			if (GetTickCount() - dwTickStart > 500)
				it_anim->second.held_mode = true;

			if (ImGui::IsMouseHoveringRect(minus_bb.Min, minus_bb.Max) && ImGui::IsMouseClicked(0) && *(int*)p_data > *(int*)p_min)
				*(int*)p_data -= 1;

			if (ImGui::IsMouseHoveringRect(plus_bb.Min, plus_bb.Max) && ImGui::IsMouseClicked(0) && *(int*)p_data < *(int*)p_max)
				*(int*)p_data += 1;
		}

		bool mouse_in_motion = (ImGui::GetIO().MouseDelta.x != 0 || ImGui::GetIO().MouseDelta.y != 0);

		if (g.ActiveId == id && mouse_in_motion)
		{
			for (int i = 0; i < particle::PARTICLE_COUT / 5; ++i)
				particle::AddParticle(grab_center, ImVec2(3, 3) / 10, 0);
		}

		if (!ImGui::IsMouseDown(0))
		{
			it_anim->second.held_mode = false;
			dwTickStart = GetTickCount();
		}

		GetWindowDrawList()->AddText(utils::center_text(minus_bb.Min, minus_bb.Max, "-"), GetColorU32(it_anim->second.minus_col), "-");
		GetWindowDrawList()->AddText(utils::center_text(plus_bb.Min, plus_bb.Max, "+"), GetColorU32(it_anim->second.plus_col), "+");
		GetWindowDrawList()->AddText(ImVec2(total_bb.Min.x, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), label);
		GetWindowDrawList()->AddText(ImVec2(slider_bb.Max.x - CalcTextSize(value_buf).x + 20, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), value_buf);
		ImGui::PopStyleVar();
		ImGui::EndDisabled();

		return value_changed;
	}

	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
	}

	bool Keybind(const char* label, int* key, int* mode)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;

		const ImGuiID id = window->GetID(label);
		const float width = GetContentRegionAvail().x;

		const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, 25));
		ItemSize(total_bb, 0.f);

		if (!ItemAdd(total_bb, id)) return false;

		static std::map<ImGuiID, key_state> anim;
		auto it_anim = anim.emplace(id, key_state()).first;

		bool hovered, held, pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		it_anim->second.alpha = ImLerp(it_anim->second.alpha,
			hovered ? 1.f : 0.8f, io.DeltaTime * 5.f);
		it_anim->second.background = ImLerp(it_anim->second.background,
		hovered ? c::anim::default: c::anim::default, io.DeltaTime * 5.f);
		it_anim->second.text = ImLerp(it_anim->second.text, hovered ? c::text::description::hovered : c::text::description::default, io.DeltaTime * 5.f);

		it_anim->second.text_color = ImLerp(it_anim->second.text_color, it_anim->second.status != KEYBIND_NONE ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::default, GetAnimSpeed());
		it_anim->second.description_col = ImLerp(it_anim->second.description_col, it_anim->second.status != KEYBIND_NONE ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::default, GetAnimSpeed());
		it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, it_anim->second.status == KEYBIND_WAITING ? c::anim::active : hovered ? c::description::hovered : c::description::default, GetAnimSpeed());


		ImVec2 key_bb_pos = ImVec2(total_bb.Max.x - it_anim->second.size_x, total_bb.Min.y);
		ImVec2 key_bb_size = ImVec2(total_bb.Max.x, total_bb.Max.y);

		ImRect key_bb(key_bb_pos, key_bb_size);

		// 🎯 Заголовок
		ImVec2 text_pos = ImVec2(total_bb.Min.x + 10.f, total_bb.Min.y + 10.f);
		GetWindowDrawList()->AddText(ImVec2(total_bb.Min.x, total_bb.GetCenter().y - CalcTextSize(label).y / 2), GetColorU32(it_anim->second.text_color), label);


		// 📝 Установка текста статуса
		if (it_anim->second.status == KEYBIND_WAITING) {
			it_anim->second.key_name = "...";
		}
		else if (*key != 0) {
			it_anim->second.key_name = keys[*key];
		}
		else {
			it_anim->second.key_name = "None";
		}

		ImVec2 key_text_size = CalcTextSize(it_anim->second.key_name);
		float target_size_x = ImMax(60.f, key_text_size.x + 55.f);
		it_anim->second.size_x = ImLerp(it_anim->second.size_x, target_size_x, io.DeltaTime * 14.f);

		GetWindowDrawList()->AddShadowRect(key_bb.Min, key_bb.Max, ImColor(0.f, 0.f, 0.f, 0.35f), 35.f, ImVec2(0, 0), ImDrawFlags_RoundCornersAll, c::elements::rounding);
		GetWindowDrawList()->AddRectFilled(key_bb.Min, key_bb.Max, GetColorU32(it_anim->second.background), style.FrameRounding);
		GetWindowDrawList()->AddText(ImVec2(key_bb.Min.x + 10, key_bb.GetCenter().y - key_text_size.y * 0.5f), ImColor(1.f, 1.f, 1.f, it_anim->second.alpha), it_anim->second.key_name);

		GetWindowDrawList()->AddRectFilled(ImVec2(key_bb.Max.x - 16.f - CalcTextSize(ICON_KEYBOARD_LINE).x, key_bb.Min.y), ImVec2(key_bb.Max.x - 15.f - CalcTextSize(ICON_KEYBOARD_LINE).x, key_bb.Max.y), utils::GetColorWithAlpha(c::text::description::default, 0.1f), 0.f);

		GetWindowDrawList()->AddText(ImVec2(key_bb.Max.x - 8.f - CalcTextSize(ICON_KEYBOARD_FILL).x, key_bb.GetCenter().y - CalcTextSize(ICON_KEYBOARD_FILL).y / 2), GetColorU32(it_anim->second.icon_col), ICON_KEYBOARD_FILL);



		// 🎮 Логика статусов
		if (pressed) {
			if (it_anim->second.status != KEYBIND_WAITING && !it_anim->second.clicked_to_waiting) {
				it_anim->second.status = KEYBIND_WAITING;
				it_anim->second.clicked_to_waiting = true;
				*key = 0;
			}
		}

		if (!io.MouseDown[0]) {
			it_anim->second.clicked_to_waiting = false;
		}

		bool value_changed = false;

		if (it_anim->second.status == KEYBIND_WAITING) {
			for (int i = 0; i < 5; i++) {
				if (io.MouseClicked[i]) {
					switch (i) {
					case 0: *key = 0x01; break;
					case 1: *key = 0x02; break;
					case 2: *key = 0x04; break;
					case 3: *key = 0x05; break;
					case 4: *key = 0x06; break;
					}
					it_anim->second.status = KEYBIND_ASSIGNED;
					value_changed = true;
				}
			}
			if (!value_changed) {
				for (int i = 0x08; i <= 0xA5; i++) {
					if (io.KeysDown[i]) {
						*key = i;
						it_anim->second.status = KEYBIND_ASSIGNED;
						value_changed = true;
					}
				}
			}

			if (IsKeyPressedMap(ImGuiKey_Escape)) {
				*key = 0;
				it_anim->second.status = KEYBIND_NONE;
			}
		}

		if (*key != 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_ASSIGNED;
		}
		else if (*key == 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_NONE;
		}

		return value_changed;
	}

	bool MiniBind(const char* label, int* key, int* mode)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;

		const ImGuiID id = window->GetID(label);
		const float width = 100;

		static std::map<ImGuiID, key_state> anim;
		auto it_anim = anim.emplace(id, key_state()).first;


		const ImRect frame_bb(window->DC.CursorPos + ImVec2(it_anim->second.total_offset, 0), window->DC.CursorPos + ImVec2(width, 40) + ImVec2(it_anim->second.total_offset, 0));

		ImVec2 key_bb_pos = frame_bb.Min;
		ImVec2 key_bb_size = ImVec2(ImGui::GetContentRegionAvail().x, frame_bb.GetSize().y);

		ImRect total_bb(key_bb_pos, key_bb_pos + key_bb_size);

		ItemSize(total_bb, 0.f);

		ItemAdd(total_bb, id);


		bool hovered, held, pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		it_anim->second.alpha = ImLerp(it_anim->second.alpha,
			hovered ? 1.f : 0.8f, io.DeltaTime * 5.f);

		it_anim->second.background = ImLerp(it_anim->second.background,
			it_anim->second.status == KEYBIND_WAITING ? utils::GetColorWithAlpha(c::anim::active, 0.3f) : c::anim::default, io.DeltaTime * 5.f);

		it_anim->second.text = ImLerp(it_anim->second.text, it_anim->second.status == KEYBIND_WAITING ? c::label::active : c::label::default, io.DeltaTime * 5.f);

		it_anim->second.icon_col = ImLerp(it_anim->second.icon_col, it_anim->second.status == KEYBIND_WAITING ? c::anim::active : hovered ? c::description::hovered : c::description::default, GetAnimSpeed());
		it_anim->second.text_color = ImLerp(it_anim->second.text_color, it_anim->second.status == KEYBIND_WAITING ? c::anim::active : c::label::default, GetAnimSpeed());
		it_anim->second.description_col = ImLerp(it_anim->second.description_col, it_anim->second.status != KEYBIND_NONE ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::default, GetAnimSpeed());

		ImVec2 text_pos = ImVec2(total_bb.Min.x + 10.f, total_bb.Min.y + 10.f);

		if (it_anim->second.status == KEYBIND_WAITING) {
			it_anim->second.key_name = "...";
		}
		else if (*key != 0) {
			it_anim->second.key_name = keys[*key];
		}
		else {
			it_anim->second.key_name = "None";
		}

		ImVec2 key_text_size = CalcTextSize(it_anim->second.key_name);
		float target_size_x = key_text_size.x + 20;
		it_anim->second.size_x = ImLerp(it_anim->second.size_x, target_size_x, io.DeltaTime * 14.f);


		it_anim->second.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			it_anim->second.total_offset = GetAnimOffset();
			it_anim->second.total_alpha = 0.f;
			it_anim->second.timer = 0.f;
		}

		it_anim->second.timer += ImGui::GetIO().DeltaTime;
		float relative = (total_bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((it_anim->second.timer - delay) / 0.3f, 0.f, 1.f);
		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		it_anim->second.total_alpha = ImLerp(it_anim->second.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);

		PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.total_alpha);

		/*GetWindowDrawList()->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, 0.35f), 35.f, ImVec2(0, 0), ImDrawFlags_RoundCornersAll, c::elements::rounding);*/
		GetWindowDrawList()->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.background), c::elements::rounding);
		GetWindowDrawList()->AddText(total_bb.GetCenter() - key_text_size * 0.5f, utils::GetColorWithAlpha(it_anim->second.text_color, it_anim->second.text_color.w * it_anim->second.alpha), it_anim->second.key_name);
						

		// 🎮 Логика статусов
		if (pressed) {
			if (it_anim->second.status != KEYBIND_WAITING && !it_anim->second.clicked_to_waiting) {
				it_anim->second.status = KEYBIND_WAITING;
				it_anim->second.clicked_to_waiting = true;
				*key = 0;
			}
		}

		if (!io.MouseDown[0]) {
			it_anim->second.clicked_to_waiting = false;
		}

		bool value_changed = false;

		if (it_anim->second.status == KEYBIND_WAITING) {
			for (int i = 0; i < 5; i++) {
				if (io.MouseClicked[i]) {
					switch (i) {
					case 0: *key = 0x01; break;
					case 1: *key = 0x02; break;
					case 2: *key = 0x04; break;
					case 3: *key = 0x05; break;
					case 4: *key = 0x06; break;
					}
					it_anim->second.status = KEYBIND_ASSIGNED;
					value_changed = true;
				}
			}
			if (!value_changed) {
				for (int i = 0x08; i <= 0xA5; i++) {
					if (io.KeysDown[i]) {
						*key = i;
						it_anim->second.status = KEYBIND_ASSIGNED;
						value_changed = true;
					}
				}
			}

			if (IsKeyPressedMap(ImGuiKey_Escape)) {
				*key = 0;
				it_anim->second.status = KEYBIND_NONE;
			}
		}

		if (*key != 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_ASSIGNED;
		}
		else if (*key == 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_NONE;
		}

		PopStyleVar();

		return value_changed;
	}

	bool Bindbox(const char* label, bool* v, int* key, int* mode)
	{
		std::string label_str = label;
		std::string key_str = label + std::string("##keybind");

		custom::MiniBind("Hello keybind", key, mode);
		ImGui::SameLine();

		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, 2.5));

		custom::Checkbox(label, v);

		return false;
	}

	struct toggle_state {
		ImVec4 background;
		float smooth_swap, alpha_line, line_size;

		float total_offset = -400.f;
		float total_alpha = 0.f;
		float timer;
		bool is_visible, can_view;
	};

	bool ToggleButton(const char* first_text, const char* second_text, bool* v, const ImVec2& size_arg) {
		ImGuiWindow* window = GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(first_text);


		static std::map<ImGuiID, toggle_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end()) {
			anim.insert({ id, toggle_state() });
			it_anim = anim.find(id);
		}

		const ImVec2 label_size = CalcTextSize(first_text, NULL, true), pos = window->DC.CursorPos + ImVec2(it_anim->second.total_offset, 0.f);

		ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

		const ImRect bb(pos, pos + size);

		const ImRect dark_bb(bb.Min, bb.Min + ImVec2(size.x / 2, size.y));
		const ImRect white_bb(bb.Min + ImVec2(size.x / 2, 0.f), bb.Max);

		ItemSize(size, 0.f);
		ItemAdd(bb, id);

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);


		if (pressed)
			*v = !*v;


		it_anim->second.is_visible = !tab_want_to_change;
		if (tab_want_to_change || ImGui::GetWindowSize().y < 35) {
			it_anim->second.total_offset = GetAnimOffset();
			it_anim->second.total_alpha = 0.f;
			it_anim->second.timer = 0.f;
		}

		it_anim->second.timer += ImGui::GetIO().DeltaTime;
		float relative = (bb.GetCenter().y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y;
		float delay = 0.1f + relative * 0.9f;
		float t = ImClamp((it_anim->second.timer - delay) / 0.3f, 0.f, 1.f);
		it_anim->second.total_offset = ImLerp(it_anim->second.total_offset, 0.f, ImGui::GetIO().DeltaTime * t * 4);
		it_anim->second.total_alpha = ImLerp(it_anim->second.total_alpha, ImGui::GetStyle().Alpha, ImGui::GetIO().DeltaTime * t * 3);


		it_anim->second.smooth_swap = ImLerp(it_anim->second.smooth_swap, *v ? 0.f : size.x / 2, g.IO.DeltaTime * 12.f);

		PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.total_alpha);
		GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, utils::GetColorWithAlpha(c::anim::default, 1.f - (1.f - style.Alpha)), c::page::rounding);

		GetWindowDrawList()->AddRectFilled(bb.Min + ImVec2(it_anim->second.smooth_swap, 0.f), ImVec2(bb.Min.x + size.x / 2 + it_anim->second.smooth_swap, bb.Max.y), utils::GetColorWithAlpha(c::anim::active, 0.5f - (1.f - style.Alpha)), c::page::rounding);

		GetWindowDrawList()->AddText(utils::center_text(dark_bb.Min, dark_bb.Max, first_text), utils::GetColorWithAlpha(c::anim::active, 1.f - (1.f - style.Alpha)), first_text);
		GetWindowDrawList()->AddText(utils::center_text(white_bb.Min, white_bb.Max, second_text), utils::GetColorWithAlpha(c::anim::active, 1.f - (1.f - style.Alpha)), second_text);
		PopStyleVar();

		return pressed;
	}


	bool Featuresbox(const char* label, bool* v)
	{
		std::string label_str = label;
		std::string key_str = label + std::string("##keybind");
		std::string popup_str = label + std::string("##popup");
		
		custom::Checkbox(label, v);

		ImGui::SameLine(0, 0);
		if (custom::rotated_text(ICON_SETTINGS_4_FILL, false))
			return true;

		return false;
	}

	void DrawHexGrid(
		const std::vector<std::string>& labels,
		const std::vector<int>& indicators)
	{
		struct HexState { bool active = false; ImVec4 bg_col = c::child::background; float ind_size = 0.0f; };
		static std::map<ImGuiID, HexState> anim;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* draw = window->DrawList;
		float         dt = ImGui::GetIO().DeltaTime;
		ImVec2        origin = ImGui::GetCursorScreenPos();
		float         availX = ImGui::GetContentRegionAvail().x;

		const float r = 40.0f;
		const float hexW = 2.0f * r;
		const float hexH = sqrtf(3.0f) * r;
		const float xStep = hexW * 1.0f;
		const float yStep = hexH;

		const int counts[5] = { 5, 6, 5, 6, 5 };

		int idx = 0;
		for (int row = 0; row < 5; ++row)
		{
			int cnt = counts[row];
			float rowTotalWidth = (cnt - 1) * xStep + hexW;
			float startX = origin.x + (availX - rowTotalWidth) * 0.5f + r;
			float y = origin.y + row * yStep + r;

			for (int col = 0; col < cnt; ++col)
			{
				ImGui::PushID(idx);
				ImGuiID id = window->GetID("hex");
				auto& s = anim[id];

				ImVec2 cen(startX + col * xStep, y);

				ImRect bb(cen - ImVec2(r, r), cen + ImVec2(r, r));
				ImGui::ItemAdd(bb, id);

				bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held);
				if (pressed) s.active = !s.active;

				s.bg_col = easing->easing_in_out<ImVec4>(id * 2, s.active, c::anim::default, c::anim::active.Value, dt * 30.f, interpolation_type::back);
				s.ind_size = easing->easing_in_out<float>(id * 3, s.active, 0.0f, r * 0.15f, dt * 30.f, interpolation_type::back);

				ImVec2 pts[6];
				for (int i = 0; i < 6; ++i)
				{
					float a = IM_PI / 6 + i * IM_PI / 3;
					pts[i] = ImVec2(cen.x + cosf(a) * r, cen.y + sinf(a) * r);
				}
				draw->AddConvexPolyFilled(pts, 6, GetColorU32(s.bg_col));
				draw->AddPolyline(pts, 6, GetColorU32(c::child::stroke), true, 1.0f);

				const char* lab = labels[idx].c_str();
				ImVec2 ts = ImGui::CalcTextSize(lab);
				draw->AddText(
					ImVec2(cen.x - ts.x * 0.5f, cen.y - ts.y * 0.5f),
					c::label::default,
					lab
				);

				if (idx < (int)indicators.size() && indicators[idx] != 0)
				{
					int v = indicators[idx];
					ImVec4 col = ImVec4(1, 1, 1, 0.1f);
					if (v == 1) col = ImVec4(0, 1, 0, 1);
					if (v == 2) col = ImVec4(1, 0, 0, 1);

					float r2 = r * 0.15f;
					ImVec2 cen2(cen.x, cen.y + r * 0.8f);
					ImVec2 pts2[6];
					for (int i = 0; i < 6; ++i)
					{
						float a = IM_PI / 6 + i * IM_PI / 3;
						pts2[i] = ImVec2(cen2.x + cosf(a) * r2, cen2.y + sinf(a) * r2);
					}
					draw->AddConvexPolyFilled(pts2, 6, GetColorU32(col));
				}

				ImGui::PopID();
				++idx;
			}
		}

		ImGui::Dummy(ImVec2(availX, 5 * yStep + hexH));
	}

}

