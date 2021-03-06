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


#include "movesens.h"
#include "champion.h"
#include "inventory.h"
#include "dungeonman.h"
#include "objectman.h"
#include "timeline.h"
#include "group.h"
#include "projexpl.h"
#include "text.h"
#include "sounds.h"


namespace DM {

MovesensMan::MovesensMan(DMEngine* vm) : _vm(vm) {
	_moveResultMapX = 0;
	_moveResultMapY = 0;
	_moveResultMapIndex = 0;
	_moveResultDir = 0;
	_moveResultCell = 0;
	_useRopeToClimbDownPit = false;
	_sensorRotationEffect = 0;
	_sensorRotationEffMapX = 0;
	_sensorRotationEffMapY = 0;
	_sensorRotationEffCell = 0;
}

bool MovesensMan::sensorIsTriggeredByClickOnWall(int16 mapX, int16 mapY, uint16 cellParam) {
	Thing L0750_T_ThingBeingProcessed;
	uint16 L0751_ui_ThingType;
	uint16 L0752_ui_Cell;
	bool L0753_B_DoNotTriggerSensor;
	Thing* L0754_ps_Generic;
	Sensor* L0755_ps_Sensor;
	int16 L0756_i_SensorEffect;
	uint16 L0757_ui_SensorType;
	int16 L0758_i_SensorData;
	bool L0759_B_AtLeastOneSensorWasTriggered;
	int16 L0760_ai_SensorCountToProcessPerCell[4];
	Thing L0761_T_LeaderHandObject;
	Thing L0762_T_ThingOnSquare;
	Thing L0763_T_LastProcessedThing;
	Thing L0764_T_SquareFirstThing;
	Sensor* L0765_ps_Sensor;


	L0759_B_AtLeastOneSensorWasTriggered = false;
	L0761_T_LeaderHandObject = _vm->_championMan->_leaderHandObject;
	for (L0752_ui_Cell = k0_CellNorthWest; L0752_ui_Cell < k3_CellSouthWest + 1; L0752_ui_Cell++) {
		L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell] = 0;
	}
	L0764_T_SquareFirstThing = L0750_T_ThingBeingProcessed = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	while (L0750_T_ThingBeingProcessed != Thing::_endOfList) {
		if ((L0751_ui_ThingType = (L0750_T_ThingBeingProcessed).getType()) == k3_SensorThingType) {
			L0760_ai_SensorCountToProcessPerCell[(L0750_T_ThingBeingProcessed).getCell()]++;
		} else {
			if (L0751_ui_ThingType >= k4_GroupThingType)
				break;
		}
		L0750_T_ThingBeingProcessed = _vm->_dungeonMan->getNextThing(L0750_T_ThingBeingProcessed);
	}
	L0763_T_LastProcessedThing = L0750_T_ThingBeingProcessed = L0764_T_SquareFirstThing;
	while (L0750_T_ThingBeingProcessed != Thing::_endOfList) {
		if ((L0751_ui_ThingType = (L0750_T_ThingBeingProcessed).getType()) == k3_SensorThingType) {
			L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell = (L0750_T_ThingBeingProcessed).getCell()]--;
			L0755_ps_Sensor = (Sensor *)_vm->_dungeonMan->getThingData(L0750_T_ThingBeingProcessed);
			if ((L0757_ui_SensorType = (L0755_ps_Sensor)->getType()) == k0_SensorDisabled)
				goto T0275058_ProceedToNextThing;
			if ((_vm->_championMan->_leaderIndex == kM1_ChampionNone) && (L0757_ui_SensorType != k127_SensorWallChampionPortrait))
				goto T0275058_ProceedToNextThing;
			if (L0752_ui_Cell != cellParam)
				goto T0275058_ProceedToNextThing;
			L0758_i_SensorData = L0755_ps_Sensor->getData();
			L0756_i_SensorEffect = L0755_ps_Sensor->getAttrEffectA();
			switch (L0757_ui_SensorType) {
			case k1_SensorWallOrnClick:
				L0753_B_DoNotTriggerSensor = false;
				if (L0755_ps_Sensor->getAttrEffectA() == k3_SensorEffHold) {
					goto T0275058_ProceedToNextThing;
				}
				break;
			case k2_SensorWallOrnClickWithAnyObj:
				L0753_B_DoNotTriggerSensor = (_vm->_championMan->_leaderEmptyHanded != L0755_ps_Sensor->getAttrRevertEffectA());
				break;
			case k17_SensorWallOrnClickWithSpecObjRemovedSensor:
			case k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors:
				if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
					goto T0275058_ProceedToNextThing;
			case k3_SensorWallOrnClickWithSpecObj:
			case k4_SensorWallOrnClickWithSpecObjRemoved:
				L0753_B_DoNotTriggerSensor = ((L0758_i_SensorData == _vm->_objectMan->getObjectType(L0761_T_LeaderHandObject)) == L0755_ps_Sensor->getAttrRevertEffectA());
				if (!L0753_B_DoNotTriggerSensor && (L0757_ui_SensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor)) {
					if (L0763_T_LastProcessedThing == L0750_T_ThingBeingProcessed) /* If the sensor is the only one of its type on the cell */
						break;
					L0765_ps_Sensor = (Sensor *)_vm->_dungeonMan->getThingData(L0763_T_LastProcessedThing);
					L0765_ps_Sensor->setNextThing(L0755_ps_Sensor->getNextThing());
					L0755_ps_Sensor->setNextThing(Thing::_none);
					L0750_T_ThingBeingProcessed = L0763_T_LastProcessedThing;
				}
				if (!L0753_B_DoNotTriggerSensor && (L0757_ui_SensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors)) {
					triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				}
				break;
			case k12_SensorWallObjGeneratorRotateSensors:
				if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
					goto T0275058_ProceedToNextThing;
				L0753_B_DoNotTriggerSensor = !_vm->_championMan->_leaderEmptyHanded;
				if (!L0753_B_DoNotTriggerSensor) {
					triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				}
				break;
			case k13_SensorWallSingleObjStorageRotateSensors:
				if (_vm->_championMan->_leaderEmptyHanded) {
					if ((L0761_T_LeaderHandObject = getObjectOfTypeInCell(mapX, mapY, L0752_ui_Cell, L0758_i_SensorData)) == Thing::_none)
						goto T0275058_ProceedToNextThing;
					_vm->_dungeonMan->unlinkThingFromList(L0761_T_LeaderHandObject, Thing(0), mapX, mapY);
					_vm->_championMan->putObjectInLeaderHand(L0761_T_LeaderHandObject, true);
				} else {
					if ((_vm->_objectMan->getObjectType(L0761_T_LeaderHandObject) != L0758_i_SensorData) || (getObjectOfTypeInCell(mapX, mapY, L0752_ui_Cell, L0758_i_SensorData) != Thing::_none))
						goto T0275058_ProceedToNextThing;
					_vm->_championMan->getObjectRemovedFromLeaderHand();
					_vm->_dungeonMan->linkThingToList(thingWithNewCell(L0761_T_LeaderHandObject, L0752_ui_Cell), Thing(0), mapX, mapY);
					L0761_T_LeaderHandObject = Thing::_none;
				}
				triggerLocalEffect(k2_SensorEffToggle, mapX, mapY, L0752_ui_Cell); /* This will cause a rotation of the sensors at the specified cell on the specified square after all sensors have been processed */
				if ((L0756_i_SensorEffect == k3_SensorEffHold) && !_vm->_championMan->_leaderEmptyHanded) {
					L0753_B_DoNotTriggerSensor = true;
				} else {
					L0753_B_DoNotTriggerSensor = false;
				}
				break;
			case k16_SensorWallObjExchanger:
				if (L0760_ai_SensorCountToProcessPerCell[L0752_ui_Cell]) /* If the sensor is not the last one of its type on the cell */
					goto T0275058_ProceedToNextThing;
				L0762_T_ThingOnSquare = _vm->_dungeonMan->getSquareFirstObject(mapX, mapY);
				if ((_vm->_objectMan->getObjectType(L0761_T_LeaderHandObject) != L0758_i_SensorData) || (L0762_T_ThingOnSquare == Thing::_none))
					goto T0275058_ProceedToNextThing;
				_vm->_dungeonMan->unlinkThingFromList(L0762_T_ThingOnSquare, Thing(0), mapX, mapY);
				_vm->_championMan->getObjectRemovedFromLeaderHand();
				_vm->_dungeonMan->linkThingToList(thingWithNewCell(L0761_T_LeaderHandObject, L0752_ui_Cell), Thing(0), mapX, mapY);
				_vm->_championMan->putObjectInLeaderHand(L0762_T_ThingOnSquare, true);
				L0753_B_DoNotTriggerSensor = false;
				break;
			case k127_SensorWallChampionPortrait:
				_vm->_championMan->addCandidateChampionToParty(L0758_i_SensorData);
				goto T0275058_ProceedToNextThing;
			default:
				goto T0275058_ProceedToNextThing;
			}
			if (L0756_i_SensorEffect == k3_SensorEffHold) {
				L0756_i_SensorEffect = L0753_B_DoNotTriggerSensor ? k1_SensorEffClear : k0_SensorEffSet;
				L0753_B_DoNotTriggerSensor = false;
			}
			if (!L0753_B_DoNotTriggerSensor) {
				L0759_B_AtLeastOneSensorWasTriggered = true;
				if (L0755_ps_Sensor->getAttrAudibleA()) {
					_vm->_sound->requestPlay(k01_soundSWITCH, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k1_soundModePlayIfPrioritized);
				}
				if (!_vm->_championMan->_leaderEmptyHanded && ((L0757_ui_SensorType == k4_SensorWallOrnClickWithSpecObjRemoved) || (L0757_ui_SensorType == k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors) || (L0757_ui_SensorType == k17_SensorWallOrnClickWithSpecObjRemovedSensor))) {
					L0754_ps_Generic = (Thing *)_vm->_dungeonMan->getThingData(L0761_T_LeaderHandObject);
					*L0754_ps_Generic = Thing::_none;
					_vm->_championMan->getObjectRemovedFromLeaderHand();
					L0761_T_LeaderHandObject = Thing::_none;
				} else {
					if (_vm->_championMan->_leaderEmptyHanded &&
						(L0757_ui_SensorType == k12_SensorWallObjGeneratorRotateSensors) &&
						((L0761_T_LeaderHandObject = _vm->_dungeonMan->getObjForProjectileLaucherOrObjGen(L0758_i_SensorData)) != Thing::_none)) {
						_vm->_championMan->putObjectInLeaderHand(L0761_T_LeaderHandObject, true);
					}
				}
				triggerEffect(L0755_ps_Sensor, L0756_i_SensorEffect, mapX, mapY, L0752_ui_Cell);
			}
			goto T0275058_ProceedToNextThing;
		}
		if (L0751_ui_ThingType >= k4_GroupThingType)
			break;
T0275058_ProceedToNextThing:
		L0763_T_LastProcessedThing = L0750_T_ThingBeingProcessed;
		L0750_T_ThingBeingProcessed = _vm->_dungeonMan->getNextThing(L0750_T_ThingBeingProcessed);
	}
	processRotationEffect();
	return L0759_B_AtLeastOneSensorWasTriggered;
}

