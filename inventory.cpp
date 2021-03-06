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

#include <graphics/surface.h>
#include "graphics/thumbnail.h"

#include "inventory.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "gfx.h"
#include "text.h"
#include "objectman.h"
#include "timeline.h"
#include "projexpl.h"
#include "sounds.h"


namespace DM {

void InventoryMan::initConstants() {
	static const char* G0428_apc_SkillLevelNames_EN_ANY[15] = {"NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN", "CRAFTSMAN",
		"ARTISAN", "ADEPT", "EXPERT", "` MASTER", "a MASTER","b MASTER", "c MASTER", "d MASTER", "e MASTER", "ARCHMASTER"};
	static const char* G0428_apc_SkillLevelNames_DE_DEU[15] = {"ANFAENGER", "NEULING", "LEHRLING", "ARBEITER", "GESELLE", "HANDWERKR", "FACHMANN",
		"EXPERTE", "` MEISTER", "a MEISTER", "b MEISTER", "c MEISTER", "d MEISTER", "e MEISTER", "ERZMEISTR"};
	static const char* G0428_apc_SkillLevelNames_FR_FRA[15] = {"NEOPHYTE", "NOVICE", "APPRENTI", "COMPAGNON", "ARTISAN", "PATRON",
		"ADEPTE", "EXPERT", "MAITRE '", "MAITRE a", "MAITRE b", "MAITRE c", "MAITRE d", "MAITRE e", "SUR-MAITRE"};
	const char **g428_byLanguage;
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		g428_byLanguage = G0428_apc_SkillLevelNames_EN_ANY;
		break;
	case Common::DE_DEU:
		g428_byLanguage = G0428_apc_SkillLevelNames_DE_DEU;
		break;
	case Common::FR_FRA:
		g428_byLanguage = G0428_apc_SkillLevelNames_FR_FRA;
		break;
	}
	for (int i = 0; i < 15; ++i)
		_skillLevelNames[i] = g428_byLanguage[i];

	_boxPanel = Box(80, 223, 52, 124); // @ G0032_s_Graphic562_Box_Panel
}

InventoryMan::InventoryMan(DMEngine *vm) : _vm(vm) {
	_inventoryChampionOrdinal = 0;
	_panelContent = k0_PanelContentFoodWaterPoisoned;
	for (uint16 i = 0; i < 8; ++i)
		_chestSlots[i] = Thing(0);
	_openChest = Thing::_none;
	_objDescTextXpos = 0;
	_objDescTextYpos = 0;

	for (int i = 0; i < 15; i++)
		_skillLevelNames[i] = nullptr;

	initConstants();
}

void InventoryMan::toggleInventory(ChampionIndex championIndex) {
	static Box g41_BoxFloppyZzzCross(174, 218, 2, 12); // @ G0041_s_Graphic562_Box_ViewportFloppyZzzCross


	uint16 L1102_ui_Multiple;
#define AL1102_ui_InventoryChampionOrdinal L1102_ui_Multiple
#define AL1102_ui_SlotIndex                L1102_ui_Multiple
	Champion* L1103_ps_Champion;


	if ((championIndex != k4_ChampionCloseInventory) && !_vm->_championMan->_champions[championIndex]._currHealth) {
		return;
	}
	if (_vm->_pressingMouth || _vm->_pressingEye) {
		return;
	}
	_vm->_stopWaitingForPlayerInput = true;
	AL1102_ui_InventoryChampionOrdinal = _inventoryChampionOrdinal;
	if (_vm->indexToOrdinal(championIndex) == AL1102_ui_InventoryChampionOrdinal) {
		championIndex = k4_ChampionCloseInventory;
	}
	_vm->_eventMan->showMouse();
	if (AL1102_ui_InventoryChampionOrdinal) {
		_inventoryChampionOrdinal = _vm->indexToOrdinal(kM1_ChampionNone);
		closeChest();
		L1103_ps_Champion = &_vm->_championMan->_champions[_vm->ordinalToIndex(AL1102_ui_InventoryChampionOrdinal)];
		if (L1103_ps_Champion->_currHealth && !_vm->_championMan->_candidateChampionOrdinal) {
			setFlag(L1103_ps_Champion->_attributes, k0x1000_ChampionAttributeStatusBox);
			_vm->_championMan->drawChampionState((ChampionIndex)_vm->ordinalToIndex(AL1102_ui_InventoryChampionOrdinal));
		}
		if (_vm->_championMan->_partyIsSleeping) {
			_vm->_eventMan->hideMouse();
			return;
		}
		if (championIndex == k4_ChampionCloseInventory) {
			_vm->_eventMan->_refreshMousePointerInMainLoop = true;
			_vm->_menuMan->drawMovementArrows();
			_vm->_eventMan->hideMouse();
			_vm->_eventMan->_secondaryMouseInput = _vm->_eventMan->_secondaryMouseInputMovement;
			_vm->_eventMan->_secondaryKeyboardInput = _vm->_eventMan->_secondaryKeyboardInputMovement;
			_vm->_eventMan->discardAllInput();
			_vm->_displayMan->drawFloorAndCeiling();
			return;
		}
	}
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_inventoryChampionOrdinal = _vm->indexToOrdinal(championIndex);
	if (!AL1102_ui_InventoryChampionOrdinal) {
		_vm->_displayMan->shadeScreenBox(&_vm->_displayMan->_boxMovementArrows, k0_ColorBlack);
	}
	L1103_ps_Champion = &_vm->_championMan->_champions[championIndex];
	_vm->_displayMan->loadIntoBitmap(k17_InventoryGraphicIndice, _vm->_displayMan->_bitmapViewport);
	if (_vm->_championMan->_candidateChampionOrdinal) {
		_vm->_displayMan->fillBoxBitmap(_vm->_displayMan->_bitmapViewport, g41_BoxFloppyZzzCross, k12_ColorDarkestGray, k112_byteWidthViewport, k136_heightViewport);
	}

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		_vm->_textMan->printToViewport(5, 116, k13_ColorLightestGray, "HEALTH");
		_vm->_textMan->printToViewport(5, 124, k13_ColorLightestGray, "STAMINA");
		break;
	case Common::DE_DEU:
		_vm->_textMan->printToViewport(5, 116, k13_ColorLightestGray, "GESUND");
		_vm->_textMan->printToViewport(5, 124, k13_ColorLightestGray, "KRAFT");
		break;
	case Common::FR_FRA:
		_vm->_textMan->printToViewport(5, 116, k13_ColorLightestGray, "SANTE");
		_vm->_textMan->printToViewport(5, 124, k13_ColorLightestGray, "VIGUEUR");
		break;
	}

	_vm->_textMan->printToViewport(5, 132, k13_ColorLightestGray, "MANA");
	for (AL1102_ui_SlotIndex = k0_ChampionSlotReadyHand; AL1102_ui_SlotIndex < k30_ChampionSlotChest_1; AL1102_ui_SlotIndex++) {
		_vm->_championMan->drawSlot(championIndex, AL1102_ui_SlotIndex);
	}
	setFlag(L1103_ps_Champion->_attributes, k0x4000_ChampionAttributeViewport | k0x1000_ChampionAttributeStatusBox | k0x0800_ChampionAttributePanel | k0x0200_ChampionAttributeLoad | k0x0100_ChampionAttributeStatistics | k0x0080_ChampionAttributeNameTitle);
	_vm->_championMan->drawChampionState(championIndex);
	_vm->_eventMan->_mousePointerBitmapUpdated = true;
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->_secondaryMouseInput = _vm->_eventMan->_secondaryMouseInputChampionInventory;
	_vm->_eventMan->_secondaryKeyboardInput = nullptr;
	_vm->_eventMan->discardAllInput();
}

