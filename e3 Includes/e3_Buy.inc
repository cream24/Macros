|- EVENT_buyitem 
|- Buys the specified item(s) from the nearest merchant. Can optionally specify a merchant
|- by targetting it prior to issuing the command.
|- Usage: /autobuy item1|amount1/item2|amount2 /only|Stryd
|- User may input as many items as desired, but there must be a space between items and
|- any logic arguments such as /not and /only. 
|- Output goes to /bc, and characters connected to EQBC will trigger an event that passes in
|- itemToBuy and numToBuy in the eventParams

|- Written by Stryd of PEQ

#EVENT AutoBuy "[#1#] [#2#] AutoBuy #3#"
#EVENT AutoBuy "<#1#> [#2#] AutoBuy #3#"
#EVENT AutoBuy "[#1#] [#2#] AutoBuy #3# |/|#*#"
#EVENT AutoBuy "<#1#> [#2#] AutoBuy #3# |/|#*#"
#EVENT AutoBuy "[#1#] AutoBuy #3#"
#EVENT AutoBuy "<#1#> AutoBuy #3#"
#EVENT AutoBuy "[#1#] AutoBuy #3# |/|#*#"
#EVENT AutoBuy "<#1#> AutoBuy #3# |/|#*#"
#EVENT noMoney "#*#You cannot afford#*#"
#EVENT noMoney "#*#you cannot afford#*#"
#EVENT noMoney "#*#You can't afford#*#"
#EVENT noMoney "#*#you can't afford#*#"
#EVENT invFull "#*#Your inventory appears full! How can you buy more?#*#" 
SUB EVENT_AutoBuy(line, ChatSender, merchantID, eventParams)
/if (${Debug}) /echo |- EVENT_AutoBuy [${eventParams}] ==>
	/if (!${checkEventArgs[${ChatSender},${line},UZR,${eventParams}]}) /return
	/declare itemToBuy string local
	/declare numToBuy int local
	/declare lastItemCount int local
	/declare numToBuyRemaining int local
	/declare startingLoc string local
	/declare listPosition int local
	/declare i int local 1
	| Record starting location.
	/varset startingLoc string local ${Me.Loc.Replace[ ,]}
	/varset noMoney FALSE
	/varset invFull FALSE
	| Open trade with the nearest merchant and waits for merchant list to populate
	/call openMerchant ${merchantID}

	:lookForItems
	| Makes sure merchant window is open
	/if (${Window[MerchantWnd].Open}) {
		/if (${eventParams.Arg[${i},/].Length}) {
			/varset itemToBuy ${eventParams.Arg[${i},/].Arg[1,|]}
			/varset numToBuy ${eventParams.Arg[${i},/].Arg[2,|]}
			/varset lastItemCount ${FindItemCount[${itemToBuy}]}
			/echo Buying ${itemToBuy} (x${numToBuy})
			| Searches merchant's item list for the item, sets variable for list position of item.
			/varset listPosition ${Window[MerchantWnd].Child[ItemList].List[=${itemToBuy},2]}
			| Checks if the item has been found on the merchant's list.
			/if (!${listPosition}) {
				/docommand ${ChatToggle} This merchant doesn't have ${itemToBuy}!
			} else {
				| Selects item on merchant's list.
				/notify MerchantWnd ItemList listselect ${listPosition}
				/delay 10 ${Window[MerchantWnd].Child[MW_SelectedItemLabel].Text.Equal[${itemToBuy}]}
				/if (${i} > 1) /delay 10
				| If the item is not stackable, need to press the buy button until we have numToBuy
				/if (!${Merchant.Item[=${itemToBuy}].Stackable}) {
					/if (${Debug}) /echo Item is not stackable. Attempting to buy...
					:buying
					| Prevents loop from repeating if out of money or inventory space.
					/if (${noMoney} || ${invFull} || ${Bool[${Cursor}]}) {
						/docommand ${ChatToggle} I am out of money, my inventory is full, or I have an item on my cursor! Exiting.
						/goto :closeOut
					}
					/notify MerchantWnd MW_Buy_Button leftmouseup
					/delay 20 ${FindItemCount[=${itemToBuy}]} != ${oldItemCount}
					/delay 5
					| Finds out how many you have bought and compares to numToBuy
					/if (${Math.Calc[${FindItemCount[=${itemToBuy}]}-${oldItemCount}]} < ${numToBuy}) /goto :buying
				} else {
					| If numToBuy is more than one stack of itemToBuy, buy full stacks until the
					| amount remaining is less than one stack. Then, buy just the number remaining.
					| This is currently broken if the /delay is any less than 2s. Yuck.
					/varset numToBuyRemaining ${numToBuy}
					:stackBuyLoop
					| Prevents loop from repeating if out of money or inventory space.
					/if (${noMoney} || ${invFull} || ${Bool[${Cursor}]}) {
						/docommand ${ChatToggle} I am out of money, my inventory is full, or I have an item on my cursor! Exiting.
						/goto :closeOut
					}
					/doevents flush
					/varset lastItemCount ${FindItemCount[=${itemToBuy}]}
					/if (${Debug}) /echo ${numToBuyRemaining} > ${Merchant.Item[=${itemToBuy}].StackSize}
					/if (${numToBuyRemaining} > ${Merchant.Item[=${itemToBuy}].StackSize}) {
						/if (${Debug}) /echo Buying ${itemToBuy} (x${Merchant.Item[=${itemToBuy}].StackSize}) (stack)
						/buyitem ${Merchant.Item[=${itemToBuy}].StackSize}
						| wait for item to show up in inventory
						/delay 20 ${FindItemCount[=${itemToBuy}]} > ${oldItemCount}
						/if (${FindItemCount[=${itemToBuy}]} > ${oldItemCount}) {
							/varset numToBuyRemaining ${Math.Calc[${numToBuyRemaining}-${Merchant.Item[=${itemToBuy}].StackSize}]}
						} else {
							/echo Oops! Previous stack buy failed. Lag? Retrying...
						}
						/delay 5
						/doevents
						/goto :stackBuyLoop
					} else {
						/doevents EVENT_noMoney
						/doevents EVENT_invFull
						| Prevents loop from repeating if out of money or inventory space.
						/if (${noMoney} || ${invFull} || ${Bool[${Cursor}]}) /goto :closeOut
						/if (${Debug}) /echo Item is stackable and amount is less than or equal to stacksize. Attempting to buy...
						/if (${Debug}) /echo Buying ${itemToBuy} (x${numToBuyRemaining})
						/buyitem ${numToBuyRemaining}
					}
				}
			}

			/varcalc i ${i}+1
			/goto :lookForItems
		}
		/docommand ${ChatToggle} Done buying items.
	}

	:closeOut
	| Close merchant window
	/if (${Window[MerchantWnd].Open}) /call closeMerchant
	| Move back to starting location.
	/squelch /moveto loc ${startingLoc.Arg[1,,]} ${startingLoc.Arg[2,,]}
	| Flush duplicate events
	/doevents flush
/if (${Debug}) /echo <== EVENT_buyitem -|
/return

Sub EVENT_noMoney
  /bc I do not have enough money for this purchase!
  /varset noMoney TRUE
/return

Sub EVENT_invFull
  /bc My inventory is full!
  /varset invFull TRUE
/return

SUB buy_Setup
  /declare noMoney bool outer FALSE
  /declare invFull bool outer FALSE
/RETURN

SUB Buy_MacroSettings
/RETURN

SUB Buy_CharacterSettings
/RETURN

Sub Buy_Aliases
	/noparse /squelch /alias /autobuy /bc ${If[${Target.ID},[${Target.ID}],]} AutoBuy
/return

Sub Buy_Background_Events
  /doevents AutoBuy
/return