bool MovesensMan::getMoveResult(Thing thing, int16 mapX, int16 mapY, int16 destMapX, int16 destMapY) {
	int16 L0708_i_Multiple = 0;
#define AL0708_i_DestinationSquare L0708_i_Multiple
#define AL0708_i_ScentIndex        L0708_i_Multiple
#define AL0708_i_ActiveGroupIndex  L0708_i_Multiple
	int16 L0709_i_Multiple;
#define AL0709_i_DestinationSquareType L0709_i_Multiple
#define AL0709_i_ChampionIndex         L0709_i_Multiple
	int16 L0710_i_ThingType;
	Champion* L0711_ps_Champion;
	Teleporter* L0712_ps_Teleporter;
	int16 L0718_i_RequiredTeleporterScope;
	// Strangerke: Only present in v2.1, but it fixes a bug, so I propose to keep it
	int16 L0719_i_TraversedPitCount;
	uint16 L0720_ui_MoveGroupResult;
	uint16 L0727_ui_Multiple;
#define AL0727_ui_ThingCell L0727_ui_Multiple
#define AL0727_ui_Outcome   L0727_ui_Multiple
#define AL0727_ui_Backup    L0727_ui_Multiple
	int16 L0728_i_ChainedMoveCount;
	uint16 L1638_ui_MovementSoundIndex;

	L0710_i_ThingType = kM1_PartyThingType;
	L0719_i_TraversedPitCount = 0;
	L0720_ui_MoveGroupResult = 0;

	uint16 L0717_ui_ThingCell = 0;

	bool L0713_B_ThingLevitates = false;
	if (thing != Thing::_party) {
		L0710_i_ThingType = thing.getType();
		L0717_ui_ThingCell = thing.getCell();
		L0713_B_ThingLevitates = isLevitating(thing);
	}
	/* If moving the party or a creature on the party map from a dungeon square then check for a projectile impact */
	if ((mapX >= 0) && ((thing == Thing::_party) || ((L0710_i_ThingType == k4_GroupThingType) && (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex)))) {
		if (moveIsKilledByProjectileImpact(mapX, mapY, destMapX, destMapY, thing)) {
			return true; /* The specified group thing cannot be moved because it was killed by a projectile impact */
		}
	}

	uint16 L0714_ui_MapIndexSource = 0;
	uint16 L0715_ui_MapIndexDestination = 0;
	bool L0721_B_GroupOnPartyMap = false;
	bool L0725_B_PartySquare = false;
	bool L0726_B_Audible = false;

	if (destMapX >= 0) {
		L0714_ui_MapIndexSource = L0715_ui_MapIndexDestination = _vm->_dungeonMan->_currMapIndex;
		L0721_B_GroupOnPartyMap = (L0714_ui_MapIndexSource == _vm->_dungeonMan->_partyMapIndex) && (mapX >= 0);
		uint16 L0716_ui_Direction = 0;
		bool L0722_B_FallKilledGroup = false;
		bool L0723_B_DrawDungeonViewWhileFalling = false;
		bool L0724_B_DestinationIsTeleporterTarget = false;
		if (thing == Thing::_party) {
			_vm->_dungeonMan->_partyMapX = destMapX;
			_vm->_dungeonMan->_partyMapY = destMapY;
			L0718_i_RequiredTeleporterScope = k0x0002_TelepScopeObjOrParty;
			L0723_B_DrawDungeonViewWhileFalling = !_vm->_inventoryMan->_inventoryChampionOrdinal && !_vm->_championMan->_partyIsSleeping;
			L0716_ui_Direction = _vm->_dungeonMan->_partyDir;
		} else {
			if (L0710_i_ThingType == k4_GroupThingType) {
				L0718_i_RequiredTeleporterScope = k0x0001_TelepScopeCreatures;
			} else {
				L0718_i_RequiredTeleporterScope = (k0x0001_TelepScopeCreatures | k0x0002_TelepScopeObjOrParty);
			}
		}
		if (L0710_i_ThingType == k14_ProjectileThingType) {
			L0712_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->getThingData(thing);
			_moveResultDir = (_vm->_timeline->_events[((Projectile *)L0712_ps_Teleporter)->_eventIndex])._C._projectile.getDir();
		}
		for (L0728_i_ChainedMoveCount = 1000; --L0728_i_ChainedMoveCount; ) { /* No more than 1000 chained moves at once (in a chain of teleporters and pits for example) */
			AL0708_i_DestinationSquare = _vm->_dungeonMan->_currMapData[destMapX][destMapY];
			if ((AL0709_i_DestinationSquareType = Square(AL0708_i_DestinationSquare).getType()) == k5_ElementTypeTeleporter) {
				if (!getFlag(AL0708_i_DestinationSquare, k0x0008_TeleporterOpen))
					break;
				L0712_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->getSquareFirstThingData(destMapX, destMapY);
				if ((L0712_ps_Teleporter->getScope() == k0x0001_TelepScopeCreatures) && (L0710_i_ThingType != k4_GroupThingType))
					break;
				if ((L0718_i_RequiredTeleporterScope != (k0x0001_TelepScopeCreatures | k0x0002_TelepScopeObjOrParty)) && !getFlag(L0712_ps_Teleporter->getScope(), L0718_i_RequiredTeleporterScope))
					break;
				L0724_B_DestinationIsTeleporterTarget = (destMapX == L0712_ps_Teleporter->getTargetMapX()) && (destMapY == L0712_ps_Teleporter->getTargetMapY()) && (L0715_ui_MapIndexDestination == L0712_ps_Teleporter->getTargetMapIndex());
				destMapX = L0712_ps_Teleporter->getTargetMapX();
				destMapY = L0712_ps_Teleporter->getTargetMapY();
				L0726_B_Audible = L0712_ps_Teleporter->isAudible();
				_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination = L0712_ps_Teleporter->getTargetMapIndex());
				if (thing == Thing::_party) {
					_vm->_dungeonMan->_partyMapX = destMapX;
					_vm->_dungeonMan->_partyMapY = destMapY;
					if (L0712_ps_Teleporter->isAudible()) {
						_vm->_sound->requestPlay(k17_soundBUZZ, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k0_soundModePlayImmediately);
					}
					L0723_B_DrawDungeonViewWhileFalling = true;
					if (L0712_ps_Teleporter->getAbsoluteRotation()) {
						_vm->_championMan->setPartyDirection(L0712_ps_Teleporter->getRotation());
					} else {
						_vm->_championMan->setPartyDirection(normalizeModulo4(_vm->_dungeonMan->_partyDir + L0712_ps_Teleporter->getRotation()));
					}
				} else {
					if (L0710_i_ThingType == k4_GroupThingType) {
						if (L0712_ps_Teleporter->isAudible()) {
							_vm->_sound->requestPlay(k17_soundBUZZ, destMapX, destMapY, k1_soundModePlayIfPrioritized);
						}
						L0720_ui_MoveGroupResult = getTeleporterRotatedGroupResult(L0712_ps_Teleporter, thing, L0714_ui_MapIndexSource);
					} else {
						if (L0710_i_ThingType == k14_ProjectileThingType) {
							thing = getTeleporterRotatedProjectileThing(L0712_ps_Teleporter, thing);
						} else {
							if (!(L0712_ps_Teleporter->getAbsoluteRotation()) && (mapX != -2)) {
								thing = thingWithNewCell(thing, normalizeModulo4(thing.getCell() + L0712_ps_Teleporter->getRotation()));
							}
						}
					}
				}
				if (L0724_B_DestinationIsTeleporterTarget)
					break;
			} else {
				if ((AL0709_i_DestinationSquareType == k2_ElementTypePit) && !L0713_B_ThingLevitates && getFlag(AL0708_i_DestinationSquare, k0x0008_PitOpen) && !getFlag(AL0708_i_DestinationSquare, k0x0001_PitImaginary)) {
					if (L0723_B_DrawDungeonViewWhileFalling && !_useRopeToClimbDownPit) {
						L0723_B_DrawDungeonViewWhileFalling = true;
						if (L0719_i_TraversedPitCount) {
							_vm->_dungeonMan->setCurrentMapAndPartyMap(L0715_ui_MapIndexDestination);
							_vm->_displayMan->loadCurrentMapGraphics();
						}
						L0719_i_TraversedPitCount++;
						_vm->_displayMan->drawDungeon(_vm->_dungeonMan->_partyDir, destMapX, destMapY); /* BUG0_28 When falling through multiple pits the dungeon view is updated to show each traversed map but the graphics used for creatures, wall and floor ornaments may not be correct. The dungeon view is drawn for each map by using the graphics loaded for the source map. Therefore the graphics for creatures, wall and floor ornaments may not look like what they should */
																												  /* BUG0_71 Some timings are too short on fast computers. When the party falls in a series of pits, the dungeon view is refreshed too quickly because the execution speed is not limited */
																												  /* BUG0_01 While drawing creatures the engine will read invalid ACTIVE_GROUP data in _vm->_groupMan->_g375_activeGroups because the data is for the creatures on the source map and not the map being drawn. The only consequence is that creatures may be drawn with incorrect bitmaps and/or directions */
					}
					L0715_ui_MapIndexDestination = _vm->_dungeonMan->getLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1, &destMapX, &destMapY);
					_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
					if (thing == Thing::_party) {
						_vm->_dungeonMan->_partyMapX = destMapX;
						_vm->_dungeonMan->_partyMapY = destMapY;
						if (_vm->_championMan->_partyChampionCount > 0) {
							if (_useRopeToClimbDownPit) {
								for (AL0709_i_ChampionIndex = k0_ChampionFirst, L0711_ps_Champion = _vm->_championMan->_champions; AL0709_i_ChampionIndex < _vm->_championMan->_partyChampionCount; AL0709_i_ChampionIndex++, L0711_ps_Champion++) {
									if (L0711_ps_Champion->_currHealth) {
										_vm->_championMan->decrementStamina(AL0709_i_ChampionIndex, ((L0711_ps_Champion->_load * 25) / _vm->_championMan->getMaximumLoad(L0711_ps_Champion)) + 1);
									}
								}
							} else {
								if (_vm->_championMan->getDamagedChampionCount(20, k0x0010_ChampionWoundLegs | k0x0020_ChampionWoundFeet, k2_attackType_SELF)) {
									_vm->_sound->requestPlay(k06_soundSCREAM, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, k0_soundModePlayImmediately);
								}
							}
						}
						_useRopeToClimbDownPit = false;
					} else {
						if (L0710_i_ThingType == k4_GroupThingType) {
							_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
							AL0727_ui_Outcome = _vm->_groupMan->getDamageAllCreaturesOutcome((Group *)_vm->_dungeonMan->getThingData(thing), mapX, mapY, 20, false);
							_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
							L0722_B_FallKilledGroup = (AL0727_ui_Outcome == k2_outcomeKilledAllCreaturesInGroup);
							if (L0722_B_FallKilledGroup)
								break;
							if (AL0727_ui_Outcome == k1_outcomeKilledSomeCreaturesInGroup) {
								_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
							}
						}
					}
				} else {
					if ((AL0709_i_DestinationSquareType == k3_ElementTypeStairs) && (thing != Thing::_party) && (L0710_i_ThingType != k14_ProjectileThingType)) {
						if (!getFlag(AL0708_i_DestinationSquare, k0x0004_StairsUp)) {
							L0715_ui_MapIndexDestination = _vm->_dungeonMan->getLocationAfterLevelChange(L0715_ui_MapIndexDestination, 1, &destMapX, &destMapY);
							_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
						}
						L0716_ui_Direction = _vm->_dungeonMan->getStairsExitDirection(destMapX, destMapY);
						destMapX += _vm->_dirIntoStepCountEast[L0716_ui_Direction], destMapY += _vm->_dirIntoStepCountNorth[L0716_ui_Direction];
						L0716_ui_Direction = returnOppositeDir((Direction)L0716_ui_Direction);
						AL0727_ui_ThingCell = thing.getCell();
						AL0727_ui_ThingCell = normalizeModulo4((((AL0727_ui_ThingCell - L0716_ui_Direction + 1) & 0x0002) >> 1) + L0716_ui_Direction);
						thing = thingWithNewCell(thing, AL0727_ui_ThingCell);
					} else
						break;
				}
			}
		}
		if ((L0710_i_ThingType == k4_GroupThingType) && (L0722_B_FallKilledGroup || !_vm->_dungeonMan->isCreatureAllowedOnMap(thing, L0715_ui_MapIndexDestination))) {
			_vm->_groupMan->dropMovingCreatureFixedPossession(thing, destMapX, destMapY);
			_vm->_groupMan->dropGroupPossessions(destMapX, destMapY, thing, k2_soundModePlayOneTickLater);
			_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
			if (mapX >= 0) {
				_vm->_groupMan->groupDelete(mapX, mapY);
			}
			return true; /* The specified group thing cannot be moved because it was killed by a fall or because it is not allowed on the destination map */
		}
		_moveResultMapX = destMapX;
		_moveResultMapY = destMapY;
		_moveResultMapIndex = L0715_ui_MapIndexDestination;
		_moveResultCell = thing.getCell();
		L0725_B_PartySquare = (L0715_ui_MapIndexDestination == L0714_ui_MapIndexSource) && (destMapX == mapX) && (destMapY == mapY);
		if (L0725_B_PartySquare) {
			if (thing == Thing::_party) {
				if (_vm->_dungeonMan->_partyDir == L0716_ui_Direction) {
					return false;
				}
			} else {
				if ((_moveResultCell == L0717_ui_ThingCell) && (L0710_i_ThingType != k14_ProjectileThingType)) {
					return false;
				}
			}
		} else {
			if ((thing == Thing::_party) && _vm->_championMan->_partyChampionCount) {
				AL0727_ui_Backup = AL0708_i_DestinationSquare;
				AL0708_i_ScentIndex = _vm->_championMan->_party._scentCount;
				while (AL0708_i_ScentIndex >= 24) {
					_vm->_championMan->deleteScent(0);
					AL0708_i_ScentIndex--;
				}
				if (AL0708_i_ScentIndex) {
					_vm->_championMan->addScentStrength(mapX, mapY, (int)(_vm->_gameTime - _vm->_projexpl->_lastPartyMovementTime));
				}
				_vm->_projexpl->_lastPartyMovementTime = _vm->_gameTime;
				_vm->_championMan->_party._scentCount++;
				if (_vm->_championMan->_party._event79Count_Footprints) {
					_vm->_championMan->_party._lastScentIndex = _vm->_championMan->_party._scentCount;
				}
				_vm->_championMan->_party._scents[AL0708_i_ScentIndex].setMapX(destMapX);
				_vm->_championMan->_party._scents[AL0708_i_ScentIndex].setMapY(destMapY);
				_vm->_championMan->_party._scents[AL0708_i_ScentIndex].setMapIndex(L0715_ui_MapIndexDestination);
				_vm->_championMan->_party._scentStrengths[AL0708_i_ScentIndex] = 0;
				_vm->_championMan->addScentStrength(destMapX, destMapY, k0x8000_mergeCycles | 24);
				AL0708_i_DestinationSquare = AL0727_ui_Backup;
			}
			if (L0715_ui_MapIndexDestination != L0714_ui_MapIndexSource) {
				_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
			}
		}
	}
	if (mapX >= 0) {
		if (thing == Thing::_party) {
			processThingAdditionOrRemoval(mapX, mapY, Thing::_party, L0725_B_PartySquare, false);
		} else {
			if (L0713_B_ThingLevitates) {
				_vm->_dungeonMan->unlinkThingFromList(thing, Thing(0), mapX, mapY);
			} else {
				processThingAdditionOrRemoval(mapX, mapY, thing, (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (mapX == _vm->_dungeonMan->_partyMapX) && (mapY == _vm->_dungeonMan->_partyMapY), false);
			}
		}
	}
	if (destMapX >= 0) {
		if (thing == Thing::_party) {
			_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
			if ((thing = _vm->_groupMan->groupGetThing(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY)) != Thing::_endOfList) { /* Delete group if party moves onto its square */
				_vm->_groupMan->dropGroupPossessions(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, thing, k1_soundModePlayIfPrioritized);
				_vm->_groupMan->groupDelete(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY);
			}
			if (L0715_ui_MapIndexDestination == L0714_ui_MapIndexSource) {
				processThingAdditionOrRemoval(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, Thing::_party, L0725_B_PartySquare, true);
			} else {
				_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
				_vm->_newPartyMapIndex = L0715_ui_MapIndexDestination;
			}
		} else {
			if (L0710_i_ThingType == k4_GroupThingType) {
				_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
				L0712_ps_Teleporter = (Teleporter *)_vm->_dungeonMan->getThingData(thing);
				AL0708_i_ActiveGroupIndex = ((Group *)L0712_ps_Teleporter)->getActiveGroupIndex();
				if (((L0715_ui_MapIndexDestination == _vm->_dungeonMan->_partyMapIndex) && (destMapX == _vm->_dungeonMan->_partyMapX) && (destMapY == _vm->_dungeonMan->_partyMapY)) || (_vm->_groupMan->groupGetThing(destMapX, destMapY) != Thing::_endOfList)) { /* If a group tries to move to the party square or over another group then create an event to move the group later */
					_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
					if (mapX >= 0) {
						_vm->_groupMan->groupDeleteEvents(mapX, mapY);
					}
					if (L0721_B_GroupOnPartyMap) {
						_vm->_groupMan->removeActiveGroup(AL0708_i_ActiveGroupIndex);
					}
					createEventMoveGroup(thing, destMapX, destMapY, L0715_ui_MapIndexDestination, L0726_B_Audible);
					return true; /* The specified group thing cannot be moved because the party or another group is on the destination square */
				}
				L1638_ui_MovementSoundIndex = getSound(((Group *)_vm->_dungeonMan->_thingData[k4_GroupThingType])[thing.getIndex()]._type);
				if (L1638_ui_MovementSoundIndex < k34_D13_soundCount) {
					_vm->_sound->requestPlay(L1638_ui_MovementSoundIndex, destMapX, destMapY, k1_soundModePlayIfPrioritized);
				}
				if (L0721_B_GroupOnPartyMap && (L0715_ui_MapIndexDestination != _vm->_dungeonMan->_partyMapIndex)) { /* If the group leaves the party map */
					_vm->_groupMan->removeActiveGroup(AL0708_i_ActiveGroupIndex);
					L0720_ui_MoveGroupResult = true;
				} else {
					if ((L0715_ui_MapIndexDestination == _vm->_dungeonMan->_partyMapIndex) && (!L0721_B_GroupOnPartyMap)) { /* If the group arrives on the party map */
						_vm->_groupMan->addActiveGroup(thing, destMapX, destMapY);
						L0720_ui_MoveGroupResult = true;
					}
				}
				if (L0713_B_ThingLevitates) {
					_vm->_dungeonMan->linkThingToList(thing, Thing(0), destMapX, destMapY);
				} else {
					processThingAdditionOrRemoval(destMapX, destMapY, thing, false, true);
				}
				if (L0720_ui_MoveGroupResult || (mapX < 0)) { /* If group moved from one map to another or if it was just placed on a square */
					_vm->_groupMan->startWanedring(destMapX, destMapY);
				}
				_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
				if (mapX >= 0) {
					if (L0720_ui_MoveGroupResult > 1) { /* If the group behavior was C6_BEHAVIOR_ATTACK before being teleported from and to the party map */
						_vm->_groupMan->stopAttacking(&_vm->_groupMan->_activeGroups[L0720_ui_MoveGroupResult - 2], mapX, mapY);
					} else {
						if (L0720_ui_MoveGroupResult) { /* If the group was teleported or leaved the party map or entered the party map */
							_vm->_groupMan->groupDeleteEvents(mapX, mapY);
						}
					}
				}
				return L0720_ui_MoveGroupResult;
			}
			_vm->_dungeonMan->setCurrentMap(L0715_ui_MapIndexDestination);
			if (L0710_i_ThingType == k14_ProjectileThingType) { /* BUG0_29 An explosion can trigger a floor sensor. Explosions do not trigger floor sensors on the square where they are created. However, if an explosion is moved by a teleporter (or by falling into a pit, see BUG0_26) after it was created, it can trigger floor sensors on the destination square. This is because explosions are not considered as levitating in the code, while projectiles are. The condition here should be (L0713_B_ThingLevitates) so that explosions would not start sensor processing on their destination square as they should be Levitating. This would work if F0264_MOVE_IsLevitating returned true for explosions (see BUG0_26) */
				_vm->_dungeonMan->linkThingToList(thing, Thing(0), destMapX, destMapY);
			} else {
				processThingAdditionOrRemoval(destMapX, destMapY, thing, (_vm->_dungeonMan->_currMapIndex == _vm->_dungeonMan->_partyMapIndex) && (destMapX == _vm->_dungeonMan->_partyMapX) && (destMapY == _vm->_dungeonMan->_partyMapY), true);
			}
			_vm->_dungeonMan->setCurrentMap(L0714_ui_MapIndexSource);
		}
	}
	return false;
}

