|----------------------------------------------------------------------------|
| Magician Functions
|----------------------------------------------------------------------------|
#event petWeapons "#1# tells you, 'Pet Weapon#*#'"
#event petWeapons "#1# tells you, 'Distribute Pet Weapon#*#'"
#event petWeapons "<#1#> Pet Weapon#*#"
#event petWeapons "<#1#> Distribute Pet Weapon#*#"
sub event_petWeapons(line, ChatSender)
/if (${Debug} || ${Debug_MAG}) /echo |- event_petWeapons ==>
  /declare wasFollowing bool local ${Following}
  /varset Following FALSE
  |/varset Debug_MAG TRUE
  /declare singlePet bool local TRUE
  /if (${Defined[petIDArray]}) /deletevar petIDArray
  /if (${Bool[${line.Find[Distribute]}]}) /varset singlePet FALSE

  /if (!${checkEventArgs[${ChatSender},${line},ZR]}) /return
  /if (!${Defined[petItems2D]}) {
    /if (${Debug} || ${Debug_MAG}) /echo I do not have any Pet Items defined...
    /if (${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} I do not have any Pet Items defined...
  } else /if (!${SpawnCount[${c_eventArgChatSender}]}) {
    /echo ${c_eventArgChatSender} is not in the zone
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} We are not in the same zone...
  } else /if (!${singlePet} && !${SpawnCount[pcpet radius 100]}) {
      /echo There are no PC Pets withing a radius of 100
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} No pets a radius of 100...
  } else /if (${singlePet} && ${Spawn[${c_eventArgChatSender}].Pet.Distance}>100) {
      /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Your pet is not in a radius of 100...
  } else {
    /if (!${singlePet} && ${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Handing out pet weapons...
    /declare originalLoc string local ${Me.Loc}
    /declare p int local
    |if distribute, give to everyone, else just give to the single chatsender
    /for p 1 to ${SpawnCount[pcpet radius 100]}
      |/echo ${p} ${NearestSpawn[${p},pcpet]} ${NearestSpawn[${p},pcpet].Name} ${NearestSpawn[${p},pcpet].Master} ${c_eventArgChatSender}
      |/echo los ${NearestSpawn[${p},pcpet].LineOfSight} d ${Bool[${line.Find[Distribute]}]}
      /if (${NearestSpawn[${p},pcpet].LineOfSight}) {
        /if (!${singlePet}) {
          /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
          |/echo all added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
        } else /if (${NearestSpawn[${p},pcpet].Master.CleanName.Equal[${c_eventArgChatSender}]}) {
          /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
          |/echo you added ${NearestSpawn[${p},pcpet].Name}-${NearestSpawn[${p},pcpet].Master} ${petIDArray.Size}
        }
      }
    /next p
    /if (${Defined[petIDArray]}) {
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Status}==1) /squelch /netfollow off
      /call give_PetsWeapons FALSE
    }

    /if (${Math.Distance[${originalLoc}]}>20) /call MoveToLoc ${originalLoc} 50
    /if (${c_eventArgChatSender.NotEqual[${Me}]}) /tell ${c_eventArgChatSender} Finished arming pets
    /varset Following ${wasFollowing}
  }
  |/varset Debug_MAG FALSE
