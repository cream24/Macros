|------------------------------------------------------------------------|
|- e3_Macro_Loot.inc v2.1 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
|${MacroQuest.ChatChannel[Creamo]}
|${MacroQuest.ChatChannel[1]}
|--------------------------------------------------------------------|
#event lootRelay "#1# tells you, 'loot #2#'"
SUB Event_lootRelay(line, ChatSender, lootSetting)
  /if (${lootSetting.Equal[on]}) {
    /if (!${Defined[Auto_Loot]}) {
      /declare Auto_Loot bool outer TRUE
    } else /if (${Defined[Auto_Loot]}) {
      /varset Auto_Loot TRUE
    }
    /r Looting Enabled
    /call Event_YourKill
  } else /if (${lootSetting.Equal[off]}) {
    /varset Auto_Loot FALSE
    /r Looting Disabled
  }
/return

|------------------------------------------------------------|
|- Automatically engage looting functions when a mob dies.	-|
#event YourKill "You gain experience!!"
#event YourKill "You gain party experience!!"
#event YourKill "You gained raid experience!"
SUB Event_YourKill
/if (${Debug} || ${Debug_Loot}) /echo |- Event_YourKill ==>
  /if (${Auto_Loot}) {
    /if (${Me.CombatState.NotEqual[Combat]} || ${combatLooting}) /call loot_It
  }
/if (${Debug} || ${Debug_Loot}) /echo <== Event_YourKill -|
/RETURN

|----------------------------------------------------------------|
|- Adds corpses that are not yours to the looted corpse list.	  -|
#EVENT NotYourKill "Someone is already looting that corpse."
#EVENT NotYourKill "You may not loot this corpse at this time."
#EVENT NotYourKill "You must first target a corpse to loot!"
SUB EVENT_NotYourKill
/RETURN SKIP_LOOT