bool MovesensMan::isLevitating(Thing thing) {
	int16 L0695_i_ThingType;


	if ((L0695_i_ThingType = thing.getType()) == k4_GroupThingType) {
		return getFlag(_vm->_dungeonMan->getCreatureAttributes(thing), k0x0020_MaskCreatureInfo_levitation);
	}
	if (L0695_i_ThingType == k14_ProjectileThingType) { /* BUG0_26 An explosion may fall in a pit. If a pit is opened while there is an explosion above then the explosion falls into the pit in F0267_MOVE_GetMoveResult_CPSCE. Explosions are not considered as levitating so they are moved when the pit is opened. This function should return true for explosions */
		return true;
	}
	return false;
}

bool MovesensMan::moveIsKilledByProjectileImpact(int16 srcMapX, int16 srcMapY, int16 destMapX, int16 destMapY, Thing thing) {
	Thing L0697_T_Thing;
	uint16 L0699_ui_Multiple;
#define AL0699_ui_Cell                      L0699_ui_Multiple
#define AL0699_ui_PrimaryDirection          L0699_ui_Multiple
#define AL0699_ui_ChampionOrCreatureOrdinal L0699_ui_Multiple
	int16 L0700_i_Multiple;
#define AL0700_B_CreatureAlive      L0700_i_Multiple
#define AL0700_i_Distance           L0700_i_Multiple
#define AL0700_i_SecondaryDirection L0700_i_Multiple
	Group* L0701_ps_Group;
	int16 L0702_i_ImpactType;
	bool L0703_B_CheckDestinationSquareProjectileImpacts;
	uint16 L0704_ui_ProjectileMapX;
	uint16 L0705_ui_ProjectileMapY;
	byte L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[4]; /* This array is used only when moving between two adjacent squares and is used to test projectile impacts when the party or group is in the 'intermediary' step between the two squares. Without this test, in the example below no impact would be detected. In this example, the party moves from the source square on the left (which contains a single champion at cell 2) to the destination square on the right (which contains a single projectile at cell 3).
																			Party:      Projectiles on target square:   Incorrect result without the test for the intermediary step (the champion would have passed through the projectile without impact):
																			00    ->    00                         00
																			01          P0                         P1 */
	byte L0707_auc_ChampionOrCreatureOrdinalInCell[4]; /* This array has an entry for each cell on the source square, containing the ordinal of the champion or creature (0 if there is no champion or creature at this cell) */


	L0703_B_CheckDestinationSquareProjectileImpacts = false;
	for (int16 i = 0; i < 4; ++i)
		L0707_auc_ChampionOrCreatureOrdinalInCell[i] = 0;
	if (thing == Thing::_party) {
		L0702_i_ImpactType = kM2_ChampionElemType;
		for (AL0699_ui_Cell = k0_CellNorthWest; AL0699_ui_Cell < k3_CellSouthWest + 1; AL0699_ui_Cell++) {
			if (_vm->_championMan->getIndexInCell((ViewCell)AL0699_ui_Cell) >= 0) {
				L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_Cell] = _vm->indexToOrdinal(AL0699_ui_Cell);
			}
		}
	} else {
		L0702_i_ImpactType = kM1_CreatureElemType;
		L0701_ps_Group = (Group *)_vm->_dungeonMan->getThingData(thing);
		for (AL0699_ui_Cell = k0_CellNorthWest, AL0700_B_CreatureAlive = false; AL0699_ui_Cell < k3_CellSouthWest + 1; AL0699_ui_Cell++) {
			AL0700_B_CreatureAlive |= L0701_ps_Group->_health[AL0699_ui_Cell];
			if (_vm->_groupMan->getCreatureOrdinalInCell(L0701_ps_Group, AL0699_ui_Cell)) {
				L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_Cell] = _vm->indexToOrdinal(AL0699_ui_Cell);
			}
		}
		if (!AL0700_B_CreatureAlive) {
			return false;
		}
	}
	if ((destMapX >= 0) && ((abs(srcMapX - destMapX) + abs(srcMapY - destMapY)) == 1)) { /* If source and destination squares are adjacent (if party or group is not being teleported) */
		AL0699_ui_PrimaryDirection = _vm->_groupMan->getDirsWhereDestIsVisibleFromSource(srcMapX, srcMapY, destMapX, destMapY);
		AL0700_i_SecondaryDirection = returnNextVal(AL0699_ui_PrimaryDirection);
		for (int16 i = 0; i < 4; ++i)
			L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[i] = 0;

		L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[returnPrevVal(AL0699_ui_PrimaryDirection)] = L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection];
		if (L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[returnPrevVal(AL0699_ui_PrimaryDirection)]) {
			L0703_B_CheckDestinationSquareProjectileImpacts = true;
		}

		L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[returnNextVal(AL0700_i_SecondaryDirection)] = L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection];
		if (L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[returnNextVal(AL0700_i_SecondaryDirection)]) {
			L0703_B_CheckDestinationSquareProjectileImpacts = true;
		}
		if (!L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection]) {
			L0707_auc_ChampionOrCreatureOrdinalInCell[AL0699_ui_PrimaryDirection] = L0707_auc_ChampionOrCreatureOrdinalInCell[returnPrevVal(AL0699_ui_PrimaryDirection)];
		}
		if (!L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection]) {
			L0707_auc_ChampionOrCreatureOrdinalInCell[AL0700_i_SecondaryDirection] = L0707_auc_ChampionOrCreatureOrdinalInCell[returnNextVal(AL0700_i_SecondaryDirection)];
		}
	}
	L0704_ui_ProjectileMapX = srcMapX; /* Check impacts with projectiles on the source square */
	L0705_ui_ProjectileMapY = srcMapY;
