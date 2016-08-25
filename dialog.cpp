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

#include "dialog.h"
#include "gfx.h"
#include "text.h"
#include "eventman.h"


namespace DM {

DialogMan::DialogMan(DMEngine *vm) : _vm(vm) {
	_selectedDialogChoice = 0;
}

void DialogMan::dialogDraw(const char *msg1, const char *msg2, const char *choice1, const char *choice2, const char *choice3, const char *choice4, bool screenDialog, bool clearScreen, bool fading) {
	static Box constBox1 = Box(0, 223, 101, 125);
	static Box constBox2 = Box(0, 223, 76, 100);
	static Box constBox3 = Box(0, 223, 51, 75);
	static Box dialog2ChoicesPatch = Box(102, 122, 89, 125);
	static Box dialog4ChoicesPatch = Box(102, 122, 62, 97);

	_vm->_displayMan->f466_loadIntoBitmap(k0_dialogBoxGraphicIndice, _vm->_displayMan->_g296_bitmapViewport);
	//Strangerke: the version should be replaced by a ScummVM/RogueVM (?) string
	// TODO: replace with ScummVM version string
	_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, 192, 7, k2_ColorLightGray, k1_ColorDarkGary, "V2.2", k136_heightViewport);
	int16 choiceCount = 1;
	if (choice2)
		choiceCount++;

	if (choice3)
		choiceCount++;

	if (choice4)
		choiceCount++;

	if (fading)
		_vm->_displayMan->f436_STARTEND_FadeToPalette(_vm->_displayMan->_g345_aui_BlankBuffer);

	if (clearScreen)
		_vm->_displayMan->fillScreen(k0_ColorBlack);

	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	if (choiceCount == 1) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox1, 0, 64, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox2, 0, 39, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox3, 0, 14, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 114);
	} else if (choiceCount == 2) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, dialog2ChoicesPatch, 102, 52, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 112, 114);
	} else if (choiceCount == 3) {
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 59, 114);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 166, 114);
	} else if (choiceCount == 4) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, dialog4ChoicesPatch, 102, 99, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 59, 77);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 166, 77);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 59, 114);
		printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice4, 166, 114);
	}

	int16 textPosX;
	int16 textPosY = 29;
	if (msg1) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg1, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosY = 21;
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
			textPosY += 8;
		} else {
			textPosX = 113 - ((strlen(msg1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, msg1, k136_heightViewport);
			textPosY += 8;
		}
	}
	if (msg2) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg2, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
		} else {
			textPosX = 113 - ((strlen(msg2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, msg2, k136_heightViewport);
		}
	}
	if (screenDialog) {
		Box displayBox;
		displayBox._y1 = 33;
		displayBox._y2 = 168;
		displayBox._x1 = 47;
		displayBox._x2 = 270;
		_vm->_eventMan->showMouse();
		_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->_g296_bitmapViewport, &displayBox, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport);
		_vm->_eventMan->hideMouse();
	} else {
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
		_vm->delay(1);
	}

	if (fading)
		_vm->_displayMan->f436_STARTEND_FadeToPalette(_vm->_displayMan->_g347_paletteTopAndBottomScreen);

	_vm->_displayMan->_g297_drawFloorAndCeilingRequested = true;
	_vm->_displayMan->updateScreen();
}

void DialogMan::printCenteredChoice(byte *bitmap, const char *str, int16 posX, int16 posY) {
	if (str) {
		posX -= (strlen(str) * 6) >> 1;
		_vm->_textMan->f40_printTextToBitmap(bitmap, k112_byteWidthViewport, posX, posY, k9_ColorGold, k5_ColorLightBrown, str, k136_heightViewport);
	}
}

bool DialogMan::isMessageOnTwoLines(const char *str, char *part1, char *part2) {
	uint16 strLength = strlen(str);
	if (strLength <= 30)
		return false;

	strcpy(part1, str);
	uint16 splitPosition = strLength >> 1;
	while ((splitPosition < strLength) && (part1[splitPosition] != ' '))
		splitPosition++;

	part1[splitPosition] = '\0';
	strcpy(part2, &part1[splitPosition + 1]);
	return true;
}