void InventoryMan::drawStatusBoxPortrait(ChampionIndex championIndex) {
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan._useByteBoxCoordinates = false;
	Box box;
	box._y1 = 0;
	box._y2 = 28;
	box._x1 = championIndex * k69_ChampionStatusBoxSpacing + 7;
	box._x2 = box._x1 + 31;
	dispMan.blitToScreen(_vm->_championMan->_champions[championIndex]._portrait, &box, k16_byteWidth, kM1_ColorNoTransparency, 29);
}

void InventoryMan::drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color) {
	Box box;
	box._x1 = x;
	box._x2 = box._x1 + pixelWidth;
	box._y1 = y;
	box._y2 = box._y1 + 6;
	_vm->_displayMan->_useByteBoxCoordinates = false;
	_vm->_displayMan->fillBoxBitmap(_vm->_displayMan->_bitmapViewport, box, color, k112_byteWidthViewport, k136_heightViewport);
}

void InventoryMan::drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color) {
	if (amount < -512) {
		color = k8_ColorRed;
	} else if (amount < 0) {
		color = k11_ColorYellow;
	}

	int16 pixelWidth = amount + 1024;
	if (pixelWidth == 3072) {
		pixelWidth = 3071;
	}
	pixelWidth /= 32;
	drawPanelHorizontalBar(115, y + 2, pixelWidth, k0_ColorBlack);
	drawPanelHorizontalBar(113, y, pixelWidth, color);
}

void InventoryMan::drawPanelFoodWaterPoisoned() {
	static Box g35_BoxFood(112, 159, 60, 68); // @ G0035_s_Graphic562_Box_Food
	static Box g36_BoxWater(112, 159, 83, 91); // @ G0036_s_Graphic562_Box_Water
	static Box g37_BoxPoisoned(112, 207, 105, 119); // @ G0037_s_Graphic562_Box_Poisoned

	Champion &champ = _vm->_championMan->_champions[_inventoryChampionOrdinal];
	closeChest();
	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k20_PanelEmptyIndice), _boxPanel, k72_byteWidth, k8_ColorRed, 73);

	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k30_FoodLabelIndice), g35_BoxFood, k24_byteWidth, k12_ColorDarkestGray, 9);
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k31_WaterLabelIndice), g36_BoxWater, k24_byteWidth, k12_ColorDarkestGray, 9);
		break;
	case Common::DE_DEU:
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k30_FoodLabelIndice), g35_BoxFood, k32_byteWidth, k12_ColorDarkestGray, 9);
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k31_WaterLabelIndice), g36_BoxWater, k32_byteWidth, k12_ColorDarkestGray, 9);
		break;
	case Common::FR_FRA:
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k30_FoodLabelIndice), g35_BoxFood, k48_byteWidth, k12_ColorDarkestGray, 9);
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k31_WaterLabelIndice), g36_BoxWater, k24_byteWidth, k12_ColorDarkestGray, 9);
		break;
	}

	if (champ._poisonEventCount) {
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k32_PoisionedLabelIndice),
								   g37_BoxPoisoned, k48_byteWidth, k12_ColorDarkestGray, 15);
	}
	drawPanelFoodOrWaterBar(champ._food, 69, k5_ColorLightBrown);
	drawPanelFoodOrWaterBar(champ._water, 92, k14_ColorBlue);
}

void InventoryMan::drawPanelResurrectReincarnate() {
	_panelContent = k5_PanelContentResurrectReincarnate;
	_vm->_displayMan->blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k40_PanelResurectReincaranteIndice),
										 _boxPanel, k72_byteWidth, k6_ColorDarkGreen, 73);
}

void InventoryMan::drawPanel() {
	closeChest();

	ChampionMan &cm = *_vm->_championMan;
	if (cm._candidateChampionOrdinal) {
		drawPanelResurrectReincarnate();
		return;
	}

	Thing thing = cm._champions[_vm->ordinalToIndex(_inventoryChampionOrdinal)].getSlot(k1_ChampionSlotActionHand);

	_panelContent = k0_PanelContentFoodWaterPoisoned;
	switch (thing.getType()) {
	case k9_ContainerThingType:
		_panelContent = k4_PanelContentChest;
		break;
	case k7_ScrollThingType:
		_panelContent = k2_PanelContentScroll;
		break;
	default:
		thing = Thing::_none;
		break;
	}
	if (thing == Thing::_none) {
		drawPanelFoodWaterPoisoned();
	} else {
		drawPanelObject(thing, false);
	}
}

void InventoryMan::closeChest() {
	DungeonMan &dunMan = *_vm->_dungeonMan;

	bool processFirstChestSlot = true;
	if (_openChest == Thing::_none)
		return;
	Container *container = (Container*)dunMan.getThingData(_openChest);
	_openChest = Thing::_none;
	container->getSlot() = Thing::_endOfList;
	Thing prevThing;
	for (int16 chestSlotIndex = 0; chestSlotIndex < 8; ++chestSlotIndex) {
		Thing thing = _chestSlots[chestSlotIndex];
		if (thing != Thing::_none) {
			_chestSlots[chestSlotIndex] = Thing::_none; // CHANGE8_09_FIX

			if (processFirstChestSlot) {
				processFirstChestSlot = false;
				*dunMan.getThingData(thing) = Thing::_endOfList.toUint16();
				container->getSlot() = prevThing = thing;
			} else {
				dunMan.linkThingToList(thing, prevThing, kM1_MapXNotOnASquare, 0);
				prevThing = thing;
			}
		}
	}
}

void InventoryMan::drawPanelScrollTextLine(int16 yPos, char* text) {
	for (char* iter = text; *iter != '\0'; ++iter) {
		if ((*iter >= 'A') && (*iter <= 'Z')) {
			*iter -= 64;
		} else if (*iter >= '{') { // this branch is CHANGE5_03_IMPROVEMENT
			*iter -= 96;
		}
	}
	_vm->_textMan->printToViewport(162 - (6 * strlen(text) / 2), yPos, k0_ColorBlack, text, k15_ColorWhite);
}

