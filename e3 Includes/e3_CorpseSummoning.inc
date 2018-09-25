|------------------------------------------------------------------------------------|
|- e3_Macro_CorpseSummoning.inc v1.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ														-|
|------------------------------------------------------------------------------------|
|- Contains functions to automatically use the corpse summoners in the guild lobby.	-|
|------------------------------------------------------------------------------------|
#event UseCorpseSummoner "<#1#> Summon Corpses"
#event UseCorpseSummoner "[#1#] Summon Corpses"
SUB event_UseCorpseSummoner(line, ChatSender)
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- event_UseCoprseSummoner ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
  | Check to see if you need to summon a corpse
  /declare itemCount int local
  /declare i int local
  /for i 1 to 30
    /if (${Bool[${InvSlot[${i}].Item}]}) /varcalc itemCount ${itemCount} + 1
  /next i

  /if (${itemCount} >= 27) {
    /echo CurrentItem ${itemCount}
    /echo Looks like I have all my gear, so I don't need to summon a corpse.
    /goto :skip
  }
  | If I'm in The Guild Lobby.
  /if (${Zone.ID} == 344) {
    | Find a starting point
	/if (!${Defined[startingPointX]}) /declare startingPointX string outer
	/if (!${Defined[startingPointY]}) /declare startingPointY string outer
	/varset startingPointX 400
	/if (${LineOfSight[400,250:${Me.Loc.Replace[ ,]}]}) {
		/varset startingPointY 250
	} else /if (${LineOfSight[400,-250:${Me.Loc.Replace[ ,]}]}) {
		/varset startingPointY -250
	}
	/if (!${Bool[${startingPointX}]}) {
      /echo Please move to the west or east side of the guild lobby, and try again.
    } else {
      /declare GiveUpTimer	int outer 600
      | Decide which soulstone to use
      /call find_Soulstone
      /declare SoulStoneToUse string outer ${Macro.Return.Arg[1,_]}
      /declare SoulStoneCost string outer ${Macro.Return.Arg[2,_]}

      /if (${Spawn[${Me}'s].ID}) /goto :lootNow

      | If I dont have a soulstone, get money to buy one
      /if (!${FindItemCount[=${SoulStoneToUse}]}) {
        /if (${Me.Platinum} < ${SoulStoneCost}) {
          /if (${Math.Calc[${Me.CashBank} / 1000].Int} > ${SoulStoneCost}) {
            /call withdraw_Plat
          } else {
            /docommand ${ChatToggle} I don't have enough money to buy a soulstone.
            /goto :skip
          }
        }
      }
      | Buy a soulstone
      /if (!${FindItemCount[=${SoulStoneToUse}]} && ${Me.Platinum} >= ${SoulStoneCost}) /call buy_SoulStone "${SoulStoneToUse}"
      | Summon Corpse.
      /if (${FindItemCount[=${SoulStoneToUse}]}) /call summon_Corpse "${SoulStoneToUse}"
      | Loot the corpse
      /call iniToVarV "${genSettings_Ini},Corpse Summoning,Loot After Summoning (On/Off)" PLACEHOLDER bool local
      :lootNow
      /if (${Bool[${Macro.Return}]} && ${Spawn[${Me}'s].ID}) /call event_LootNow "PLACEHOLDER" "${Me.CleanName}"
      /call MoveToLoc ${startingPointX} ${startingPointY} 80 15
	  :skip
	  /if (${Defined[startingPointX]}) /deletevar startingPointX
	  /if (${Defined[startingPointY]}) /deletevar startingPointY
      /if (${Defined[SoulStoneToUse]}) /deletevar SoulStoneToUse
      /if (${Defined[SoulStoneCost]}) /deletevar SoulStoneCost
      /if (${Defined[GiveUpTimer]}) /deletevar GiveUpTimer
    }
  } else {
    | in shadowrest
  }
/if (${Debug} || ${Debug_CorpseSummoning}) /echo <== event_UseCorpseSummoner -|
/RETURN

|-------------------------------------------------------------------------------|
SUB find_Soulstone
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- find_Soulstone ==>
	/declare summoningReagent string local
	/declare summoningReagent_Cost int local
	/if (${Me.Level} <= 20) {
		/varset summoningReagent Minor Soulstone
		/varset summoningReagent_Cost 12
	} else /if (${Me.Level} > 20 && ${Me.Level} <= 30) {
		/varset summoningReagent Lesser Soulstone
		/varset summoningReagent_Cost 28
	} else /if (${Me.Level} > 30 && ${Me.Level} <= 40) {
		/varset summoningReagent Soulstone
		/varset summoningReagent_Cost 55
	} else /if (${Me.Level} > 40 && ${Me.Level} <= 50) {
		/varset summoningReagent Greater Soulstone
		/varset summoningReagent_Cost 87
	} else /if (${Me.Level} > 50 && ${Me.Level} <= 55) {
		/varset summoningReagent Faceted Soulstone
		/varset summoningReagent_Cost 120
	} else /if (${Me.Level} > 55 && ${Me.Level} <= 70) {
		/varset summoningReagent Pristine Soulstone
		/varset summoningReagent_Cost 165
	} else /if (${Me.Level} > 70 && ${Me.Level} <= 75) {
		/varset summoningReagent Glowing Soulstone
		/varset summoningReagent_Cost 265
	} else /if (${Me.Level} > 75 && ${Me.Level} <= 80) {
		/varset summoningReagent Prismatic Soulstone
		/varset summoningReagent_Cost 425
	} else /if (${Me.Level} > 80 && ${Me.Level} <= 85) {
		/varset summoningReagent Iridescent Soulstone
		/varset summoningReagent_Cost 530
	} else /if (${Me.Level} > 85 && ${Me.Level} <= 90) {
		/varset summoningReagent Phantasmal Soulstone
		/varset summoningReagent_Cost 635
	} else /if (${Me.Level} > 90 && ${Me.Level} <= 95) {
		/varset summoningReagent Luminous Soulstone
		/varset summoningReagent_Cost 750
	} else /if (${Me.Level} > 95 && ${Me.Level} <= 100) {
		/varset summoningReagent Coalescent Soulstone
		/varset summoningReagent_Cost 865
	}
/if (${Debug} || ${Debug_CorpseSummoning}) /echo <== find_Soulstone -|
/RETURN ${summoningReagent}_${summoningReagent_Cost}

|-------------------------------------------------------------------------------|
SUB withdraw_Plat
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- withdraw_Plat ==>
	| Move to the startingPoint
	/call MoveToLoc ${startingPointX} ${startingPointY} 80 15
	/delay 2s
	| Look for the west side banker.
	/if (!${Spawn[npc A Shrewd Banker].LineOfSight}) {
		/echo I couldn't find a banker.  Move to either the west or east wings of the Guild Lobby, and try again.
	} else {
		| Move to the banker.
		/call TrueTarget ${Spawn[npc A Shrewd Banker].ID}
		/call MoveToLoc ${Target.Y} ${Target.X} 80 15
		/if (${Target.Distance} > 15) {
			/docommand ${ChatToggle} I failed to move to the banker
		} else {
			| Create a retryTimer and set it for 30 seconds.
			/declare retryTimer timer local ${GiveUpTimer}
			/declare miscTimer timer local
			/declare i int local
			:withdraw_Loop
      | Target the banker.
			/if (${Target.ID} != ${Spawn[npc A Shrewd Banker].ID}) /call TrueTarget ${Spawn[npc A Shrewd Banker].ID}
			| Open the bank window.
			/click right target
			/delay 10 ${Window[BigBankWnd].Open}
			| If the bank window has not opened.
			/if (!${Window[BigBankWnd].Open}) {
				| If there is still time, retry.
				/if (${retryTimer}) {
					/goto :withdraw_Loop
				| Else, announce to user.
				} else {
					/docommand ${ChatToggle} ERROR: Failed to open the bank window.
				}
			| Else, if the bank window is open.
			} else {
				| Open the platinum QuantityWnd.
				/notify BigBankWnd BIGB_Money0 leftmouseup
				/delay 5 ${Window[QuantityWnd].Open}
				| If the platinum QuantityWnd did not open.
				/if (!${Window[QuantityWnd].Open}) {
					| If there is still time, retry.
					/if (${retryTimer}) {
						/goto :withdraw_Loop
					| Else, announce to user.
					} else {
						/docommand ${ChatToggle} ERROR: Failed to open the platinum withdraw box.
					}
				| Else, if the platinum QuantityWnd did open.
				} else {
					| Clear the text in the platinum QuantityWnd.
					/varset miscTimer 20
					:clearText
					/nomodkey /keypress backspace chat hold
					/if (${miscTimer}) /goto :clearText
					| Withdraw money.
					/varset miscTimer 3s
					/for i 1 to ${SoulStoneCost.Length}
						/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- withdraw_Plat -| Entering [${SoulStoneCost.Mid[${i},1]}]
						/keypress ${SoulStoneCost.Mid[${i},1]} chat
						/delay 3
					/next i
					/notify QuantityWnd QTYW_Accept_Button leftmouseup
					/delay 2
					/autoinventory
					| If I didn't get enough money, try again. Otherwise, close the bank.
					/if (${Me.Platinum} < ${SoulStoneCost}) {
						/if (${retryTimer}) {
							/goto :withdraw_Loop
						} else {
							/docommand ${ChatToggle}  ERROR: Failed to withdraw platinum from the bank.
						}
					} else  {
						/notify BigBankWnd BIGB_DoneButton leftmouseup
					}		
				}
			}
		}
	}
/if (${Debug} || ${Debug_CorpseSummoning}) /echo <== withdraw_Plat -|
/RETURN

|-------------------------------------------------------------------------------|
SUB buy_SoulStone(SoulStoneToUse)
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- buy_SoulStone ==>
	| Move to the startingPoint
  /call MoveToLoc ${startingPointX} ${startingPointY} 80 15
  /delay 2s
	|-------------------------------------------------------------------------------------------------------------------------------------------------------------Look for the west side banker.
	/if (!${Spawn[npc A Disciple of Luclin].LineOfSight}) {
		/echo ERROR: I couldn't find the soulstone merchant.  Move to the Throne area, or the west side of The Guild Lobby, and try again.
	} else {
		|--------------------------------------------------------------------------------------------------------------------------------------------------------------------Move to the banker.
    /call TrueTarget ${Spawn[npc A Disciple of Luclin].ID}
    /call MoveToLoc ${Target.Y} ${Target.X} 80 15
		/if (${Target.Distance} > 15) {
			/echo ERROR: I failed to move to the soulstone merchant. Move me to the west side of The Guild Lobby, and try again.
		} else {
			|-------------------------------------------------------------------------------------------------------------------------------------Create a retryTimer and set it for 30 seconds.
			/declare retryTimer timer local ${GiveUpTimer}
			/declare miscTimer timer local
			/declare i int local
			/declare merchantTotal int local
			:buyLoop
			| Target the merchant
			/if (${Target.ID} != ${Spawn[npc A Disciple of Luclin].ID}) /call TrueTarget ${Spawn[npc A Disciple of Luclin].ID}
			|Open the soulstone merchant.
			/click right target
			/delay 10 ${Window[MerchantWnd].Open}
			|If the soulstone merchant has not opened.
			/if (!${Window[MerchantWnd].Open}) {
				|If there is still time, retry.
				/if (${retryTimer}) {
					/goto :buyLoop
				|Else, announce to user.
				} else {
					/echo ERROR: Failed to open the soulstone merchant.
				}
			|Else, if the merchant window is open.
			} else {
				|Wait for the merchant to populate.
				/varset merchantTotal -1
				:populateMerchant
				/if (${merchantTotal}!=${Window[MerchantWnd].Child[ItemList].Items}) {
					/varset merchantTotal ${Window[MerchantWnd].Child[ItemList].Items}
					/delay 2
					/goto :populateMerchant
				}
				|Buy the soulstone.
				/nomodkey /notify MerchantWnd ItemList listselect ${Window[MerchantWnd].Child[ItemList].List[=${SoulStoneToUse},2]}
				/delay 2
				/if (!${FindItemCount[${SoulStoneToUse}]}) /notify MerchantWnd MW_Buy_Button leftmouseup
				/delay 2
				|If I failed to buy the soulstone.
				/if (!${FindItemCount[${SoulStoneToUse}]}) {
					/if (${retryTimer}) {
						/goto :buyLoop
					} else {
						/echo ERROR: Failed to buy a soulstone!
					}
				} else {
					/nomodkey /notify MerchantWnd MW_Done_Button leftmouseup
				}
			}
		}
	}
/if (${Debug} || ${Debug_CorpseSummoning}) /echo <== buy_SoulStone -|
/RETURN

|-------------------------------------------------------------------------------|
SUB summon_Corpse(SoulStoneToUse)
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- summon_Corpse ==>
	| Move to the startingPoint
	/call MoveToLoc ${startingPointX} ${startingPointY} 80 15
	/delay 2s
	|---Look for the corpse summoner.
	/if (!${Spawn[npc A Priest of Luclin].LineOfSight}) {
		/echo ERROR: I couldn't find the corpse summoner.  Move to the Throne area, or the west side of The Guild Lobby, and try again.
	} else {
		|---Move to the corpse summoner.
    /call TrueTarget ${Spawn[npc A Priest of Luclin].ID}
    /call MoveToLoc ${Target.Y} ${Target.X} 80 15
		/if (${Target.Distance} > 15) {
			/echo ERROR: I failed to move to the corpse summoner. Move me to the west side of The Guild Lobby, and try again.
		} else {
			| Target the summoner
			/if (${Target.ID} != ${Spawn[npc A Priest of Luclin].ID}) /call TrueTarget ${Spawn[npc A Priest of Luclin].ID}
      |---Create a retryTimer and set it for 30 seconds.
			/declare retryTimer timer local ${GiveUpTimer}
			/declare miscTimer timer local
			/declare i int local
			/declare e int local
			:summonCorpse
			/if (${Spawn[${Me}'s].ID}) /goto :check_ForCorpse
			|---Open InventoryWindow.
			/if (!${Window[InventoryWindow].Open}) /nomodkey /keypress i
			/delay 10 ${Window[InventoryWindow].Open}
			|---If my inventory is not open.
			/if (!${Window[InventoryWindow].Open}) {
				|---If there is still time, retry.
				/if (${retryTimer}) {
					/goto :summonCorpse
				|---Else, announce to user.
				} else {
					/echo ERROR: Failed to open my inventory.
				}
			|---Else, if my inventory is open.
			} else {
				|--Search packslots.
				/for i 1 to ${numInventorySlots}
					|---If the item in the packslot is a soulstone, pick it up.
					/if (!${Me.Inventory[pack${i}].Container} && ${SoulStoneToUse.Equal[${Me.Inventory[pack${i}]}]}) {
						/itemnotify pack${i} leftmouseup
					} else {
						|---If the item in the packslot is a container, search it.
						/for e 1 to ${Me.Inventory[pack${i}].Container}
							/if (${SoulStoneToUse.Equal[${Me.Inventory[pack${i}].Item[${e}]}]}) {
								/if (!${Window[pack${i}].Open}) /itemnotify pack${i} rightmouseup
								/delay 1s ${Window[pack${i}].Open}
								/itemnotify in pack${i} ${e} leftmouseup
							}
						/next e
					}
				/next i
				|---Check that the soulstone was picked up.
				/if (!${Cursor.ID}) {
					|---If there is still time, retry.
					/if (${retryTimer}) {
						/goto :summonCorpse
					|---Else, announce to user.
					} else {
						/echo ERROR: Failed to pick up my soulstone.
					}
				|---Else, if the soulstone was picked up.
				} else {
					|----Open trade with the corpse summoner.
					/click left target
					/delay 10 ${Window[GiveWnd].Open}
					|---If the GiveWnd did not open.
					/if (!${Window[GiveWnd].Open}) {
						|---If there is still time, retry.
						/if (${retryTimer}) {
							/goto :summonCorpse
						|---Else, announce to user.
						} else {
							/echo ERROR: Failed to open the give window.
						}
					|---Else, if the give window is open.
					} else {
						|---Give the soulstone to the corpse summoner.
						/delay 5
						/notify GiveWnd GVW_Give_Button leftmouseup
						/delay 10 !${Window[GiveWnd].Open}
						|---If the GiveWnd did not open
						/if (${Window[GiveWnd].Open} || ${FindItemCount[=${SoulStoneToUse}]}) {
							|---If there is still time, retry.
							/if (${retryTimer}) {
								/goto :summonCorpse
							|---Else, announce to user.
							} else {
								/echo ERROR: Failed to complete the trade.
							}
						|---Else, if the give window is open.
						} else {
							:check_ForCorpse
							|---Loot your summoned corpse.
							/delay 3s ${Spawn[${Me}'s].ID}
							/if (!${Spawn[${Me}'s].ID}) /echo ERROR: My corpse did not appear.
						}
					}
				}
			}
		}
	}
/if (${Debug} || ${Debug_CorpseSummoning}) /echo <== summon_Corpse -|
/RETURN
|-----------------------------------------
SUB corpseSummon_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug CorpseSummoning (On/Off)" Debug_CorpseSummoning bool outer
/RETURN
|-----------------------------------------
SUB corpseSummon_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug CorpseSummoning (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Corpse Summoning,Loot After Summoning (On/Off)" Off
/RETURN
|-----------------------------------------
SUB corpseSummon_CharacterSettings
/RETURN
|-----------------------------------------
Sub corpseSummon_Aliases
	/squelch /alias /summoncorpses /bc Summon Corpses
	/squelch /alias /summoncorpse /echo Summon Corpses
/return
|-----------------------------------------
Sub corpseSummon_Background_Events
	/doevents UseCorpseSummoner
/return
