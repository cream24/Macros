|--------------------------------------------------------------------------|
|- e3_Macro_Pets.inc v3.0 ( originally designed to work with e3.mac )	    -|
|- Written by Killians of PEQ											                        -|
|--------------------------------------------------------------------------|
|- Checks your pets status. Automatically, resummons, buffs, and shrinks.	-|
|--------------------------------------------------------------------------|
SUB check_Pets
/if (${Debug} || ${Debug_Pets}) /echo |- check_Pets ==>
  /if (${Debug} || ${Debug_Pets}) /echo Pets buffcheck ${BuffCheck} petid ${Me.Pet.ID} action !${ActionTaken} active !${activeTimer} pb ${Defined[petBuffs2D]} med !${medBreak} idle !${idle} pbc ${petBuffCombat} combat !${Me.Combat}
	/if (!${ActionTaken} && !${suspend_Pets} && !${activeTimer} && !${rebuffTimer} && !${Me.Invis}) {
    /if (${Me.Pet.ID} && ${Defined[petHeal2D]}) /call check_petHeal
    /if (!${Me.Pet.ID} && !${ActionTaken} && ${Defined[petSpell2D]} && ${SpawnCount[pc ${Me}]} && (${petSummonCombat} || ${Me.CombatState.NotEqual[COMBAT]})) /call summonPet
    /if (${BuffCheck} && ${Me.Pet.ID} && ${Defined[petBuffs2D]} && !${medBreak} && (${petBuffCombat} || ${Me.CombatState.NotEqual[COMBAT]}))	/call buffPet
    /if (${BuffCheck} && ${Me.Pet.ID} && ${petAutoShrink} && !${combatTimer}) /call check_AutoShrink
	}
/if (${Debug} || ${Debug_Pets}) /echo <== check_Pets -|
/RETURN

|------------------------------------------------------------|
|- Summons a new pet.										                     |
SUB summonPet
/if (${Debug} || ${Debug_Pets}) /echo |- summonPet ==>
  |handle zone delay to prevent summoning pet if one exists
  /if (!${Bool[${Zone.ID}]} || ${currentZone} != ${Zone.ID} || ${SpawnCount[pc = ${Me}]}==0) {
    /delay 3s
    /call check_Zone
    /return
  }
  /call check_Ready "petSpell2D" 1
  /if (${c_Ready}) {
    /if (${check_Mana["petSpell2D",1]}) {
	    /call e3_Cast ${Me.ID} "petSpell2D" 1
    }
  }
  /if (${Me.Pet.ID} && ${petTaunt}) {
    /pet taunt on
  } else {
    /pet taunt off
  }
	/if (${Debug} || ${Debug_Pets}) /echo <== summonPet -|
/RETURN

|------------------------------------------|
|- Automatically shrinks pets to min size	-|
SUB check_AutoShrink
/if (${Debug} || ${Debug_Pets}) /echo |- check_AutoShrink ==>
  /if (!${Defined[petShrink2D]}) /return
	/if (${Me.Pet.ID} && ${Me.Pet.Height}>1 && ${Me.Pet.Distance}) {
    /declare s int local
    /for s 1 to ${petShrink2D.Size[1]}
      /if (${check_Ready[petShrink2D,${s}]} && ${check_Mana[petShrink2D,${s}]} && ${Me.Pet.Distance} < ${petShrink2D[${s},${iMyRange}]}) {
        /call e3_Cast ${Me.Pet.ID} "petShrink2D" ${s}
      }
    /if (!${ActionTaken}) /next s
  }
/if (${Debug} || ${Debug_Pets}) /echo <== check_AutoShrink -|
/RETURN

|-------------------------------------------------|
|- Manually shrinks pets once.	 /bc Shrink Pets -|
#event shrinkPets "<#1#> Shrink Pets"
SUB event_shrinkPets(line, ChatSender)
/if (${Debug} || ${Debug_Pets}) /echo |- event_shrinkPets ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /call check_AutoShrink
/if (${Debug} || ${Debug_Pets}) /echo <== event_shrinkPets -|	
/RETURN

