#pragma once
#include "../Sounds.h"
#include "..//HitPossitionHelper.h"
#include "..//runtime_saver.h"
#include "..//esp.hpp"
#include "../soundesp.h"

#pragma comment(lib, "Winmm.lib") //PlaySoundA fix

ConVar* game_type = nullptr;
ConVar* game_mode = nullptr;

std::map<std::string, std::string> weaponNames =
{
{ "weapon_ak47", "AK47" },
{ "weapon_aug", "AUG" },
{ "weapon_famas", "FAMAS" },
{ "weapon_galilar", "Galil AR" },
{ "weapon_m4a1", "M4A1" },
{ "weapon_m4a1_silencer", "M4A1-S" },
{ "weapon_sg556", "SG 556" },

{ "weapon_awp", "AWP" },
{ "weapon_g3sg1", "G3SG1" },
{ "weapon_scar20", "SCAR20" },
{ "weapon_ssg08", "SSG 08" },

{ "weapon_mac10", "MAC10" },
{ "weapon_mp7", "MP7" },
{ "weapon_mp9", "MP9" },
{ "weapon_bizon", "PP-Bizon" },
{ "weapon_p90", "P90" },
{ "weapon_ump45", "UMP-45" },
{ "weapon_mp5sd", "MP5-SD" },

{ "weapon_m249", "M249" },
{ "weapon_negev", "Negev" },

{ "weapon_hkp2000", "P2000" },
{ "weapon_usp_silencer", "USP-S" },
{ "weapon_p250", "P250" },
{ "weapon_elite", "Dual Berettas" },
{ "weapon_fiveseven", "Five-Seven" },
{ "weapon_glock", "Glock" },
{ "weapon_tec9", "Tec-9" },
{ "weapon_deagle", "Deagle" },
{ "weapon_cz75a", "CZ75-A" },
{ "weapon_revolver", "Revolver" },

{ "weapon_mag7", "MAG-7" },
{ "weapon_nova", "Nova" },
{ "weapon_sawedoff", "Sawed Off" },
{ "weapon_xm1014", "XM1014" },

{ "item_defuser", "Defuse Kit" },
{ "item_kevlar", "Kevlar" },
{ "item_assaultsuit", "Kevlar + Helmet" },

{ "weapon_hegrenade", "HE" },
{ "weapon_flashbang", "Flashbang" },
{ "weapon_molotov", "Molotov" },
{ "weapon_incendiary", "Incendiary" },
{ "weapon_decoy", "Decoy" },
{ "weapon_taser", "Zeus x27" },
};

std::string allcolors[] =
{
	" \x01", " \x02", " \x03", " \x04", " \x05", " \x06", " \x7", " \x8", " \x9", " \x0A", " \x0B", " \x0C", " \x0D", " \x0E", " \x0F", " \x10"
};

void WeaponCheck(std::string weapon, c_base_player* player)
{
	std::string wepName;
	
	auto it = weaponNames.find(weapon);
	if (it != weaponNames.end())
		wepName = it->second;
	else
		wepName = weapon;

	auto filter = CHudChat::ChatFilters::CHAT_FILTER_NONE;
	static int green = 3;
	static int yellow = 15;
	static int white = 0;

	std::stringstream text;
	text << allcolors[yellow] << "[Buy]" << " " << allcolors[green] << "Player " << allcolors[white] << player->GetPlayerInfo().szName << allcolors[green] << "bought" << allcolors[white] << wepName;

	g::hud_chat->ChatPrintf(0, filter, text.str().c_str());
}

class c_game_event_listener final : public IGameEventListener2
{
	const char* hitgroups[10] = 
	{
		"generic", "head", "chest", "stomach", "arm", "arm", "leg", "leg", "gear"
	};

	void FireGameEvent(IGameEvent* context) override
	{
		const auto name = fnv::hash_runtime(context->GetName());

		if (name == FNV("game_newmap"))
		{
			no_smoke::event();
			color_modulation::event();
			globals::team_damage.clear();
		}
		else if (name == FNV("player_footstep"))
		{
			sound_esp.event_player_footstep(context);
		}
		else if (name == FNV("player_death"))
		{
			auto attacker = c_base_player::GetPlayerByUserId(context->GetInt("attacker"));
			auto target = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!attacker || !target)
				return;

			if (attacker->m_iTeamNum() == target->m_iTeamNum())
				globals::team_kill[context->GetInt("attacker")] += globals::teamkills + 1;

		}
		else if (name == FNV("player_hurt"))
		{
			auto attacker = c_base_player::GetPlayerByUserId(context->GetInt("attacker"));
			auto target = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!attacker || !target)
				return;

			HitPossitionHelper::Get().OnFireEvent(context);

			sound_esp.event_player_hurt(context);

			if (attacker->m_iTeamNum() == target->m_iTeamNum())
				globals::team_damage[context->GetInt("attacker")] += context->GetInt("dmg_health");

			if (attacker == interfaces::local_player && target != interfaces::local_player)
			{
				char buf[256];
				sprintf_s(buf, "%s -%d (%d hp left)",target->GetPlayerInfo().szName, context->GetInt("dmg_health"), context->GetInt("health"));

				notifies::push(buf, notify_state_s::debug_state);

					/*if (!game_mode)
						game_mode = interfaces::cvar->find("game_mode");

					if (!game_type)
						game_type = interfaces::cvar->find("game_type");

					if (game_type->GetInt() == 0 && game_mode->GetInt() == 0) //casual
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 1) //demolition
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 0) //arms race
						return;

					if (game_type->GetInt() == 1 && game_mode->GetInt() == 2) //deathmatch
						return;  */
			}

