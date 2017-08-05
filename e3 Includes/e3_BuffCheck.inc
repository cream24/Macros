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
	/if (${BuffCheck}) {
		/if (!${ActionTaken} && !${activeTimer} && !${medBreak} && !${idle} && ${BuffCombatCheck})	/call buffBots "CombatBuffs2D"
    /if (!${ActionTaken} && !${activeTimer} && !${medBreak} && ${procBuff} && !${idle} && ${BuffCombatCheck}) /call buffProcs
    /if (!${ActionTaken} && !${activeTimer} && ${Bool[${auraSpell}]} && (${auraCombat} || ${Me.CombatState.NotEqual[COMBAT]})) /call checkAuras
		/if (!${ActionTaken} && ${InstantBuffs2D.Size[1]})		/call buffBots "InstantBuffs2D"
		/if (!${ActionTaken} && !${activeTimer} && !${combatTimer} && ${Me.CombatState.NotEqual[COMBAT]} && !${Assisting} && !${Me.Moving} && ${BuffCheck}) {
			/if (!${ActionTaken} && ${SelfBuffs2D.Size[1]})	/call buffBots "SelfBuffs2D"
			/if (!${ActionTaken} && ${BotBuffs2D.Size[1]})	/call buffBots "BotBuffs2D"
      /if (!${ActionTaken} && ${Bool[${auraSpell}]}) /call checkAuras
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

 |/varset Debug_BuffCheck FALSE
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
					/if ((${Spell[${buffSpellName}].Stacks} || ${buffSpellName.Equal[Shield of Dreams]}) && !${Bool[${Me.Buff[${buffCheckForName}]}]}) {
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
|- Casts Proc Buffs (e.x: Panther, Mana Flare           -|
|--------------------------------------------------------|
SUB buffProcs
/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffProcs ==>
  /if (!${ActionTaken}) {
    /declare t int local
    /declare g int local

    /for t 1 to ${ProcBuffs2D.Size[1]}
      /for g 1 to ${Math.Calc[${Group.GroupSize}-1]}
        /if (${Group.Member[${g}].ID}) {
          /if (${procBuffClass.Find[${Group.Member[${g}].Class.ShortName}]}) {
            /if (!${NetBots[${Group.Member[${g}]}].Buff.Find[${ProcBuffs2D[${t},${iSpellID}]} ]} && !${NetBots[${Group.Member[${g}]}].Buff.Find[${ProcBuffs2D[${t},${iCheckFor}]} ]}) {
              /call check_Ready "ProcBuffs2D" ${t}
              /if (${Macro.Return}) {
                /call check_Distance ${Group.Member[${g}].ID} ${ProcBuffs2D[${t},${iMyRange}]}
                /if (${Macro.Return}) {
                  /call check_Mana "ProcBuffs2D" ${t}
                  /if (${Macro.Return}) {
                    /call e3_Cast ${Group.Member[${g}].ID} "ProcBuffs2D" ${t}
                  } else /if (${Debug} || ${Debug_Cures}) /echo ${ProcBuffs2D[${t},${iCastName}]} failed mana check
                } else /if (${Debug} || ${Debug_Cures}) /echo ${ProcBuffs2D[${t},${iCastName}]} is too far away
              } else /if (${Debug} || ${Debug_Cures}) /echo ${ProcBuffs2D[${t},${iCastName}]} is not ready for ${Group.Member[${g}]}
            }
          }
        }
      /if (!${ActionTaken}) /next g
    /if (!${ActionTaken})  /next t
  }
/if (${Debug} || ${Debug_BuffCheck}) /echo <==  buffProcs -|
/return
|--------------------------------------------------------|
|- Casts Proc Buffs (e.x: Panther, Mana Flare           -|
|--------------------------------------------------------|
SUB checkAuras
  /if (${Debug} || ${Debug_BuffCheck}) /echo sub checkAuras ==>
  /if (!${Bool[${Me.Aura[1]}]}) {
    /if (${Me.Class.ShortName.Equal[BRD]}) {
      /if (${Twist.Twisting}) /call pauseTwist
      /delay 3s !${Me.Casting.ID}
      /call check_Ready "auraSpells2D" 1
      /if (${Macro.Return}) {
        /cast ${Me.Gem[${auraSpells2D[1,${iCastName}]}]}
        /delay 5s !${Me.Casting.ID}
      }
    } else {
      /call check_Ready "auraSpells2D" 1
      /if (${Macro.Return}) {
        /call check_Mana "auraSpells2D" 1
        /if (${Macro.Return}) {
            /call e3_Cast ${Me.ID} "auraSpells2D" "1"
        }
      }
    }
  }
  /if (${Debug} || ${Debug_BuffCheck}) /echo <== sub checkAuras
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

  /if (${Ini[${Character_Ini},Buffs,Proc Buff Class].Length})     /call IniToVar "${Character_Ini},Buffs,Proc Buff Class"     procBuffClass string outer
  /if (${Ini[${Character_Ini},Buffs,Proc Buff (On/Off)].Length})	/call IniToVar "${Character_Ini},Buffs,Proc Buff (On/Off)"	procBuff bool outer

  /if (${Ini[${Character_Ini},Buffs,Cast Aura Combat (On/Off)].Length})	/call IniToVar "${Character_Ini},Buffs,Cast Aura Combat (On/Off)"	auraCombat bool outer
	/declare auraSpell string outer

|--  doability
	/if (${Me.CombatAbility[Myrmidon's Aura]}) /varset auraSpell Myrmidon's Aura
	/if (${Me.CombatAbility[Champion's Aura]}) /varset auraSpell Champion's Aura
	/if (${Me.CombatAbility[Disciple's Aura]}) /varset auraSpell Disciple's Aura
	/if (${Me.CombatAbility[Master's Aura]}) /varset auraSpell Master's Aura
	/if (${Me.CombatAbility[Aura of Rage]}) /varset auraSpell Aura of Rage
	/if (${Me.CombatAbility[Bloodlust Aura]}) /varset auraSpell Bloodlust Aura
|-- casting
	/if (${Me.Book[Aura of Insight]}) /varset auraSpell Aura of Insight
	/if (${Me.Book[Aura of the Muse]}) /varset auraSpell Aura of the Muse
  /if (${Me.Book[Aura of the Zealot]}) /varset auraSpell Aura of the Zealot
  /if (${Me.Book[Aura of the Pious]}) /varset auraSpell Aura of the Pious
  /if (${Me.Book[Aura of the Grove]}) /varset auraSpell Aura of the Grove
  /if (${Me.Book[Aura of Life]}) /varset auraSpell Aura of Life
  /if (${Me.Book[Beguiler's Aura]}) /varset auraSpell Beguiler's Aura
  /if (${Me.Book[Illusionist's Aura]}) /varset auraSpell Illusionist's Aura
  /if (${Me.Book[Holy Aura]}) /varset auraSpell Holy Aura
  /if (${Me.Book[Blessed Aura]}) /varset auraSpell Blessed Aura

  /if (${auraSpell.Length}) /call BuildArray auraSpells "${auraSpell}"
  /call buff_SpellArrays

/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffCheck_Setup -|
/return

SUB buff_SpellArrays
  /call #IniToArray "${Character_Ini},Buffs,Instant Buff#" 	InstantBuffs
  /call #IniToArray "${Character_Ini},Buffs,Self Buff#"			SelfBuffs
  /call #IniToArray "${Character_Ini},Buffs,Group Buff#"		GroupBuffs
  /call #IniToArray "${Character_Ini},Buffs,Bot Buff#"			BotBuffs
  /call #IniToArray "${Character_Ini},Buffs,Combat Buff#"		CombatBuffs
  /call #IniToArray "${Character_Ini},Buffs,Proc Buff#"		  ProcBuffs

  /if (${CombatBuffs.Size})	/varset BuffCombatCheck TRUE
  /if (${InstantBuffs.Size} || ${SelfBuffs.Size} || ${BotBuffs.Size} || ${CombatBuffs.Size}) /varset BuffCheck TRUE

  /if (${InstantBuffs.Size})	/call BuildSpellArray "InstantBuffs"	"InstantBuffs2D"
  /if (${SelfBuffs.Size})		  /call BuildSpellArray "SelfBuffs"			"SelfBuffs2D"
  /if (${BotBuffs.Size})		  /call BuildSpellArray "BotBuffs"			"BotBuffs2D"
  /if (${CombatBuffs.Size}) 	/call BuildSpellArray "CombatBuffs"		"CombatBuffs2D"
  /if (${GroupBuffs.Size})	  /call BuildSpellArray "GroupBuffs"		"GroupBuffs2D"
  /if (${ProcBuffs.Size})	    /call BuildSpellArray "ProcBuffs"		  "ProcBuffs2D"
  /if (${auraSpells.Size})    /call BuildSpellArray "auraSpells"		"auraSpells2D"

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
	/call WriteToIni "${Character_Ini},Buffs,Instant Buff"
	/call WriteToIni "${Character_Ini},Buffs,Self Buff"
	/call WriteToIni "${Character_Ini},Buffs,Bot Buff"
	/call WriteToIni "${Character_Ini},Buffs,Combat Buff"
	/call WriteToIni "${Character_Ini},Buffs,Group Buff"
  /if (${Select[${Me.Class.ShortName},WAR,CLR,PAL]}) /call WriteToIni "${Character_Ini},Buffs,Cast Aura Combat (On/Off)" Off
	/if (${Select[${Me.Class.ShortName},DRU,MNK,BER,BRD,ENC]}) /call WriteToIni "${Character_Ini},Buffs,Cast Aura Combat (On/Off)" On

  /if (${Select[${Me.Class.ShortName},ENC,SHM]}) {
    /call WriteToIni "${Character_Ini},Buffs,Proc Buff (On/Off)" Off
    /call WriteToIni "${Character_Ini},Buffs,Proc Buff"
    /if (${Me.Class.ShortName.Equal[SHM]}) {
      /call WriteToIni "${Character_Ini},Buffs,Proc Buff Class" "WAR/BRD/ROG/MNK/BER/RNG"
    } else {
      /call WriteToIni "${Character_Ini},Buffs,Proc Buff Class" "WIZ/MAG
    }
  }
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