void InventoryMan::drawPanelScroll(Scroll* scroll) {
	DisplayMan &dispMan = *_vm->_displayMan;

	char stringFirstLine[300];
	_vm->_dungeonMan->decodeText(stringFirstLine, Thing(scroll->getTextStringThingIndex()), (TextType)(k2_TextTypeScroll | k0x8000_DecodeEvenIfInvisible));
	char *charRed = stringFirstLine;
	while (*charRed && (*charRed != '\n')) {
		charRed++;
	}
	*charRed = '\0';
	dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k23_PanelOpenScrollIndice),
							   _boxPanel, k72_byteWidth, k8_ColorRed, 73);
	int16 lineCount = 1;
	charRed++;
	char *charGreen = charRed; // first char of the second line
	while (*charGreen) {
		/* BUG0_47 Graphical glitch when you open a scroll. If there is a single line of text in a scroll
		(with no carriage return) then charGreen points to undefined data. This may result in a graphical
		glitch and also corrupt other memory. This is not an issue in the original dungeons where all
		scrolls contain at least one carriage return character */
		if (*charGreen == '\n') {
			lineCount++;
		}
		charGreen++;
	}
	if (*(charGreen - 1) != '\n') {
		lineCount++;
	} else if (*(charGreen - 2) == '\n') {
		lineCount--;
	}
	int16 yPos = 92 - (7 * lineCount) / 2; // center the text vertically
	drawPanelScrollTextLine(yPos, stringFirstLine);
	charGreen = charRed;
	while (*charGreen) {
		yPos += 7;
		while (*charRed && (*charRed != '\n')) {
			charRed++;
		}
		if (!(*charRed)) {
			charRed[1] = '\0';
		}
		*charRed++ = '\0';
		drawPanelScrollTextLine(yPos, charGreen);
		charGreen = charRed;
	}
}

void InventoryMan::openAndDrawChest(Thing thingToOpen, Container* chest, bool isPressingEye) {
	DisplayMan &dispMan = *_vm->_displayMan;
	ObjectMan &objMan = *_vm->_objectMan;

	if (_openChest == thingToOpen)
		return;

	if (_openChest != Thing::_none)
		closeChest(); // CHANGE8_09_FIX

	_openChest = thingToOpen;
	if (!isPressingEye) {
		objMan.drawIconInSlotBox(k9_SlotBoxInventoryActionHand, k145_IconIndiceContainerChestOpen);
	}
	dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k25_PanelOpenChestIndice),
							   _boxPanel, k72_byteWidth, k8_ColorRed, 73);
	int16 chestSlotIndex = 0;
	Thing thing = chest->getSlot();
	int16 thingCount = 0;
	while (thing != Thing::_endOfList) {
		if (++thingCount > 8)
			break; // CHANGE8_08_FIX, make sure that no more than the first 8 objects in a chest are drawn

		objMan.drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, objMan.getIconIndex(thing));
		_chestSlots[chestSlotIndex++] = thing;
		thing = _vm->_dungeonMan->getNextThing(thing);
	}
	while (chestSlotIndex < 8) {
		objMan.drawIconInSlotBox(chestSlotIndex + k38_SlotBoxChestFirstSlot, kM1_IconIndiceNone);
		_chestSlots[chestSlotIndex++] = Thing::_none;
	}
}

void InventoryMan::drawIconToViewport(IconIndice iconIndex, int16 xPos, int16 yPos) {
	static byte iconBitmap[16 * 16];
	Box box;
	box._x2 = (box._x1 = xPos) + 15;
	box._y2 = (box._y1 = yPos) + 15;
	_vm->_objectMan->extractIconFromBitmap(iconIndex, iconBitmap);
	_vm->_displayMan->blitToViewport(iconBitmap, box, k8_byteWidth, kM1_ColorNoTransparency, 16);
}

void InventoryMan::buildObjectAttributeString(int16 potentialAttribMask, int16 actualAttribMask, const char** attribStrings, char* destString, const char* prefixString, const char* suffixString) {
	uint16 identicalBitCount = 0;
	int16 attribMask = 1;
	for (uint16 stringIndex = 0; stringIndex < 16; stringIndex++, attribMask <<= 1) {
		if (attribMask & potentialAttribMask & actualAttribMask) {
			identicalBitCount++;
		}
	}

	if (identicalBitCount == 0) {
		*destString = '\0';
		return;
	}

	strcpy(destString, prefixString);

	attribMask = 1;
	for (uint16 stringIndex = 0; stringIndex < 16; stringIndex++, attribMask <<= 1) {
		if (attribMask & potentialAttribMask & actualAttribMask) {
			strcat(destString, attribStrings[stringIndex]);
			if (identicalBitCount-- > 2) {
				strcat(destString, ", ");
			} else if (identicalBitCount == 1) {

				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY: strcat(destString, " AND "); break;
				case Common::DE_DEU: strcat(destString, " UND "); break;
				case Common::FR_FRA: strcat(destString, " ET "); break;
				}

			}
		}
	}

	strcat(destString, suffixString);
}

void InventoryMan::drawPanelObjectDescriptionString(const char *descString) {
	if (descString[0] == '\f') { // form feed
		descString++;
		_objDescTextXpos = 108;
		_objDescTextYpos = 59;
	}

	if (descString[0]) {
		char stringTmpBuff[128];
		strcpy(stringTmpBuff, descString);

		char *stringLine = stringTmpBuff;
		bool severalLines = false;
		char *string = nullptr;
		while (*stringLine) {
			if (strlen(stringLine) > 18) { // if string is too long to fit on one line
				string = &stringLine[17];
				while (*string != ' ') // go back to the last space character
					string--;

				*string = '\0'; // and split the string there
				severalLines = true;
			}

			_vm->_textMan->printToViewport(_objDescTextXpos, _objDescTextYpos, k13_ColorLightestGray, stringLine);
			_objDescTextYpos += 7;
			if (severalLines) {
				severalLines = false;
				stringLine = ++string;
			} else {
				*stringLine = '\0';
			}
		}
	}
}

void InventoryMan::drawPanelArrowOrEye(bool pressingEye) {
	static Box boxArrowOrEye(83, 98, 57, 65); // @ G0033_s_Graphic562_Box_ArrowOrEye 

	DisplayMan &dispMan = *_vm->_displayMan;
	dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(pressingEye ? k19_EyeForObjectDescriptionIndice : k18_ArrowForChestContentIndice),
							   boxArrowOrEye, k8_byteWidth, k8_ColorRed, 9);
}

#define k0x0001_DescriptionMaskConsumable 0x0001 // @ MASK0x0001_DESCRIPTION_CONSUMABLE
#define k0x0002_DescriptionMaskPoisoned 0x0002 // @ MASK0x0002_DESCRIPTION_POISONED  
#define k0x0004_DescriptionMaskBroken 0x0004 // @ MASK0x0004_DESCRIPTION_BROKEN    
#define k0x0008_DescriptionMaskCursed 0x0008 // @ MASK0x0008_DESCRIPTION_CURSED    

