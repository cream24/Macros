|----------------------------------------------------------------------------|
|- e3_Macro_buffCheck.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ																								-|
|----------------------------------------------------------------------------|
|----------------------------------------------------------------------------|
|- Contains buffCheck functions for Spells, AAs, Items, and Discs.					-|
|----------------------------------------------------------------------------|

|--------------------------------------------------------------------------------|
|- Calls buff functions, returning to the mainloop whenever a spell is cast.		-|
|--------------------------------------------------------------------------------|
SUB check_Buffs
	/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffCheck ==>
	/if (${BuffCheck} ) {
		/if (!${ActionTaken} && !${activeTimer} && ${BuffCombatCheck} && !${idle} && (!${medBreak} || ${Me.Combat}))	/call buffBots "CombatBuffs2D"
		/if (!${ActionTaken} && ${InstantBuffs2D.Size[1]})		/call buffBots "InstantBuffs2D"
		/if (!${ActionTaken} && !${combatTimer} && !${activeTimer} && ${Me.CombatState.NotEqual[COMBAT]} && !${Assisting} && !${Me.Moving} && ${BuffCheck}) {
			/if (!${ActionTaken} && ${SelfBuffs2D.Size[1]})	/call buffBots "SelfBuffs2D"
			/if (!${ActionTaken} && ${BotBuffs2D.Size[1]})	/call buffBots "BotBuffs2D"
		}
	}
	/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffCheck -|
/RETURN

|----------------------------------------|
|- Cast and maintain buffs on a NetBot.	-|
|- the space after ${buffSpellID} is important "Find[${buffSpellID} ]"
|----------------------------------------|
SUB buffBots(ArrayName)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffBots [${ArrayName}] ==>
	/declare s					int local
	/declare buffTargetID		int local
	/declare buffTarget			string local
	/declare buffSpellID		int local
	/declare buffCheckForID		int local	
	/declare buffSpellName		string local
	/declare buffCheckForName	string local
	/declare currentBuffs		string local
	/declare currentSongs		string local

 /for s 1 to ${${ArrayName}.Size[1]}
	/if (${Debug} || ${Debug_BuffCheck}) /echo buffing ${${ArrayName}[${s},${iCastName}]} on ${${ArrayName}[${s},${iCastTarget}]} checkFor ${${ArrayName}[${s},${iCheckFor}]}

	/if (${Select[${ArrayName},InstantBuffs2D,SelfBuffs2D]}) {
		/varset buffTargetID ${Me.ID}
		/varset buffTarget ${Me.Name}
	} else /if (${Select[${ArrayName},CombatBuffs2D,BotBuffs2D]}) {
		/varset buffTargetID ${Spawn[=${${ArrayName}[${s},${iCastTarget}]}].ID}
		/varset buffTarget ${${ArrayName}[${s},${iCastTarget}]}
	}
	/varset buffSpellName		${${ArrayName}[${s},${iSpellName}]}
	/varset buffSpellID			${${ArrayName}[${s},${iSpellID}]}]
	/varset buffCheckForName	${${ArrayName}[${s},${iCheckFor}]}
	/varset buffCheckForID		${${ArrayName}[${s},${iCheckForID}]}

  /if (!${Bool[${NetBots[${buffTarget}].ID}]}) /goto :skipBuff
	|/echo ${buffSpellName} ${buffSpellID} ${buffCheckForName} ${buffCheckForID} ${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}
  |/echo ${buffSpellName} ${Select["${buffSpellName}","Thief's Eyes", "Fists of Wu", "Commanding Voice","Cry Havoc"]}
  /if (${Select[${buffSpellID},8001,8002,8000,8003]}) {
    /call meleeCombatBuffs "${ArrayName}" ${s}
    /goto :skipBuff
  }
	/call check_Mana "${ArrayName}" ${s}
	/if (${Macro.Return}) {
		|------- Self ------|
		/if (${Select[${ArrayName},SelfBuffs2D,InstantBuffs2D]}) {
			/if (!${Bool[${Me.Buff[${buffSpellName}]}]} || (${Bool[${Me.Buff[${buffSpellName}]}]} && ${Me.Buff[${buffSpellName}].Duration.TotalSeconds} <= ${buffRecastTime})) {
			  |/echo ${ArrayName} ${s} b ${buffSpellName} cn ${buffCheckForName}  ${Bool[${Me.Song[${buffSpellName}]}]} ${Bool[${Me.Song[${buffCheckForName}]}]}
				/if (!${Bool[${Me.Song[${buffSpellName}]}]} && !${Bool[${Me.Song[${buffCheckForName}]}]}) {
					/if (${Spell[${buffSpellName}].Stacks} && !${Bool[${Me.Buff[${buffCheckForName}]}]}) {
            /call check_Ready "${ArrayName}" ${s}
            /if (${Macro.Return}) {
              /if (${buffSpellName.Equal[Pyromancy]}) {
                |/bc havebuff ${buffSpellName} ${Me.Buff[${buffSpellName}]} checkfor ${buffCheckForName} ${Me.Buff[${buffCheckForName}]}
                /alt act 514
              } else {
                /call e3_Cast ${buffTargetID} "${ArrayName}" ${s}
              }
            } else /if (${Debug} || ${Debug_BuffCheck}) /echo ${buffSpellName} is not ready for ${buffTarget}
					} else /if (${Debug} || ${Debug_BuffCheck}) /echo buff does not stack, or checkfor buff exists
				} else /if (${Debug} || ${Debug_BuffCheck}) /echo buff or checkfor buff exists in songs						
			} else /if (${Debug} || ${Debug_BuffCheck}) /echo buff exists with duration > 30 seconds
		} else {
		|------- Netbots ------| 		
			/call check_Distance ${buffTargetID} ${${ArrayName}[${s},${iMyRange}]}
			/if (${Macro.Return}) {
			  /if (!${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffSpellID} ]}]}) {
				/if (!${Bool[${NetBots[${buffTarget}].Buff.Find[${buffCheckForID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffCheckForID} ]}]}) {
				  /if (!${${ArrayName}[${s},${iTargetType}].Equal[Single in Group]} || ${Bool[${Group.Member[${buffTarget}]}]}) {
            /call check_Ready "${ArrayName}" ${s}
            /if (${Macro.Return}) {
					    /call e3_Cast ${buffTargetID} "${ArrayName}" ${s}
					    /delay 3 ${Target.BuffsPopulated}
              /if (${castReturn.Equal[CAST_SUCCESS]} && !${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffSpellID} ]}]}) {
                /call CreateTimer "nht${Target.ID}-${buffSpellID}" "${noHoldDelay}"
              }
            }
				  } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot is not in group for groupbuff~		
				} else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot has checkFoR buff~
			  } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot has buff or short buff~
			} else /if (${Debug} || ${Debug_BuffCheck}) /echo ~failed check_Distance~	${buffTargetID} ${${ArrayName}[${s},${iMyRange}]}
		}	
	  }
  :skipBuff
 /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffBots -|
