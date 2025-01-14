#include "render.h"
#include "../globals.h"
#include "../options.hpp"
#include "../helpers/imdraw.h"
#include "../helpers/entities.h"
#include "../features/features.h"
#include "..//helpers/notifies.h"
#include "render.h"
#include "..//Sounds.h"
#include "..//esp.hpp"

namespace render
{
	namespace timers
	{
		auto m_Framerate = 0.f;

		ConVar* cl_hud_playercount_pos = nullptr;

		decltype(entities::m_local) m_local;

		void render_panel()
		{
			if (!cl_hud_playercount_pos)
				cl_hud_playercount_pos = interfaces::cvar->find("cl_hud_playercount_pos");

			if (!cl_hud_playercount_pos)
				return;

			ImGui::PushFont(render::fonts::low_size);

			const auto screen_size = ImGui::GetIO().DisplaySize;

			const auto is_top = cl_hud_playercount_pos->GetBool();

			const auto start_color = ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
			const auto end_color = ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.f));

			static const auto fps_text_size = ImGui::CalcTextSize("FPS");

			const auto start_pos = ImVec2(screen_size.x / 2.f - 240.f, (is_top ? fps_text_size.y + 4.f : screen_size.y - fps_text_size.y - 4.f));
			const auto end_pos = ImVec2(screen_size.x / 2.f + 240.f, is_top ? 0.f : screen_size.y);

			globals::draw_list->AddRectFilledMultiColor(
				start_pos, ImVec2(start_pos.x + 240.f, end_pos.y), start_color, end_color, end_color, start_color
			);

			globals::draw_list->AddRectFilledMultiColor(
				ImVec2(start_pos.x + 240.f, start_pos.y), end_pos, end_color, start_color, start_color, end_color
			);

			char fps[256];
			sprintf(fps, "%4i", (int)(1.0f / m_Framerate));

			ImVec2 txt_size;