void InventoryMan::drawPanelObject(Thing thingToDraw, bool pressingEye) {
	static Box boxObjectDescCircle(105, 136, 53, 79); // @ G0034_s_Graphic562_Box_ObjectDescriptionCircle 

	DungeonMan &dunMan = *_vm->_dungeonMan;
	ObjectMan &objMan = *_vm->_objectMan;
	DisplayMan &dispMan = *_vm->_displayMan;
	ChampionMan &champMan = *_vm->_championMan;
	TextMan &textMan = *_vm->_textMan;

	if (_vm->_pressingEye || _vm->_pressingMouth) {
		closeChest();
	}

	uint16 *rawThingPtr = dunMan.getThingData(thingToDraw);
	drawPanelObjectDescriptionString("\f"); // form feed
	ThingType thingType = thingToDraw.getType();
	if (thingType == k7_ScrollThingType) {
		drawPanelScroll((Scroll*)rawThingPtr);
	} else if (thingType == k9_ContainerThingType) {
		openAndDrawChest(thingToDraw, (Container *)rawThingPtr, pressingEye);
	} else {
		IconIndice iconIndex = objMan.getIconIndex(thingToDraw);
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k20_PanelEmptyIndice),
								   _boxPanel, k72_byteWidth, k8_ColorRed, 73);
		dispMan.blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k29_ObjectDescCircleIndice),
								   boxObjectDescCircle, k16_byteWidth, k12_ColorDarkestGray, 27);

		const char *descString = nullptr;
		char str[40];
		if (iconIndex == k147_IconIndiceJunkChampionBones) {
			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
			case Common::DE_DEU: // german and english versions are the same
				strcpy(str, champMan._champions[((Junk *)rawThingPtr)->getChargeCount()]._name);
				strcat(str, " ");
				strcat(str, objMan._objectNames[iconIndex]);
				break;
			case Common::FR_FRA:
				strcat(str, objMan._objectNames[iconIndex]);
				strcat(str, " ");
				strcpy(str, champMan._champions[((Junk *)rawThingPtr)->getChargeCount()]._name);
				break;
			}

			descString = str;
		} else if ((thingType == k8_PotionThingType)
				   && (iconIndex != k163_IconIndicePotionWaterFlask)
				   && (champMan.getSkillLevel((ChampionIndex)_vm->ordinalToIndex(_inventoryChampionOrdinal), k2_ChampionSkillPriest) > 1)) {
			str[0] = '_' + ((Potion *)rawThingPtr)->getPower() / 40;
			str[1] = ' ';
			str[2] = '\0';
			strcat(str, objMan._objectNames[iconIndex]);
			descString = str;
		} else {
			descString = objMan._objectNames[iconIndex];
		}

		textMan.printToViewport(134, 68, k13_ColorLightestGray, descString);
		drawIconToViewport(iconIndex, 111, 59);


		_objDescTextYpos = 87;

		uint16 potentialAttribMask = 0;
		uint16 actualAttribMask = 0;
		switch (thingType) {
		case k5_WeaponThingType: {
			potentialAttribMask = k0x0008_DescriptionMaskCursed | k0x0002_DescriptionMaskPoisoned | k0x0004_DescriptionMaskBroken;
			Weapon *weapon = (Weapon *)rawThingPtr;
			actualAttribMask = (weapon->getCursed() << 3) | (weapon->getPoisoned() << 1) | (weapon->getBroken() << 2);
			if ((iconIndex >= k4_IconIndiceWeaponTorchUnlit)
				&& (iconIndex <= k7_IconIndiceWeaponTorchLit)
				&& (weapon->getChargeCount() == 0)) {

				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY: drawPanelObjectDescriptionString("(BURNT OUT)"); break;
				case Common::DE_DEU: drawPanelObjectDescriptionString("(AUSGEBRANNT)"); break;
				case Common::FR_FRA: drawPanelObjectDescriptionString("(CONSUME)"); break;
				}
			}
			break;
		}
		case k6_ArmourThingType: {
			potentialAttribMask = k0x0008_DescriptionMaskCursed | k0x0004_DescriptionMaskBroken;
			Armour *armour = (Armour *)rawThingPtr;
			actualAttribMask = (armour->getCursed() << 3) | (armour->getBroken() << 2);
			break;
		}
		case k8_PotionThingType: {
			potentialAttribMask = k0x0001_DescriptionMaskConsumable;
			Potion *potion = (Potion *)rawThingPtr;
			actualAttribMask = _vm->_dungeonMan->_objectInfo[k2_ObjectInfoIndexFirstPotion + potion->getType()].getAllowedSlots();
			break;
		}
		case k10_JunkThingType: {
			if ((iconIndex >= k8_IconIndiceJunkWater) && (iconIndex <= k9_IconIndiceJunkWaterSkin)) {
				potentialAttribMask = 0;
				const char *descString_EN_ANY[4] = {"(EMPTY)", "(ALMOST EMPTY)", "(ALMOST FULL)", "(FULL)"};
				const char *descString_DE_DEU[4] = {"(LEER)", "(FAST LEER)", "(FAST VOLL)", "(VOLL)"};
				const char *descString_FR_FRA[4] = {"(VIDE)", "(PRESQUE VIDE)", "(PRESQUE PLEINE)", "(PLEINE)"};

				Junk *junk = (Junk *)rawThingPtr;
				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY:
					descString = descString_EN_ANY[junk->getChargeCount()];
					break;
				case Common::DE_DEU:
					descString = descString_DE_DEU[junk->getChargeCount()];
					break;
				case Common::FR_FRA:
					descString = descString_FR_FRA[junk->getChargeCount()];
					break;
				}

				drawPanelObjectDescriptionString(descString);
			} else if ((iconIndex >= k0_IconIndiceJunkCompassNorth) && (iconIndex <= k3_IconIndiceJunkCompassWest)) {
				potentialAttribMask = 0;

				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY:
					strcpy(str, "PARTY FACING ");
					break;
				case Common::DE_DEU:
					strcpy(str, "GRUPPE BLICKT NACH ");
					break;
				case Common::FR_FRA:
					strcpy(str, "GROUPE FACE ");
					break;
				}


				const static char* directionName_EN_ANY[4] = {"NORTH", "EAST", "SOUTH", "WEST"};
				const static char* directionName_DE_DEU[4] = {"NORDEN", "OSTEN", "SUEDEN", "WESTEN"};
				const static char* directionName_FR_FRA[4] = {"AU NORD", "A L'EST", "AU SUD", "A L'OUEST"};
				switch (_vm->getGameLanguage()) { // localized
				default:
				case Common::EN_ANY: strcat(str, directionName_EN_ANY[iconIndex]); break;
				case Common::DE_DEU: strcat(str, directionName_DE_DEU[iconIndex]); break;
				case Common::FR_FRA: strcat(str, directionName_FR_FRA[iconIndex]); break;
				}

				drawPanelObjectDescriptionString(str);
			} else {
				Junk *junk = (Junk *)rawThingPtr;
				potentialAttribMask = k0x0001_DescriptionMaskConsumable;
				actualAttribMask = _vm->_dungeonMan->_objectInfo[k127_ObjectInfoIndexFirstJunk + junk->getType()].getAllowedSlots();
			}
			break;
		}
		default:
			break;
		} // end of switch 

		if (potentialAttribMask) {
			static const char *attribString_EN_ANY[4] = {"CONSUMABLE", "POISONED", "BROKEN", "CURSED"};
			static const char *attribString_DE_DEU[4] = {"ESSBAR", "VERGIFTET", "DEFEKT", "VERFLUCHT"};
			static const char *attribString_FR_FRA[4] = {"COMESTIBLE", "EMPOISONNE", "BRISE", "MAUDIT"};
			static const char **attribString = nullptr;

			switch (_vm->getGameLanguage()) { // localized
			default:
			case Common::EN_ANY: attribString = attribString_EN_ANY; break;
			case Common::DE_DEU: attribString = attribString_DE_DEU; break;
			case Common::FR_FRA: attribString = attribString_FR_FRA; break;
			}

			buildObjectAttributeString(potentialAttribMask, actualAttribMask, attribString, str, "(", ")");
			drawPanelObjectDescriptionString(str);
		}

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: strcpy(str, "WEIGHS "); break;
		case Common::DE_DEU: strcpy(str, "WIEGT "); break;
		case Common::FR_FRA: strcpy(str, "PESE "); break;
		}


		uint16 weight = dunMan.getObjectWeight(thingToDraw);
		strcat(str, champMan.getStringFromInteger(weight / 10, false, 3).c_str());

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY: strcat(str, "."); break;
		case Common::DE_DEU: strcat(str, ","); break;
		case Common::FR_FRA: strcat(str, "KG,"); break;
		}

		weight -= (weight / 10) * 10;
		strcat(str, champMan.getStringFromInteger(weight, false, 1).c_str());

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
		case Common::DE_DEU: strcat(str, " KG."); break;
		case Common::FR_FRA: strcat(str, "."); break;
		}

		drawPanelObjectDescriptionString(str);
	}
	drawPanelArrowOrEye(pressingEye);
}