/RETURN

|--------------------------------------------------------|
SUB meleeCombatBuffs(ArrayName, int s)
|/echo mcb ${s} ${NetBots[${${ArrayName}[${s},${iCastTarget}]}].Name} ${${ArrayName}[${s},${iCastName}]}
/if (${Debug} || ${Debug_BuffCheck}) /echo |- meleeCombatBuffs ${ArrayName} ${s} ==>
  /if (${Me.CombatState.Equal[COMBAT]}) {
    /declare buffTargetID	int local ${NetBots[${${ArrayName}[${s},${iCastTarget}]}].ID}
    /if (!${Bool[${NetBots[${${ArrayName}[${s},${iCastTarget}]}].ShortBuff.Find[${${ArrayName}[${s},${iCastID}]} ]}]}) {
      | if the target is me, ensure my group leader does not have the buff (cleaving anger + cry havoc)
      /if (!${Bool[${Select[${buffTargetID},${NetBots[${Me}].ID}]}]} || (${Bool[${Select[${buffTargetID},${NetBots[${Me}].ID}]}]} && !${Bool[${NetBots[${Group.Leader}].ShortBuff.Find[${${ArrayName}[${s},${iCastID}]} ]}]})) {
        /call check_Distance ${buffTargetID} ${${ArrayName}[${s},${iMyRange}]}
        /if (${Macro.Return}) {
          /if (${Me.PctEndurance} >= ${${ArrayName}[${s},${iMinEnd}]} && ${Spell[${${ArrayName}[${s},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}) {
              /call check_Ready "${ArrayName}" ${s}
              /if (${Macro.Return}) {
                /if (${buffTargetID}!=${Me.ID}) /call TrueTarget ${buffTargetID}
                /delay 1
                /disc ${${ArrayName}[${s},${iCastName}]}
                /delay 1
              }
          }
        }
      }
    }
  }
/if (${Debug} || ${Debug_BuffCheck}) /echo <== meleeCombatBuffs -|
/RETURN
|--------------------------------------------------------|
|- Casts GroupBuffs array on the specified buffTargetID.-|
|--------------------------------------------------------|
|- @param line: '<Name> Buff me'.						            -|
|- @param ChatSender: The buff requestor.				        -|
|--------------------------------------------------------|
#event castGroupBuffs "<#1#> Buff #*#"
SUB event_castGroupBuffs(line, ChatSender)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- event_castGroupBuffs ==>	
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[3,_]}]} && ${line.Arg[3, ].NotEqual[all]}) {
	
		/declare buffTargetID int local			
		| -------------------Find buffTargetID------------------- |		
		/if (${Int[${line.Arg[3, ]}]}) {
			/varset buffTargetID ${line.Arg[3, ]}
		} else /if (${line.Find[Buff me]}) {
			/varset buffTargetID ${Spawn[pc =${ChatSender}].ID}
		} else /if (${line.Find[Buff pet]} || ${line.Find[Buff my pet]}) {
			/varset buffTargetID ${Spawn[pet =${Spawn[pc ${ChatSender}].Pet}].ID}
		} else  /if (${SpawnCount[pc =${line.Arg[3, ]}]}) {
			/varset buffTargetID ${Spawn[pc =${line.Arg[3, ]}].ID}
		} else /if (${line.Find['s pet]}) {
			/varset buffTargetID ${Spawn[pet =${Spawn[pc ${line.Arg[3, ].Arg[1,']}].Pet}].ID}
		} else /if (${line.Find[s pet]}) {
			/varset buffTargetID ${Spawn[pet =${Spawn[pc ${line.Arg[3, ].Left[${Math.Calc[${line.Arg[3, ].Length} - 1].Int}]}].Pet}].ID}
		}
		
		/declare s int local 1
		/declare buffAttempts int local 1
		
		:verifyCast
		/if (${s} <= ${GroupBuffs2D.Size[1]}) {
			/if (${Debug} || ${Debug_BuffCheck}) /echo ${s} ${GroupBuffs2D[${s},${iCastName}]} ${GroupBuffs2D.Size[1]}
			/call check_Distance ${buffTargetID} ${GroupBuffs2D[${s},${iMyRange}]}
			/if (${Macro.Return}) {
				/call check_Mana "GroupBuffs2D" ${s}
				/if (${Macro.Return}) {					
					/if (${buffAttempts} <= ${GroupBuffs2D[${s},${iMaxTries}]}) {
						/call e3_Cast ${buffTargetID} "GroupBuffs2D" "${s}"
						/if (${castReturn.Equal[CAST_SUCCESS]}) /varcalc s ${s}+1
						/goto :verifyCast
					} else /if (${Debug} || ${Debug_BuffCheck}) /echo failed buffattempts
				} else /if (${Debug} || ${Debug_BuffCheck}) /echo failed mana
			} else /if (${Debug} || ${Debug_BuffCheck}) /echo failed dist			
		}
		/if (${GroupBuffs2D.Size[1]}) /docommand ${ChatToggle} Done buffing ${Spawn[id ${buffTargetID}].CleanName}.
	}
/if (${Debug} || ${Debug_BuffCheck}) /echo <== castGroupBuffs -|
/RETURN

|----------------------------------------------------|
|- Command group leaders to request group buffs.	-|
|----------------------------------------------------|
#event BuffAll "<#*#> Buff All"
SUB event_BuffAll
/if (${Debug} || ${Debug_BuffCheck}) /echo |- event_BuffAll ==>
	/if (${Select[${Group.Leader},${Me}]}) /bc Buff me
/if (${Debug} || ${Debug_BuffCheck}) /echo <== event_BuffAll -|	
/RETURN

|----------------------------------------------------|
|- Tell a character to turn on/off all buffing		-|
|----------------------------------------------------|
#EVENT toggleBuffs "<#1#> Toggle Buffs #2#"
#EVENT toggleBuffs "#1# tells you, 'Toggle Buffs #2#'"
SUB EVENT_toggleBuffs(line, ChatSender, buffSetting)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- EVENT_toggleBuffs ==>
	/if (${buffSetting.Equal[on]}) {
		/varset buffSetting TRUE
	} else /if (${buffSetting.Equal[off]}) {
		/varset buffSetting FALSE
	}	
	/varset BuffCheck ${buffSetting}	
	/docommand ${ChatToggle} BuffCheck [${If[${BuffCheck},ON,OFF]}]
/if (${Debug} || ${Debug_BuffCheck}) /echo <== EVENT_toggleBuffs -|
/RETURN

|----------------------------------------------------|
|- Tell a character to turn on/off combat buffing	-|
|----------------------------------------------------|
#EVENT toggleCombatBuffs "<#1#> Toggle Combat Buffs #2#"
#EVENT toggleCombatBuffs "#1# tells you, 'Toggle Combat Buffs #2#'"
SUB EVENT_toggleCombatBuffs(line, ChatSender, buffSetting)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- EVENT_toggleBuffs ==>
	/if (${buffSetting.Equal[on]}) {
		/varset buffSetting TRUE
	} else /if (${buffSetting.Equal[off]}) {
		/varset buffSetting FALSE
	}	
	/varset BuffCombatCheck ${buffSetting}	
	/docommand ${ChatToggle} BuffCombatCheck [${If[${BuffCombatCheck},ON,OFF]}]
/if (${Debug} || ${Debug_BuffCheck}) /echo <== EVENT_toggleBuffs -|
/RETURN

|----------------------------------------------------|
Sub buffCheck_Setup
/call IniToVar "${advSettings_Ini},Debug,Debug BuffCheck (On/Off)" Debug_BuffCheck bool outer
/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffCheck_Setup ==>
	/declare BuffCheck 				    bool outer FALSE
	/declare BuffCombatCheck		  bool outer FALSE
	/declare min_BuffCheckHealth 	int outer 80
	/declare buffRecastTime			  int outer 60
  /call buff_SpellArrays
/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffCheck_Setup -|
/return

SUB buff_SpellArrays
  /if (${Ini[${Character_Ini},Buffs,Instant Buff#1].Length})	/call IniToArray "${Character_Ini},Buffs,Instant Buff#" InstantBuffs
  /if (${Ini[${Character_Ini},Buffs,Self Buff#1].Length})		/call IniToArray "${Character_Ini},Buffs,Self Buff#"	SelfBuffs
  /if (${Ini[${Character_Ini},Buffs,Group Buff#1].Length})	/call IniToArray "${Character_Ini},Buffs,Group Buff#"	GroupBuffs
  /if (${Ini[${Character_Ini},Buffs,Bot Buff#1].Length})		/call IniToArray "${Character_Ini},Buffs,Bot Buff#"	BotBuffs
  /if (${Ini[${Character_Ini},Buffs,Combat Buff#1].Length})	/call IniToArray "${Character_Ini},Buffs,Combat Buff#"	CombatBuffs

  /if (${CombatBuffs.Size})	/varset BuffCombatCheck TRUE
  /if (${InstantBuffs.Size} || ${SelfBuffs.Size} || ${BotBuffs.Size} || ${CombatBuffs.Size}) /varset BuffCheck TRUE

  /if (${InstantBuffs.Size})	/call BuildSpellArray "InstantBuffs"	"InstantBuffs2D"
  /if (${SelfBuffs.Size})		/call BuildSpellArray "SelfBuffs"		"SelfBuffs2D"
  /if (${BotBuffs.Size})		/call BuildSpellArray "BotBuffs"		"BotBuffs2D"
  /if (${CombatBuffs.Size})	/call BuildSpellArray "CombatBuffs"		"CombatBuffs2D"
  /if (${GroupBuffs.Size})	/call BuildSpellArray "GroupBuffs"		"GroupBuffs2D"

  /declare s int local
  /if (${InstantBuffs2D.Size[1]}) {
    /for s 1 to ${InstantBuffs2D.Size[1]}
    /if (${InstantBuffs2D[${s},${iMyCastTime}]} > 100) {
      /popup *** Instant buff [${InstantBuffs2D[${s},${iCastName}]}] has a cast time > .1 second, ending macro on ${Me.Name} ***
      /bc *** Instant buff [${InstantBuffs2D[${s},${iCastName}]}] has a cast time > .1 second, ending macro on ${Me.Name} ***
      /beep
      /end
    }
    /next s
  }

  /if (${GroupBuffs2D.Size[1]}) {
    /for s 1 to ${GroupBuffs2D.Size[1]}
    /varset GroupBuffs2D[${s},${iMaxTries}] 2
    /next s
  }
/return

SUB buffCheck_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug BuffCheck (On/Off)" Off
/RETURN

SUB buffCheck_CharacterSettings
	/call WriteToIni "${Character_Ini},Buffs,Instant Buff#1"
	/call WriteToIni "${Character_Ini},Buffs,Self Buff#1"
	/call WriteToIni "${Character_Ini},Buffs,Bot Buff#1"
	/call WriteToIni "${Character_Ini},Buffs,Combat Buff#1"
	/call WriteToIni "${Character_Ini},Buffs,Group Buff#1"
/RETURN

Sub buffCheck_Aliases
/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffCheck_Aliases ==>
	/squelch /alias /Buff /bc Buff me
	/squelch /alias /BuffMe /bc Buff me
	/squelch /alias /Buffs /bc Buff me
	/noparse /squelch /alias /BuffT /bc Buff ${Target.ID}
	/noparse /squelch /alias /BuffIt /bc Buff ${Target.ID}
	/squelch /alias /BuffAll /bc Buff All
	/squelch /alias /BuffGroups /bc Buff All
	/noparse /squelch /alias /BuffPet /bc Buff ${Spawn[pet ${Spawn[id ${Target.ID}].Pet}].ID}
	/squelch /alias /BuffCheck /bc Toggle Buffs
/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffCheck_Aliases -|
/return

Sub buffCheck_Background_Events
	/doevents toggleBuffs
	/doevents toggleCombatBuffs
	/doevents BuffAll
	/doevents castGroupBuffs
/return