|----------------------------------------------------------------------------|
|- e3_Classes_Cleric.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
|- Contains Cleric specific functions related to,							-|
|-		Setup and Ini entry information										-|
|-		MainLoop, and AssistLoop hooks										-|
|-		Over-all functionality / flow										-|
|----------------------------------------------------------------------------|



SUB check_Yaulp
/if (${Debug}) /echo |- check_clrYaulp ==>
	/if (${AutoYaulp} && !${medBreak})  {
		/declare castName string local ${yaulpSpell.Arg[1,/]}
		| Auto-yaulp spell
		/if (!${Bool[${Me.Buff[${castName}]}]} && ${Me.PctMana} < 95 && ${Spell[${castName}].Stacks}) {
			/if (${Target.ID}) /declare tempTarget int local ${Target.ID}
			/casting "${castName}"
		}
	}
/if (${Debug}) /echo <== check_clrYaulp -|
/RETURN

|----------------------------------------------------------------------------|
sub check_DivineArb
/if (${Debug}) /echo |- check_DivineArb ==>
	/if (${DivineArb_Pct} && ${Group}>1 ) {
		/declare i int local
		/for i 0 to ${Group}
			/if (${SpawnCount[pc ${Group.Member[${i}]}]}) {
				/if (${SpawnCount[ID ${Spawn[${Group.Member[${g}]}].ID} radius 70]} > 0 &&  ${NetBots[${Group.Member[${g}]}].Zone} == ${Zone.ID} && (${NetBots[${Group.Member[${i}]}].PctHPs} < ${DivineArb_Pct} || ${Spawn[pc ${Group.Member[${i}]}].PctHPs} < ${DivineArb_Pct})) {
					/if (${NetBots[${Group.Member[${i}]}].PctHPs}<${DivineArb_Pct}) {
						/if (${FindItemCount[Harmony of the Soul]} && ${FindItem[Harmony of the Soul].Timer}==0) {
              /casting "Harmony of the Soul|item"
						} else /if (${Me.AltAbilityReady[Divine Arbitration]}) {
              /casting "Divine Arbitration|alt"
						}
					}
				}
			}
		/if (!${ActionTaken}) /next i
	}
/if (${Debug}) /echo <== check_DivineArb -|
/return

|----------------------------------------------------------------------------|
sub check_celestialRegen
/if (${Debug}) /echo |- check_celestialRegen ==>
		/if (${celestialRegen_Pct} && ${Group}>2 && ${Me.AltAbilityReady[Celestial Regeneration]}) {
		/declare groupSize int local
		/declare averageHP int local
		/declare i int local
		| Count group members in zone
		/for i 0 to ${Group}
			/if (${SpawnCount[pc ${Group.Member[${i}]}]}) {
				/varcalc averageHP ${averageHP} + ${If[${NetBots[${Group.Member[${i}]}].ID},${NetBots[${Group.Member[${i}]}].PctHPs},${Group.Member[${i}].PctHPs}]}
				/varcalc groupSize ${groupSize} + 1
			}
		/next i
		
		/if (${groupSize}) {
			/varcalc averageHP ${averageHP} / ${groupSize}
			/if (${averageHP} < ${celestialRegen_Pct})  /casting "Celestial Regeneration|alt" "-targetid|${Me.ID}" "-maxtries|2"
		}
	}
/if (${Debug}) /echo <== check_celestialRegen -|
/return

