|--------------------------------------------------------------------------------|
|- e3_Classes_Necromancer.inc v4.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ													-|
|--------------------------------------------------------------------------------|

SUB check_Lich
/if (${Debug}) /echo |- check_castLich ==>
	/if (${Defined[lichSpell]} && !${Me.Buff[${lichSpell}].ID} && ${Me.PctMana} < 90 && ${Spell[${lichSpell}].Stacks} && ${Me.PctHPs} > 80) /casting "${lichSpell}"
	/delay 5s !${Me.Casting.ID}
  /if (${Me.Buff[${lichSpell}].ID} && ${Me.PctHPs} < 20 && ${Me.CombatState.NotEqual[COMBAT]})  /removebuff ${lichSpell}
/if (${Debug}) /echo <== check_castLich -|
/RETURN

SUB check_manaDump
/if (${Debug}) /echo |- check_manaDump ==>
	/if (${manaDump} && !${medBreak}) {
		/declare manaDump_Target string local
		/declare manaDumpSpellIndex int local 0
    /declare i int local
    /declare s int local

    /for s 1 to ${manaDump_Spells2D.Size[1]}
      /call check_Ready "manaDump_Spells2D" ${s}
      /if (${Macro.Return}) {
      	/call check_Mana "manaDump_Spells2D" ${s}
				/if (${Macro.Return}) {
      		/varset manaDumpSpellIndex ${s}
				}
			}
    /if (${manaDumpSpellIndex}==0) /next s

    /if (${manaDumpSpellIndex} > 0 ) {
      /for i 1 to ${whoto_manaDump.Size}
        /call check_Distance ${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID} ${manaDump_Spells2D[${manaDumpSpellIndex},${iMyRange}]}
        /if (${Macro.Return}) {
        	/if (${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].PctMana} <= ${whoto_manaDump[${i}].Arg[2,/]}) {
          	/call e3_Cast ${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID} "manaDump_Spells2D" ${manaDumpSpellIndex}
          }
				}
			/if (!${ActionTaken}) /next i
    }
  }
/if (${Debug}) /echo <== check_manaDump -|
/RETURN

Sub NEC_Setup
/if (${Debug}) /echo |- NEC_Setup ==>
	/if (${Ini[${Character_Ini},${Me.Class},Lich Spell].Length}) /call IniToVar "${Character_Ini},${Me.Class},Lich Spell" lichSpell string outer
	/if (${Ini[${Character_Ini},${Me.Class},LifeTap#1].Length}) /call IniToArray "${Character_Ini},${Me.Class},LifeTap#" lifeTaps
	/if (${lifeTaps.Size}) /call BuildSpellArray "lifeTaps"	"lifeTaps2D"
	/if (${Ini[${Character_Ini},${Me.Class},Mana Dump (On/Off)].Length}) /call IniToVar "${Character_Ini},${Me.Class},Mana Dump (On/Off)" manaDump bool outer
  /if (${Ini[${Character_Ini},${Me.Class},Who to Mana Dump#1].Length}) /call IniToArray "${Character_Ini},${Me.Class},Who to Mana Dump#" whoto_manaDump
	/if (${Ini[${Character_Ini},${Me.Class},Mana Dump#1].Length})        /call IniToArray "${Character_Ini},${Me.Class},Mana Dump#" manaDump_Spells
  /if (${manaDump_Spells.Size}) /call BuildSpellArray "manaDump_Spells"	"manaDump_Spells2D"
/if (${Debug}) /echo <== NEC_Setup -|
/return

SUB NEC_MacroSettings
/if (${Debug}) /echo |- NEC_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#2" "check_Lich" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#3" "check_manaDump" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#4" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#5" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#6" "check_lifeTap" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#7" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#8" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function#9" "check_Pets" 1
/if (${Debug}) /echo <== NEC_MacroSettings -|
/RETURN

SUB NEC_CharacterSettings
/if (${Debug}) /echo |- NEC_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Lich Spell"
	/call WriteToIni "${Character_Ini},${Me.Class},LifeTap#1"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump (On/Off)"
  |/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump Engage Pct" 70
	/call WriteToIni "${Character_Ini},${Me.Class},Who to Mana Dump#1"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump#1"
/if (${Debug}) /echo <== NEC_CharacterSettings -|
/RETURN

Sub NEC_Aliases
/if (${Debug}) /echo |- NEC_Aliases ==>
	/noparse /squelch /alias /manadump /bc Mana dump
/if (${Debug}) /echo <== NEC_Aliases -|
/return

Sub NEC_Background_Events
/return