|--------------------------------------------------------------------------|
|- Moves to corpses and begins looting, reads Loot_Ini for loot handles.	-|
SUB loot_It
/if (${Debug} || ${Debug_Loot}) /echo |- loot_It ==>
  /if (${Auto_Loot}) {
    /squelch /hidecor looted
    | Declare local variables
    /declare lootSetting string local
    /declare lootTotal int local -1
    /declare lootTotal_Timer timer local
    /declare i int local
    /declare c int local
    /declare looted_Corpses string local
    /declare corpseID int local
    /declare startX int ${Me.X}
    /declare startY int ${Me.Y}
    |NPC corpses have _'s in the name
    |/echo ${Bool[${Select[${NearestSpawn[corpse radius 100].ID},${looted_Corpses}]}]}
  :moreCorpsesToLoot
    /doevents flush YourKill
    |/if (${Bool[${looted_Corpses}]} && ${Select[${NearestSpawn[${i},corpse radius ${seek_Radius}].ID},${looted_Corpses}]}) /goto :skip_Loot
    /if (${Debug} || ${Debug_Loot}) /echo morecorpses  ${SpawnCount[npc corpse radius ${seek_Radius} "_"]}
    /if (${Twist.Twisting}) /call pauseTwist
    /for c 1 to ${SpawnCount[npc corpse radius ${seek_Radius} "_"]}
      /if (${Bool[${NearestSpawn[${c},corpse radius ${seek_Radius} "_"].ID}]} && ${Spawn[id ${NearestSpawn[${c},corpse radius ${seek_Radius} "_"].ID}].LineOfSight} && !${Bool[${Select[${NearestSpawn[${c},corpse radius ${seek_Radius} "_"].ID},${looted_Corpses}]}]}) {
      |/echo vs1
      /if (${Corpse.Open}) /notify LootWnd DoneButton leftmouseup
      /varset corpseID ${NearestSpawn[${c},corpse radius ${seek_Radius} "_"].ID}
      /if (${Debug} || ${Debug_Loot}) /echo corpse ${corpseID} list ${looted_Corpses}
      /if (${NetAdvPath.Status}==1) /squelch /netfollow off

      /delay 1
      /call TrueTarget ${corpseID}
      /delay 1
      /if (${Debug} || ${Debug_Loot}) /echo moving to cor ${corpseID} targ ${Target.ID}
      /if (${Spawn[id ${corpseID}].Distance} > 10 && ${Spawn[id ${corpseID}].Distance} < ${seek_Radius}) /call MoveTo -SpawnID|${corpseID}
      | If corpseID is in range, loot it.
      /delay 3s ${Spawn[id ${corpseID}].Distance} < 15
      /if (${Spawn[id ${corpseID}].Distance} < 15 && ${Spawn[id ${corpseID}].DistanceZ} < 15) {
        | If looter is a bard, pause melody.


        /if (${Debug} || ${Debug_Loot}) /echo ${Target.ID}
        /loot
        /delay 1
        | Check to see if the corpse can be looted
        /doevents NotYourKill
        /delay 15 ${Window[LootWnd].Open} || ${Bool[${Macro.Return.Equal[SKIP_LOOT]}]}
        /varset looted_Corpses ${looted_Corpses},${corpseID}
        /if (${Bool[${Macro.Return.Equal[SKIP_LOOT]}]}) /goto :SKIP_LOOT

        /if (${Debug} || ${Debug_Loot}) /echo yourkill ${Macro.Return} corpse ${corpseID}
        /varset lootTotal_Timer 3s

        | Count items on the corpse, loop until no new items are found.
        :autoxplootlag
        /if (${lootTotal}!=${Corpse.Items}) {
          |/echo vs4
          /varset lootTotal ${Corpse.Items}
          /delay 5 ${lootTotal}==${Corpse.Items}
          /if (${lootTotal_Timer}) /goto :autoxplootlag
        }

        | If there are items to loot
        /if (${lootTotal}) {
          /for i 1 to ${lootTotal}
            | If my inventory is full, and I haven't alerted the user.
            /if (!${Me.FreeInventory}) {
              | If I haven't alerted the user
              /if (!${fullInventory_Alert}) {
                /beep
                /declare fullInventory_Alert bool outer TRUE
                /docommand ${ChatToggle} My inventory is full!  I will continue to link items on corpses, but cannot loot anything else.
              }
            } else {
              | Wait for an item ID in the loot slot, and set iniEntryVariables
              /delay 1s ${Corpse.Item[${i}].ID}
              /if (${Debug} || ${Debug_Loot}) /echo itemID ${Corpse.Item[${i}].ID}

              /call get_lootSetting "${i}"
              |/echo vs5
              /varset lootSetting ${Macro.Return}
              | Destroy the item
              /if (${lootSetting.Find[Destroy]}) {
                /echo [Auto-Loot]: Destroying [${Corpse.Item[${i}].Name}]
                /if (${Corpse.Item[i].Lore} && ${FindItemCount[=${Corpse.Item[${i}].Name}]}) /goto :skip_Loot
                /call loot_Handle ${i} destroy
              | Keep the item
              } else /if (${lootSetting.Find[Keep]}) {
                | Check for a max stock
                /if (${lootSetting.Find[keep|]}) {
                  /call argueString keep| "${lootSetting}"
                  /if (${Bool[${Macro.Return}]} && ${Int[${Macro.Return}]} < ${Math.Calc[${FindItemCount[${Corpse.Item[i].Name}]} + 1].Int}) {
                    /echo [Auto-Loot]: Fully stocked on [${Corpse.Item[i].Name}].
                    /goto :skip_Item
                  }
                }
                | If the item is lore, make sure I don't have one.  If I do, skip it.
                /if (${Corpse.Item[${i}].Lore}) {
    |								/echo [Auto-Loot]: [${Corpse.Item[i].Name}] is lore check the corpse - broken RoF
                  /if (${FindItemCount[=${Corpse.Item[${i}].Name}]} || ${FindItemBankCount[=${Corpse.Item[${i}].Name}]}) {
                    /echo [Auto-Loot]: [${Corpse.Item[${i}].Name}] is lore, and I already have one${If[${FindItemBankCount[=${Corpse.Item[${i}].Name}]}, in the bank,]}.
                    /goto :skip_Item
                  }
                }
                | Check that I have an inventory space large enough for the item
                /if (${Me.LargestFreeInventory} < ${Corpse.Item[${i}].Size}) {
                  /echo [Auto-Loot]: I don't have a free inventory space large enough to hold [${Corpse.Item[${i}].Name}].
                  /goto :skip_Item
                }
                | If the item is a container, skip it.
                /if (${Corpse.Item[${i}].Container}) {
                  /echo [Auto-Loot]: Skipping [${Corpse.Item[${i}].Name}], item is a container.
                  /goto :skip_Item
                }
                | loot the item
                /echo [Auto-Loot]: Looting [${Corpse.Item[${i}].Name}]
                /call loot_Handle ${i} keep
              }
              :skip_Item
              /next i
            }
        }
        | If I've finished looting, and there are still items on the corpse, announce in chat.
        /if (${Corpse.Items}) {
          |/echo in finished loot link section
          /keypress /
          /delay 1
          /call Type "${linkChannel} ${Target.ID}-"
          /delay 1
          /notify LootWnd BroadcastButton leftmouseup
          /delay 1
          /keypress enter chat
        }
        :SKIP_LOOT

        /if (${Corpse.Open}) /notify LootWnd DoneButton leftmouseup
        /delay 2
      |end of distance check
      }
    |end of if eligible corpse
    }
    |check for loot off message to allow an escape from looting
    /doevents lootRelay
    /if (${Auto_Loot}) /next c
    |if more corpses to loot goto
    /if (${Auto_Loot} && ${Bool[${NearestSpawn[corpse radius ${seek_Radius} "_"].ID}]} && ${Spawn[id ${NearestSpawn[corpse radius ${seek_Radius} "_"].ID}].LineOfSight} && !${Bool[${Select[${NearestSpawn[corpse radius ${seek_Radius} "_"].ID},${looted_Corpses}]}]}) /goto :moreCorpsesToLoot
    |if no more corpses, Click 'Done' and return to group.
    |/echo no more corpses, loot done ${looted_Corpses}
    /if (${Corpse.Open}) /notify LootWnd DoneButton leftmouseup
    |/if (${returnTwist}) /call unpauseTwist
    /if (${Following}) {
      /call AquireFollow
    } else {
      /call MoveTo -Loc|${startY},${startX}
    }
  }