/if (${Debug} || ${Debug_MAG}) /echo <== event_petWeapons -|
/return
|----------------------------------------------------------------------------|
SUB give_PetsWeapons(autoWeap)
  /declare p int local
  /declare w int local
  /declare allWeapSuccess bool local
  /declare retryWeapTimer timer local 10s

  /for p 1 to ${petIDArray.Size}
    |/echo ${p}-${petIDArray[${p}]}-${Spawn[id ${petIDArray[${p}]}].Name}-${Spawn[id ${petIDArray[${p}]}].Master}
    /if (${SpawnCount[id ${petIDArray[${p}]}]}) {
      /varset allWeapSuccess TRUE
      /if (${Spawn[id ${petIDArray[${p}]}].Master.CleanName.NotEqual[${Me}]}) /tell ${Spawn[id ${petIDArray[${p}]}].Master} Incoming pet weapons, hold still!
      /call TrueTarget ${petIDArray[${p}]}
      /if (${Target.ID} && ${Target.Distance}>19) /call MoveToLoc ${Target.Y} ${Target.X} 50 15
        /for w 1 to ${petItems2D.Size[1]}
          :SummonWeap_Loop
          /if (${petItems2D[${w},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItemCount[=Summoned: Burning Shank]}) {
            /call SwapItem "Summoned: Burning Shank" "Cursor"
          } else /if (${FindItemCount[=${petItems2D[${w},${iCastName}].Replace[Summon,Summoned]}]}) {
            /call SwapItem "${petItems2D[${w},${iCastName}].Replace[Summon,Summoned]}" "Cursor"
          } else {
            /call check_Ready "petItems2D" ${w}
            /if (${c_Ready}) {
              /if (${check_Mana["petItems2D",${w}]}) {
                /call e3_Cast ${Me.ID} "petItems2D" ${w}
                /delay 5s ${Cursor.ID}
              }
            }
          }
          /if (!${Cursor.ID}) {
            /if (${retryWeapTimer}) {
              /goto :SummonWeap_Loop
            } else {
              /varset allWeapSuccess FALSE
            }
          } else /if (${Cursor.ID}) {
            /call TrueTarget ${petIDArray[${p}]}
            :OpenTrade_Loop
            /click left target
            /delay 1s ${Window[GiveWnd].Open}
            /if (!${Window[GiveWnd].Open}) {
              /if (${retryWeapTimer} && ${Cursor.ID}) {
                /goto :OpenTrade_Loop
              } else {
                /echo Failed to open trade with ${Target.CleanName}.
              }
            } else {
              :WaitAccept_Loop
              /notify GiveWnd GVW_Give_Button LeftMouseUp
              /delay 1s !${Window[GiveWnd].Open}
              /if (${Window[GiveWnd].Open}) {
                /if (${retryWeapTimer}) {
                  /goto :WaitAccept_Loop
                } else {
                  /echo Failed to open trade with ${Target.CleanName}.
                }
              }
            }
          }
        /next w
      /if (${autoWeap} && ${allWeapSuccess}) {
        /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${Spawn[id ${petIDArray[${p}]}].Master}" "${Spawn[id ${petIDArray[${p}]}].CleanName}" 1
      }
    }
  /next p
/return
|----------------------------------------------------------------------------|
SUB check_SummonItems
/if (${Debug} || ${Debug_MAG}) /echo |- check_SummonItems ==>
  /if (${summonMoltenOrb}) {
    /if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
        /if (!${FindItemCount[=Molten Orb]}) {
          /call check_Ready "moltenOrb2D" 1
          /if (${c_Ready}) {
            /if (${check_Mana["moltenOrb2D",1]}) {
              /call e3_Cast ${Me.ID} "moltenOrb2D" 1
              /delay 2s ${Cursor.ID}
              /if (${Cursor.ID}) /call ClearCursor
            }
          }
        }
    }
  }

	/if (${autoWeapons} && ${Macro.RunTime} > 10) {
		/if (!${Me.Moving} && !${combatTimer} && !${activeTimer} && !${rebuffTimer}) {
			/declare i int local
      /declare p int local
      /declare masterName string local
      /declare petName string local
      /declare petID int local
      /if (${Defined[petIDArray]}) /deletevar petIDArray
      /for p 1 to ${SpawnCount[pcpet radius 100]}
        /varset masterName ${NearestSpawn[${p},pcpet].Master}
        /varset petName ${NearestSpawn[${p},pcpet].CleanName}
        /varset petID ${NearestSpawn[${p},pcpet].ID}
        /if (!${Bool[${petName}]} || !${Bool[${masterName}]}) /return
        /if (${NetBots[${masterName}].ID}) {
          /if (${Spawn[id ${petID}].LineOfSight}) {
            /if (${petName.Equal[${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${masterName}]}]}) {
						  /if (${Debug} || ${Debug_MAG}) /echo check_SummonItems: I have already given weapons to ${petName}
					  } else {
              /call BuildArray petIDArray ${NearestSpawn[${p},pcpet].ID}
					  }
				  }
		    }
		  /next p
		  /if (${Defined[petIDArray]}) {
        /call give_PetsWeapons TRUE
      }
	  }
	}