void InventoryMan::setDungeonViewPalette() {
	static const int16 g40_palIndexToLightAmmount[6] = {99, 75, 50, 25, 1, 0}; // @ G0040_ai_Graphic562_PaletteIndexToLightAmount

	if (_vm->_dungeonMan->_currMap->_difficulty == 0) {
		_vm->_displayMan->_dungeonViewPaletteIndex = 0; /* Brightest color palette index */
	} else {
		/* Get torch light power from both hands of each champion in the party */
		int16 L1038_i_Counter = 4; /* BUG0_01 Coding error without consequence. The hands of four champions are inspected even if there are less champions in the party. No consequence as the data in unused champions is set to 0 and _vm->_objectMan->f32_getObjectType then returns -1 */
		Champion *L1043_ps_Champion = _vm->_championMan->_champions;
		int16 L1045_ai_TorchesLightPower[8];
		int16 *AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		while (L1038_i_Counter--) {
			uint16 AL1039_ui_SlotIndex = k1_ChampionSlotActionHand + 1;
			while (AL1039_ui_SlotIndex--) {
				uint16 AL1044_T_Thing = L1043_ps_Champion->_slots[AL1039_ui_SlotIndex].toUint16();
				if ((_vm->_objectMan->getObjectType(Thing(AL1044_T_Thing)) >= k4_IconIndiceWeaponTorchUnlit) &&
					(_vm->_objectMan->getObjectType(Thing(AL1044_T_Thing)) <= k7_IconIndiceWeaponTorchLit)) {
					Weapon *L1042_ps_Weapon = (Weapon*)_vm->_dungeonMan->getThingData(Thing(AL1044_T_Thing));
					*AL1040_pi_TorchLightPower = L1042_ps_Weapon->getChargeCount();
				} else {
					*AL1040_pi_TorchLightPower = 0;
				}
				AL1040_pi_TorchLightPower++;
			}
			L1043_ps_Champion++;
		}
		/* Sort torch light power values so that the four highest values are in the first four entries in the array L1045_ai_TorchesLightPower in decreasing order. The last four entries contain the smallest values but they are not sorted */
		AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		int16 AL1039_ui_Counter = 0;
		while (AL1039_ui_Counter != 4) {
			L1038_i_Counter = 7 - AL1039_ui_Counter;
			int16 *L1041_pi_TorchLightPower = &L1045_ai_TorchesLightPower[AL1039_ui_Counter + 1];
			while (L1038_i_Counter--) {
				if (*L1041_pi_TorchLightPower > *AL1040_pi_TorchLightPower) {
					int16 AL1044_ui_TorchLightPower = *L1041_pi_TorchLightPower;
					*L1041_pi_TorchLightPower = *AL1040_pi_TorchLightPower;
					*AL1040_pi_TorchLightPower = AL1044_ui_TorchLightPower;
				}
				L1041_pi_TorchLightPower++;
			}
			AL1040_pi_TorchLightPower++;
			AL1039_ui_Counter++;
		}
		/* Get total light amount provided by the four torches with the highest light power values and by the fifth torch in the array which may be any one of the four torches with the smallest ligh power values */
		uint16 L1037_ui_TorchLightAmountMultiplier = 6;
		AL1039_ui_Counter = 5;
		int16 L1036_i_TotalLightAmount = 0;
		AL1040_pi_TorchLightPower = L1045_ai_TorchesLightPower;
		while (AL1039_ui_Counter--) {
			if (*AL1040_pi_TorchLightPower) {
				L1036_i_TotalLightAmount += (_vm->_championMan->_lightPowerToLightAmount[*AL1040_pi_TorchLightPower] << L1037_ui_TorchLightAmountMultiplier) >> 6;
				L1037_ui_TorchLightAmountMultiplier = MAX(0, L1037_ui_TorchLightAmountMultiplier - 1);
			}
			AL1040_pi_TorchLightPower++;
		}
		L1036_i_TotalLightAmount += _vm->_championMan->_party._magicalLightAmount;
		/* Select palette corresponding to the total light amount */
		const int16 *AL1040_pi_LightAmount = g40_palIndexToLightAmmount;
		int16 AL1039_ui_PaletteIndex;
		if (L1036_i_TotalLightAmount > 0) {
			AL1039_ui_PaletteIndex = 0; /* Brightest color palette index */
			while (*AL1040_pi_LightAmount++ > L1036_i_TotalLightAmount) {
				AL1039_ui_PaletteIndex++;
			}
		} else {
			AL1039_ui_PaletteIndex = 5; /* Darkest color palette index */
		}
		_vm->_displayMan->_dungeonViewPaletteIndex = AL1039_ui_PaletteIndex;
	}

	_vm->_displayMan->_refreshDungeonViewPaleteRequested = true;
}