int16 DialogMan::getChoice(uint16 choiceCount, uint16 dialogSetIndex, int16 driveType, int16 automaticChoiceIfFlopyInDrive) {
	MouseInput *L1298_ps_PrimaryMouseInputBackup;
	MouseInput *L1299_ps_SecondaryMouseInputBackup;
	KeyboardInput *L1300_ps_PrimaryKeyboardInputBackup;
	KeyboardInput *L1301_ps_SecondaryKeyboardInputBackup;
	Box L1303_s_BoxB;
	Box L1304_s_BoxA;

	_vm->_eventMan->hideMouse();
	L1298_ps_PrimaryMouseInputBackup = _vm->_eventMan->_primaryMouseInput;
	L1299_ps_SecondaryMouseInputBackup = _vm->_eventMan->_secondaryMouseInput;
	L1300_ps_PrimaryKeyboardInputBackup = _vm->_eventMan->_primaryKeyboardInput;
	L1301_ps_SecondaryKeyboardInputBackup = _vm->_eventMan->_secondaryKeyboardInput;
	_vm->_eventMan->_secondaryMouseInput = nullptr;
	_vm->_eventMan->_primaryKeyboardInput = nullptr;
	_vm->_eventMan->_secondaryKeyboardInput = nullptr;
	_vm->_eventMan->_primaryMouseInput = _vm->_eventMan->_primaryMouseInputDialogSets[dialogSetIndex][choiceCount - 1];
	_vm->_eventMan->discardAllInput();
	_selectedDialogChoice = 99;
	do {
		Common::Event key;
		Common::EventType eventType = _vm->_eventMan->processInput(&key);
		_vm->_eventMan->processCommandQueue();
		_vm->delay(1);
		_vm->_displayMan->updateScreen();
		if ((_selectedDialogChoice == 99) && (choiceCount == 1) 
			&& (eventType != Common::EVENT_INVALID) && key.kbd.keycode == Common::KEYCODE_RETURN) {
			/* If a choice has not been made yet with the mouse and the dialog has only one possible choice and carriage return was pressed on the keyboard */
			_selectedDialogChoice = k1_DIALOG_CHOICE_1;
		}
	} while (_selectedDialogChoice == 99);
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	L1304_s_BoxA = _vm->_eventMan->_primaryMouseInput[_selectedDialogChoice - 1]._hitbox;
	L1304_s_BoxA._x1 -= 3;
	L1304_s_BoxA._x2 += 3;
	L1304_s_BoxA._y1 -= 3;
	L1304_s_BoxA._y2 += 4;
	_vm->_eventMan->showMouse();
	_vm->_displayMan->_g297_drawFloorAndCeilingRequested = true;
	L1303_s_BoxB._x1 = 0;
	L1303_s_BoxB._y1 = 0;
	L1303_s_BoxB._y2 = L1304_s_BoxA._y2 - L1304_s_BoxA._y1 + 3;
	L1303_s_BoxB._x2 = L1304_s_BoxA._x2 - L1304_s_BoxA._x1 + 3;
	_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g348_bitmapScreen, _vm->_displayMan->_g296_bitmapViewport,
										L1303_s_BoxB, L1304_s_BoxA._x1, L1304_s_BoxA._y1, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 25);
	_vm->delay(1);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._y2 = L1303_s_BoxB._y1;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k5_ColorLightBrown);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._x2 = L1303_s_BoxB._x1;
	L1303_s_BoxB._y2--;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k5_ColorLightBrown);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._y2--;
	L1303_s_BoxB._y1 = L1303_s_BoxB._y2;
	L1303_s_BoxB._x1 -= 2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k0_ColorBlack);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._x1 = L1303_s_BoxB._x2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k0_ColorBlack);
	_vm->delay(2);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._y1++;
	L1303_s_BoxB._y2 = L1303_s_BoxB._y1;
	L1303_s_BoxB._x2 -= 2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k5_ColorLightBrown);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._x1++;
	L1303_s_BoxB._x2 = L1303_s_BoxB._x1;
	L1303_s_BoxB._y2--;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k5_ColorLightBrown);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._x2--;
	L1303_s_BoxB._x1 = L1303_s_BoxB._x2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k0_ColorBlack);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._y1 = L1303_s_BoxB._y2 = L1303_s_BoxB._y2 - 2;
	L1303_s_BoxB._x1++;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k0_ColorBlack);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._y1 = L1303_s_BoxB._y2 = L1303_s_BoxB._y2 + 2;
	L1303_s_BoxB._x1--;
	L1303_s_BoxB._x2 += 2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k13_ColorLightestGray);
	L1303_s_BoxB = L1304_s_BoxA;
	L1303_s_BoxB._x1 = L1303_s_BoxB._x2 = L1303_s_BoxB._x2 + 3;
	L1303_s_BoxB._y2 += 2;
	_vm->_displayMan->D24_fillScreenBox(L1303_s_BoxB, k13_ColorLightestGray);
	_vm->delay(2);
	L1304_s_BoxA._x2 += 3;
	L1304_s_BoxA._y2 += 3;
	_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g348_bitmapScreen,
										L1304_s_BoxA, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 25, k200_heightScreen);
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->_primaryMouseInput = L1298_ps_PrimaryMouseInputBackup;
	_vm->_eventMan->_secondaryMouseInput = L1299_ps_SecondaryMouseInputBackup;
	_vm->_eventMan->_primaryKeyboardInput = L1300_ps_PrimaryKeyboardInputBackup;
	_vm->_eventMan->_secondaryKeyboardInput = L1301_ps_SecondaryKeyboardInputBackup;
	_vm->_eventMan->discardAllInput();
	_vm->_eventMan->showMouse();
	return _selectedDialogChoice;
}
}