/if (${Debug} || ${Debug_MAG}) /echo <== check_SummonItems -|
/RETURN
|----------------------------------------------------------------------------|
#event auto_coh "#1# tells you, 'autocoh #2#'"
SUB event_auto_coh(line, ChatSender, toggleCoH)
  /declare i int local
  /if (${toggleCoH.Equal[on]}) {
    /if (${Bool[${Group.Member[${ChatSender}]}]}) {
      /if (${Defined[cohTargets]}) {
        /for i 1 to ${cohTargets.Size}
          /if (${cohTargets[${i}].Equal[${ChatSender}]}) {
            /docommand ${ChatToggle} ${ChatSender} is already in the AutoCoH array
            /return
          }
        /next i
      }
      /call BuildArray cohTargets "${ChatSender}"
      /docommand ${ChatToggle} Auto CoH Enabled on ${ChatSender}
    } else {
      /docommand ${ChatToggle} Auto CoH will not be enabled until you are in a group with this mage
    }
  } else {
    /if (${Defined[cohTargets]}) /call RemoveArrayElement cohTargets "${ChatSender}"
    /docommand ${ChatToggle} Auto CoH Disabled on ${ChatSender}
  }
/RETURN
|----------------------------------------------------------------------------|
SUB check_CoH
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_checkCoH ==>
  /if (${Defined[cohTargets]}) {
    /declare t int local
    /declare s int local
    /for t 1 to ${cohTargets.Size}
      /if (${Bool[${Group.Member[${cohTargets[${t}]}]}]}) {
        /for s 1 to ${cohSpells2D.Size[1]}
          /call check_Ready "cohSpells2D" ${s}
            /if (${c_Ready}) {
              /if (${Spawn[pc =${cohTargets[${t}]}].Distance} >= ${coh_dist}) {
                /if (${check_Mana["cohSpells2D",${s}]}) {
                  /docommand ${ChatToggle} << Auto CoH casting on ${cohTargets[${t}]} >>
                  /g << Auto CoH casting on ${cohTargets[${t}]} >>
                  /call e3_cast ${Spawn[pc =${cohTargets[${t}]}].ID} "cohSpells2D" ${s}
                } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
              }
            } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH CoH is not ready for ${cohTargets[${t}]}  >>
        /if (!${ActionTaken})  /next s
      } else /if (${Debug} || ${Debug_MAG}) /docommand ${ChatToggle} << AutoCoH ${cohTargets[${t}]} is not in my group >>
    /if (!${ActionTaken})  /next t
  }
  /if (${Debug} || ${Debug_MAG}) /echo <== MAG_checkCoH -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Setup
/declare Debug_MAG bool outer FALSE
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_Setup ==>
  /declare coh_dist int outer 150
	/call iniToVarV "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)" autoWeapons bool outer
	/call IniToArrayV "${Character_Ini},Magician,Summoned Pet Item#" petItems
	/declare mageRangedSwap bool outer FALSE
	/if (${Defined[petItems]}) /call BuildSpellArray "petItems"	"petItems2D"

  /if (${Bool[${Me.Book[Call of the Hero]}]}) {
    /declare cohSpells[1] string outer Call of the Hero
    /call BuildSpellArray "cohSpells" "cohSpells2D"
  }

  /declare moltenOrb[1] string outer Summon: Molten Orb
  /call BuildSpellArray "moltenOrb" "moltenOrb2D"
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_Setup -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_MacroSettings
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_MacroSettings ==>
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#1" "check_CoH" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#2" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#3" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#4" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#5" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#6" "check_Ae" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#7" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function#8" "check_SummonItems" 1
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_CharacterSettings
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)"
	/call WriteToIni "${Character_Ini},Magician,Summoned Pet Item"
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Aliases
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_Aliases ==>
	/squelch /alias /dpw /bc Distribute Pet Weapons
	/squelch /alias /PetWeapons /bc Distribute Pet Weapons
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_Aliases -|
/RETURN
|----------------------------------------------------------------------------|
SUB MAG_Background_Events
|**
	/if (${Bool[${Me.AltAbility[Companion of Necessity]}]} && ${Me.AltAbilityReady[Companion of Necessity]}) {
		/if (${Assisting} && ${Me.CombatState.Equal[Combat]} && ${Target.Named} && ${Target.Level} >= ${Math.Calc[${Me.Level}+3]} && !${Select[${Me.TargetOfTarget.Class},WAR,PAL,SHD]} && ${Spawn[${Target.ID}].PlayerState}==4) {
			/if (${Me.Casting.ID}) /call interrupt
			/delay 1s !${Me.Casting.ID}
			/gu ${Target.Name} has aggro on ${Me.TargetOfTarget.Name}, casting Companion of Necessity
			/casting "Companion of Necessity"
		}
	}
**|
  /doevents auto_coh
  /doevents petWeapons
/RETURN