T0266017_CheckProjectileImpacts:
	L0697_T_Thing = _vm->_dungeonMan->getSquareFirstThing(L0704_ui_ProjectileMapX, L0705_ui_ProjectileMapY);
	while (L0697_T_Thing != Thing::_endOfList) {
		if (((L0697_T_Thing).getType() == k14_ProjectileThingType) &&
			(_vm->_timeline->_events[(((Projectile *)_vm->_dungeonMan->_thingData[k14_ProjectileThingType])[(L0697_T_Thing).getIndex()])._eventIndex]._type != k48_TMEventTypeMoveProjectileIgnoreImpacts) && (AL0699_ui_ChampionOrCreatureOrdinal = L0707_auc_ChampionOrCreatureOrdinalInCell[(L0697_T_Thing).getCell()]) &&
			_vm->_projexpl->hasProjectileImpactOccurred(L0702_i_ImpactType, srcMapX, srcMapY, _vm->ordinalToIndex(AL0699_ui_ChampionOrCreatureOrdinal), L0697_T_Thing)) {
			_vm->_projexpl->projectileDeleteEvent(L0697_T_Thing);
			if (_vm->_projexpl->_creatureDamageOutcome == k2_outcomeKilledAllCreaturesInGroup) {
				return true;
			}
			goto T0266017_CheckProjectileImpacts;
		}
		L0697_T_Thing = _vm->_dungeonMan->getNextThing(L0697_T_Thing);
	}
	if (L0703_B_CheckDestinationSquareProjectileImpacts) {
		srcMapX |= ((L0704_ui_ProjectileMapX = destMapX) + 1) << 8; /* Check impacts with projectiles on the destination square */
		srcMapY |= (L0705_ui_ProjectileMapY = destMapY) << 8;
		for (uint16 i = 0; i < 4; ++i)
			L0707_auc_ChampionOrCreatureOrdinalInCell[i] = L0706_auc_IntermediaryChampionOrCreatureOrdinalInCell[i];
		L0703_B_CheckDestinationSquareProjectileImpacts = false;
		goto T0266017_CheckProjectileImpacts;
	}
	return false;
}

