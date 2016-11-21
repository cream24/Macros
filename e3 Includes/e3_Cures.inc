|----------------------------------------------------------------------------|
| Performing curing configured in character ini
| 1- checks Radiant Curing: AutoRadiant + RadiantCure#
| 2- checks specified Target curing: Cure#
| 3- checks all netbot curing: CureAll#
|----------------------------------------------------------------------------|
Sub check_Cures
	/declare t int local
	/declare n int local
| 1- checks Radiant Curing: AutoRadiant + RadiantCure#
  /if (${autoRCure}) /call do_RadiantCure

| 2- checks specified Target curing: Cure#
  /if (!${ActionTaken} && ${doCures}) {
    /declare curePlayer string local
    /for t 1 to ${cureTargets2D.Size[1]}
      /varset curePlayer ${cureTargets2D[${t},${iCastTarget}]}
      |/echo ${SpawnCount[pc =${curePlayer}]}
      /if (${SpawnCount[pc =${curePlayer}]} && ${NetBots[${curePlayer}].Buff.Find[${Spell[${cureTargets2D[${t},${iCheckFor}]}].ID} ]}) {
        /call check_Ready "cureTargets2D" ${t}
        |/echo check_Ready ${Macro.Return}
        /if (${Macro.Return}) {
          /call check_Distance ${NetBots[${curePlayer}].ID} ${cureTargets2D[${t},${iMyRange}]}
          |/echo check_Distance ${NetBots[${curePlayer}].ID}  ${cureTargets2D[${t},${iMyRange}]} ${Macro.Return}
          /if (${Macro.Return}) {
            /call check_Mana "cureTargets2D" ${t}
            |/echo check_Mana ${Macro.Return}
            /if (${Macro.Return}) {
              /call e3_Cast ${NetBots[${curePlayer}].ID} "cureTargets2D" ${t}
            } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} failed mana check
          } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} is too far away
        } else /if (${Debug} || ${Debug_Cures}) /echo ${cureTargets2D[${t},${iCastName}]} is not ready for ${NetBots[${curePlayer}].Name}
      }
    /if (!${ActionTaken})  /next t
  }

| 3- checks all netbot curing: CureAll#
  /if (!${ActionTaken} && ${doCureAll}) {
    /for t 1 to ${cureAllTargets2D.Size[1]}
    /if (${Math.Rand[2]}==0) {
      /for n 1 to ${NetBots.Counts}
    } else {
      /for n ${NetBots.Counts} downto 1
    }
      |/echo ${NetBots.Client[${n}]} sp ${SpawnCount[pc =${NetBots.Client[${n}]}]} bf ${NetBots[${NetBots.Client[${n}]}].Buff.Find[${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID} ]} sp ${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID}
      /if (${SpawnCount[pc =${NetBots.Client[${n}]}]} && ${NetBots[${NetBots.Client[${n}]}].Buff.Find[${Spell[${cureAllTargets2D[${t},${iCheckFor}]}].ID} ]}) {
        /call check_Ready "cureAllTargets2D" ${t}
        |/echo check_Ready ${Macro.Return}
        /if (${Macro.Return}) {
          /call check_Distance ${NetBots[${NetBots.Client[${n}]}].ID} ${cureAllTargets2D[${t},${iMyRange}]}
          |/echo check_Distance ${NetBots[${NetBots.Client[${n}]}].ID}  ${cureTargets2D[${t},${iMyRange}]} ${Macro.Return}
          /if (${Macro.Return}) {
            /call check_Mana "cureAllTargets2D" ${t}
            |/echo check_Mana ${Macro.Return}
            /if (${Macro.Return}) {
              /call e3_Cast ${NetBots[${NetBots.Client[${n}]}].ID} "cureAllTargets2D" ${t}
            } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} failed mana check
          } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is too far away
        } else /if (${Debug} || ${Debug_Cures}) /echo ${cureAllTargets2D[${t},${iCastName}]} is not ready for ${NetBots.Client[${n}]}
      }
      /if (!${ActionTaken})  /next n
    /if (!${ActionTaken})  /next t
  }
/RETURN

|----------------------------------------------------------------------------|
SUB do_RadiantCure
  /declare t int local
  /declare n int local
