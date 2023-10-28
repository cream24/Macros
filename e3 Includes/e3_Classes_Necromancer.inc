|----------------------------------------------------------------------------|
| Necromancer Functions
|----------------------------------------------------------------------------|
SUB check_Lich
  /if (${Debug}) /echo |- check_castLich ==>
  |/echo in checklich  ${lichSpell2D[1,${iMinHP}]}, ${Me.PctHPs} ${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}
  /if (${Defined[lichSpell2D]}) {
    /if (${Me.PctHPs} <= 10) {
      /if (${Bool[${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}]}) {
        /echo Removing Lich due to low HP
        /removebuff ${lichSpell2D[1,${iSpellName}]}
      }
    } else {
      /if (!${Bool[${Me.Buff[${lichSpell2D[1,${iSpellName}]}]}]} && !${Bool[${Me.Buff[${lichSpell2D[1,${iCheckFor}]}]}]}) {
        /if (${Spell[${lichSpell2D[1,${iSpellName}]}].Stacks} && ${Spell[${lichSpell2D[1,${iSpellName}]}].NewStacks}) {
          /if (${check_Mana["lichSpell2D",1]}) {
            /call check_Ready "lichSpell2D" 1
            /if (${c_Ready}) {
            |/if (${check_Ready["lichSpell2D",1]}) {
              /if (${lichSpell2D[1,${iMinHP}]} <= ${Me.PctHPs} && ${Me.CurrentHPs} > 100) {
                /call e3_Cast ${Me.ID} "lichSpell2D" 1
              }
            }
          }
        }
      }
    }
  }
  /if (${Debug}) /echo <== check_castLich -|
/return
|----------------------------------------------------------------------------|
SUB check_manaDump
/if (${Debug}) /echo |- check_manaDump ==>
	/if (${manaDump} && !${medBreak} && !${Following}) {
		/declare manaDump_Target string local
		/declare manaDumpSpellIndex int local 0
    /declare i int local
    /declare s int local

    /for s 1 to ${manaDump_Spells2D.Size[1]}
      /call check_Ready "manaDump_Spells2D" ${s}
      /if (${c_Ready}) {
        /if (${check_Mana["manaDump_Spells2D",${s}]}) {
      		/varset manaDumpSpellIndex ${s}
				}
			}
    /if (${manaDumpSpellIndex}==0) /next s

    /if (${manaDumpSpellIndex} > 0 ) {
      /for i 1 to ${whoto_manaDump.Size}
        /if (${check_Distance[${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID},${manaDump_Spells2D[${manaDumpSpellIndex},${iMyRange}]}]}) {
        	/if (${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].PctMana} <= ${whoto_manaDump[${i}].Arg[2,/]}) {
          	/call e3_Cast ${NetBots[${whoto_manaDump[${i}].Arg[1,/]}].ID} "manaDump_Spells2D" ${manaDumpSpellIndex}
          }
				}
			/if (!${ActionTaken}) /next i
    }
  }
/if (${Debug}) /echo <== check_manaDump -|
/return

|----------------------------------------------------------------------------|
#event AE_DMF "#*# tells you, 'AEDMF'"
sub Event_AE_DMF(string line)
/if (${Me.Class.ShortName.Equal[NEC]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Bool[${Me.Book[Dead Men Floating]}]}) {
     /bc MGB Dead Men Floating inc...
     /casting "Mass Group Buff|alt" "-maxtries|3"
     /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
     /delay 5
     /casting "Dead Men Floating" "-maxtries|3"
     /delay 5 !${Me.SpellReady[Dead Men Floating]}
     /casting "Dead Men Floating" "-maxtries|3"
     /rs MGB Dead Men Floating inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Dead Men Floating is not in your Spell Book, Go and buy it fool...
}
/return

|----------------------------------------------------------------------------|
Sub NEC_Setup
/if (${Debug}) /echo |- NEC_Setup ==>
	/call IniToArrayV "${Character_Ini},${Me.Class},Lich Spell" lichSpell
	/call IniToArrayV "${Character_Ini},${Me.Class},LifeTap" lifeTaps
  /call IniToArrayV "${Character_Ini},${Me.Class},Mana Dump" manaDump_Spells
  /call IniToArrayV "${Character_Ini},${Me.Class},Who to Mana Dump" whoto_manaDump
  /call iniToVarV "${Character_Ini},${Me.Class},Mana Dump (On/Off)" manaDump bool

  /if (${Defined[lichSpell]})       /call BuildSpellArray "lichSpell"	"lichSpell2D"
  /if (${Defined[lifeTaps]})        /call BuildSpellArray "lifeTaps"	"lifeTaps2D"
  /if (${Defined[manaDump_Spells]}) /call BuildSpellArray "manaDump_Spells"	"manaDump_Spells2D"

/if (${Debug}) /echo <== NEC_Setup -|
/return
|----------------------------------------------------------------------------|
SUB NEC_MacroSettings
/if (${Debug}) /echo |- NEC_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Lich" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_manaDump" 1
  /call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_lifeTap" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},NEC Functions,NEC Function" "check_Pets" 1
/if (${Debug}) /echo <== NEC_MacroSettings -|
/return
|----------------------------------------------------------------------------|
SUB NEC_CharacterSettings
/if (${Debug}) /echo |- NEC_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Lich Spell"
	/call WriteToIni "${Character_Ini},${Me.Class},LifeTap"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump (On/Off)"
  |/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump Engage Pct" 70
	/call WriteToIni "${Character_Ini},${Me.Class},Who to Mana Dump"
	/call WriteToIni "${Character_Ini},${Me.Class},Mana Dump"
/if (${Debug}) /echo <== NEC_CharacterSettings -|
/return
|----------------------------------------------------------------------------|
Sub NEC_Aliases
/if (${Debug}) /echo |- NEC_Aliases ==>
	/noparse /squelch /alias /manadump /bc Mana dump
/if (${Debug}) /echo <== NEC_Aliases -|
/return
|----------------------------------------------------------------------------|
Sub NEC_Background_Events
  /doevents AE_DMF
/return