|------------------------------------------------------------------------|
|- e3_Macro_Sell.inc v1.5 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
|- Companion file for e3_Macro_Loot.inc.								-|
|- Sells items flagged with [/Sell] in your Loot_Ini.					-|
|------------------------------------------------------------------------|



|----------------------------------------------------|
|- syncInventory									-|
|----------------------------------------------------|
|- Adds items from you inventory to your Loot_Ini.	-|
|----------------------------------------------------|
#EVENT syncInventory "<#*#> Inventory Items"
#EVENT syncInventory "[MQ2] Inventory Items"
#EVENT syncInventory "<#*#> Update Loot.ini"
#EVENT syncInventory "[MQ2] Update Loot.ini"
#EVENT syncInventory "<#*#> Sync Inventory"
#EVENT syncInventory "[MQ2] Sync Inventory"
#EVENT syncInventory "<#*#> Sync Loot.ini"
#EVENT syncInventory "[MQ2] Sync Loot.ini"
SUB EVENT_syncInventory
/if (${Debug} || ${Debug_Sell}) /echo |- event_inventoryItems ==>
	
	/echo Syncing inventory with ${Loot_Ini}...

	/declare i int local
	/declare x int local

	/declare lootSetting string local
	
	| Check inventory slots 1 - 8 for items, and containers.
	/for i 1 to 8
	
		| If there is an item in inventory slot ${i}
		/if (${Bool[${InvSlot[pack${i}].Item}]}) {
			/call get_lootSetting "pack${i}" 0
		
			| If the item is a container..
			/if (${InvSlot[pack${i}].Item.Container}) {
			
				/for x 1 to ${InvSlot[pack${i}].Item.Container}
			
					/if (${Bool[${Me.Inventory[pack${i}].Item[${x}]}]}) /call get_lootSetting "pack${i}" "${x}"
					
				/next x
			}
		}
	
	/next i
	
/if (${Debug} || ${Debug_Sell}) /echo <== event_inventoryItems -|
/RETURN



|------------------------------------------------------------------------------------------------------------------------------------|
|- EVENT_sellItems																													-|
|------------------------------------------------------------------------------------------------------------------------------------|
|- Commands bots, including yourself, to open trade with the nearest merchant, and sell items flagged as [/Sell] in your Loot_Ini.	-|
|------------------------------------------------------------------------------------------------------------------------------------|
#EVENT sellItems "[MQ2] Sell Items"
#EVENT sellItems "<#*#> Sell Items"
#EVENT sellItems "[MQ2] AutoSell"
#EVENT sellItems "<#*#> AutoSell"
#EVENT sellItems "[MQ2] Auto Sell"
#EVENT sellItems "<#*#> Auto Sell"
SUB EVENT_sellItems
/if (${Debug} || ${Debug_Sell}) /echo |- EVENT_sellItems ==>
	
	| Record starting location.
	/declare startingLoc string local ${Me.Loc.Replace[ ,]}
		
	| Sync inventory with Loot_Ini.
	| /call EVENT_syncInventory
		
	| Open trade with the nearest merchant.
	/call openMerchant
		
	/if (${Window[MerchantWnd].Open}) {
	
		| Open bags.
		/keypress OPEN_INV_BAGS
	
		| Sell Items.
		/call sellItems
		
		| Move back to starting location.	
		/squelch /moveto loc ${startingLoc.Arg[1,,]} ${startingLoc.Arg[2,,]}
		
		| Close merchant.
		/if (${Window[MerchantWnd].Open}) /call closeMerchant
	
		| Destroy bad items.
		/if (${DestroyableItems.Size}) /call destroyItems
	
		| Close bags.
		/keypress CLOSE_INV_BAGS
		
		/echo I have finished selling.
	}
	
/if (${Debug} || ${Debug_Sell}) /echo <== EVENT_sellItems -|
/RETURN