|------------------------------------------------------------|
|- Checks pet health, and attempts to cast PetHeal spells.	-|
SUB check_petHeal
/if (${Debug} || ${Debug_Pets}) /echo |- PetHeal ==>
	/if (${Me.Pet.ID}) {
    /declare petMended bool local FALSE
		| Check Mend Companion AA.
		/if (${Me.Pet.PctHPs} < ${petMendPct}) {
		  /if (${Me.AltAbilityReady[Replenish Companion]}) {
        /casting "Replenish Companion" "-targetid|${Me.Pet.ID}"
        /varset petMended TRUE
      } else /if (${Me.AltAbilityReady[Mend Companion]}) {
        /casting "Mend Companion" "-targetid|${Me.Pet.ID}"
        /varset petMended TRUE
      }
		}
    /if (!${Defined[petHeal2D]}) /return
		/if (!${petMended}) {
      /declare s int local
      /for s 1 to ${petHeal2D.Size[1]}
        /if (${Me.Pet.PctHPs} <= ${petHeal2D[${s},${iHealPct}]}) {
          /call check_Ready "petHeal2D" ${s}
          /if (${c_Ready}) {
            /if (${Me.Pet.Distance} < ${petHeal2D[${s},${iMyRange}]}) {
              /if (${check_Mana["petHeal2D",${s}]}) {
                /call e3_Cast ${Me.Pet.ID} "petHeal2D" ${s}
              }
            }
          }
        }
      /next s
    }
	}
/if (${Debug} || ${Debug_Pets}) /echo <== PetHeal -|
/RETURN

|------------------------------------------------------------------------|
|- Rebuff pet.															                            -|
SUB buffPet
/if (${Debug} || ${Debug_Pets}) /echo |- buffPet ==>
  /if (!${Defined[petBuffs2D]}) /return
	/declare s int local
	/for s 1 to ${petBuffs2D.Size[1]}
	|/echo ${petBuffs2D[${s},${iCastName}]} stack ${petBuffs2D[${s},${iSpellName}]} ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} has !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]}
	|removing ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} for now as balikor's mark reports non-stacking
  /if (!${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]} && !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iCheckFor}]}]}]} && !${Defined[nht${petBuffs2D[${s},${iCastID}]}]}) {
    /call check_Ready "petBuffs2D" ${s}
    /if (${c_Ready}) {
      /if (${Me.Pet.Distance} < ${petBuffs2D[${s},${iMyRange}]}) {
        /if (${check_Mana["petBuffs2D",${s}]}) {
          /if (${petBuffs2D[${s},${iSpellName}].NotEqual[Growl of the Panther]} || !${Bool[${Me.Song[Growl of the Panther]}]}) {
            /call e3_Cast ${Me.Pet.ID} "petBuffs2D" ${s}
            |/delay 5 ${Target.BuffsPopulated}
            |/if (${castReturn.Equal[CAST_SUCCESS]} && ${Me.Pet.ID} && !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]}) {
            |  /call CreateTimer "nht${petBuffs2D[${s},${iCastID}]}" "${noHoldDelay}"
            |  /if (${Debug} || ${Debug_Pets}) /bc [+y+] ${Time} ${petBuffs2D[${s},${iSpellName}]} not on Pet, creating NHT
            |}
          }
        }
      }
    }
  }
  :skipBuff
  /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_Pets}) /echo <== buffPet -|
/RETURN

