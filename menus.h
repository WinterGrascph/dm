/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_MENUS_H
#define DM_MENUS_H

#include "dm.h"
#include "champion.h"

namespace DM {

extern Box g1_BoxActionArea; // @ G0001_s_Graphic562_Box_ActionArea
extern Box g0_BoxSpellArea; // @ G0000_s_Graphic562_Box_SpellArea

class ActionList {
public:
	byte _minimumSkillLevel[3]; /* Bit 7: requires charge, Bit 6-0: minimum skill level. */
	ChampionAction _actionIndices[3];
	ActionList() {
		for (uint16 i = 0; i < 3; ++i) {
			_minimumSkillLevel[i] = 0;
			_actionIndices[i] = k255_ChampionActionNone;
		}
	}
}; // @ ACTION_LIST

class MenuMan {
	DMEngine *_vm;
public:
	explicit MenuMan(DMEngine *vm);
	~MenuMan();

	bool _g508_refreshActionArea; // @ G0508_B_RefreshActionArea
	bool _g509_actionAreaContainsIcons; // @ G0509_B_ActionAreaContainsIcons
	int16 _g513_actionDamage; // @ G0513_i_ActionDamage
	ActionList _g713_actionList; // @ G0713_s_ActionList
	byte *_gK72_bitmapSpellAreaLine; // @ K0072_puc_Bitmap_SpellAreaLine

	void f388_clearActingChampion(); // @ F0388_MENUS_ClearActingChampion
	void f386_drawActionIcon(ChampionIndex championIndex); // @ F0386_MENUS_DrawActionIcon

	void f395_drawMovementArrows(); // @ F0395_MENUS_DrawMovementArrows
	void f456_drawDisabledMenu(); // @ F0456_START_DrawDisabledMenus
	void f390_refreshActionAreaAndSetChampDirMaxDamageReceived(); // @ F0390_MENUS_RefreshActionAreaAndSetChampionDirectionMaximumDamageReceived
	void f387_drawActionArea(); // @ F0387_MENUS_DrawActionArea
	const char* f384_getActionName(ChampionAction actionIndex); // @ F0384_MENUS_GetActionName
	void f393_drawSpellAreaControls(ChampionIndex champIndex); // @ F0393_MENUS_DrawSpellAreaControls
	void f392_buildSpellAreaLine(int16 spellAreaBitmapLine);// @ F0392_MENUS_BuildSpellAreaLine
	void f394_setMagicCasterAndDrawSpellArea(int16 champIndex); // @ F0394_MENUS_SetMagicCasterAndDrawSpellArea
	void f457_drawEnabledMenus(); // @ F0457_START_DrawEnabledMenus_CPSF
};

}

#endif // !DM_MENUS_H