void MovesensMan::addEvent(byte type, byte mapX, byte mapY, byte cell, byte effect, int32 time) {
	TimelineEvent L0729_s_Event;

	setMapAndTime(L0729_s_Event._mapTime, _vm->_dungeonMan->_currMapIndex, time);
	L0729_s_Event._type = type;
	L0729_s_Event._priority = 0;
	L0729_s_Event._B._location._mapX = mapX;
	L0729_s_Event._B._location._mapY = mapY;
	L0729_s_Event._C.A._cell = cell;
	L0729_s_Event._C.A._effect = effect;
	_vm->_timeline->addEventGetEventIndex(&L0729_s_Event);
}

int16 MovesensMan::getSound(byte creatureType) {
	if (_vm->_championMan->_partyIsSleeping) {
		return 35;
	}

	switch (creatureType) {
	case k3_CreatureTypeWizardEyeFlyingEye:
	case k8_CreatureTypeGhostRive:
	case k11_CreatureTypeBlackFlame:
	case k19_CreatureTypeMaterializerZytaz:
	case k23_CreatureTypeLordChaos:
	case k25_CreatureTypeLordOrder:
	case k26_CreatureTypeGreyLord:
		return 35;
	case k2_CreatureTypeGiggler:
	case k9_CreatureTypeStoneGolem:
	case k10_CreatureTypeMummy:
	case k14_CreatureTypeVexirk:
	case k16_CreatureTypeTrolinAntman:
	case k22_CreatureTypeDemon:
		return k24_soundMOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON;
	case k0_CreatureTypeGiantScorpionScorpion:
	case k4_CreatureTypePainRatHellHound:
	case k5_CreatureTypeRuster:
	case k6_CreatureTypeScreamer:
	case k7_CreatureTypeRockpile:
	case k15_CreatureTypeMagnetaWormWorm:
	case k21_CreatureTypeOitu:
		return k26_soundMOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU;
	case k24_CreatureTypeRedDragon:
		return k32_soundMOVE_RED_DRAGON;
	case k12_CreatureTypeSkeleton:
		return k33_soundMOVE_SKELETON;
	case k18_CreatureTypeAnimatedArmourDethKnight:
		return k22_soundMOVE_ANIMATED_ARMOUR_DETH_KNIGHT;
	case k1_CreatureTypeSwampSlimeSlime:
	case k20_CreatureTypeWaterElemental:
		return k27_soundMOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL;
	case k13_CreatureTypeCouatl:
	case k17_CreatureTypeGiantWaspMuncher:
		return k23_soundMOVE_COUATL_GIANT_WASP_MUNCHER;
	}
	return -1000; // if this is returned, it's an error, this should break it good
}