| 1- checks Radiant Curing: AutoRadiant + RadiantCure#
  /if (${Me.AltAbilityReady[Radiant Cure]} && !${rCureTimer}) {
    /declare debuffToCure string local
    /declare minSick int local
    /declare numSick int local

    /for t 1 to ${rCureArray.Size}
      /varset debuffToCure ${rCureArray[${t}].Arg[1,/]}
      /varset minSick ${rCureArray[${t}].Arg[2,|]}
      /varset numSick 0
      |/echo ${debuffToCure} myrange ${rCure2D[1,${iMyRange}]}
      /for n 0 to ${Group}
        /if (${Bool[${Group.Member[${n}].Distance}]} && ${Group.Member[${n}].Distance} < ${rCure2D[1,${iMyRange}]} && ${Bool[${NetBots[${Group.Member[${n}]}].Buff.Find[${Spell[${debuffToCure}].ID} ]}]}) /varcalc numSick ${numSick}+1
      /next n
      |if enough people have the debuff to cure
      /if (${numSick} >= ${minSick}) {
        |check no one else in group is casting rcure, this is marginally effective
        /for n 1 to ${Group}
          /if (${Select[${NetBots[${Group.Member[${n}]}].Casting},Radiant Cure]}) {
            /bc ${NetBots[${Group.Member[${n}]}].Name} is already casting Rcure ${Time}
            /return
          }
        /next n

        /g RadiantCure
        /bc RadiantCure inc...
        /call e3_Cast ${Me.ID} "rCure2D" 1
      }
    /if (!${ActionTaken})  /next t
  }
/RETURN

|----------------------------------------------------------------------------|
SUB check_cureCasting(ArrayName, ArrayIndex)
/delay 1
/RETURN ${Bool[${NetBots[${Target.CleanName}].Buff.Find[${Spell[${${ArrayName}[${ArrayIndex},${iCheckFor}]}].ID} ]}]}

|--------------------------------------------|
|- Listen for /bc Radiant Cure				      -|
#event RadiantCure "<#1#> Radiant Cure"
sub event_RadiantCure(string line, string ChatSender)
  /if (${Bool[${Me.AltAbility[Radiant Cure]}]}) {
    /if (${Me.AltAbilityReady[Radiant Cure]}) {
      /g RadiantCure inc...
      /bc RadiantCure inc...
      /call e3_Cast ${Me.ID} "rCure2D" 1
    } else {
      /bc RadiantCure is not ready
    }
  }
/return

|--------------------------------------------|
Sub cures_Setup
  /if (${Bool[${Ini[${Character_Ini},Cures]}]}) {
	  /declare doCures   bool outer FALSE
	  /declare doCureAll bool outer FALSE
    /declare autoRCure bool outer FALSE
    /if (${Ini[${Character_Ini},Cures,AutoRadiant (On/Off)].Length}) /call INItoVar "${Character_Ini},Cures,AutoRadiant (On/Off)" autoRCure bool outer
    /call cure_spellArrays
	}
/return

|--------------------------------------------|
Sub cure_spellArrays
  /declare i int local
  /if (${Ini[${Character_Ini},Cures,RadiantCure#1].Length})	/call IniToArray "${Character_Ini},Cures,RadiantCure#"	rCureArray

  /if (${Ini[${Character_Ini},Cures,Cure#1].Length})	 /call IniToArray "${Character_Ini},Cures,Cure#" cureTargets
  /if (${Bool[${cureTargets.Size}]}) {
    /varset doCures TRUE
    /call BuildSpellArray "cureTargets"	"cureTargets2D"
    /for i 1 to ${cureTargets2D.Size[1]}
      /varset cureTargets2D[${i},${iSubToRun}] check_cureCasting
    /next i
  }

  /if (${Ini[${Character_Ini},Cures,CureAll#1].Length})	 /call IniToArray "${Character_Ini},Cures,CureAll#" cureAllTargets
  /if (${cureAllTargets.Size}) {
    /varset doCureAll TRUE
    /call BuildSpellArray "cureAllTargets" "cureAllTargets2D"
    /for i 1 to ${cureAllTargets2D.Size[1]}
      /varset cureAllTargets2D[${i},${iSubToRun}] check_cureCasting
    /next i
  }
  /if (${Bool[${Me.AltAbility[Radiant Cure]}]} && ${Bool[${rCureArray.Size}]}) {
    /declare rCure[1] string outer Radiant Cure
    /call BuildSpellArray "rCure" "rCure2D"
  } else {
    /if (${Debug}) /echo I do not have Radiant Cure
    /if (${Defined[autoRCure]}) /varset autoRCure FALSE
  }
/RETURN

|--------------------------------------------|
SUB cures_CharacterSettings
  /if (${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL,BST,RNG]}) {
    /call WriteToIni "${Character_Ini},Cures,Cure#1"
    /call WriteToIni "${Character_Ini},Cures,CureAll#1"
    /if (${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL]}) /call WriteToIni "${Character_Ini},Cures,AutoRadiant (On/Off)" Off
    /if (${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL]}) /call WriteToIni "${Character_Ini},Cures,RadiantCure#1" "Fulmination/MinSick|1"
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