|----------------------------------------|
|- Opens all containers in inventory.	-|
|----------------------------------------|
SUB openBags
/if (${Debug} || ${Debug_Sell}) /echo |- openBags ==>

	| Open all bags in inventory.
	/declare i int local
	/declare retryTimer timer local
	/declare miscTimer timer local
	
	/for i 1 to 8
		
		/varset miscTimer 30
		/if (${Me.Inventory[pack${i}].Container}) {
			
			:openBag
			/if (${Debug} || ${Debug_Sell}) /echo |- openBags -| :openBag
		
			| Open container
			/if (!${Window[pack${i}].Open}) {
				/itemnotify pack${i} rightmouseup
				/delay 5 ${Window[pack${i}].Open} || ${Window[TradeSkillWnd].Open}
				/if (!${Window[pack${i}].Open} && !${Window[TradeSkillWnd].Open} && ${miscTimer}) /goto :openBag
				
				| Open tradeskill container.
				/if (${Window[TradeSkillWnd].Open}) {
					:openExperimentWnd
					/notify TradeSkillWnd ExperimentButton leftmouseup
					/delay 5 !${Window[TradeSkillWnd].Open}
					/if (${Window[TradeSkillWnd].Open} && ${miscTimer}) /goto :openExperimentWnd
				}
			}
		}
	
	/next i
	
/if (${Debug} || ${Debug_Sell}) /echo <== openBags -|
/RETURN



|----------------------------------------|
|- Closes all containers in inventory.	-|
|----------------------------------------|
SUB closeBags
/if (${Debug} || ${Debug_Sell}) /echo |- closeBags ==>

	| Close all bags in inventory.
	/declare i int local
	/declare retryTimer timer local
	/declare miscTimer timer local
	
	/for i 1 to 8
		
		/varset miscTimer 30
		/if (${Me.Inventory[pack${i}].Container}) {
			
		:closeBag
		/if (${Debug} || ${Debug_Sell}) /echo |- closeBags -| :closeBag
		
			| Open container
			/if (${Window[pack${i}].Open}) {
				/itemnotify pack${i} rightmouseup
				/delay 5 !${Window[pack${i}].Open}
				/if (${Window[pack${i}].Open} && ${miscTimer}) /goto :closeBag
			}
		}
	
	/next i
	
/if (${Debug} || ${Debug_Sell}) /echo <== closeBags -|
/RETURN



|--------------------------------------------|
|- Opens trade with the nearest merchant.	-|
|--------------------------------------------|
SUB openMerchant(int providedID)
/if (${Debug} || ${Debug_Sell}) /echo |- openMerchant ==>

	/declare miscTimer timer local
	/declare merchantTotal int local -1
	/declare merchantID int local ${If[${providedID},${providedID},${Spawn[Merchant].ID}]}

	/if (!${Spawn[Merchant].ID} || !${Spawn[Merchant].LineOfSight}) {
		/echo There are no merchants nearby!
	} else {
	
		| Move to the merchant.
		/call TrueTarget ${merchantID}
		/call MoveTo ${Target.Y} ${Target.X} 80
	
		| Open Trade with merchant.
		/if (${Target.Distance} < 20) {
		
			| Set timer
			/varset miscTimer 100
		
			| Right click merchant, and wait for window to open.
			:openMerchant
			/if (${Debug} || ${Debug_Sell}) /echo |- openMerchant -| :openMerchant
			
			/if (!${Window[MerchantWnd].Open}) /click right target
			/delay 5 ${Window[MerchantWnd].Open}
			/if (!${Window[MerchantWnd].Open}) {
				/if (${miscTimer}) {
					/goto :openMerchant
				} else {
					/echo Failed to open trade with [${Target.CleanName}].
				}
			}
			
			| Wait for merchant's item list to populate.
			:populateMerchant
			/if (${Debug} || ${Debug_Sell}) /echo |- openMerchant -| :populateMerchant
			/if (${merchantTotal}!=${Window[MerchantWnd].Child[ItemList].Items}) {
				/varset merchantTotal ${Window[MerchantWnd].Child[ItemList].Items}
				/delay 2
				/if (${miscTimer}) /goto :populateMerchant
			}
		}
	}

/if (${Debug} || ${Debug_Sell}) /echo <== openMerchant -|
/RETURN