void InventoryMan::decreaseTorchesLightPower() {
	bool L1048_B_TorchChargeCountChanged = false;
	int16 L1046_i_ChampionCount = _vm->_championMan->_partyChampionCount;
	if (_vm->_championMan->_candidateChampionOrdinal) {
		L1046_i_ChampionCount--;
	}
	Champion *L1050_ps_Champion = _vm->_championMan->_champions;
	while (L1046_i_ChampionCount--) {
		int16 L1047_i_SlotIndex = k1_ChampionSlotActionHand + 1;
		while (L1047_i_SlotIndex--) {
			int16 L1049_i_IconIndex = _vm->_objectMan->getIconIndex(L1050_ps_Champion->_slots[L1047_i_SlotIndex]);
			if ((L1049_i_IconIndex >= k4_IconIndiceWeaponTorchUnlit) && (L1049_i_IconIndex <= k7_IconIndiceWeaponTorchLit)) {
				Weapon *L1051_ps_Weapon = (Weapon *)_vm->_dungeonMan->getThingData(L1050_ps_Champion->_slots[L1047_i_SlotIndex]);
				if (L1051_ps_Weapon->getChargeCount()) {
					if (L1051_ps_Weapon->setChargeCount(L1051_ps_Weapon->getChargeCount() - 1) == 0) {
						L1051_ps_Weapon->setDoNotDiscard(false);
					}
					L1048_B_TorchChargeCountChanged = true;
				}
			}
		}
		L1050_ps_Champion++;
	}
	if (L1048_B_TorchChargeCountChanged) {
		setDungeonViewPalette();
		_vm->_championMan->drawChangedObjectIcons();
	}
}

void InventoryMan::drawChampionSkillsAndStatistics() {

	uint16 L1090_ui_Multiple;
#define AL1090_ui_SkillIndex     L1090_ui_Multiple
#define AL1090_ui_StatisticIndex L1090_ui_Multiple
	int16 L1091_i_Y;
	int16 L1092_i_Multiple;
#define AL1092_i_SkillLevel            L1092_i_Multiple
#define AL1092_i_StatisticCurrentValue L1092_i_Multiple
	uint16 L1093_ui_ChampionIndex;
	Champion* L1094_ps_Champion;
	int16 L1095_i_StatisticColor;
	uint16 L1096_ui_StatisticMaximumValue;
	char L1097_ac_String[20];

	static const char* G0431_apc_StatisticNames_EN_ANY[7] = {"L", "STRENGTH", "DEXTERITY", "WISDOM", "VITALITY", "ANTI-MAGIC", "ANTI-FIRE"};
	static const char* G0431_apc_StatisticNames_DE_DEU[7] = {"L", "STAERKE", "FLINKHEIT", "WEISHEIT", "VITALITAET", "ANTI-MAGIE", "ANTI-FEUER"};
	static const char* G0431_apc_StatisticNames_FR_FRA[7] = {"L", "FORCE", "DEXTERITE", "SAGESSE", "VITALITE", "ANTI-MAGIE", "ANTI-FEU"};
	const char **G0431_apc_StatisticNames;
	switch (_vm->getGameLanguage()) { // localized
	default:
	case Common::EN_ANY: G0431_apc_StatisticNames = G0431_apc_StatisticNames_EN_ANY; break;
	case Common::DE_DEU: G0431_apc_StatisticNames = G0431_apc_StatisticNames_DE_DEU; break;
	case Common::FR_FRA: G0431_apc_StatisticNames = G0431_apc_StatisticNames_FR_FRA; break;
	}

	closeChest();
	L1094_ps_Champion = &_vm->_championMan->_champions[L1093_ui_ChampionIndex = _vm->ordinalToIndex(_inventoryChampionOrdinal)];
	_vm->_displayMan->blitToViewport(_vm->_displayMan->getNativeBitmapOrGraphic(k20_PanelEmptyIndice), _boxPanel, k72_byteWidth, k8_ColorRed, 73);
	L1091_i_Y = 58;
	for (AL1090_ui_SkillIndex = k0_ChampionSkillFighter; AL1090_ui_SkillIndex <= k3_ChampionSkillWizard; AL1090_ui_SkillIndex++) {
		AL1092_i_SkillLevel = MIN((uint16)16, _vm->_championMan->getSkillLevel(L1093_ui_ChampionIndex, AL1090_ui_SkillIndex | k0x8000_IgnoreTemporaryExperience));
		if (AL1092_i_SkillLevel == 1)
			continue;

		switch (_vm->getGameLanguage()) { // localized
		default:
		case Common::EN_ANY:
		case Common::DE_DEU: // english and german versions are the same
			strcpy(L1097_ac_String, _skillLevelNames[AL1092_i_SkillLevel - 2]);
			strcat(L1097_ac_String, " ");
			strcat(L1097_ac_String, _vm->_championMan->_baseSkillName[AL1090_ui_SkillIndex]);
			break;
		case Common::FR_FRA:
			strcat(L1097_ac_String, _vm->_championMan->_baseSkillName[AL1090_ui_SkillIndex]);
			strcat(L1097_ac_String, " ");
			strcpy(L1097_ac_String, _skillLevelNames[AL1092_i_SkillLevel - 2]);
			break;
		}
		_vm->_textMan->printToViewport(108, L1091_i_Y, k13_ColorLightestGray, L1097_ac_String);
		L1091_i_Y += 7;
	}
	L1091_i_Y = 86;
	for (AL1090_ui_StatisticIndex = k1_ChampionStatStrength; AL1090_ui_StatisticIndex <= k6_ChampionStatAntifire; AL1090_ui_StatisticIndex++) {
		_vm->_textMan->printToViewport(108, L1091_i_Y, k13_ColorLightestGray, G0431_apc_StatisticNames[AL1090_ui_StatisticIndex]);
		AL1092_i_StatisticCurrentValue = L1094_ps_Champion->_statistics[AL1090_ui_StatisticIndex][k1_ChampionStatCurrent];
		L1096_ui_StatisticMaximumValue = L1094_ps_Champion->_statistics[AL1090_ui_StatisticIndex][k0_ChampionStatMaximum];
		if (AL1092_i_StatisticCurrentValue < L1096_ui_StatisticMaximumValue) {
			L1095_i_StatisticColor = k8_ColorRed;
		} else {
			if (AL1092_i_StatisticCurrentValue > L1096_ui_StatisticMaximumValue) {
				L1095_i_StatisticColor = k7_ColorLightGreen;
			} else {
				L1095_i_StatisticColor = k13_ColorLightestGray;
			}
		}
		_vm->_textMan->printToViewport(174, L1091_i_Y, (Color)L1095_i_StatisticColor, _vm->_championMan->getStringFromInteger(AL1092_i_StatisticCurrentValue, true, 3).c_str());
		strcpy(L1097_ac_String, "/");
		strcat(L1097_ac_String, _vm->_championMan->getStringFromInteger(L1096_ui_StatisticMaximumValue, true, 3).c_str());
		_vm->_textMan->printToViewport(192, L1091_i_Y, k13_ColorLightestGray, L1097_ac_String);
		L1091_i_Y += 7;
	}
}