			if (settings::misc::damage_indicator)
			{
				if (target != g::local_player && attacker == g::local_player)
				{
					dmg_indicator DmgIndicator;
					DmgIndicator.iDamage = context->GetInt("dmg_health");
					DmgIndicator.Player = target;
					DmgIndicator.flEraseTime = g::local_player->m_nTickBase() * g::global_vars->interval_per_tick + 2.f; //was 3.f
					DmgIndicator.bInitialized = false;

					indicator.push_back(DmgIndicator);
				}
			}

			if (settings::visuals::hitmarker)
			{

				int _attacker = context->GetInt("attacker");
				if (g::engine_client->GetPlayerForUserID(_attacker) == g::engine_client->GetLocalPlayer())
				{
					using namespace Sounds;
					saver.HitmarkerInfo = HitmarkerInfoStruct{ g::global_vars->realtime, 0.f };
					switch (settings::visuals::hitsound)
					{
					case 0:
						//PlaySoundA((g_Config.AppdataFolder + "hitsound0.wav").data(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
						PlaySoundA(cod, NULL, SND_ASYNC | SND_MEMORY); //cod sound
						break;
					case 1:
						//PlaySoundA((g_Config.AppdataFolder + "hitsound1.wav").data(), NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
						PlaySoundA(skeet, NULL, SND_ASYNC | SND_MEMORY); //skeet sound
						break;
					case 2:
						PlaySoundA(punch, NULL, SND_ASYNC | SND_MEMORY); //punch sound
						break;
					case 3:
						PlaySoundA(metal, NULL, SND_ASYNC | SND_MEMORY); //metal sound
						break;
					case 4:
						PlaySoundA(boom, NULL, SND_ASYNC | SND_MEMORY); //boom sound
						break;
					}
				}
			}
		}
		else if (name == FNV("bullet_impact"))
		{
			HitPossitionHelper hp;

			c_base_player* shooter = static_cast<c_base_player*>(g::entity_list->GetClientEntity(g::engine_client->GetPlayerForUserID(context->GetInt("userid"))));

			if (!shooter || shooter != g::local_player)
				return;

			Vector p = Vector(context->GetFloat("x"), context->GetFloat("y"), context->GetFloat("z"));
			hp.ShotTracer(saver.LastShotEyePos, p);
		}
		else if (name == FNV("item_purchase"))
		{
			auto enemy = c_base_player::GetPlayerByUserId(context->GetInt("userid"));
			if (!enemy || !interfaces::local_player || enemy->m_iTeamNum() == interfaces::local_player->m_iTeamNum())
				return;

			if (!game_mode)
				game_mode = interfaces::cvar->find("game_mode");

			if (!game_type)
				game_type = interfaces::cvar->find("game_type");

			if (game_type->GetInt() == 0 && game_mode->GetInt() == 0) //casual
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 1) //demolition
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 0) //arms race
				return;

			if (game_type->GetInt() == 1 && game_mode->GetInt() == 2) //deathmatch
				return;

			std::string buf2 = context->GetString("weapon");

			WeaponCheck(buf2, enemy);
		}
		else if (name == FNV("round_start") && settings::misc::esp_random)
		{
			int chance;

			chance = rand() % 100 + 1;

			char number[256];

			sprintf_s(number, "Chance: %i%%", chance);

			notifies::push(number, notify_state_s::debug_state);

			auto filter = CHudChat::ChatFilters::CHAT_FILTER_NONE;
			static int green = 3;
			static int yellow = 15;
			static int white = 0;

			std::stringstream txt;
			txt << allcolors[yellow] << "[Info]" << allcolors[white] << " " << number << "%";

			g::hud_chat->ChatPrintf(0, filter, txt.str().c_str());

			if (chance >= settings::esp::esp_chance)
			{
				settings::esp::visible_only = false;

				/* CHAMS MODES: */

				if (settings::chams::enemymodenew == 0) //Normal 
					settings::chams::enemymodenew = 5; //XQZ

				if (settings::chams::enemymodenew == 1) //Flat
					settings::chams::enemymodenew = 7; //Flat XQZ

				if (settings::chams::enemymodenew == 2) //Wireframe
					settings::chams::enemymodenew = 5; //XQZ

				if (settings::chams::enemymodenew == 3) //Glass
					settings::chams::enemymodenew = 5; //XQZ

				if (settings::chams::enemymodenew == 4) //Metallic
					settings::chams::enemymodenew = 6; //Metallic XQZ

			}
			else if (chance < settings::esp::esp_chance)
			{
				settings::esp::visible_only = true;

				/* CHAMS MODES: */

				if (settings::chams::enemymodenew == 5) //XQZ
					settings::chams::enemymodenew = 0; //Normal

				if (settings::chams::enemymodenew == 7) //Flat XQZ
					settings::chams::enemymodenew = 1; //Flat

				if (settings::chams::enemymodenew == 5) //XQZ
					settings::chams::enemymodenew = 2; //Wireframe

				if (settings::chams::enemymodenew == 5) //XQZ
					settings::chams::enemymodenew = 3; //Glass

				if (settings::chams::enemymodenew == 6) //Metallic XQZ
					settings::chams::enemymodenew = 4; //Metallic

			}
		}
		else if (name == FNV("cs_pre_restart") || name == FNV("switch_team") || name == FNV("announce_phase_end") || name == FNV("round_freeze_end"))
			clantag::restore();
		else if (name == FNV("bullet_impact") && settings::esp::beams)
			utils::create_beam(context->GetInt("userid"), Vector(context->GetFloat("x"), context->GetFloat("y"), context->GetFloat("z")));
	}

	int GetEventDebugID(void) override
	{
		return EVENT_DEBUG_ID_INIT;
	}
};