int16 MovesensMan::getTeleporterRotatedGroupResult(Teleporter* teleporter, Thing thing, uint16 mapIndex) {
	int16 L0683_i_Rotation;
	uint16 L0684_ui_GroupDirections;
	uint16 L0685_ui_UpdatedGroupDirections;
	Group* L0686_ps_Group;
	uint16 L0687_ui_UpdatedGroupCells;
	int16 L0688_i_CreatureIndex;
	bool L0689_B_AbsoluteRotation;
	uint16 L0690_ui_GroupCells;
	int16 L0691_i_CreatureSize;
	int16 L0692_i_RelativeRotation;

	L0686_ps_Group = (Group *)_vm->_dungeonMan->getThingData(thing);
	L0683_i_Rotation = teleporter->getRotation();
	L0684_ui_GroupDirections = _vm->_groupMan->getGroupDirections(L0686_ps_Group, mapIndex);

	L0689_B_AbsoluteRotation = teleporter->getAbsoluteRotation();
	if (L0689_B_AbsoluteRotation) {
		L0685_ui_UpdatedGroupDirections = L0683_i_Rotation;
	} else {
		L0685_ui_UpdatedGroupDirections = normalizeModulo4(L0684_ui_GroupDirections + L0683_i_Rotation);
	}

	L0687_ui_UpdatedGroupCells = _vm->_groupMan->getGroupCells(L0686_ps_Group, mapIndex);
	if (L0687_ui_UpdatedGroupCells != k255_CreatureTypeSingleCenteredCreature) {
		L0690_ui_GroupCells = L0687_ui_UpdatedGroupCells;
		L0691_i_CreatureSize = getFlag(g243_CreatureInfo[L0686_ps_Group->_type]._attributes, k0x0003_MaskCreatureInfo_size);
		L0692_i_RelativeRotation = normalizeModulo4(4 + L0685_ui_UpdatedGroupDirections - L0684_ui_GroupDirections);
		for (L0688_i_CreatureIndex = 0; L0688_i_CreatureIndex <= L0686_ps_Group->getCount(); L0688_i_CreatureIndex++) {
			L0685_ui_UpdatedGroupDirections = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(L0685_ui_UpdatedGroupDirections, L0688_i_CreatureIndex, L0689_B_AbsoluteRotation ? L0683_i_Rotation : normalizeModulo4(L0684_ui_GroupDirections + L0683_i_Rotation));
			if (L0691_i_CreatureSize == k0_MaskCreatureSizeQuarter) {
				L0692_i_RelativeRotation = !L0689_B_AbsoluteRotation;
				if (L0692_i_RelativeRotation) 
					L0692_i_RelativeRotation = L0683_i_Rotation;
			}
			if (L0692_i_RelativeRotation) {
				L0687_ui_UpdatedGroupCells = _vm->_groupMan->getGroupValueUpdatedWithCreatureValue(L0687_ui_UpdatedGroupCells, L0688_i_CreatureIndex, normalizeModulo4(L0690_ui_GroupCells + L0692_i_RelativeRotation));
			}
			L0684_ui_GroupDirections >>= 2;
			L0690_ui_GroupCells >>= 2;
		}
	}
	_vm->_dungeonMan->setGroupDirections(L0686_ps_Group, L0685_ui_UpdatedGroupDirections, mapIndex);
	_vm->_dungeonMan->setGroupCells(L0686_ps_Group, L0687_ui_UpdatedGroupCells, mapIndex);
	if ((mapIndex == _vm->_dungeonMan->_partyMapIndex) && (L0686_ps_Group->setBehaviour(k6_behavior_ATTACK))) {
		return L0686_ps_Group->getActiveGroupIndex() + 2;
	}
	return 1;
}

Thing MovesensMan::getTeleporterRotatedProjectileThing(Teleporter* teleporter, Thing projectileThing) {
	int16 L0693_i_UpdatedDirection;
	int16 L0694_i_Rotation;

	L0693_i_UpdatedDirection = _moveResultDir;
	L0694_i_Rotation = teleporter->getRotation();
	if (teleporter->getAbsoluteRotation()) {
		L0693_i_UpdatedDirection = L0694_i_Rotation;
	} else {
		L0693_i_UpdatedDirection = normalizeModulo4(L0693_i_UpdatedDirection + L0694_i_Rotation);
		projectileThing = thingWithNewCell(projectileThing, normalizeModulo4((projectileThing).getCell() + L0694_i_Rotation));
	}
	_moveResultDir = L0693_i_UpdatedDirection;
	return projectileThing;
}

