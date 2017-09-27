|----------------------------------------------------------------------------|
| Performing curing configured in character ini
| 1- checks Radiant Curing: AutoRadiant + RadiantCure#
| 2- checks specified Target curing: Cure#
| 3- checks all netbot curing: CureAll#
|----------------------------------------------------------------------------|
Sub check_Cures
/declare Debug_Cures bool local FALSE
	/declare t int local
	/declare n int local
  /declare g int local
  /declare numSick int local
| 1- check Radiant Curing
  /if (${autoRCure} && ${doRadiantCure} && ${Me.AltAbilityReady[Radiant Cure]}) {
    /if (${Debug} || ${Debug_Cures}) /echo Checking Rcures
    /for t 1 to ${rCure2D.Size[1]}
      /varset numSick 0
      /for n 0 to ${Group}
        /if (${SpawnCount[pc =${Group.Member[${n}]}]} && ${NetBots[${Group.Member[${n}]}].Counters}>0) {
          /if (${Bool[${Group.Member[${n}].Distance}]} && ${Group.Member[${n}].Distance} < ${rCure2D[${t},${iMyRange}]} && ${Bool[${NetBots[${Group.Member[${n}]}].Buff.Find[${rCure2D[${t},${iCheckForID}]} ]}]}) /varcalc numSick ${numSick}+1
        }
      /next n
      |if enough people have the debuff to cure
      /if (${numSick} >= ${rCure2D[${t},${iMinSick}]}) /call cast_RadiantCure
    /if (!${ActionTaken})  /next t
  }

| 2- checks specified Target curing: Cure#
  /if (!${ActionTaken} && ${doCures}) {
    /if (${Debug} || ${Debug_Cures}) /echo Checking Cures
    /declare curePlayer string local
    /for t 1 to ${cureTargets2D.Size[1]}
      /varset curePlayer ${cureTargets2D[${t},${iCastTarget}]}
      /if (${Debug} || ${Debug_Cures}) /echo Checking ${curePlayer} ${NetBots[${curePlayer}].Buff.Find[${Spell[${cureTargets2D[${t},${iCheckFor}]}].ID} ]} ${NetBots[${curePlayer}]}
      /if (${Debug} || ${Debug_Cures}) /echo for ${cureTargets2D[${t},${iCheckFor}]} sid ${Spell[${cureTargets2D[${t},${iCheckFor}]}].ID}
      |/echo ${SpawnCount[pc =${curePlayer}]}
      /if (${SpawnCount[pc =${curePlayer}]} && ${NetBots[${curePlayer}].Counters}>0) {
        /if (${NetBots[${curePlayer}].Buff.Find[${Spell[${cureTargets2D[${t},${iCheckFor}]}].ID} ]}) {
          /call check_Ready "cureTargets2D" ${t}
          /if (${c_Ready}) {
            /if (${check_Distance[${NetBots[${curePlayer}].ID},${cureTargets2D[${t},${iMyRange}]}]}) {
              /if (${check_Mana["cureTargets2D",${t}]}) {
                /call e3_Cast ${NetBots[${curePlayer}].ID} "cureTargets2D" ${t}
              } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} failed mana check
            } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} is too far away
          } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} is not ready for ${NetBots[${curePlayer}].Name}
        }
      }
    /if (!${ActionTaken})  /next t
  }

| 3a - cure group first in cureall
  /if (!${ActionTaken} && ${doCureAll}) {
    /if (${Debug} || ${Debug_Cures}) /echo Checking All
    /for t 1 to ${cureAllTargets2D.Size[1]}
      /for g 1 to ${Group}
        /if (${SpawnCount[pc =${Group.Member[${n}]}]} && ${NetBots[${Group.Member[${n}]}].Counters}>0) {
          /if (${NetBots[${Group.Member[${g}]}].Buff.Find[${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID} ]}) {
            /call check_Ready "cureAllTargets2D" ${t}
            /if (${c_Ready}) {
              /if (${check_Distance[${NetBots[${NetBots.Client[${n}]}].ID},${cureAllTargets2D[${t},${iMyRange}]}]}) {
                /if (${check_Mana["cureAllTargets2D",${t}]}) {
                  /call e3_Cast ${NetBots[${Group.Member[${g}]}].ID} "cureAllTargets2D" ${t}
                } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} failed mana check
              } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is too far away
            } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is not ready for ${NetBots.Client[${n}]}
          }
        }
      /if (!${ActionTaken}) /next g
    /if (!${ActionTaken})  /next t
  }