void InventoryMan::drawStopPressingMouth() {
	drawPanel();
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	_vm->_eventMan->_hideMousePointerRequestCount = 1;
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
}

void InventoryMan::drawStopPressingEye() {
	Thing L1100_T_LeaderHandObject;

	drawIconToViewport(k202_IconIndiceEyeNotLooking, 12, 13);
	drawPanel();
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
	if ((L1100_T_LeaderHandObject = _vm->_championMan->_leaderHandObject) != Thing::_none) {
		_vm->_objectMan->drawLeaderObjectName(L1100_T_LeaderHandObject);
	}
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
	_vm->_eventMan->showMouse();
}

void InventoryMan::clickOnMouth() {
	static int16 G0242_ai_Graphic559_FoodAmounts[8] = {
		500,    /* Apple */
		600,    /* Corn */
		650,    /* Bread */
		820,    /* Cheese */
		550,    /* Screamer Slice */
		350,    /* Worm round */
		990,    /* Drumstick / Shank */
		1400}; /* Dragon steak */

	Thing L1078_T_Thing;
	uint16 L1079_ui_IconIndex;
	uint16 L1080_ui_ChampionIndex;
	bool L1081_B_RemoveObjectFromLeaderHand;
	Junk* L1082_ps_Junk;
	Champion* L1083_ps_Champion;
	TimelineEvent L1084_s_Event;
	uint16 L1085_ui_Multiple;
#define AL1085_ui_PotionPower         L1085_ui_Multiple
#define AL1085_ui_AdjustedPotionPower L1085_ui_Multiple
#define AL1085_ui_Counter             L1085_ui_Multiple
	uint16 L1086_ui_Counter;
	int16 L1087_i_Wounds;
	uint16 L1088_ui_Multiple;
#define AL1088_ui_ThingType               L1088_ui_Multiple
#define AL1088_ui_Mana                    L1088_ui_Multiple
#define AL1088_ui_HealWoundIterationCount L1088_ui_Multiple
	uint16 L1089_ui_Weight;


	if (_vm->_championMan->_leaderEmptyHanded) {
		if (_panelContent == k0_PanelContentFoodWaterPoisoned) {
			return;
		}
		_vm->_eventMan->_ignoreMouseMovements = true;
		_vm->_pressingMouth = true;
		if (!_vm->_eventMan->isMouseButtonDown(k1_LeftMouseButton)) {
			_vm->_eventMan->_ignoreMouseMovements = false;
			_vm->_pressingMouth = false;
			_vm->_stopPressingMouth = false;
		} else {
			_vm->_eventMan->showMouse();
			_vm->_eventMan->_hideMousePointerRequestCount = 1;
			drawPanelFoodWaterPoisoned();
			_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
		}
		return;
	}
	if (_vm->_championMan->_candidateChampionOrdinal) {
		return;
	}
	if (!getFlag(_vm->_dungeonMan->_objectInfo[_vm->_dungeonMan->getObjectInfoIndex(L1078_T_Thing = _vm->_championMan->_leaderHandObject)]._allowedSlots, k0x0001_ObjectAllowedSlotMouth)) {
		return;
	}
	L1079_ui_IconIndex = _vm->_objectMan->getIconIndex(L1078_T_Thing);
	AL1088_ui_ThingType = L1078_T_Thing.getType();
	L1089_ui_Weight = _vm->_dungeonMan->getObjectWeight(L1078_T_Thing);
	L1083_ps_Champion = &_vm->_championMan->_champions[L1080_ui_ChampionIndex = _vm->ordinalToIndex(_inventoryChampionOrdinal)];
	L1082_ps_Junk = (Junk*)_vm->_dungeonMan->getThingData(L1078_T_Thing);
	if ((L1079_ui_IconIndex >= k8_IconIndiceJunkWater) && (L1079_ui_IconIndex <= k9_IconIndiceJunkWaterSkin)) {
		if (!(L1082_ps_Junk->getChargeCount())) {
			return;
		}
		L1083_ps_Champion->_water = MIN(L1083_ps_Champion->_water + 800, 2048);
		L1082_ps_Junk->setChargeCount(L1082_ps_Junk->getChargeCount() - 1);
		L1081_B_RemoveObjectFromLeaderHand = false;
	} else {
		if (AL1088_ui_ThingType == k8_PotionThingType) {
			L1081_B_RemoveObjectFromLeaderHand = false;
		} else {
			L1082_ps_Junk->setNextThing(Thing::_none);
			L1081_B_RemoveObjectFromLeaderHand = true;
		}
	}
	_vm->_eventMan->showMouse();
	if (L1081_B_RemoveObjectFromLeaderHand) {
		_vm->_championMan->getObjectRemovedFromLeaderHand();
	}
	if (AL1088_ui_ThingType == k8_PotionThingType) {
		AL1085_ui_PotionPower = ((Potion*)L1082_ps_Junk)->getPower();
		L1086_ui_Counter = ((511 - AL1085_ui_PotionPower) / (32 + (AL1085_ui_PotionPower + 1) / 8)) >> 1;
		AL1085_ui_AdjustedPotionPower = (AL1085_ui_PotionPower / 25) + 8; /* Value between 8 and 18 */
		switch (((Potion*)L1082_ps_Junk)->getType()) {
		case k6_PotionTypeRos:
			adjustStatisticCurrentValue(L1083_ps_Champion, k2_ChampionStatDexterity, AL1085_ui_AdjustedPotionPower);
			break;
		case k7_PotionTypeKu:
			adjustStatisticCurrentValue(L1083_ps_Champion, k1_ChampionStatStrength, (((Potion*)L1082_ps_Junk)->getPower() / 35) + 5); /* Value between 5 and 12 */
			break;
		case k8_PotionTypeDane:
			adjustStatisticCurrentValue(L1083_ps_Champion, k3_ChampionStatWisdom, AL1085_ui_AdjustedPotionPower);
			break;
		case k9_PotionTypeNeta:
			adjustStatisticCurrentValue(L1083_ps_Champion, k4_ChampionStatVitality, AL1085_ui_AdjustedPotionPower);
			break;
		case k10_PotionTypeAntivenin:
			_vm->_championMan->unpoison(L1080_ui_ChampionIndex);
			break;
		case k11_PotionTypeMon:
			L1083_ps_Champion->_currStamina += MIN(L1083_ps_Champion->_maxStamina - L1083_ps_Champion->_currStamina, L1083_ps_Champion->_maxStamina / L1086_ui_Counter);
			break;
		case k12_PotionTypeYa:
			AL1085_ui_AdjustedPotionPower += AL1085_ui_AdjustedPotionPower >> 1;
			if (L1083_ps_Champion->_shieldDefense > 50) {
				AL1085_ui_AdjustedPotionPower >>= 2;
			}
			L1083_ps_Champion->_shieldDefense += AL1085_ui_AdjustedPotionPower;
			L1084_s_Event._type = k72_TMEventTypeChampionShield;
			setMapAndTime(L1084_s_Event._mapTime, _vm->_dungeonMan->_partyMapIndex, _vm->_gameTime + (AL1085_ui_AdjustedPotionPower * AL1085_ui_AdjustedPotionPower));
			L1084_s_Event._priority = L1080_ui_ChampionIndex;
			L1084_s_Event._B._defense = AL1085_ui_AdjustedPotionPower;
			_vm->_timeline->addEventGetEventIndex(&L1084_s_Event);
			setFlag(L1083_ps_Champion->_attributes, k0x1000_ChampionAttributeStatusBox);
			break;
		case k13_PotionTypeEe:
			AL1088_ui_Mana = MIN(900, (L1083_ps_Champion->_currMana + AL1085_ui_AdjustedPotionPower) + (AL1085_ui_AdjustedPotionPower - 8));
			if (AL1088_ui_Mana > L1083_ps_Champion->_maxMana) {
				AL1088_ui_Mana -= (AL1088_ui_Mana - MAX(L1083_ps_Champion->_currMana, L1083_ps_Champion->_maxMana)) >> 1;
			}
			L1083_ps_Champion->_currMana = AL1088_ui_Mana;
			break;
		case k14_PotionTypeVi:
			AL1088_ui_HealWoundIterationCount = MAX(1, (((Potion*)L1082_ps_Junk)->getPower() / 42));
			L1083_ps_Champion->_currHealth += L1083_ps_Champion->_maxHealth / L1086_ui_Counter;
			L1087_i_Wounds = L1083_ps_Champion->_wounds;
			if (L1087_i_Wounds) { /* If the champion is wounded */
				L1086_ui_Counter = 10;
				do {
					for (AL1085_ui_Counter = 0; AL1085_ui_Counter < AL1088_ui_HealWoundIterationCount; AL1085_ui_Counter++) {
						L1083_ps_Champion->_wounds &= _vm->getRandomNumber(65536);
					}
					AL1088_ui_HealWoundIterationCount = 1;
				} while ((L1087_i_Wounds == L1083_ps_Champion->_wounds) && --L1086_ui_Counter); /* Loop until at least one wound is healed or there are no more heal iterations */
			}
			setFlag(L1083_ps_Champion->_attributes, k0x0200_ChampionAttributeLoad | k0x2000_ChampionAttributeWounds);
			break;
		case k15_PotionTypeWaterFlask:
			L1083_ps_Champion->_water = MIN(L1083_ps_Champion->_water + 1600, 2048);
			break;
		default:
			break;
		}
		((Potion*)L1082_ps_Junk)->setType(k20_PotionTypeEmptyFlask);
	} else if ((L1079_ui_IconIndex >= k168_IconIndiceJunkApple) && (L1079_ui_IconIndex < k176_IconIndiceJunkIronKey)) {
		L1083_ps_Champion->_food = MIN(L1083_ps_Champion->_food + G0242_ai_Graphic559_FoodAmounts[L1079_ui_IconIndex - k168_IconIndiceJunkApple], 2048);
	}

	if (L1083_ps_Champion->_currStamina > L1083_ps_Champion->_maxStamina)
		L1083_ps_Champion->_currStamina = L1083_ps_Champion->_maxStamina;

	if (L1083_ps_Champion->_currHealth > L1083_ps_Champion->_maxHealth)
		L1083_ps_Champion->_currHealth = L1083_ps_Champion->_maxHealth;

	if (L1081_B_RemoveObjectFromLeaderHand) {
		for (L1086_ui_Counter = 5; --L1086_ui_Counter; _vm->delay(8)) { /* Animate mouth icon */
			_vm->_objectMan->drawIconToScreen(k205_IconIndiceMouthOpen + !(L1086_ui_Counter & 0x0001), 56, 46);
			_vm->_eventMan->discardAllInput();
			if (_vm->_engineShouldQuit)
				return;
			_vm->_displayMan->updateScreen();
		}
	} else {
		_vm->_championMan->drawChangedObjectIcons();
		_vm->_championMan->_champions[_vm->_championMan->_leaderIndex]._load += _vm->_dungeonMan->getObjectWeight(L1078_T_Thing) - L1089_ui_Weight;
		setFlag(_vm->_championMan->_champions[_vm->_championMan->_leaderIndex]._attributes, k0x0200_ChampionAttributeLoad);
	}
	_vm->_sound->requestPlay(k08_soundSWALLOW, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k0_soundModePlayImmediately);
	setFlag(L1083_ps_Champion->_attributes, k0x0100_ChampionAttributeStatistics);
	if (_panelContent == k0_PanelContentFoodWaterPoisoned) {
		setFlag(L1083_ps_Champion->_attributes, k0x0800_ChampionAttributePanel);
	}
	_vm->_championMan->drawChampionState((ChampionIndex)L1080_ui_ChampionIndex);
	_vm->_eventMan->hideMouse();
}

