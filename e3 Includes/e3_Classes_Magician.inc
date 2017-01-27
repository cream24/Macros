|----------------------------------------------------------------------------|
|- e3_Classes_Magician.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
#event petWeapons "#1# tells you, 'Pet Weapons'"
#event petWeapons "<#1#> Pet Weapons"
sub event_petWeapons(string line, string ChatSender)
|/varset Debug_MAG TRUE
/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons ==>
	/if (${petItems2D.Size[1]}) {
		| -Verify the event
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|    |-----------Valid User----------|
		/if (!${Bool[${Macro.Return.Arg[2,_]}]}) {
			/call IniToVar "${genSettings_Ini},Buff Requests,Allow Buff Requests (On/Off)" allow_BuffRequests bool local
			| If the user is not valid, check allow_BuffRequests
			/if (${Macro.Return}) {
				| If allow_BuffRequests=TRUE, check restricked PCs
				/if (!${Select[${ChatSender},${Ini[${Macro_Ini},Buff Requests,Restricted PCs]}]}) /goto :allowAccess
			}
		} else {
		
			:allowAccess
			/if (!${SpawnCount[pc ${ChatSender}]}) {
				/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons -| [${ChatSender}] is not in ${Zone}.
			} else {
				/if (${Spawn[pc ${ChatSender}].Distance} > 200) {
					/tell ${ChatSender} You're too far away.
				} else {
					/if (!${Spawn[pc ${ChatSender}].LineOfSight}) {
						/tell ${ChatSender} I can't see you.
					} else {
						/if (!${Spawn[pet ${Spawn[pc ${ChatSender}].Pet}].ID}) {
							/tell ${ChatSender} You don't have a pet, jackass! 
						} else {
							/if (${ChatSender.NotEqual[${Me}]}) /tell ${ChatSender} Incoming pet weapons.  Please stay still for a few moments.
							| Declare local variables
							/declare i int local
							/declare x int local
							/declare e int local
							/declare retryTimer timer local 5s
							/declare miscTimer timer local
							/declare summonedItem string local
							/declare oldItem string local

							| Target the pet, and move to it
							/call TrueTarget ${Spawn[pet ${Spawn[pc ${ChatSender}].Pet}].ID}
							/delay 2
							/call MoveTo -SpawnID|${Spawn[pet ${Spawn[pc ${ChatSender}].Pet}].ID}
							/delay 2

              /if (${Debug} || ${Debug_MAG}) /echo ${ChatSender} ${Line} ${Target.Name} ${Target.ID}

							/for i 1 to ${petItems2D.Size[1]}
              |swap ranged in first to not spam bags
                /if (${petItems2D[${i},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItem[=Blazing Stone of Demise].ItemSlot} != 11 ) {
                  /varset oldItem ${Me.Inventory[11].Name}
                  /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${oldItem}/Ranged" 1
                  /call SwapItem "Blazing Stone of Demise" "Ranged"
                }
							/if (${Target.Distance} < 20) {
								/call e3_Cast ${Me.ID} "petItems2D" ${i}
								/delay 5s ${Cursor.ID}
                /call TrueTarget ${Spawn[pet ${Spawn[pc ${ChatSender}].Pet}].ID}

								/if (${Cursor.ID}) {
									/varset retryTimer 50
									/varset summonedItem ${Cursor.Name}
									:OpenTrade_Loop
									
									/click left target
									/delay 10 ${Window[GiveWnd].Open}
									
									/if (!${Window[GiveWnd].Open}) {
										/if (${retryTimer}) {
											/goto :OpenTrade_Loop
										} else {
											/echo Failed to open trade with ${Target.CleanName}.
										}
									} else {
										
										/varset retryTimer 30
										:WaitAccept_Loop
										
										/notify GiveWnd GVW_Give_Button LeftMouseUp
										/delay 5 !${Window[GiveWnd].Open}
										
										/if (${Window[GiveWnd].Open}) {
											/if (${retryTimer}) {
												/goto :WaitAccept_Loop
											} else {
												/echo Failed to open trade with ${Target.CleanName}.
											}
										}
									}
								}
								/if (${FindItemCount[=${summonedItem}]}) {
									/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons -| Dropped [${summonedItem}] in my inventory.
									| Open InventoryWindow.
									/varset miscTimer 3s

									:openInventory
									/if (!${Window[InventoryWindow].Open}) /nomodkey /keypress i
									/delay 5 ${Window[InventoryWindow].Open}
									/if (!${Window[InventoryWindow].Open} && ${miscTimer}) /goto :openInventory
									| Search packslots.
									/varset e 0
									/for x 1 to 8
										| If the item in the packslot is the summoned item, pick it up.
										/if (!${Me.Inventory[pack${x}].Container} && ${summonedItem.Equal[${Me.Inventory[pack${x}]}]}) {
											/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons -| Picking up [${summonedItem}] form pack${x}
											/itemnotify pack${x} leftmouseup
										} else {
											| If the item in the packslot is a container, search it.
											/for e 1 to ${Me.Inventory[pack${x}].Container}
												/if (${summonedItem.Equal[${Me.Inventory[pack${x}].Item[${e}]}]}) {
													/if (!${Window[pack${x}].Open}) /itemnotify pack${x} rightmouseup
													/delay 1s ${Window[pack${x}].Open}
													/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons -| Picking up [${summonedItem}] form pack${x} slot${e}
													/itemnotify in pack${x} ${e} leftmouseup
												}
											/next e
										}
									/next x
									/if (${retryTimer}) {
										/goto :OpenTrade_Loop
									} else {
										/if (${Cursor.Name.Equal[${summonedItem}]}) /destroy 
									}
								}
							}
							/next i
						}
					}
				}
			}
		}
	}
  /if (${oldItem.Length} && !${mageRangedSwap}) /call SwapItem "${oldItem}" "Ranged"
/if (${Debug} || ${Debug_MAG}) /echo <== EVENT_petWeapons -|
/return

|----------------------------------------------------------------------------|
#event distributePetWeapons "#1# tells you, 'Distribute Pet Weapons'"
#event distributePetWeapons "<#1#> Distribute Pet Weapons"
sub event_distributePetWeapons(line, ChatSender)
/if (${Debug} || ${Debug_MAG}) /echo |- event_distributePetWeapons ==>
	/if (${petItems2D.Size[1]}) {
		| -Verify the event
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|    |-----------Valid User----------|
		/if (!${Bool[${Macro.Return.Arg[2,_]}]}) {
			/call IniToVar "${genSettings_Ini},Buff Requests,Allow Buff Requests (On/Off)" allow_BuffRequests bool local
			| If the user is not valid, check allow_BuffRequests
			/if (${Macro.Return}) {
				| If allow_BuffRequests=TRUE, check restricked PCs
				/if (!${Select[${ChatSender},${Ini[${Macro_Ini},Buff Requests,Restricted PCs]}]}) /goto :allowAccess
			}
		} else {
		
			:allowAccess
			
			/if (!${SpawnCount[pc ${ChatSender}]}) {
				/if (${Debug} || ${Debug_MAG}) /echo |- EVENT_petWeapons -| [${ChatSender}] is not in ${Zone}.
			} else {
				/if (${Spawn[pc ${ChatSender}].Distance} > 200) {
					/tell ${ChatSender} You're too far away.
				} else {
					/if (!${Spawn[pc ${ChatSender}].LineOfSight}) {
						/tell ${ChatSender} I can't see you.
					} else {
						/if (${ChatSender.NotEqual[${Me}]}) /tell ${ChatSender} Handing out pet weapons...
						/declare originalLoc string local ${Me.Loc.Replace[ ,]}
						/declare startingLoc string local ${Spawn[pc ${ChatSender}].Loc.Replace[ ,]}
						/declare nearbyPetOwners string local
						/declare petOwner string local
						/declare petID int local
						/declare i int local
						/declare oldItem string local

						| Move to the starting point
						/call MoveTo -Loc|${startingLoc}
						| Make a list of all the pets nearby
						/for i 1 to ${SpawnCount[radius 100]}
							/if (${NearestSpawn[${i}].Type.Equal[PC]} && ${Spawn[pc ${NearestSpawn[${i}]}].Pet.ID} && ${Spawn[id ${Spawn[pc ${NearestSpawn[${i}]}].Pet.ID}].LineOfSight}) /varset nearbyPetOwners ${nearbyPetOwners}${NearestSpawn[${i}]},
						/next i
						| Pass out weapons to each person on the nearbyPetOwners list
						/varset i 1
						
						:next_PetOwner
						
						| set petOwner and petID, then send the owner a msg.
						/varset petOwner ${nearbyPetOwners.Arg[${i},,]}

						|switch to ranged to prevent swap spam and set mageRangedSwap to TRUE since multiple targets
						/if (${petItems2D[${i},${iCastName}].Equal["Blazing Stone of Demise"]} && ${FindItem[=Blazing Stone of Demise].ItemSlot} != 11 ) {
							/varset oldItem ${Me.Inventory[11].Name}
							/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${oldItem}/Ranged" 1
							/call SwapItem "Blazing Stone of Demise" "Ranged"
							/varset mageRangedSwap TRUE
						}

						/call EVENT_petWeapons "${Line}" "${petOwner}"
						/varcalc i ${i} + 1
						/if (${Bool[${nearbyPetOwners.Arg[${i},,]}]}) /goto :next_PetOwner
						| Move to the starting point
						/call MoveTo -Loc|${originalLoc}

						/if (${mageRangedSwap} && ${oldItem.Length}) {
						  /call SwapItem "${oldItem}" "Ranged"
						  /varset mageRangedSwap FALSE
            }
						/if (${ChatSender.NotEqual[${Me}]}) /tell ${ChatSender} Finished arming pets
					}
				}
			}
		}
	}
/if (${Debug} || ${Debug_MAG}) /echo <== event_distributePetWeapons -|
/return

|----------------------------------------------------------------------------|
SUB check_PetWeapons
/if (${Debug} || ${Debug_MAG}) /echo |- check_petWeapons ==>
	/if (${autoWeapons} && ${Zone.ID}) {
		/if (!${Me.Moving} && !${combatTimer} && !${activeTimer}) {
			/declare i int local
			/for i 1 to ${NetBots.Counts}
				/if (${NetBots[${NetBots.Client[${i}]}].Zone} && ${NetBots[${NetBots.Client[${i}]}].PetID}) {
					/if (${Spawn[id ${NetBots[${NetBots.Client[${i}]}].PetID}].CleanName.Equal[${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${NetBots.Client[${i}]}]}]}) {
						/if (${Debug} || ${Debug_MAG}) /echo |- check_petWeapons -| I have already given weapons to [${Spawn[id ${NetBots[${NetBots.Client[${i}]}].PetID}].CleanName}].
					} else {
						| /docommand ${ChatToggle} Handing out pet weapons to [${Spawn[id ${NetBots[${NetBots.Client[${i}]}].PetID}].CleanName}(${NetBots.Client[${i}]})], please wait a few moments...
						/call EVENT_petWeapons PLACEHOLDER ${NetBots.Client[${i}]}
						/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Auto-Pet Weapons-${NetBots.Client[${i}]}" "${Spawn[id ${NetBots[${NetBots.Client[${i}]}].PetID}].CleanName}" 1
					}
				}
			/next i
		}
	}