|--------------------|
|- closeMerchant	-|
|--------------------|
SUB closeMerchant
/if (${Debug} || ${Debug_Sell}) /echo |- closeMerchant ==>

	/declare miscTimer timer local 50
	
	:closeMerchant
	/if (${Debug} || ${Debug_Sell}) /echo |- closeMerchant -| :closeMerchant
	/notify MerchantWnd MW_Done_Button leftmouseup
	/delay 5 ${Window[MerchantWnd].Open}
	/if (${Window[MerchantWnd].Open} && ${miscTimer}) /goto :closeMerchant
	
	
/if (${Debug} || ${Debug_Sell}) /echo <== closeMerchant -|
/RETURN



|----------------------------------------------------------------------------------------|
|- Sells items in your inventory that have been flagged as [/Sell] in your Loot_Ini.	-|
|----------------------------------------------------------------------------------------|
SUB sellItems
/if (${Debug} || ${Debug_Sell}) /echo |- sellItems ==>

	/declare lootSetting string local
	/declare retryTimer timer local
	/declare i int local
	/declare e int local
	
	| Scan inventory for items to sell
	/for i 1 to 8
	
		/if (${Bool[${Me.Inventory[pack${i}]}]}) {
		
			| If the item in pack slot 'i' IS a container
			/if (${Me.Inventory[pack${i}].Container}) {
			
				| Look at each bag slot for items to sell
				/for e 1 to ${Me.Inventory[pack${i}].Container}
				
					/if (${Bool[${Me.Inventory[pack${i}].Item[${e}]}]}) {
					
						| Get the loot setting for the item in pack slot 'i' slot 'e'
						/call get_lootSetting "pack${i}" "${e}"
					
						| Destroy the item
						/if (${lootSetting.Find[Destroy]}) {
							/echo Flagging [${Me.Inventory[pack${i}].Item[${e}]}] to be destroyed...
							/call BuildArray DestroyableItems "${Me.Inventory[pack${i}].Item[${e}]}"
						
						| Sell the item
						} else /if (${lootSetting.Find[Sell]}) {
					
							| Check that the item has value.
							/if (!${Me.Inventory[pack${i}].Item[${e}].Value}) {
								/if (${destroyUnsold}) {
									/echo [${Target.CleanName}] will not buy [${Me.Inventory[pack${i}].Item[${e}]}].  Flagging it to be destroyed.
									/call BuildArray DestroyableItems "${Me.Inventory[pack${i}].Item[${e}]}"
								}
							} else {
					
								| Select the item to sell
								/varset retryTimer 30

								:retrySelect_Pack
								/if (${Debug} || ${Debug_Sell}) /echo |- sellItems -| :retrySelect_Pack
								/itemnotify in pack${i} ${e} leftmouseup
								
								/delay ${retryTimer} ${Window[MerchantWnd].Child[MW_SelectedItemLabel].Text.Equal[${Me.Inventory[pack${i}].Item[${e}]}]}
					
								| If the item was not selected
								/if (!${Window[MerchantWnd].Child[MW_SelectedItemLabel].Text.Equal[${Me.Inventory[pack${i}].Item[${e}]}]}) {
									
									| If I have time, retry to select the item
									/if (${retryTimer}) {
										/goto :retrySelect_Pack
									} else {
										/echo ERROR: Failed to select [${Me.Inventory[pack${i}].Item[${e}]}], skipping.
									}
								} else {
						
									| Check that the buy button is enabled
									/if (!${Window[MerchantWnd].Child[MW_Sell_Button].Enabled}) {
										/if (${destroyUnsold}) {
											/echo [${Target.CleanName}] will not buy [${Me.Inventory[pack${i}].Item[${e}]}].  Flagging it to be destroyed.
											/call BuildArray DestroyableItems "${Me.Inventory[pack${i}].Item[${e}]}"
										}
									} else {
						
										| Sell the item
										/varset retryTimer 30
										/echo Selling [${Me.Inventory[pack${i}].Item[${e}]}].
										
										:SellItem_Pack
										/if (${Debug} || ${Debug_Sell}) /echo |- sellItems -| :SellItem_Pack
										/notify MerchantWnd MW_Sell_Button leftmouseup
										/delay ${retryTimer} ${Window[QuantityWnd].Open} || !${Bool[${Me.Inventory[pack${i}].Item[${e}]}]}
						
										| Close the quantity window
										/if (${Window[QuantityWnd].Open}) {
											/notify QuantityWnd QTYW_Accept_Button leftmouseup
											/delay ${retryTimer} !${Window[QuantityWnd].Open}
											/delay ${retryTimer} !${Bool[${Me.Inventory[pack${i}]}]}
										}
						
										| If the item is still in my inventory
										/if (${Bool[${Me.Inventory[pack${i}].Item[${e}]}]} && ${retryTimer}) {
											/goto :SellItem
										} else {
											/echo ERROR: Failed to sell [${Me.Inventory[pack${i}].Item[${e}]}], skipping.
										}
									}
								}
							}
						}
					}
				
				/next e
			
			| If the item in pack slot 'i' is NOT a container
			} else {
			
				| Get the loot setting for the item in pack slot 'i'
				/call get_lootSetting "pack${i}" 0
			
				| Destroy the item
				/if (${lootSetting.Find[Destroy]}) {
					/echo Flagging [${InvSlot[pack${i}].Item.Name}] to be destroyed...
					/call BuildArray DestroyableItems "${InvSlot[pack${i}].Item.Name}"
				
				| Sell the item
				} else /if (${lootSetting.Find[Sell]}) {
				
					| Check that the item has value
					/if (!${Me.Inventory[pack${i}].Item.Value}) {
						/if (${destroyUnsold}) {
							/echo [${Target.CleanName}] will not buy [${Me.Inventory[pack${i}].Item.Name}].  Flagging it to be destroyed.
							/call BuildArray DestroyableItems "${Me.Inventory[pack${i}].Item.Name}"
						}
					} else {
				
						| Select the item to sell
						/varset retryTimer 30

						:retrySelect
						/if (${Debug} || ${Debug_Sell}) /echo |- sellItems -| :retrySelect
						/itemnotify in pack${i} leftmouseup
						
						/delay ${retryTimer} ${Window[MerchantWnd].Child[MW_SelectedItemLabel].Text.Equal[${Me.Inventory[pack${i}]}]}
					
						| If the item was not selected
						/if (!${Window[MerchantWnd].Child[MW_SelectedItemLabel].Text.Equal[${Me.Inventory[pack${i}]}]}) {
							
							| If I have time, retry to select the item
							/if (${retryTimer}) {
								/goto :retrySelect
							} else {
								/echo ERROR: Failed to select [${Me.Inventory[pack${i}]}], skipping.
							}
						} else {
				
							| Check that the buy button is enabled
							/if (!${Window[MerchantWnd].Child[MW_Sell_Button].Enabled}) {
								/if (${destroyUnsold}) {
									/echo [${Target.CleanName}] will not buy [${Me.Inventory[pack${i}]}].  Flagging it to be destroyed.
									/call BuildArray DestroyableItems "${Me.Inventory[pack${i}]}"
								}
							} else {
				
								| Sell the item
								/varset retryTimer 30
								/echo Selling [${Me.Inventory[pack${i}]}].
								
								:SellItem
								/if (${Debug} || ${Debug_Sell}) /echo |- sellItems -| :SellItem
								/notify MerchantWnd MW_Sell_Button leftmouseup
								/delay ${retryTimer} ${Window[QuantityWnd].Open} || !${Bool[${Me.Inventory[pack${i}]}]}
				
								| Close the quantity window
								/if (${Window[QuantityWnd].Open}) {
									/notify QuantityWnd QTYW_Accept_Button leftmouseup
									/delay ${retryTimer} !${Window[QuantityWnd].Open}
									/delay ${retryTimer} !${Bool[${Me.Inventory[pack${i}]}]}
								}
								
								| If the item is still in my inventory
								/if (${Bool[${Me.Inventory[pack${i}]}]} && ${retryTimer}) {
									/goto :SellItem
								} else {
									/echo ERROR: Failed to sell [${Me.Inventory[pack${i}]}], skipping.
								}
							}
						}
					}
				}
			}
		}
		
	/next i

