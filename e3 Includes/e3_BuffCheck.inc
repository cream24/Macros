|--------------------------------------------------------------------------------|
|- Calls buff functions, returning to the mainloop whenever a spell is cast.		-|
|--------------------------------------------------------------------------------|
SUB check_Buffs
	/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffCheck ==>
	/if (${BuffCheck} && !${ActionTaken} && !${activeTimer} && !${rebuffTimer}) {
		/if (!${medBreak} && !${idle} && ${BuffCombatCheck} && !${Me.Moving})	/call buffBots "CombatBuffs2D"
    /if (!${medBreak} && ${procBuff} && !${idle} && ${BuffCombatCheck} && !${Me.Moving}) /call buffProcs
    /if (${Defined[auraSpells2D]} && (${auraCombat} || ${Me.CombatState.NotEqual[COMBAT]}) && !${Me.Moving}) /call checkAuras
		/if (${Defined[InstantBuffs2D]})		/call buffBots "InstantBuffs2D"
		/if (!${combatTimer} && ${Me.CombatState.NotEqual[COMBAT]} && !${Assisting} && !${Me.Moving} && ${BuffCheck}) {
			/if (${Defined[SelfBuffs2D]}) /call buffBots "SelfBuffs2D"
			/if (${Defined[BotBuffs2D]})	/call buffBots "BotBuffs2D"
      /if (${Defined[auraSpells2D]}) /call checkAuras
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

  | workaround so Growl of the Panther properly identifies the songbuff on BST, to work as a CombatBuff instead of PetBuff which burns mana
  /if (${${ArrayName}[${s},${iSpellName}].Equal[Growl of the Panther]}) /varset buffSpellID 6717

  /if (!${Bool[${NetBots[${buffTarget}].ID}]}) /goto :skipBuff
	|/echo ${buffSpellName} ${buffSpellID} ${buffCheckForName} ${buffCheckForID} ${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}
  |/echo ${buffSpellName} ${Select["${buffSpellName}","Thief's Eyes", "Fists of Wu", "Commanding Voice","Cry Havoc"]}
  /if (${Select[${buffSpellID},8001,8002,8000,8003]}) {
    /call meleeCombatBuffs "${ArrayName}" ${s}
    /goto :skipBuff
  }
  |------- Self ------|
  /if (${Select[${ArrayName},SelfBuffs2D,InstantBuffs2D]}) {
    /if (!${Bool[${Me.Buff[${buffSpellName}]}]} || (${Bool[${Me.Buff[${buffSpellName}]}]} && ${Me.Buff[${buffSpellName}].Duration.TotalSeconds} <= ${buffRecastTime})) {
      /if (!${Bool[${Me.Song[${buffSpellName}]}]} && !${Bool[${Me.Song[${buffCheckForName}]}]}) {
        /if (${Spell[${buffSpellID}].NewStacks} && !${Bool[${Me.Buff[${buffCheckForName}]}]}) {
          /call check_Ready "${ArrayName}" ${s}
          /if (${c_Ready} && ${${ArrayName}[${s},${iIfs}]}) {
            /if (${check_Mana["${ArrayName}",${s}]}) {
              /call e3_Cast ${buffTargetID} "${ArrayName}" ${s}
            }
          } else /if (${Debug} || ${Debug_BuffCheck}) /echo ${buffSpellName} is not ready for ${buffTarget}
        } else /if (${Debug} || ${Debug_BuffCheck}) /echo buff does not stack, or checkfor buff exists
      } else /if (${Debug} || ${Debug_BuffCheck}) /echo buff or checkfor buff exists in songs
    } else /if (${Debug} || ${Debug_BuffCheck}) /echo buff exists with duration > 30 seconds
  } else {
  |------- Netbots ------|
    /if (!${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffSpellID} ]}]} && ${NetBots[${buffTarget}].FreeBuffSlots}>0 && ${Bool[${NetBots[${buffTarget}].InZone}]}) {
      /if (!${Bool[${NetBots[${buffTarget}].Buff.Find[${buffCheckForID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffCheckForID} ]}]}) {
        /if (!${${ArrayName}[${s},${iTargetType}].Equal[Single in Group]} || ${Bool[${Group.Member[${buffTarget}]}]}) {
          /call check_Ready "${ArrayName}" ${s}
          /if (${c_Ready} && ${${ArrayName}[${s},${iIfs}]}) {
            /if (${check_Distance[${buffTargetID},${${ArrayName}[${s},${iMyRange}]}]}) {
              /if (${check_Mana["${ArrayName}",${s}]}) {
                /call e3_Cast ${buffTargetID} "${ArrayName}" ${s}
                /delay 3 ${Target.BuffsPopulated}
                /if (${castReturn.Equal[CAST_SUCCESS]} && !${Bool[${NetBots[${buffTarget}].Buff.Find[${buffSpellID} ]}]}	&& !${Bool[${NetBots[${buffTarget}].ShortBuff.Find[${buffSpellID} ]}]}) {
                  /call CreateTimer "nht${Target.ID}-${buffSpellID}" "${noHoldDelay}"
                }
              }
            } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~failed check_Distance~	${buffTargetID} ${${ArrayName}[${s},${iMyRange}]}
          }
        } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot is not in group for groupbuff~
      } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot has checkFoR buff~
    } else /if (${Debug} || ${Debug_BuffCheck}) /echo ~Bot has buff or short buff~
  }
  :skipBuff
  /doevents Follow
  /doevents Stop
  /doevents MoveHere
  /doevents clickit
  /doevents bark
  /doevents BackOff
  /doevents Assist
 /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_BuffCheck}) /echo <== buffBots -|