| 3b- checks all netbot curing: CureAll#
  /if (!${ActionTaken} && ${doCureAll}) {
    /if (${Math.Rand[2]}==0) {
      /for n 1 to ${NetBots.Counts}
    } else {
      /for n ${NetBots.Counts} downto 1
    }
      /if (${SpawnCount[pc =${NetBots.Client[${n}]}]} && ${NetBots[${NetBots.Client[${n}]}].Counters}>0) {
        /for t 1 to ${cureAllTargets2D.Size[1]}
          /if (${Debug} || ${Debug_Cures}) /echo ${NetBots.Client[${n}]} sp ${SpawnCount[pc =${NetBots.Client[${n}]}]} bf ${NetBots[${NetBots.Client[${n}]}].Buff.Find[${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID} ]} sp ${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID}
          /if (${NetBots[${NetBots.Client[${n}]}].Buff.Find[${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID} ]}) {
            /call check_Ready "cureAllTargets2D" ${t}
            /if (${c_Ready}) {
               /if (${check_Distance[${NetBots[${NetBots.Client[${n}]}].ID},${cureAllTargets2D[${t},${iMyRange}]}]}) {
                /if (${check_Mana["cureAllTargets2D",${t}]}) {
                  /call e3_Cast ${NetBots[${NetBots.Client[${n}]}].ID} "cureAllTargets2D" ${t}
                } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} failed mana check
              } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is too far away
            } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is not ready for ${NetBots.Client[${n}]}
          }
        /if (!${ActionTaken})  /next t
      }
    /if (!${ActionTaken})  /next n

  }
/RETURN

|----------------------------------------------------------------------------|
sub cast_RadiantCure
  /if (${Bool[${WhoCastingRcureList.Length}]}) {
    /if (!${notifyOtherCuring}) /echo ${WhoCastingRcureList.Replace[|,]} is casting Rcure
    /varset notifyOtherCuring TRUE
    /return
  }
  /bc RadiantCure inc...
  /squelch /bcg //CastingRadiantCure ${Me.Name} Y
  /call e3_Cast ${Me.ID} "rCure2D" 1
  /squelch /bcg //CastingRadiantCure ${Me.Name} N
/return

|----------------------------------------------------------------------------|
SUB check_cureCasting(ArrayName, ArrayIndex)
  /varset c_SubToRun ${Bool[${NetBots[${Target.CleanName}].Buff.Find[${Spell[${${ArrayName}[${ArrayIndex},${iCheckFor}]}].ID} ]}]}
  /delay 1
/RETURN

|----------------------------------------------------------------------------|

#bind CastingRadiantCure /CastingRadiantCure
SUB Bind_CastingRadiantCure(whoCasting, toggle)
  /if (${Defined[rCure2D]}) {
    /if (${toggle.Equal[Y]}) {
      /varset WhoCastingRcureList ${WhoCastingRcureList}${whoCasting}|
    } else {
      /varset WhoCastingRcureList ${WhoCastingRcureList.Replace[${whoCasting}|,]}
      /varset notifyOtherCuring FALSE
    }
  }
/return

|--------------------------------------------|
|- Listen for /bc Radiant Cure				      -|
#event RadiantCure "<#1#> Radiant Cure"
sub event_RadiantCure(string line, string ChatSender)
  /if (${Bool[${Me.AltAbilityReady[Radiant Cure]}]}) /call cast_RadiantCure
/return

|--------------------------------------------|
Sub cures_Setup
  /if (${Bool[${Ini[${Character_Ini},Cures]}]}) {
    /declare autoRCure bool outer FALSE
    /if (${Ini[${Character_Ini},Cures,AutoRadiant (On/Off)].Length}) /call IniToVarV "${Character_Ini},Cures,AutoRadiant (On/Off)" autoRCure bool outer
	  /declare doCures       bool outer FALSE
	  /declare doCureAll     bool outer FALSE
    /declare doRadiantCure bool outer FALSE
    /declare WhoCastingRcureList string outer
    /declare notifyOtherCuring string outer FALSE
    /call cure_spellArrays
	}