void MovesensMan::processThingAdditionOrRemoval(uint16 mapX, uint16 mapY, Thing thing, bool partySquare, bool addThing) {
	Thing L0766_T_Thing;
	int16 L0767_i_ThingType;
	bool L0768_B_TriggerSensor;
	Sensor* L0769_ps_Sensor;
	int16 L0770_ui_SensorTriggeredCell;
	uint16 L0771_ui_ThingType;
	bool L0772_B_SquareContainsObject;
	bool L0773_B_SquareContainsGroup;
	int16 L0774_i_ObjectType;
	bool L0775_B_SquareContainsThingOfSameType;
	bool L0776_B_SquareContainsThingOfDifferentType;
	uint16 L0777_ui_Square;
	int16 L0778_i_Effect;
	int16 L0779_i_SensorData;


	if (thing != Thing::_party) {
		L0767_i_ThingType = thing.getType();
		L0774_i_ObjectType = _vm->_objectMan->getObjectType(thing);
	} else {
		L0767_i_ThingType = kM1_PartyThingType;
		L0774_i_ObjectType = kM1_IconIndiceNone;
	}
	if ((!addThing) && (L0767_i_ThingType != kM1_PartyThingType)) {
		_vm->_dungeonMan->unlinkThingFromList(thing, Thing(0), mapX, mapY);
	}

	L0777_ui_Square = _vm->_dungeonMan->_currMapData[mapX][mapY];
	if (Square(L0777_ui_Square).getType() == k0_WallElemType) {
		L0770_ui_SensorTriggeredCell = thing.getCell();
	} else {
		L0770_ui_SensorTriggeredCell = kM1_CellAny; // this will wrap around
	}
	L0772_B_SquareContainsObject = L0773_B_SquareContainsGroup = L0775_B_SquareContainsThingOfSameType = L0776_B_SquareContainsThingOfDifferentType = false;
	L0766_T_Thing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	if (L0770_ui_SensorTriggeredCell == kM1_CellAny) {
		while (L0766_T_Thing != Thing::_endOfList) {
			if ((L0771_ui_ThingType = (L0766_T_Thing).getType()) == k4_GroupThingType) {
				L0773_B_SquareContainsGroup = true;
			} else {
				if ((L0771_ui_ThingType == k2_TextstringType) && (L0767_i_ThingType == kM1_PartyThingType) && addThing && !partySquare) {
					_vm->_dungeonMan->decodeText(_vm->_stringBuildBuffer, L0766_T_Thing, k1_TextTypeMessage);
					_vm->_textMan->printMessage(k15_ColorWhite, _vm->_stringBuildBuffer);
				} else {
					if ((L0771_ui_ThingType > k4_GroupThingType) && (L0771_ui_ThingType < k14_ProjectileThingType)) {
						L0772_B_SquareContainsObject = true;
						L0775_B_SquareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(L0766_T_Thing) == L0774_i_ObjectType);
						L0776_B_SquareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(L0766_T_Thing) != L0774_i_ObjectType);
					}
				}
			}
			L0766_T_Thing = _vm->_dungeonMan->getNextThing(L0766_T_Thing);
		}
	} else {
		while (L0766_T_Thing != Thing::_endOfList) {
			if ((L0770_ui_SensorTriggeredCell == (L0766_T_Thing).getCell()) && ((L0766_T_Thing).getType() > k4_GroupThingType)) {
				L0772_B_SquareContainsObject = true;
				L0775_B_SquareContainsThingOfSameType |= (_vm->_objectMan->getObjectType(L0766_T_Thing) == L0774_i_ObjectType);
				L0776_B_SquareContainsThingOfDifferentType |= (_vm->_objectMan->getObjectType(L0766_T_Thing) != L0774_i_ObjectType);
			}
			L0766_T_Thing = _vm->_dungeonMan->getNextThing(L0766_T_Thing);
		}
	}
	if (addThing && (L0767_i_ThingType != kM1_PartyThingType)) {
		_vm->_dungeonMan->linkThingToList(thing, Thing(0), mapX, mapY);
	}
	L0766_T_Thing = _vm->_dungeonMan->getSquareFirstThing(mapX, mapY);
	while (L0766_T_Thing != Thing::_endOfList) {
		L0771_ui_ThingType = (L0766_T_Thing).getType();
		if (L0771_ui_ThingType == k3_SensorThingType) {
			L0769_ps_Sensor = (Sensor *)_vm->_dungeonMan->getThingData(L0766_T_Thing);
			if ((L0769_ps_Sensor)->getType() == k0_SensorDisabled)
				goto T0276079;
			L0779_i_SensorData = L0769_ps_Sensor->getData();
			L0768_B_TriggerSensor = addThing;
			if (L0770_ui_SensorTriggeredCell == kM1_CellAny) {
				switch (L0769_ps_Sensor->getType()) {
				case k1_SensorFloorTheronPartyCreatureObj:
					if (partySquare || L0772_B_SquareContainsObject || L0773_B_SquareContainsGroup) /* BUG0_30 A floor sensor is not triggered when you put an object on the floor if a levitating creature is present on the same square. The condition to determine if the sensor should be triggered checks if there is a creature on the square but does not check whether the creature is levitating. While it is normal not to trigger the sensor if there is a non levitating creature on the square (because it was already triggered by the creature itself), a levitating creature should not prevent triggering the sensor with an object. */
						goto T0276079;
					break;
				case k2_SensorFloorTheronPartyCreature:
					if ((L0767_i_ThingType > k4_GroupThingType) || partySquare || L0773_B_SquareContainsGroup)
						goto T0276079;
					break;
				case k3_SensorFloorParty:
					if ((L0767_i_ThingType != kM1_PartyThingType) || (_vm->_championMan->_partyChampionCount == 0))
						goto T0276079;
					if (L0779_i_SensorData == 0) {
						if (partySquare)
							goto T0276079;
					} else {
						if (!addThing) {
							L0768_B_TriggerSensor = false;
						} else {
							L0768_B_TriggerSensor = (L0779_i_SensorData == _vm->indexToOrdinal(_vm->_dungeonMan->_partyDir));
						}
					}
					break;
				case k4_SensorFloorObj:
					if ((L0779_i_SensorData != _vm->_objectMan->getObjectType(thing)) || L0775_B_SquareContainsThingOfSameType)
						goto T0276079;
					break;
				case k5_SensorFloorPartyOnStairs:
					if ((L0767_i_ThingType != kM1_PartyThingType) || (Square(L0777_ui_Square).getType() != k3_StairsElemType))
						goto T0276079;
					break;
				case k6_SensorFloorGroupGenerator:
					goto T0276079;
				case k7_SensorFloorCreature:
					if ((L0767_i_ThingType > k4_GroupThingType) || (L0767_i_ThingType == kM1_PartyThingType) || L0773_B_SquareContainsGroup)
						goto T0276079;
					break;
				case k8_SensorFloorPartyPossession:
					if (L0767_i_ThingType != kM1_PartyThingType)
						goto T0276079;
					L0768_B_TriggerSensor = isObjcetInPartyPossession(L0779_i_SensorData);
					break;
				case k9_SensorFloorVersionChecker:
					if ((L0767_i_ThingType != kM1_PartyThingType) || !addThing || partySquare)
						goto T0276079;
					// Strangerke: 20 is a hardcoded version of the game. later version uses 21. Not present in the original dungeons anyway.
					L0768_B_TriggerSensor = (L0779_i_SensorData <= 20);
					break;
				default:
					goto T0276079;
				}
			} else {
				if (L0770_ui_SensorTriggeredCell != (L0766_T_Thing).getCell())
					goto T0276079;
				switch (L0769_ps_Sensor->getType()) {
				case k1_SensorWallOrnClick:
					if (L0772_B_SquareContainsObject)
						goto T0276079;
					break;
				case k2_SensorWallOrnClickWithAnyObj:
					if (L0775_B_SquareContainsThingOfSameType || (L0769_ps_Sensor->getData() != _vm->_objectMan->getObjectType(thing)))
						goto T0276079;
					break;
				case k3_SensorWallOrnClickWithSpecObj:
					if (L0776_B_SquareContainsThingOfDifferentType || (L0769_ps_Sensor->getData() == _vm->_objectMan->getObjectType(thing)))
						goto T0276079;
					break;
				default:
					goto T0276079;
				}
			}
			L0768_B_TriggerSensor ^= L0769_ps_Sensor->getAttrRevertEffectA();
			L0778_i_Effect = L0769_ps_Sensor->getAttrEffectA();
			if (L0778_i_Effect == k3_SensorEffHold) {
				L0778_i_Effect = L0768_B_TriggerSensor ? k0_SensorEffSet : k1_SensorEffClear;
			} else if (!L0768_B_TriggerSensor)
				goto T0276079;

			if (L0769_ps_Sensor->getAttrAudibleA())
				_vm->_sound->requestPlay(k01_soundSWITCH, mapX, mapY, k1_soundModePlayIfPrioritized);

			triggerEffect(L0769_ps_Sensor, L0778_i_Effect, mapX, mapY, (uint16)kM1_CellAny); // this will wrap around
			goto T0276079;
		}
		if (L0771_ui_ThingType >= k4_GroupThingType)
			break;
T0276079:
		L0766_T_Thing = _vm->_dungeonMan->getNextThing(L0766_T_Thing);
	}
	processRotationEffect();
}

bool MovesensMan::isObjcetInPartyPossession(int16 objectType) {
	int16 L0742_i_ChampionIndex;
	uint16 L0743_ui_SlotIndex = 0;
	Thing L0744_T_Thing = Thing::_none;
	Champion* L0745_ps_Champion;
	Thing* L0746_pT_Thing = nullptr;
	int16 L0747_i_ObjectType;
	bool L0748_B_LeaderHandObjectProcessed;
	Container* L0749_ps_Container;


	L0748_B_LeaderHandObjectProcessed = false;
	for (L0742_i_ChampionIndex = k0_ChampionFirst, L0745_ps_Champion = _vm->_championMan->_champions; L0742_i_ChampionIndex < _vm->_championMan->_partyChampionCount; L0742_i_ChampionIndex++, L0745_ps_Champion++) {
		if (L0745_ps_Champion->_currHealth) {
			L0746_pT_Thing = L0745_ps_Champion->_slots;
			for (L0743_ui_SlotIndex = k0_ChampionSlotReadyHand; (L0743_ui_SlotIndex < k30_ChampionSlotChest_1) && !L0748_B_LeaderHandObjectProcessed; L0743_ui_SlotIndex++) {
				L0744_T_Thing = *L0746_pT_Thing++;
T0274003:
				L0747_i_ObjectType = _vm->_objectMan->getObjectType(L0744_T_Thing);
				if (L0747_i_ObjectType == objectType) {
					return true;
				}
				if (L0747_i_ObjectType == k144_IconIndiceContainerChestClosed) {
					L0749_ps_Container = (Container *)_vm->_dungeonMan->getThingData(L0744_T_Thing);
					L0744_T_Thing = L0749_ps_Container->getSlot();
					while (L0744_T_Thing != Thing::_endOfList) {
						if (_vm->_objectMan->getObjectType(L0744_T_Thing) == objectType) {
							return true;
						}
						L0744_T_Thing = _vm->_dungeonMan->getNextThing(L0744_T_Thing);
					}
				}
			}
		}
	}
	if (!L0748_B_LeaderHandObjectProcessed) {
		L0748_B_LeaderHandObjectProcessed = true;
		L0744_T_Thing = _vm->_championMan->_leaderHandObject;
		goto T0274003;
	}
	return false;
}