/if (${Debug} || ${Debug_Loot}) /echo <== loot_It -|
/RETURN JUST_LOOTED

|--------------------------------------------------------------------------|
SUB get_lootSetting(invSlot, itemSlot)
/if (${Debug} || ${Debug_Loot}) /echo |- get_lootSetting invSlot: ${invSlot} ${If[${Defined[itemSlot]},itemSlot: ${itemSlot},]}==>
	/declare itemName string local
	/declare itemValue string local
	/declare iniEntryVariables string local
	/declare lootSetting string local
	| If the item is not in a bag
	/if (!${Defined[itemSlot]}) {
		|/echo invSlot variable: ${invSlot} 
		| Reset itemName if it contains a ':'.  ':'s cause errors when reading from the ini, because they act as a delimiter, just like '='s
|		/echo ${Corpse.Item[${invSlot}].Name}		
		/varset itemName ${Corpse.Item[${invSlot}].Name}
|		/echo itemName variable: ${itemName}
		/if (${itemName.Find[:]}) /varset itemName ${itemName.Replace[:,;]}
		| Set item value
		/varset itemValue ${Corpse.Item[${invSlot}].Value}
		/varset itemValue ${If[${Bool[${itemValue.Left[${Math.Calc[${itemValue.Length} - 3].Int}]}]},${itemValue.Left[${Math.Calc[${itemValue.Length} - 3].Int}]}p,]}${If[${Bool[${itemValue.Mid[${Math.Calc[${itemValue.Length} - 2].Int}]}]},${itemValue.Mid[${Math.Calc[${itemValue.Length} - 2].Int}]}g,]}${If[${Bool[${itemValue.Mid[${Math.Calc[${itemValue.Length} - 1].Int}]}]},${itemValue.Mid[${Math.Calc[${itemValue.Length} - 1].Int}]}s,]}${If[${Bool[${itemValue.Right[1]}]},${itemValue.Right[1]}c,]}
		
		| Set ini variables like stack size, (C), (ND) etc.
		/varset iniEntryVariables ${If[${Corpse.Item[${invSlot}].Stackable},(${Corpse.Item[${invSlot}].StackSize}),]}${If[${Corpse.Item[${invSlot}].NoDrop},(ND),]}${If[${Corpse.Item[${invSlot}].Lore},(L),]}${If[${Corpse.Item[${invSlot}].Container},(C),]}
		| Check for a Loot_Ini entry
		|/echo /if (!{Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}].Length})
		/if (!${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}].Length}) /echo ifsuccess
		/if (!${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}].Length}) {
			/echo /call WriteToIni "${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}" ${If[${Corpse.Item[${invSlot}].Container},Container,${If[${Corpse.Item[${invSlot}].NoDrop},Skip,Keep${If[${Corpse.Item[${invSlot}].Stackable},|${Corpse.Item[${invSlot}].StackSize},]}]}]}
			/call WriteToIni "${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}" ${If[${Corpse.Item[${invSlot}].Container},Container,${If[${Corpse.Item[${invSlot}].NoDrop},Skip,Keep${If[${Corpse.Item[${invSlot}].Stackable},|${Corpse.Item[${invSlot}].StackSize},]}]}]}
			/echo /docommand ${ChatToggle} Added: [${Corpse.Item[${invSlot}].Name}(${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}]})] to [${Loot_Ini}].
			/docommand ${ChatToggle} Added: [${Corpse.Item[${invSlot}].Name}(${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}]})] to [${Loot_Ini}].
			| Add scripts to import old loot settings from ninjaloot.ini, and old e3 loot inis
		}
	| If the item is in a bag
	} else {
		| Reset itemName if it contains a ':'.  ':'s cause errors when reading from the ini, because they act as a delimiter, just like '='s
		/varset itemName ${Me.Inventory[${invSlot}].Item[${itemSlot}].Name}
		/if (${itemName.Find[:]}) /varset itemName ${itemName.Replace[:,;]}
		| Set item value
		/varset itemValue ${Me.Inventory[${invSlot}].Item[${itemSlot}].Value}
		/varset itemValue ${If[${Bool[${itemValue.Left[${Math.Calc[${itemValue.Length} - 3].Int}]}]},${itemValue.Left[${Math.Calc[${itemValue.Length} - 3].Int}]}p,]}${If[${Bool[${itemValue.Mid[${Math.Calc[${itemValue.Length} - 2].Int}]}]},${itemValue.Mid[${Math.Calc[${itemValue.Length} - 2].Int}]}g,]}${If[${Bool[${itemValue.Mid[${Math.Calc[${itemValue.Length} - 1].Int}]}]},${itemValue.Mid[${Math.Calc[${itemValue.Length} - 1].Int}]}s,]}${If[${Bool[${itemValue.Right[1]}]},${itemValue.Right[1]}c,]}
		| Set ini variables like stack size, (C), (ND) etc.
		/varset iniEntryVariables ${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].Stackable},(${Me.Inventory[${invSlot}].Item[${itemSlot}].StackSize}),]}${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].NoDrop},(ND),]}${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].Lore},(L),]}${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].Container},(C),]}
		| Check for a Loot_Ini entry
		/if (!${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}].Length}) {
			/call WriteToIni "${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}" ${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].Container},Container,${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].NoDrop},Skip,Keep${If[${Me.Inventory[${invSlot}].Item[${itemSlot}].Stackable},|${Me.Inventory[${invSlot}].Item[${itemSlot}].StackSize},]}]}]}
			/docommand ${ChatToggle} Added: [${Me.Inventory[${invSlot}].Item[${itemSlot}].Name}(${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}]})] to [${Loot_Ini}].
			
			| Add scripts to import old loot settings from ninjaloot.ini, and old e3 loot inis
		}
	}
	/varset lootSetting ${Ini[${Loot_Ini},${itemName.Left[1]},${itemName} ${itemValue}${iniEntryVariables}]}
	/if (${Debug} || ${Debug_Loot}) /echo |- get_lootSetting -| lootSetting=${lootSetting}