/return

|--------------------------------------------|
|remove cures from arrays if /Zone is not equal to the current zone shortname or All
Sub cure_spellArrays
  /declare i int local
  /declare removeIndexZone string local
  /call IniToArrayV "${Character_Ini},Cures,RadiantCure#"	rCureArray
  /call IniToArrayV "${Character_Ini},Cures,Cure#"        cureTargets
  /call IniToArrayV "${Character_Ini},Cures,CureAll#"     cureAllTargets

  /if (${Defined[cureTargets]}) {
    /varset removeIndexZone
    /for i 1 to ${cureTargets.Size}
      /if (${cureTargets[${i}].Find[/Zone|]}) {
        /call argueString Zone| "${cureTargets[${i}]}"
        /if (!${c_argueString.Find[All]} && !${c_argueString.Find[${Zone.ShortName}]}) /varset removeIndexZone ${removeIndexZone}${i},
      }
    /next i
    /if (${Bool[${removeIndexZone}]}) /call RemoveArrayElements "cureTargets" ${removeIndexZone}"
    /if (${Defined[cureTargets]}) {
      /call BuildSpellArray "cureTargets"	"cureTargets2D"
      /if (${Defined[cureTargets2D]}) /varset doCures TRUE
    }
  }
  |-------------
  /if (${Defined[cureAllTargets]}) {
    /varset removeIndexZone
    /for i 1 to ${cureAllTargets.Size}
      /if (${cureAllTargets[${i}].Find[/Zone|]}) {
        /call argueString Zone| "${cureAllTargets[${i}]}"
        /if (!${c_argueString.Find[All]} && !${c_argueString.Find[${Zone.ShortName}]}) /varset removeIndexZone ${removeIndexZone}${i},
      }
    /next i
    /if (${Bool[${removeIndexZone}]})  /call RemoveArrayElements "cureAllTargets" ${removeIndexZone}"
    /if (${Defined[cureAllTargets]}) {
      /call BuildSpellArray "cureAllTargets"	"cureAllTargets2D"
      /if (${Defined[cureAllTargets2D]}) /varset doCureAll TRUE
    }
  }
  |-------------
  /if (${Bool[${Me.AltAbility[Radiant Cure]}]}) {
    /if (${Defined[rCureArray]}) {
      /varset removeIndexZone
      /for i 1 to ${rCureArray.Size}
        /if (${rCureArray[${i}].Find[/Zone|]}) {
          /call argueString Zone| "${rCureArray[${i}]}"
          /if (!${c_argueString.Find[All]} && !${c_argueString.Find[${Zone.ShortName}]}) /varset removeIndexZone ${removeIndexZone}${i},
        }
        /varset rCureArray[${i}] Radiant Cure/CheckFor|${rCureArray[${i}]}
      /next i
      /if (${Bool[${removeIndexZone}]})  /call RemoveArrayElements "rCureArray" ${removeIndexZone}"
      /if (${Defined[rCureArray]}) {
        /call BuildSpellArray "rCureArray" "rCure2D"
        /varset doRadiantCure TRUE
      }
    }
    |if no array exists due to /Zone, create a stub for on command RC
    /if (!${Defined[rCure2D]}) {
      /declare rCureSpell[1] string outer "Radiant Cure"
      /call BuildSpellArray "rCureSpell" "rCure2D"
    }
  }
/RETURN

|--------------------------------------------|
SUB cures_CharacterSettings
  /if (${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL,BST,RNG]}) {
    /call WriteToIni "${Character_Ini},Cures,Cure"
    /call WriteToIni "${Character_Ini},Cures,CureAll"
    /if (${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL]} && !${Ini[${Character_Ini},Cures,AutoRadiant (On/Off)].Length}) /call WriteToIni "${Character_Ini},Cures,RadiantCure" "Fulmination/MinSick|1/Zone|txevu"
  }
/RETURN

|--------------------------------------------|
SUB cures_MacroSettings
  /call WriteToIni "${advSettings_Ini},Debug,Debug Cures (On/Off)" Off
/RETURN

|--------------------------------------------|
SUB cures_Aliases
/RETURN

|--------------------------------------------|
Sub cures_Background_Events
  /doevents RadiantCure
/return