/if (${Debug} || ${Debug_MAG}) /echo <== check_petWeapons -|
/RETURN

|----------------------------------------------------------------------------|
#event auto_coh "#1# tells you, 'autocoh #2#'"
SUB event_auto_coh(line, ChatSender, toggleCoH)
  /declare i int local
  |/echo ${ChatSender} ${toggleCoH} ${Defined[${cohTargets}]}  ${cohTargets.Size}
  /if (${toggleCoH.Equal[on]}) {
    /if (${Bool[${Group.Member[${ChatSender}]}]}) {
      /if (${cohTargets.Size}>0) {
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
    /if (${cohTargets.Size}>0) /call RemoveArrayElement cohTargets "${ChatSender}"
    /docommand ${ChatToggle} Auto CoH Disabled on ${ChatSender}
  }
/RETURN

|----------------------------------------------------------------------------|
SUB check_CoH
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_checkCoH ==>
  /if (${cohTargets.Size}) {
    /declare t int local
    /declare s int local
    /for t 1 to ${cohTargets.Size}
      /if (${Bool[${Group.Member[${cohTargets[${t}]}]}]}) {
        /for s 1 to ${cohSpells2D.Size[1]}
          /call check_Ready "cohSpells2D" ${s}
            /if (${Macro.Return}) {
              /if (${Spawn[pc =${cohTargets[${t}]}].Distance} >= ${coh_dist}) {
                /call check_Mana "cohSpells2D" ${s}
                /if (${Macro.Return}) {
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
	/call IniToVar "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)" autoWeapons bool outer
	/call #IniToArray "${Character_Ini},Magician,Summoned Pet Item#" petItems
	/declare mageRangedSwap bool outer FALSE
	/if (${petItems.Size}) /call BuildSpellArray "petItems"	"petItems2D"
	|/echo ${petItems.Size} ${petItems2D.Size[1]}

  /if (${Bool[${Me.Book[Call of the Hero]}]}) {
    /declare cohSpells[1] string outer Call of the Hero
    /call BuildSpellArray "cohSpells" "cohSpells2D"
  }
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_Setup -|
/RETURN

|----------------------------------------------------------------------------|
SUB MAG_MacroSettings
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_MacroSettings ==>
	| Adv Settings
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_CoH" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Buffs" 1
  /call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Pets" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Ae" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},MAG Functions,MAG Function" "check_PetWeapons" 1
/if (${Debug} || ${Debug_MAG}) /echo <== MAG_MacroSettings -|
/RETURN

|----------------------------------------------------------------------------|
SUB MAG_CharacterSettings
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Magician,Auto-Pet Weapons (On/Off)"
	/call WriteToIni "${Character_Ini},Magician,Summoned Pet Item#1"

/if (${Debug} || ${Debug_MAG}) /echo <== MAG_CharacterSettings -|
/RETURN

|----------------------------------------------------------------------------|
SUB MAG_Aliases
/if (${Debug} || ${Debug_MAG}) /echo |- MAG_Aliases ==>
	/squelch /alias /dpw /bc Distribute Pet Weapons
	/squelch /alias /PetWeapons /bc Pet Weapons
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
  /doevents distributePetWeapons
/RETURN