void MovesensMan::triggerEffect(Sensor* sensor, int16 effect, int16 mapX, int16 mapY, uint16 cell) {
	byte g59_squareTypeToEventType[7] = { // @ G0059_auc_Graphic562_SquareTypeToEventType
		k6_TMEventTypeWall,
		k5_TMEventTypeCorridor,
		k9_TMEventTypePit,
		k0_TMEventTypeNone,
		k10_TMEventTypeDoor,
		k8_TMEventTypeTeleporter,
		k7_TMEventTypeFakeWall}; /* 1 byte of padding inserted by compiler */

	int16 L0736_i_TargetMapX;
	int16 L0737_i_TargetMapY;
	int32 L0738_l_Time;
	uint16 L0739_ui_SquareType;
	uint16 L0740_ui_TargetCell;


	if (sensor->getAttrOnlyOnce()) {
		sensor->setTypeDisabled();
	}
	L0738_l_Time = _vm->_gameTime + sensor->getAttrValue();
	if (sensor->getAttrLocalEffect()) {
		triggerLocalEffect(sensor->getActionLocalEffect(), mapX, mapY, cell);
	} else {
		L0736_i_TargetMapX = sensor->getActionTargetMapX();
		L0737_i_TargetMapY = sensor->getActionTargetMapY();
		L0739_ui_SquareType = Square(_vm->_dungeonMan->_currMapData[L0736_i_TargetMapX][L0737_i_TargetMapY]).getType();
		if (L0739_ui_SquareType == k0_ElementTypeWall) {
			L0740_ui_TargetCell = sensor->getActionTargetCell();
		} else {
			L0740_ui_TargetCell = k0_CellNorthWest;
		}
		addEvent(g59_squareTypeToEventType[L0739_ui_SquareType], L0736_i_TargetMapX, L0737_i_TargetMapY, L0740_ui_TargetCell, effect, L0738_l_Time);
	}
}

void MovesensMan::triggerLocalEffect(int16 localEffect, int16 effX, int16 effY, int16 effCell) {
	if (localEffect == k10_SensorEffAddExp) {
		addSkillExperience(k8_ChampionSkillSteal, 300, localEffect != kM1_CellAny);
		return;
	}
	_sensorRotationEffect = localEffect;
	_sensorRotationEffMapX = effX;
	_sensorRotationEffMapY = effY;
	_sensorRotationEffCell = effCell;
}

void MovesensMan::addSkillExperience(int16 skillIndex, uint16 exp, bool leaderOnly) {

	if (leaderOnly) {
		if (_vm->_championMan->_leaderIndex != kM1_ChampionNone) {
			_vm->_championMan->addSkillExperience(_vm->_championMan->_leaderIndex, skillIndex, exp);
		}
	} else {
		exp /= _vm->_championMan->_partyChampionCount;
		Champion *L0731_ps_Champion = _vm->_championMan->_champions;
		for (int16 L0730_i_ChampionIndex = k0_ChampionFirst; L0730_i_ChampionIndex < _vm->_championMan->_partyChampionCount; L0730_i_ChampionIndex++, L0731_ps_Champion++) {
			if (L0731_ps_Champion->_currHealth) {
				_vm->_championMan->addSkillExperience(L0730_i_ChampionIndex, skillIndex, exp);
			}
		}
	}
}

void MovesensMan::processRotationEffect() {
	Thing L0732_T_FirstSensorThing;
	Thing L0733_T_LastSensorThing;
	Sensor* L0734_ps_FirstSensor;
	Sensor* L0735_ps_LastSensor;


	if (_sensorRotationEffect == kM1_SensorEffNone) {
		return;
	}
	switch (_sensorRotationEffect) {
	case k1_SensorEffClear:
	case k2_SensorEffToggle:
		L0732_T_FirstSensorThing = _vm->_dungeonMan->getSquareFirstThing(_sensorRotationEffMapX, _sensorRotationEffMapY);
		while (((L0732_T_FirstSensorThing).getType() != k3_SensorThingType) || ((_sensorRotationEffCell != kM1_CellAny) && ((L0732_T_FirstSensorThing).getCell() != _sensorRotationEffCell))) {
			L0732_T_FirstSensorThing = _vm->_dungeonMan->getNextThing(L0732_T_FirstSensorThing);
		}
		L0734_ps_FirstSensor = (Sensor *)_vm->_dungeonMan->getThingData(L0732_T_FirstSensorThing);
		L0733_T_LastSensorThing = L0734_ps_FirstSensor->getNextThing();
		while ((L0733_T_LastSensorThing != Thing::_endOfList) && (((L0733_T_LastSensorThing).getType() != k3_SensorThingType) || ((_sensorRotationEffCell != kM1_CellAny) && ((L0733_T_LastSensorThing).getCell() != _sensorRotationEffCell)))) {
			L0733_T_LastSensorThing = _vm->_dungeonMan->getNextThing(L0733_T_LastSensorThing);
		}
		if (L0733_T_LastSensorThing == Thing::_endOfList)
			break;
		_vm->_dungeonMan->unlinkThingFromList(L0732_T_FirstSensorThing, Thing(0), _sensorRotationEffMapX, _sensorRotationEffMapY);
		L0735_ps_LastSensor = (Sensor *)_vm->_dungeonMan->getThingData(L0733_T_LastSensorThing);
		L0733_T_LastSensorThing = _vm->_dungeonMan->getNextThing(L0733_T_LastSensorThing);
		while (((L0733_T_LastSensorThing != Thing::_endOfList) && ((L0733_T_LastSensorThing).getType() == k3_SensorThingType))) {
			if ((_sensorRotationEffCell == kM1_CellAny) || ((L0733_T_LastSensorThing).getCell() == _sensorRotationEffCell)) {
				L0735_ps_LastSensor = (Sensor *)_vm->_dungeonMan->getThingData(L0733_T_LastSensorThing);
			}
			L0733_T_LastSensorThing = _vm->_dungeonMan->getNextThing(L0733_T_LastSensorThing);
		}
		L0734_ps_FirstSensor->setNextThing(L0735_ps_LastSensor->getNextThing());
		L0735_ps_LastSensor->setNextThing(L0732_T_FirstSensorThing);
	}
	_sensorRotationEffect = kM1_SensorEffNone;
}

void MovesensMan::createEventMoveGroup(Thing groupThing, int16 mapX, int16 mapY, int16 mapIndex, bool audible) {
	TimelineEvent L0696_s_Event;

	setMapAndTime(L0696_s_Event._mapTime, mapIndex, _vm->_gameTime + 5);
	L0696_s_Event._type = audible ? k61_TMEventTypeMoveGroupAudible : k60_TMEventTypeMoveGroupSilent;
	L0696_s_Event._priority = 0;
	L0696_s_Event._B._location._mapX = mapX;
	L0696_s_Event._B._location._mapY = mapY;
	L0696_s_Event._C._slot = groupThing.toUint16();
	_vm->_timeline->addEventGetEventIndex(&L0696_s_Event);
}

Thing MovesensMan::getObjectOfTypeInCell(int16 mapX, int16 mapY, int16 cell, int16 objectType) {
	Thing L0741_T_Thing;


	L0741_T_Thing = _vm->_dungeonMan->getSquareFirstObject(mapX, mapY);
	while (L0741_T_Thing != Thing::_endOfList) {
		if (_vm->_objectMan->getObjectType(L0741_T_Thing) == objectType) {
			if ((cell == kM1_CellAny) || ((L0741_T_Thing.getCell()) == cell)) {
				return L0741_T_Thing;
			}
		}
		L0741_T_Thing = _vm->_dungeonMan->getNextThing(L0741_T_Thing);
	}
	return Thing::_none;
}
}