void InventoryMan::adjustStatisticCurrentValue(Champion* champ, uint16 statIndex, int16 valueDelta) {
	int16 L1077_i_Multiple;
#define AL1077_i_CurrentValue L1077_i_Multiple
#define AL1077_i_Delta        L1077_i_Multiple

	if (valueDelta >= 0) {
		if ((AL1077_i_CurrentValue = champ->_statistics[statIndex][k1_ChampionStatCurrent]) > 120) {
			valueDelta >>= 1;
			if (AL1077_i_CurrentValue > 150) {
				valueDelta >>= 1;
			}
			valueDelta++;
		}
		AL1077_i_Delta = MIN(valueDelta, (int16)(170 - AL1077_i_CurrentValue));
	} else { /* BUG0_00 Useless code. The function is always called with valueDelta having a positive value */
		AL1077_i_Delta = MAX(valueDelta, int16(champ->_statistics[statIndex][k2_ChampionStatMinimum] - champ->_statistics[statIndex][k1_ChampionStatCurrent]));
	}
	champ->_statistics[statIndex][k1_ChampionStatCurrent] += AL1077_i_Delta;
}

void InventoryMan::clickOnEye() {
	_vm->_eventMan->_ignoreMouseMovements = true;
	_vm->_pressingEye = true;
	if (!_vm->_eventMan->isMouseButtonDown(k1_LeftMouseButton)) {
		_vm->_eventMan->_ignoreMouseMovements = false;
		_vm->_pressingEye = false;
		_vm->_stopPressingEye = false;
		return;
	}
	_vm->_eventMan->discardAllInput();
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->hideMouse();
	_vm->delay(8);
	drawIconToViewport(k203_IconIndiceEyeLooking, 12, 13);
	if (_vm->_championMan->_leaderEmptyHanded) {
		drawChampionSkillsAndStatistics();
	} else {
		_vm->_objectMan->clearLeaderObjectName();
		drawPanelObject(_vm->_championMan->_leaderHandObject, true);
	}
	_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);

}
}