/if (${Debug} || ${Debug_Loot}) /echo <== get_lootSetting -|
/RETURN ${lootSetting}

|--------------------------------------------------------------------|
|- Loots items mobs you have killed, to destroy or autoinventory.	  -|
|--------------------------------------------------------------------|
|- @param slotNum: The slot number you are looting from.			      -|
|- @param handle: What to do with the item once you loot it.		    -|
|--------------------------------------------------------------------|
SUB loot_Handle(int slotNum, handle)
/if (${Debug} || ${Debug_Loot}) /echo |- loot_Handle ==>
	/declare lootTimer timer local 50
	:retry_Loot
	/if (${Debug} || ${Debug_Loot}) /echo |- loot_Handle -| :retry_Loot
	| Try to loot the specified item from the corpse.
	/itemnotify loot${slotNum} leftmouseup
	/delay 5 ${Cursor.ID} || ${Window[ConfirmationDialogBox].Open} || ${Window[QuantityWnd].Open}
	| If the item has been looted, decide what to do with it.
	/if (${Cursor.ID}) {
		| Destroy the item.
		/if (${handle.Equal[destroy]}) {
			:destroy_Item
			/destroy
			| If the destroy attempt was unsuccessful, try again.	
			/if (${Cursor.ID} && ${lootTimer}) /goto :destroy_Item
		| Else, keep the item.
		} else {
			/call ClearCursor
		}
	| Else, if loot attempt was unsuccessful, try again.		
	} else {
		| Click yes on confirmation box.
		/if (${Window[ConfirmationDialogBox].Open}) {
			/notify ConfirmationDialogBox Yes_Button leftmouseup
		} else /if (${Window[QuantityWnd].Open}) {
			/notify QuantityWnd QTYW_Accept_Button leftmouseup
		}
		/if (${lootTimer}) /goto :retry_Loot
	}