/RETURN

|--------------------------------------------------------|
SUB meleeCombatBuffs(ArrayName, int s)
|/echo mcb ${s} ${NetBots[${${ArrayName}[${s},${iCastTarget}]}].Name} ${${ArrayName}[${s},${iCastName}]}
/if (${Debug} || ${Debug_BuffCheck}) /echo |- meleeCombatBuffs ${ArrayName} ${s} ==>
  /if (${Me.CombatState.Equal[COMBAT]}||${Me.Combat}) {
    /declare buffTargetID	int local ${NetBots[${${ArrayName}[${s},${iCastTarget}]}].ID}
    /if (!${Bool[${NetBots[${${ArrayName}[${s},${iCastTarget}]}].ShortBuff.Find[${${ArrayName}[${s},${iCastID}]} ]}]}) {
      | if the target is me, ensure my group leader does not have the buff (cleaving anger + cry havoc)
      /if (!${Bool[${Select[${buffTargetID},${NetBots[${Me}].ID}]}]} || (${Bool[${Select[${buffTargetID},${NetBots[${Me}].ID}]}]} && !${Bool[${NetBots[${Group.Leader}].ShortBuff.Find[${${ArrayName}[${s},${iCastID}]} ]}]})) {
        /if (${check_Distance[${buffTargetID},${${ArrayName}[${s},${iMyRange}]}]}) {
          /if (${Me.PctEndurance} >= ${${ArrayName}[${s},${iMinEnd}]} && ${Spell[${${ArrayName}[${s},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}) {
              /call check_Ready "${ArrayName}" ${s}
              /if (${c_Ready}) {
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
    /declare s int local
    /declare g int local

    /for s 1 to ${ProcBuffs2D.Size[1]}
      /for g 1 to ${Math.Calc[${Group.GroupSize}-1]}
        /if (${Group.Member[${g}].ID}) {
          /if (${procBuffClass.Find[${Group.Member[${g}].Class.ShortName}]}) {
            /if (!${NetBots[${Group.Member[${g}]}].Buff.Find[${ProcBuffs2D[${s},${iSpellID}]} ]} && !${NetBots[${Group.Member[${g}]}].Buff.Find[${ProcBuffs2D[${s},${iCheckFor}]} ]}) {
              /call check_Ready "ProcBuffs2D" ${s}
              /if (${c_Ready}) {
                /call check_Distance
                /if (${check_Distance[${Group.Member[${g}].ID},${ProcBuffs2D[${s},${iMyRange}]}]}) {
                  /if (${check_Mana["ProcBuffs2D",${s}]}) {
                    /call e3_Cast ${Group.Member[${g}].ID} "ProcBuffs2D" ${s}
                  } else /if (${Debug} || ${Debug_BuffCheck}) /echo ${ProcBuffs2D[${s},${iCastName}]} failed mana check
                } else /if (${Debug} || ${Debug_BuffCheck}) /echo ${ProcBuffs2D[${s},${iCastName}]} is too far away
              } else /if (${Debug} || ${Debug_BuffCheck}) /echo ${ProcBuffs2D[${s},${iCastName}]} is not ready for ${Group.Member[${g}]}
            }
          }
        }
      /doevents Follow
      /doevents Stop
      /doevents MoveHere
      /doevents clickit
      /doevents bark
      /doevents BackOff
      /doevents Assist
      /if (!${ActionTaken}) /next g
    /if (!${ActionTaken})  /next s
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
      /if (${Me.Casting.ID}) /call interrupt
      /call check_Ready "auraSpells2D" 1
      /if (${c_Ready}) {
        /cast ${Me.Gem[${auraSpells2D[1,${iCastName}]}]}
        /delay 7s !${Me.Casting.ID}
      }
      /call unpauseTwist
    } else {
      /call check_Ready "auraSpells2D" 1
      /if (${c_Ready}) {
        /if (${check_Mana["auraSpells2D",1]}) {
            /call e3_Cast ${Me.ID} "auraSpells2D" "1"
        }
      }
    }
  }
  /if (${Debug} || ${Debug_BuffCheck}) /echo <== sub checkAuras
/RETURN
|--------------------------------------------------------|
|- Casts GroupBuffs array on the specified buffTargetID.-|
|- @param line: '<Name> Buff me'.						            -|
|- @param ChatSender: The buff requestor.				        -|
|--------------------------------------------------------|
#event castGroupBuffs "<#1#> Buff #*#"
SUB event_castGroupBuffs(line, ChatSender)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- event_castGroupBuffs ==>
  /if (!${Defined[GroupBuffs2D]}) /return
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
  | -------------------Find buffTarget------------------- |
  /declare buffTarget string local ${line.Right[-${Math.Calc[${line.Find[buff]}+4]}]}
  /if (${Int[${buffTarget}]}) {
    /varset buffTarget ${Spawn[id ${buffTarget}].ID}
  } else /if (${buffTarget.Equal[me]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].ID}
  } else /if (${buffTarget.Find['s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-6]}].Pet.ID}
  } else /if (${buffTarget.Find[s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-5]}].Pet.ID}
  } else /if (${buffTarget.Equal[pet]} || ${buffTarget.Equal[my pet]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].Pet.ID}
  } else /if (${SpawnCount[=${buffTarget}]}) {
    /varset buffTarget ${Spawn[=${buffTarget}].ID}
  }

  /declare s int local 1
  /declare buffAttempts int local 1

  :verifyCast
  /if (${s} <= ${GroupBuffs2D.Size[1]}) {
    /if (${Debug} || ${Debug_BuffCheck}) /echo ${s} ${GroupBuffs2D[${s},${iCastName}]} ${GroupBuffs2D.Size[1]}
    /if (${check_Distance[${buffTarget},${GroupBuffs2D[${s},${iMyRange}]}]}) {
      /if (${check_Mana["GroupBuffs2D",${s}]}) {
        /if (${buffAttempts} <= ${GroupBuffs2D[${s},${iMaxTries}]}) {
          /call e3_Cast ${buffTarget} "GroupBuffs2D" "${s}"
          /if (${castReturn.Equal[CAST_SUCCESS]}) /varcalc s ${s}+1
          /goto :verifyCast
        } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed buffattempts
      } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed mana
    } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed dist
  }
  /if (${GroupBuffs2D.Size[1]}) /docommand ${ChatToggle} Done buffing ${Spawn[id ${buffTarget}].CleanName}.
/if (${Debug} || ${Debug_BuffCheck}) /echo <== castGroupBuffs -|
/RETURN

|duplicating code as usage of the group buff may vary widely
#event castGroupBuffsL1 "<#1#> L1 Buff #*#"
SUB event_castGroupBuffsL1(line, ChatSender)
  /if (${Debug} || ${Debug_BuffCheck}) /echo |- event_castGroupBuffs L1 ==>
  /if (!${Defined[GroupBuffsL12D]}) /return
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
| -------------------Find buffTarget------------------- |
  /declare buffTarget string local ${line.Right[-${Math.Calc[${line.Find[buff]}+4]}]}
  /if (${Int[${buffTarget}]}) {
    /varset buffTarget ${Spawn[id ${buffTarget}].ID}
  } else /if (${buffTarget.Equal[me]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].ID}
  } else /if (${buffTarget.Find['s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-6]}].Pet.ID}
  } else /if (${buffTarget.Find[s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-5]}].Pet.ID}
  } else /if (${buffTarget.Equal[pet]} || ${buffTarget.Equal[my pet]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].Pet.ID}
  } else /if (${SpawnCount[=${buffTarget}]}) {
    /varset buffTarget ${Spawn[=${buffTarget}].ID}
  }

  /declare s int local 1
  /declare buffAttempts int local 1

  :verifyCast
  /if (${s} <= ${GroupBuffsL12D.Size[1]}) {
    /if (${Debug} || ${Debug_BuffCheck}) /echo ${s} ${GroupBuffsL12D[${s},${iCastName}]} ${GroupBuffsL12D.Size[1]}
    /if (${check_Distance[${buffTarget},${GroupBuffsL12D[${s},${iMyRange}]}]}) {
      /if (${check_Mana["GroupBuffsL12D",${s}]}) {
        /if (${buffAttempts} <= ${GroupBuffsL12D[${s},${iMaxTries}]}) {
          /call e3_Cast ${buffTarget} "GroupBuffsL12D" "${s}"
          /if (${castReturn.Equal[CAST_SUCCESS]}) /varcalc s ${s}+1
          /goto :verifyCast
        } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed buffattempts
      } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed mana
    } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed dist
  }
  /if (${GroupBuffsL12D.Size[1]}) /docommand ${ChatToggle} Done Level 1 buffing ${Spawn[id ${buffTarget}].CleanName}.
  /if (${Debug} || ${Debug_BuffCheck}) /echo <== castGroupBuffs -|
/RETURN

|duplicating code as usage of the group buff may vary widely
#event castGroupBuffsL47 "<#1#> L47 Buff #*#"
SUB event_castGroupBuffsL47(line, ChatSender)
  /echo IN event_castGroupBuffsL47
  /if (${Debug} || ${Debug_BuffCheck}) /echo |- event_castGroupBuffs L47 ==>
  /if (!${Defined[GroupBuffsL472D]}) /return
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
| -------------------Find buffTarget------------------- |
  /declare buffTarget string local ${line.Right[-${Math.Calc[${line.Find[buff]}+4]}]}
  /if (${Int[${buffTarget}]}) {
    /varset buffTarget ${Spawn[id ${buffTarget}].ID}
  } else /if (${buffTarget.Equal[me]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].ID}
  } else /if (${buffTarget.Find['s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-6]}].Pet.ID}
  } else /if (${buffTarget.Find[s pet]}) {
    /varset buffTarget ${Spawn[pc =${buffTarget.Left[-5]}].Pet.ID}
  } else /if (${buffTarget.Equal[pet]} || ${buffTarget.Equal[my pet]}) {
    /varset buffTarget ${Spawn[pc =${ChatSender}].Pet.ID}
  } else /if (${SpawnCount[=${buffTarget}]}) {
    /varset buffTarget ${Spawn[=${buffTarget}].ID}
  }

  /declare s int local 1
  /declare buffAttempts int local 1

  :verifyCast
  /if (${s} <= ${GroupBuffsL472D.Size[1]}) {
    /if (${Debug} || ${Debug_BuffCheck}) /echo ${s} ${GroupBuffsL472D[${s},${iCastName}]} ${GroupBuffsL472D.Size[1]}
    /if (${check_Distance[${buffTarget},${GroupBuffsL472D[${s},${iMyRange}]}]}) {
      /if (${check_Mana["GroupBuffsL472D",${s}]}) {
        /if (${buffAttempts} <= ${GroupBuffsL472D[${s},${iMaxTries}]}) {
          /call e3_Cast ${buffTarget} "GroupBuffsL472D" "${s}"
          /if (${castReturn.Equal[CAST_SUCCESS]}) /varcalc s ${s}+1
          /goto :verifyCast
        } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed buffattempts
      } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed mana
    } else /if (${Debug} || ${Debug_BuffCheck}) /echo failed dist
  }
  /if (${GroupBuffsL472D.Size[1]}) /docommand ${ChatToggle} Done Level 47 buffing ${Spawn[id ${buffTarget}].CleanName}.
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
|- Tell a character to turn on/off all buffing      -|
|----------------------------------------------------|
#EVENT toggleBuffs "<#1#> Toggle Buffs #2#"
#EVENT toggleBuffs "#1# tells you, 'Toggle Buffs #2#'"
SUB EVENT_toggleBuffs(line, ChatSender, parms)
  /if (${Debug} || ${Debug_BuffCheck}) /echo |- EVENT_toggleBuffs ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${parms}]}) /return
  /declare buffSetting bool local ${BuffCheck}
  /varset parms ${c_eventArgData}
  /if (${parms.Equal[on]} && !${BuffCheck}) {
    /varset buffSetting TRUE
  } else /if (${parms.Equal[off]} && ${BuffCheck}) {
    /varset buffSetting FALSE
  }
  /if (${buffSetting} != ${BuffCheck}) {
    /varset BuffCheck ${buffSetting}
    /docommand ${ChatToggle} BuffCheck [${If[${BuffCheck},ON,OFF]}]
  }
  /if (${Debug} || ${Debug_BuffCheck}) /echo <== EVENT_toggleBuffs -|
/RETURN

|----------------------------------------------------|
|- Tell a character to turn on/off combat buffing	  -|
|----------------------------------------------------|
#EVENT toggleCombatBuffs "<#1#> Toggle Combat Buffs #2#"
#EVENT toggleCombatBuffs "#1# tells you, 'Toggle Combat Buffs #2#'"
SUB EVENT_toggleCombatBuffs(line, ChatSender, buffSetting)
/if (${Debug} || ${Debug_BuffCheck}) /echo |- EVENT_toggleBuffs ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${buffSetting}]}) /return
  /varset buffSetting ${c_eventArgData}
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
/call iniToVarV "${advSettings_Ini},Debug,Debug BuffCheck (On/Off)" Debug_BuffCheck bool
/if (${Debug} || ${Debug_BuffCheck}) /echo |- buffCheck_Setup ==>
	/declare BuffCheck 				    bool outer TRUE
	/declare BuffCombatCheck		  bool outer FALSE
	/declare min_BuffCheckHealth 	int outer 80
	/declare buffRecastTime			  int outer 60
  /call iniToVarV "${Character_Ini},Buffs,Proc Buff Class"  procBuffClass string
  /call iniToVarV "${Character_Ini},Buffs,Proc Buff (On/Off)"	procBuff bool

  /call iniToVarV "${Character_Ini},Buffs,Cast Aura Combat (On/Off)"	auraCombat bool
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
  /call IniToArrayV "${Character_Ini},Buffs,Instant Buff" 	InstantBuffs
  /call IniToArrayV "${Character_Ini},Buffs,Self Buff"			SelfBuffs
  /call IniToArrayV "${Character_Ini},Buffs,Group Buff"		GroupBuffs
  /call IniToArrayV "${Character_Ini},Buffs,Bot Buff"			BotBuffs
  /call IniToArrayV "${Character_Ini},Buffs,Combat Buff"		CombatBuffs
  /call IniToArrayV "${Character_Ini},Buffs,Proc Buff"		  ProcBuffs

  /call IniToArrayV "${Character_Ini},Buffs,Group Buff L1"		GroupBuffsL1
  /call IniToArrayV "${Character_Ini},Buffs,Group Buff L47"		GroupBuffsL47

  /if (${Defined[InstantBuffs]}) /call BuildSpellArray "InstantBuffs"	"InstantBuffs2D"
  /if (${Defined[SelfBuffs]})    /call BuildSpellArray "SelfBuffs"	  "SelfBuffs2D"
  /if (${Defined[BotBuffs]})     /call BuildSpellArray "BotBuffs"	    "BotBuffs2D"
  /if (${Defined[CombatBuffs]})  /call BuildSpellArray "CombatBuffs"	"CombatBuffs2D"
  /if (${Defined[GroupBuffs]})   /call BuildSpellArray "GroupBuffs"	  "GroupBuffs2D"
  /if (${Defined[ProcBuffs]})    /call BuildSpellArray "ProcBuffs"	  "ProcBuffs2D"
  /if (${Defined[auraSpells]})   /call BuildSpellArray "auraSpells"	  "auraSpells2D"

  /if (${Defined[GroupBuffsL1]})   /call BuildSpellArray "GroupBuffsL1"	  "GroupBuffsL12D"
  /if (${Defined[GroupBuffsL47]})   /call BuildSpellArray "GroupBuffsL47"	  "GroupBuffsL472D"

  /if (${Defined[CombatBuffs2D]} || ${Defined[ProcBuffs]})	/varset BuffCombatCheck TRUE

  |/if (${InstantBuffs.Size})	/call BuildSpellArray "InstantBuffs"	"InstantBuffs2D"
  |/if (${SelfBuffs.Size})		  /call BuildSpellArray "SelfBuffs"			"SelfBuffs2D"
  |/if (${BotBuffs.Size})		  /call BuildSpellArray "BotBuffs"			"BotBuffs2D"
  |/if (${CombatBuffs.Size}) 	/call BuildSpellArray "CombatBuffs"		"CombatBuffs2D"
  |/if (${GroupBuffs.Size})	  /call BuildSpellArray "GroupBuffs"		"GroupBuffs2D"
 | /if (${ProcBuffs.Size})	    /call BuildSpellArray "ProcBuffs"		  "ProcBuffs2D"
  |/if (${auraSpells.Size})    /call BuildSpellArray "auraSpells"		"auraSpells2D"

  /declare s int local
  /if (${Defined[InstantBuffs2D]}) {
    /for s 1 to ${InstantBuffs2D.Size[1]}
    /if (${InstantBuffs2D[${s},${iMyCastTime}]} > 100) {
      /popup *** Instant buff [${InstantBuffs2D[${s},${iCastName}]}] has a cast time > .1 second, ending macro on ${Me.Name} ***
      /bc *** Instant buff [${InstantBuffs2D[${s},${iCastName}]}] has a cast time > .1 second, ending macro on ${Me.Name} ***
      /beep
    }
    /next s
  }

  /if (${Defined[GroupBuffs2D]}) {
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
  /call WriteToIni "${Character_Ini},Buffs,Group Buff L1"
  /call WriteToIni "${Character_Ini},Buffs,Group Buff L47"

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
/if (${Debug}) /echo |- buffCheck_Aliases ==>
	/squelch /alias /Buff /bc Buff me
	/squelch /alias /BuffMe /bc Buff me
	/squelch /alias /Buffs /bc Buff me
	/noparse /squelch /alias /BuffT /bc Buff ${Target.ID}
	/noparse /squelch /alias /BuffIt /bc Buff ${Target.ID}
	/squelch /alias /BuffAll /bc Buff All
	/squelch /alias /BuffGroups /bc Buff All
	/noparse /squelch /alias /BuffPet /bc Buff ${Spawn[pet ${Spawn[id ${Target.ID}].Pet}].ID}
	/squelch /alias /BuffCheck /bc Toggle Buffs
/if (${Debug}) /echo <== buffCheck_Aliases -|
/return

Sub buffCheck_Background_Events
	/doevents toggleBuffs
	/doevents toggleCombatBuffs
	/doevents BuffAll
	/doevents castGroupBuffs
	/doevents castGroupBuffsL1
	/doevents castGroupBuffsL47
/return