			//256 FPS - 36 and +-10 px
			txt_size = ImGui::CalcTextSize(fps);
			auto text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);

			const auto fps_color = interfaces::global_vars->interval_per_tick * 0.8f < interfaces::global_vars->absoluteframetime ? ImVec4(1.f, 0.37f, 0.15f, 1.f) : ImVec4(0.64f, 0.82f, 0.45f, 1.f);

			//imdraw::outlined_text(fps, text_pos, ImGui::GetColorU32(fps_color));

			txt_size = ImGui::CalcTextSize("FPS");
			text_pos = ImVec2(start_pos.x + 240.f + 2.f, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);
			//imdraw::outlined_text("FPS", text_pos, ImGui::GetColorU32(ImVec4::White));

			//

			if (m_local.is_matchmaking)
			{
				char bomb_time[16];
				sprintf_s(bomb_time, "%.2f", m_local.bomb_time);

				const auto ping_text_size = ImGui::CalcTextSize("BOMB");

				txt_size = ImGui::CalcTextSize(bomb_time);

				const auto bomb_color = m_local.bomb_time > 0.f ? ImVec4(0.88f, 0.82f, 0.45f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f);


				text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x - 25.f - 30.f - ping_text_size.x - 4.f - txt_size.x, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);
				imdraw::outlined_text(bomb_time, text_pos, ImGui::GetColorU32(bomb_color));

				if (m_local.AfterPlant == false)
				{
					text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x - 25.f - 30.f - ping_text_size.x, !is_top ? end_pos.y - 2.f - ping_text_size.y : end_pos.y + 2.f);
					imdraw::outlined_text("BOMB", text_pos, ImGui::GetColorU32(ImVec4::White));
				}

				if (m_local.bomb_indexStatus == 0 && m_local.isBombPlantedStatus && m_local.AfterPlant == true)
				{
					text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x - 25.f - 30.f - ping_text_size.x, !is_top ? end_pos.y - 2.f - ping_text_size.y : end_pos.y + 2.f);
					imdraw::outlined_text("BOMB A", text_pos, ImGui::GetColorU32(ImVec4::White));
				}
				else if (m_local.bomb_indexStatus == 1 && m_local.isBombPlantedStatus && m_local.AfterPlant == true)
				{
					text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x - 25.f - 30.f - ping_text_size.x, !is_top ? end_pos.y - 2.f - ping_text_size.y : end_pos.y + 2.f);
					imdraw::outlined_text("BOMB B", text_pos, ImGui::GetColorU32(ImVec4::White));
				}

				//

				char defuse_time[16];
				sprintf_s(defuse_time, "%.2f", m_local.defuse_time);

				txt_size = ImGui::CalcTextSize(defuse_time);
				text_pos = ImVec2(start_pos.x + 240.f + 2.f + 25.f + 30.f + 22.f - txt_size.x - 4.f, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);

				const auto defuse_color = m_local.defuse_time > 0.f ? ImVec4(0.f, 1.f, 0.15f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f);
				imdraw::outlined_text(defuse_time, text_pos, ImGui::GetColorU32(defuse_color));

				const auto speed_text_size = ImGui::CalcTextSize("DEFUSE");

				text_pos = ImVec2(start_pos.x + 240.f + 2.f + 25.f + 30.f + 22.f, !is_top ? end_pos.y - 2.f - speed_text_size.y : end_pos.y + 2.f);
				imdraw::outlined_text("DEFUSE", text_pos, ImGui::GetColorU32(ImVec4::White));

				//

				char hp[16];
				sprintf_s(hp, "%d", m_local.hp);

				const auto TXTsize = ImGui::CalcTextSize("HP");

				txt_size = ImGui::CalcTextSize(hp);

				//const auto bomb_color = m_local.bomb_time > 0.f ? ImVec4(0.88f, 0.82f, 0.45f, 1.f) : ImVec4(1.f, 1.f, 1.f, 1.f);



				if (g::local_player->IsAlive() && m_local.damage <= g::local_player->m_iHealth()) {
					text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size.x, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);
					imdraw::outlined_text(hp, text_pos, ImGui::GetColorU32(bomb_color));

					text_pos = ImVec2(start_pos.x + 240.f + 2.f, !is_top ? end_pos.y - 2.f - txt_size.y : end_pos.y + 2.f);
					imdraw::outlined_text("HP", text_pos, ImGui::GetColorU32(ImVec4::White));
				}

				//render fatal check
				if (g::local_player->IsAlive() && m_local.damage >= g::local_player->m_iHealth()) {
					auto txt_size2 = ImGui::CalcTextSize("0");
					text_pos = ImVec2(start_pos.x + 240.f - 2.f - txt_size2.x, !is_top ? end_pos.y - 2.f - txt_size2.y : end_pos.y + 2.f);
					imdraw::outlined_text("0", text_pos, ImGui::GetColorU32(ImVec4::Red));

					text_pos = ImVec2(start_pos.x + 240.f + 2.f, !is_top ? end_pos.y - 2.f - txt_size2.y : end_pos.y + 2.f);
					imdraw::outlined_text("HP", text_pos, ImGui::GetColorU32(ImVec4::White));
				}
			}

			struct player_info_t
			{
				bool is_ct_team;
				std::string name;
				int money;
				int hp;
				int id;
				int level;
				int wins;
				float distance;
			};

			struct grief_box_info_t
			{
				std::string name;
				int id;
			};

			std::vector<player_info_t> pinfo;
			std::vector<grief_box_info_t> ginfo;

			auto player_resource = *interfaces::player_resource;
			if (!player_resource)
				return;

			for (int i = 1; i < interfaces::engine_client->GetMaxClients(); ++i)
			{
				c_base_player* player = c_base_player::GetPlayerByIndex(i);

				if (!player)
					continue;

				auto info = player->GetPlayerInfo();

				//std::string a = std::to_string(logbuy::buy[257]);

				if (player->IsEnemy() && player->IsPlayer() && !(player->GetPlayerInfo().ishltv))
				{

					pinfo.push_back({
						player->m_iTeamNum() == team::team_ct,
						info.szName,
						player->m_iAccount(),
						player->m_iHealth(),
						player->GetPlayerInfo().userId,
						player_resource->GetLevel()[player->GetIndex()],
						player_resource->GetWins()[player->GetIndex()],
						player->IsDead() ? 0 : g::local_player->m_vecOrigin().DistTo(player->m_vecOrigin())
						});
				}

				if (!(player->IsEnemy()) && player->IsPlayer() && !(player->GetPlayerInfo().ishltv))
				{
					ginfo.push_back({
						info.szName,
						player->GetPlayerInfo().userId
						});
				}
			}

			if (settings::visuals::player_info_box)
			{
				ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;
				ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);
				ImGui::SetNextWindowSize(ImVec2(215.f, 130.f)); //175
				ImGui::SetNextWindowBgAlpha(settings::visuals::player_info_box_alpha);

				if (ImGui::Begin("Player Info Box", &settings::visuals::player_info_box, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
				{

					columns(6);
					{
						ImGui::SetColumnWidth(-1, 40.f);
						ImGui::Text(___("Name", u8"���"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 40.f);
						ImGui::Text(___("Money", u8"������"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 25.f); //48
						ImGui::Text(___(" HP", u8"�����"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 30.f); //45
						ImGui::Text(___("Level", u8"�����"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 30.f);
						ImGui::Text(___("Wins", u8"�����"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 30.f);
						ImGui::Text(___("Dist", u8"�����"));
					}
					columns(1);

					for (const auto it : pinfo)
					{

						/*ImGui::Text("%s", it.name.c_str());
						ImGui::SameLine();
						ImGui::Text("$%i", it.money);
						ImGui::SameLine();
						ImGui::Text("%i", it.hp);
						ImGui::SameLine();
						ImGui::Text("%d/300", globals::team_damage[it.id]); */

						columns(6);
						{
							ImGui::SetColumnWidth(-1, 40.f);
							ImGui::Text("%s", it.name.c_str());

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 40.f);
							ImGui::Text("$%i", it.money);

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 25.f);
							ImGui::Text("%i", it.hp); //globals::team_damage[it.id]

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 30.f);
							ImGui::Text("%i", it.level == -1 ? 0 : it.level);

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 30.f);
							ImGui::Text("%i", it.wins);

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 30.f);
							ImGui::Text("%1.f", it.distance);
						}
						columns(1);
					}
				}
				pinfo.clear();

				ImGui::End();

				ImGui::GetStyle().WindowMinSize = OldMinSize;
			}

			if (settings::visuals::grief_box)
			{
				ImVec2 OldMinSize = ImGui::GetStyle().WindowMinSize;
				ImGui::GetStyle().WindowMinSize = ImVec2(0.f, 0.f);
				ImGui::SetNextWindowSize(ImVec2(145.f, 130.f)); //145.f
				ImGui::SetNextWindowBgAlpha(settings::visuals::grief_box_alpha);

				if (ImGui::Begin("Grief Box", &settings::visuals::grief_box, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
				{

					columns(3);
					{
						ImGui::SetColumnWidth(-1, 40.f);
						ImGui::Text(___("Name", u8"���"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 48.f);
						ImGui::Text(___("Damage", u8"�����"));

						ImGui::NextColumn();

						ImGui::SetColumnWidth(-1, 45.f);
						ImGui::Text(___("Kills", u8"�����"));
					}
					columns(1);

					for (const auto it : ginfo)
					{

						columns(3);
						{
							ImGui::SetColumnWidth(-1, 40.f);
							ImGui::Text("%s", it.name.c_str());

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 48.f);
							ImGui::Text(" %d/300", globals::team_damage[it.id]);

							ImGui::NextColumn();

							ImGui::SetColumnWidth(-1, 45.f);
							ImGui::Text(" %d/3", globals::team_kill[it.id]);
						}
						columns(1);
					}
				}
				pinfo.clear();

				ImGui::End();

				ImGui::GetStyle().WindowMinSize = OldMinSize;
			}

			ImGui::PopFont();
		}

		void show()
		{
			m_Framerate = 0.9 * m_Framerate + (1.0 - 0.9) * interfaces::global_vars->absoluteframetime;

			if (!interfaces::local_player || !render::fonts::low_size)
				return;

			if (entities::local_mutex.try_lock())
			{
				m_local = entities::m_local;

				entities::local_mutex.unlock();
			}

			render_panel();
		}
	}
}