|------------------------------------------------------------|
|- Suspend a pet. /bc Suspend Pets
#event suspendMinion "<#1#> Suspend Pets#*#"
SUB event_suspendMinion(live, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_suspendMinion ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (${Me.Pet.ID}) {
    /docommand ${ChatToggle} Suspending my pet...
    /if (${Me.AltAbilityReady[Suspended Minion]}) {
      /if (${Me.Casting.ID}) /call interrupt
      /casting "Suspended Minion|alt" -maxtries|2
      /delay 5s !${Me.Pet.ID}
      /varset currently_Suspended TRUE
    }
    /if (${Me.Pet.ID}) /pet get lost
    /if (${Defined[suspend_Pets]}) /varset suspend_Pets TRUE
  }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_suspendMinion -|
/RETURN

|------------------------------------------------------------|
|- Return suspended pet. /bc return Pets
#event returnMinion "<#1#> return Pets#*#"
SUB event_returnMinion(line, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_returnMinion ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (!${c_eventArg}) /return
    /if (${suspend_Pets}) {
      /docommand ${ChatToggle} Returning my pet...
      /if (${currently_Suspended}) {
        /casting "Suspended Minion|alt" -maxtries|2
        /varset currently_Suspended FALSE
      }
      /varset suspend_Pets FALSE
    }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_returnMinion -|
/RETURN

|------------------------------------------------------------------------|
|- Setup															                                  -|
Sub pet_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Pets (On/Off)" Debug_Pets bool outer
/if (${Debug} || ${Debug_Pets}) /echo |- pet_Setup ==>
	/if (${Ini[${Character_Ini},Pets].Length}) {
		/if (!${Defined[suspend_Pets]})        /declare suspend_Pets bool global FALSE
		/if (!${Defined[currently_Suspended]}) /declare currently_Suspended bool global FALSE
    /if (${Ini[${Character_Ini},Pets,Pet Mend (Pct)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Mend (Pct)" petMendPct int outer
    /if (${Ini[${Character_Ini},Pets,Pet Taunt (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Taunt (On/Off)" petTaunt bool outer
		/if (${Ini[${Character_Ini},Pets,Pet Auto-Shrink (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" petAutoShrink bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Summon Combat (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Summon Combat (On/Off)" petSummonCombat bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Buff Combat (On/Off)].Length}) /call iniToVarV "${Character_Ini},Pets,Pet Buff Combat (On/Off)" petBuffCombat bool outer
	}

	/call pet_spellArrays
/if (${Debug} || ${Debug_Pets}) /echo <== pet_Setup -|
/return

|------------------------------------------------------------------------|
|- spell setup
SUB pet_spellArrays
  /call IniToArrayV "${Character_Ini},Pets,Pet Spell#" petSpell
  /call IniToArrayV "${Character_Ini},Pets,Pet Buff#" petBuffs
  /call IniToArrayV "${Character_Ini},Pets,Pet Heal#" petHeal

  /if (${Defined[petSpell]}) /call BuildSpellArray "petSpell"	"petSpell2D"

  /if (${Defined[petBuffs]}) {
    /varset BuffCheck TRUE
    /call BuildSpellArray "petBuffs" "petBuffs2D"
  }
  /if (${Defined[petHeal]})   /call BuildSpellArray "petHeal"	"petHeal2D"

  /if (${FindItemCount[=Gemstone of Dark Flame]}) {
    /call BuildArray petShrink "Gemstone of Dark Flame"
  } else /if (${FindItemCount[=Symbol of Ancient Summoning]}) {
    /call BuildArray petShrink "Symbol of Ancient Summoning"
  } else /if (${Me.Book[Tiny Companion]}) {
    /call BuildArray petShrink "Tiny Companion"
  }
  /if (${Defined[petShrink]}) /call BuildSpellArray "petShrink"	"petShrink2D"

/RETURN

|------------------------------------------------------------------------|
SUB pet_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug Pets (On/Off)" Off
/RETURN

|------------------------------------------------------------------------|
SUB pet_CharacterSettings
/if (${Debug} || ${Debug_Pets}) /echo |- pet_CharacterSettings ==>
	/if (${Select[${Me.Class.ShortName},BST,ENC,MAG,NEC,SHD,SHM]}) {	
		/call WriteToIni "${Character_Ini},Pets,Pet Spell"
    /call WriteToIni "${Character_Ini},Pets,Pet Heal"
		/call WriteToIni "${Character_Ini},Pets,Pet Buff"
    /if (${Select[${Me.Class.ShortName},BST,MAG,NEC]}) /call WriteToIni "${Character_Ini},Pets,Pet Mend (Pct)"
    /call WriteToIni "${Character_Ini},Pets,Pet Taunt (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Summon Combat (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Buff Combat (On/Off)" Off
	}
/if (${Debug} || ${Debug_Pets}) /echo <== pet_CharacterSettings -|
/RETURN

|------------------------------------------------------------------------|
Sub pet_Aliases
	/squelch /alias /SuspendPets /bc Suspend Pets
	/squelch /alias /ReturnPets /bc Return Pets
	/squelch /alias /ShrinkPets /bc Shrink Pets
/return

|------------------------------------------------------------------------|
Sub pet_Background_Events
  /doevents suspendMinion
  /doevents returnMinion
  /doevents shrinkPets
/return