/if (${Debug} || ${Debug_Loot}) /echo <== loot_Handle -|
/RETURN

|--------------------------------------------------------------------|
|- Check Loot Setting
#EVENT checkLootSetting "<#*#> Check Loot Setting for #1#"
#EVENT checkLootSetting "[MQ2] Check Loot Setting for #1#"
SUB EVENT_checkLootSetting(line, itemName)
/if (${Debug} || ${Debug_Loot}) /echo |- EVENT_checkLootSetting ==>
	/declare itemToCheck string local ${itemName}
	/if (${itemToCheck.Find[:]}) /varset itemToCheck ${itemToCheck.Replace[:,;]}
	/declare i int local 1
	
	:check_Loot_Ini
	/if (!${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Arg[1,${If[${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Find[~]},~,(]}].Equal[${itemToCheck}]}) {
		/varcalc i ${i} + 1
		/if (${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Length}) {
			/goto :check_Loot_Ini
		} else {
			/echo No entry found.
		}
	} else {
		/echo >>> [${itemName}=${Ini[${Loot_Ini},${itemToCheck.Left[1]},${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|]}]}] <<<
	}
/if (${Debug} || ${Debug_Loot}) /echo <== EVENT_checkLootSetting -|
/RETURN

|--------------------------------------------------------------------|
|- Update Loot Setting
#EVENT updateLootSetting "<#1#> Set Loot Setting for #2# /#3#"
#EVENT updateLootSetting "[MQ2] Set Loot Setting for #2# /#3#"
SUB EVENT_updateLootSetting(line, ChatSender, itemName, iniSetting)
/if (${Debug} || ${Debug_Loot}) /echo |- EVENT_updateLootSetting ==>
	/if (${ChatSender.Equal[${Me.CleanName}]} || !${Defined[ChatSender]}) {
		/declare itemToCheck string local ${itemName}
		/if (${itemToCheck.Find[:]}) /varset itemToCheck ${itemToCheck.Replace[:,;]}
		/declare i int local 1
		
		:check_Loot_Ini
		/if (!${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Arg[1,${If[${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Find[~]},~,(]}].Equal[${itemToCheck}]}) {
			/varcalc i ${i} + 1
			/if (${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|].Length}) {
				/goto :check_Loot_Ini
			} else {
				/echo No entry found.
			}
		} else {
			/call WriteToIni "${Loot_Ini},${itemToCheck.Left[1]},${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|]}" "${iniSetting}" 1
			/echo >>> Loot_Ini entry for [${itemName}] has been changed to [${Ini[${Loot_Ini},${itemToCheck.Left[1]},${Ini[${Loot_Ini},${itemToCheck.Left[1]}].Arg[${i},|]}]}]. <<<
		}
	}