/if (${Debug} || ${Debug_Sell}) /echo <== sellItems -|
/RETURN



SUB destroyItems
/if (${Debug} || ${Debug_Sell}) /echo |- destroyItem ==>
	
	/declare i int local
	/declare e int local
	/declare y int local
	
	/declare retryTimer timer local
	/declare badItem string local
	
	/for i 1 to ${DestroyableItems.Size}
	
		/varset badItem ${DestroyableItems[${i}]}

		/for e 1 to 8
		
			/if (!${Me.Inventory[pack${e}].Container}) {
				
				/if (${badItem.Equal[${Me.Inventory[pack${e}]}]}) {
					/varset retryTimer 50
					
					:retry_SlotPickup
					/itemnotify pack${e} leftmouseup
					/delay 5 ${Cursor.ID} || ${Window[QuantityWnd].Open}
					
					| If the quantity window is open.
					/if (${Window[QuantityWnd].Open}) {
						/notify QuantityWnd QTYW_Accept_Button leftmouseup
						/delay 10 !${Window[QuantityWnd].Open}
					}
					
					/delay 5 ${Cursor.ID}
					
					/if (${badItem.Equal[${Cursor}]}) {
						/echo Destroying [${Cursor}].
						/destroy
					} else {
						/if (${retryTimer}) {
							/goto :retry_SlotPickup
						} else {
							/echo ERROR: I failed to destroy [${badItem}], skipping.
						}
					}
				}
			} else {
			
				/for y 1 to ${Me.Inventory[pack${e}].Container}
				
					/if (${badItem.Equal[${Me.Inventory[pack${e}].Item[${y}]}]}) {
					
						/varset retryTimer 50
					
						:retry_BagPickup
						/itemnotify in pack${e} ${y} leftmouseup
						/delay 5 ${Cursor.ID} || ${Window[QuantityWnd].Open}
						
						| If the quantity window is open.
						/if (${Window[QuantityWnd].Open}) {
							/notify QuantityWnd QTYW_Accept_Button leftmouseup
							/delay 10 !${Window[QuantityWnd].Open}
						}
						
						/delay 5 ${Cursor.ID}
						
						/if (${badItem.Equal[${Cursor}]}) {
							/echo Destroying [${Cursor}].
							/destroy
						} else {
							/if (${retryTimer}) {
								/goto :retry_BagPickup
							} else {
								/echo ERROR: I failed to destroy [${badItem}], skipping.
							}
						}
					}
				/next y
			}
		
		/next e
		
	/next i