|----------------------------------------------------------------------------|
#event Rez "<#1#> Rez"
#event Rez "<#*#> Rez On #1#"
#event Rez "#1# tells you, 'Rez'"
#event Rez "#*# tells you, 'Rez On #1#'"
#event Rez "#1# tells the group, 'Rez'"
sub Event_Rez(string line, string ChatSender)
  /call defineRezSpell
  |/echo ${rezArray2D.Size[1]} ${rezArray2D[1,${iCastName}]}  ${rezArray2D[1,${iCastType}]}
  |/return
	/if (${ChatSender.Left[1].Compare[ ]}<0) /varset ChatSender ${ChatSender.Right[-2].Left[-1]}
	/if ( ${Me.Class.ShortName.Equal[CLR]} && ${Spawn[${ChatSender}'s].ID}) {
	  /varset canRez 0
    /call defineRezSpell
    /delay 15s ${Bool[${Cast.Status.Find[I]}]}
    /call TrueTarget ${Spawn[${ChatSender}'s].ID}
    /echo ${Target.ID}
    /call check_CorpseRezzable
    /echo checkcanrez ${Macro.Return}
    /if (${canRez}==1) {
      /call check_Ready "rezArray2D" 1
      /if (${Macro.Return}) {
        /tell ${ChatSender} Wait4Rez
        /call e3_cast ${Spawn[${ChatSender}'s].ID} "rezArray2D" 1
      } else {
        /echo ${rezArray2D[1,${iCastName}]} is not ready to cast
      }
    } else {
      /tell ${ChatSender} Your corpse ${Target.Name} is not eligible for rez
    }
	} else /if (!${Spawn[${ChatSender}'s].ID}) {
		/tell ${ChatSender} I don't see any of your corpses in ${Zone}.
	}
/return


|----------------------------------------------------------------------------|
#EVENT AE_Rez "[MQ2] AE Rez"
#EVENT AE_Rez "#1# tells you, 'AERez'"
SUB EVENT_AE_Rez(string line, string ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_AE_Rez ==>

  /call BuildCorpseList

	/if (!${CorpseListSize}) {
		/echo I couldn't find any corpses to ressurect...
	} else {
    /if (${NetAdvPath.Status}==1) /squelch /netfollow off
    /if (${Stick.Active}) /squelch /stick off

    /declare numIterations int local 3
    /declare i int local
    /declare n int local
		/declare playerName string local
		/declare corpseID int local

    /for n 1 to ${numIterations}
      /varset otherRezzing
      /for i 1 to ${CorpseListSize}
        /varset canRez 0
        /varset corpseID ${CorpseListPriority[${i}]}
        /varset playerName ${Spawn[id ${corpseID}].CleanName}
        /varset playerName ${playerName.Arg[1,']}

        /call defineRezSpell
        /call TrueTarget ${corpseID}
        /call check_CorpseRezzable
        |/echo ${playerName} cid${corpseID} canrez${canRez} dist${Target.Distance}

        /if (${canRez}!=1) {
          /echo ${Target.Name} is not eligible for rez
          /next i
        } else {
          /tell ${playerName} Wait4Rez
          /if (${Target.Distance} > 35 && ${Target.Distance} < 100) {
            /corpse
            /delay 4
            /corpse
            /delay 4
            /if (${Target.Distance} > 35) {
              /tell ${playerName} skipping rez, corpse drag failed: ${Target.Distance} distance
              /goto :skipRez
            }
          }
          |check if anyone else has rezd the corpse this iteration
          /doevents check_otherRezzing
          /delay 1s
          |/echo or${otherRezzing} cid${corpseID}-
          /if (${Select[${otherRezzing},${corpseID}-]}) {
            /echo Someone is already rezzing ${playerName} ${corpseID}
            /goto :skipRez
          }
          |corpse can be rezd and is in range
          /call check_Ready "rezArray2D" 1
          /if (${Macro.Return}) {
            /delay 2
            /bc Resurrecting-${corpseID}
            /delay 2
            /call e3_cast ${corpseID} "rezArray2D" 1
          } else {
            /echo ${rezArray2D[1,${iCastName}]} is not ready to cast
          }
        }

      :skipRez
    /next i
  /next n
  /docommand ${ChatToggle} All viable corpses nearby, have been ressurected.
 }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_AE_Rez -|
/RETURN

|----------------------------------------------------------------------------|
#event CorpseExpired "This corpse #1# be resurrected."
Sub Event_CorpseExpired(line, corpseExpired)
/if (${Debug}) /echo |- EVENT_CorpseExpired
  /if (!${corpseExpired.Equal[cannot]}) {
    /varset canRez 1
  } else  {
    /varset canRez 2
  }
/return !${corpseExpired.Equal[cannot]}

|----------------------------------------------------------------------------|
#event check_otherRezzing "<#1#> Resurrecting-#2#"
SUB event_check_otherRezzing(line, ChatSender,corpseID)
  /if (${Me.Name.NotEqual[${ChatSender}]}) /varset otherRezzing ${otherRezzing}${corpseID}-
  /echo ${line} ${ChatSender} ${corpseID} ${Target.ID} ${otherRezzing}
/RETURN ${otherRezzing}

|----------------------------------------------------------------------------|
Sub check_CorpseRezzable
  /declare conTimer timer local 5s
  /declare returnResult bool local FALSE
  /declare corpseOwner string local
  /consider
  :waitForExpired
  /doevents CorpseExpired
  /if (${canRez}==0 && ${conTimer}) {
    /goto :waitForExpired
  }
  /if (${canRez}==1) /varset returnResult TRUE

|  /varset corpseOwner ${Target.CleanName}
|  /varset corpseOwner ${corpseOwner.Arg[1,']}
  |if corpse is a netbot, that netbot is targeting its corpse and looting, then dont try to rez
|  /if (${NetBots[${corpseOwner}]} && ${NetBots[${corpseOwner}].TargetID}==${Target.ID} && ${NetBots[${corpseOwner}].State.Equal[BIND]}) {
|    /varset canRez 2
|  }
|  /echo check_rezzable ${canRez}
/RETURN ${returnResult}

|----------------------------------------------------------------------------|
Sub BuildCorpseList
  /declare c int local
  /declare p int local 1
  /declare cID int local
  /declare CorpseList[100] int local 0
  /if (${Defined[CorpseListPriority]}) /deletevar CorpseListPriority
  /declare CorpseListPriority[100] int outer 0
  /varset CorpseListSize 0
  /if (!${SpawnCount[pccorpse radius 100]}) /return
  /for c 1 to ${SpawnCount[pccorpse radius 100]}
    /varset cID ${NearestSpawn[${c},pccorpse radius 100].ID}
      /varcalc CorpseListSize ${CorpseListSize}+1
      /varset CorpseList[${CorpseListSize}] ${cID}
  /next c

  |add clr
  /for c 1 to ${CorpseListSize}
    /if (${Spawn[id ${CorpseList[${c}]}].Class.ShortName.Equal[CLR]}) {
      /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
      /varcalc p ${p}+1
    }
  /next c

  |add war/dru/shm
  /for c 1 to ${CorpseListSize}
  /if (${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName},DRU,SHM,WAR]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c

  |add bst,enc,pal,rng
  /for c 1 to ${CorpseListSize}
  /if (${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName},BST,ENC,PAL,RNG]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c

  |add the rest
  /for c 1 to ${CorpseListSize}
  /if (!${Select[${Spawn[id ${CorpseList[${c}]}].Class.ShortName.Equal[CLR]},CLR,DRU,SHM,WAR,BST,ENC,PAL,RNG]}) {
    /varset CorpseListPriority[${p}] ${CorpseList[${c}]}
    /varcalc p ${p}+1
  }
  /next c

|  /for c 1 to ${CorpseListSize}
|    /echo c ${c}:  cl-${CorpseList[${c}]}-${Spawn[id ${CorpseList[${c}]}].Name} clp ${CorpseListPriority[${c}]}-${Spawn[id ${CorpseListPriority[${c}]}].Name}
|  /next c
/return

|----------------------------------------------------------------------------|
Sub defineRezSpell
  /declare rezSpell string local
  /if (${FindItem[Water Sprinkler of Nem Ankh].ID}) {
    /varset rezSpell Water Sprinkler of Nem Ankh
  } else /if (${Me.AltAbility[Blessing of Resurrection].ID}) {
    /varset rezSpell Blessing of Resurrection
  } else /if (${Me.Book[Reviviscence]}) {
    /varset rezSpell Reviviscence
  } else /if (${Me.Book[Ressurection]}) {
    /varset rezSpell Ressurection
  } else /if (${Me.Book[Restoration]}) {
    /varset rezSpell Restoration
  } else /if (${Me.Book[Resuscitate]}) {
    /varset rezSpell Resuscitate
  } else /if (${Me.Book[Renewal]}) {
    /varset rezSpell Renewal
  } else /if (${Me.Book[Revive]}) {
    /varset rezSpell Revive
  } else /if (${Me.Book[Reparation]}) {
    /varset rezSpell Reparation
  } else /if (${Me.Book[Reconstitution]}) {
    /varset rezSpell Reconstitution
  } else /if (${Me.Book[Reanimation]}) {
    /varset rezSpell Reanimation
  }
  /if (${Bool[${rezSpell}]}) {
    /declare rezArray[1] string outer ${rezSpell}
    /if (${rezArray.Size}) {
      /call BuildSpellArray "rezArray" "rezArray2D"
      |/varset rezArray2D[1,${iSubToRun}] check_CorpseRezzable
      }
  } else {
    /bc I do not have a resurrection spell available
  }
/RETURN

|----------------------------------------------------------------------------|
Sub CLR_Setup
/if (${Debug}) /echo |- CLR_Setup ==>
  | 0 unknown/event not finished
  | 1 can rez
  | 2 cannot rez
  /declare canRez int outer 0
  /declare CorpseListSize int outer 0
  /declare otherRezzing string outer
	/call InitoVar "${Character_Ini},Cleric,Divine Arbitration (Pct)" DivineArb_Pct int outer
	/call InitoVar "${Character_Ini},Cleric,Celestial Regeneration (Pct)" celestialRegen_Pct int outer
	/call InitoVar "${Character_Ini},Cleric,Auto-Yaulp (On/Off)" AutoYaulp bool outer
	/call InitoVar "${Character_Ini},Cleric,Yaulp Spell" yaulpSpell string outer
	/if (${AutoYaulp}) {
		/if (!${Defined[yaulpSpell]} || !${yaulpSpell.Length} || !${Me.Book[${yaulpSpell.Arg[1,/]}]}) {
			/echo ERROR! ${Character_Ini} > [Cleric] > Yaulp Spell is not correctly defined.  Please check this entry, and restart the macro.
			/varset AutoYaulp FALSE
		}
	}
/if (${Debug}) /echo <== CLR_Setup -|
/return

|----------------------------------------------------------------------------|
SUB CLR_MacroSettings
/if (${Debug}) /echo |- CLR_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#1" "check_DivineArb" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#2" "check_Yaulp" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#3" "check_celestialRegen" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#4" "check_healChain" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#5" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#6" "check_Cures" 1
  /call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#7" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#8" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},CLR Functions,CLR Function#9" "check_Nukes" 1
/if (${Debug}) /echo <== CLR_MacroSettings -|
/RETURN

|----------------------------------------------------------------------------|
SUB CLR_CharacterSettings
/if (${Debug}) /echo |- CLR_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Cleric,Divine Arbitration (Pct)" 35 0
	/call WriteToIni "${Character_Ini},Cleric,Celestial Regeneration (Pct)" 0 0
	/call WriteToIni "${Character_Ini},Cleric,Auto-Yaulp (On/Off)" Off 0
	/call WriteToIni "${Character_Ini},Cleric,Yaulp Spell"
/if (${Debug}) /echo <== CLR_CharacterSettings -|
/RETURN

|----------------------------------------------------------------------------|
Sub CLR_Aliases
/if (${Debug}) /echo |- CLR_Aliases ==>
	/noparse /squelch /alias /RezIt /bc Rez on ${Target.CleanName.Arg[1,']}
	/squelch /alias /AERez /echo AE Rez
/if (${Debug}) /echo <== CLR_Aliases -|
/return

|----------------------------------------------------------------------------|
#event AE_CR "#*# tells you, 'AECR'"
sub Event_AE_CR(string line)
/if (${Me.Class.ShortName.Equal[CLR]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Celestial Regeneration]}) {
      /bc MGB Celestial Regeneration inc...
      /rs MGB Celestial Regeneration inc...
     /casting "Mass Group Buff|alt" "-maxtries|3"
     /casting "Celestial Regeneration|alt" "-maxtries|3"
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Celestial Regeneration is not available...
}
/return

Sub CLR_Background_Events
  /doevents AE_CR
  /doevents AE_Rez
  /doevents Rez
/return