/if (${Debug} || ${Debug_Loot}) /echo <== EVENT_updateLootSetting -|
/RETURN

|--------------------------------------------------------------------|
SUB loot_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Loot (On/Off)" Debug_Loot bool outer
/if (${Debug} || ${Debug_Loot}) /echo |- _Setup ==>
	/if (!${Ini[${MacroData_Ini},File Paths,Loot Settings File].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Loot Settings File" "e3 Macro Inis\Loot Settings.ini" 1
	/if (!${Ini[${MacroData_Ini},File Paths,Loot Settings File].Length}) {
		/echo ERROR: Could not find designated file path for [Loot Settings File], please review [e3Data.ini] and restart.
		/endmacro
	} else {
		/declare Loot_Ini string outer ${Ini[${MacroData_Ini},File Paths,Loot Settings File]}
	}
	/if (!${Ini[${Loot_Ini}].Length}) /call Build_Alphabetized_Ini "${Loot_Ini}"
	/call IniToVar "${genSettings_Ini},Loot,Loot Link Channel" linkChannel string outer
	/call IniToVar "${genSettings_Ini},Loot,Corpse Seek Radius" seek_Radius int outer
  /call IniToVar "${genSettings_Ini},Loot,Loot in Combat" combatLooting bool outer
	| Import character settings
	/if (${Bool[${Ini[${Character_Ini},Misc,Auto-Loot (On/Off)]}]}) /call IniToVar "${Character_Ini},Misc,Auto-Loot (On/Off)" Auto_Loot bool outer
/if (${Debug} || ${Debug_Loot}) /echo <== _Setup -|
/RETURN

|--------------------------------------------------------------------|
SUB loot_MacroSettings
/if (${Debug} || ${Debug_Loot}) /echo |- _MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Loot (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Loot,Auto-Loot Mode (Auto/Manual)" Auto
	/call WriteToIni "${genSettings_Ini},Loot,Loot Link Channel" say
	/call WriteToIni "${genSettings_Ini},Loot,Corpse Seek Radius" 125
  /call WriteToIni "${genSettings_Ini},Loot,Loot in Combat" TRUE
/if (${Debug} || ${Debug_Loot}) /echo <== _MacroSettings -|
/RETURN

|--------------------------------------------------------------------|
SUB loot_CharacterSettings
	/call WriteToIni "${Character_Ini},Misc,Auto-Loot (On/Off)" Off
/RETURN

|--------------------------------------------------------------------|
Sub loot_Aliases
/return

|--------------------------------------------------------------------|
Sub loot_Background_Events
  /doevents lootRelay
  /doevents YourKill
  /doevents checkLootSetting
  /doevents updateLootSetting
/return