/if (${Debug} || ${Debug_Sell}) /echo <== destroyItem -|
/RETURN



SUB sell_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Sell (On/Off)" Debug_Sell bool outer
/if (${Debug} || ${Debug_Sell}) /echo |- sell_Setup ==>

	/call iniToVarV "${genSettings_Ini},Sell,Destroy Unsold Items(On/Off)" destroyUnsold bool outer

/if (${Debug} || ${Debug_Sell}) /echo <== sell_Setup -|
/RETURN


 
Sub sell_Background_Events
	/doevents syncInventory
	/doevents sellItems
  /doevents Combine
/return



SUB sell_MacroSettings
/if (${Debug} || ${Debug_Sell}) /echo |- _MacroSettings ==>

	/call WriteToIni "${advSettings_Ini},Debug,Debug Sell (On/Off)" Off
	
	/call WriteToIni "${genSettings_Ini},Sell,Destroy Unsold Items(On/Off)" Off

/if (${Debug} || ${Debug_Sell}) /echo <== _MacroSettings -|
/RETURN



SUB sell_CharacterSettings
/if (${Debug} || ${Debug_Sell}) /echo |- _CharacterSettings ==>

/if (${Debug} || ${Debug_Sell}) /echo <== _CharacterSettings -|
/RETURN



Sub sell_Aliases
/if (${Debug} || ${Debug_Sell}) /echo |- _Aliases ==>

	/squelch /alias /syncInventory /bc Sync Inventory
	/squelch /alias /autosell /echo Auto Sell

/if (${Debug} || ${Debug_Sell}) /echo <== _Aliases -|
/return