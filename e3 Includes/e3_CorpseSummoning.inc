|------------------------------------------------------------------------------------|
|- e3_Macro_CorpseSummoning.inc v1.0 ( originally designed to work with e3.mac )    -|
|- Written by Killians of PEQ                                                        -|
|------------------------------------------------------------------------------------|
|- Contains functions to automatically use the corpse summoners in the guild lobby.    -|
|------------------------------------------------------------------------------------|
#event UseCorpseSummoner "<#1#> Summon Corpses"
#event UseCorpseSummoner "[#1#] Summon Corpses"
SUB event_UseCorpseSummoner(line, ChatSender)
    /bcaa //stand
  /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return
  | Check to see if you need to summon a corpse
    /declare itemCount int local
    /declare SoulStoneCost string outer ${Macro.Return.Arg[2,_]}
    /declare SoulStoneToUse string outer ${Macro.Return.Arg[1,_]}
    /declare HaveSoulStone int outer
    /varset HaveSoulStone 0
    /declare count_i int outer
    /declare merchantTotal int outer
    /varset count_i 0
    /for count_i 1 to 30
    /if (${Bool[${InvSlot[${count_i}].Item}]}) /varcalc itemCount ${itemCount} + 1
    /next count_i

    /if (${itemCount} >= 27) {
    /echo Looks like i have all my gear, so i don't need to summon a corpse.
    /goto :skip
    }
    | If I'm in The Guild Lobby.
    /if (${Zone.ID} == 344) {
    /echo I'm in The Guild Lobby
    } else {
    /return
    }
    /bcaa //stand
    /debuffsoff
    /call find_Soulstone
    /call checkforstone
    /call runtomiddlepoint
    /call movetobanker
    /call getmoneyfrombanker
    /call runtomiddlepoint
    /call movetocomponentsven
    /call buysoulstone
    /call runtomiddlepoint
    /call movetosummoner
    /call summon_Corpse
    /call runtomiddlepoint
    :skip
/RETURN

SUB checkforstone
/varset HaveSoulStone ${FindItemCount[${SoulStoneToUse}]}
/echo Soulstones ${HaveSoulStone}
/RETURN

SUB find_Soulstone
    /if (${Me.Level} <= 20) {
        /varset SoulStoneToUse Minor Soulstone
        /varset SoulStoneCost 12
    } else /if (${Me.Level} > 20 && ${Me.Level} <= 30) {
        /varset SoulStoneToUse Lesser Soulstone
        /varset SoulStoneCost 28
    } else /if (${Me.Level} > 30 && ${Me.Level} <= 40) {
        /varset SoulStoneToUse Soulstone
        /varset SoulStoneCost 55
    } else /if (${Me.Level} > 40 && ${Me.Level} <= 50) {
        /varset SoulStoneToUse Greater Soulstone
        /varset SoulStoneCost 87
    } else /if (${Me.Level} > 50 && ${Me.Level} <= 55) {
        /varset SoulStoneToUse Faceted Soulstone
        /varset SoulStoneCost 120
    } else /if (${Me.Level} > 55 && ${Me.Level} <= 70) {
        /varset SoulStoneToUse Pristine Soulstone
        /varset SoulStoneCost 165
    } else /if (${Me.Level} > 70 && ${Me.Level} <= 75) {
        /varset SoulStoneToUse Glowing Soulstone
        /varset SoulStoneCost 265
    } else /if (${Me.Level} > 75 && ${Me.Level} <= 80) {
        /varset SoulStoneToUse Prismatic Soulstone
        /varset SoulStoneCost 425
    } else /if (${Me.Level} > 80 && ${Me.Level} <= 85) {
        /varset SoulStoneToUse Iridescent Soulstone
        /varset SoulStoneCost 530
    } else /if (${Me.Level} > 85 && ${Me.Level} <= 90) {
        /varset SoulStoneToUse Phantasmal Soulstone
        /varset SoulStoneCost 635
    } else /if (${Me.Level} > 90 && ${Me.Level} <= 95) {
        /varset SoulStoneToUse Luminous Soulstone
        /varset SoulStoneCost 750
    } else /if (${Me.Level} > 95 && ${Me.Level} <= 100) {
        /varset SoulStoneToUse Coalescent Soulstone
        /varset SoulStoneCost 865
    }

/RETURN ${SoulStoneToUse}_${SoulStoneCost}


SUB summon_Corpse(SoulStoneToUse)
    :tarpriest
    /Target "A Priest of Luclin"
    /if (${Target.ID}) {
    |got a target
    } else {
    /goto :tarpriest
    }
    /declare e int local
    /varset count_i 0
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
        /for count_i 1 to ${numInventorySlots}
        |---If the item in the packslot is a soulstone, pick it up.
        /if (!${Me.Inventory[pack${count_i}].Container} && ${SoulStoneToUse.Equal[${Me.Inventory[pack${count_i}]}]}) {
        /itemnotify pack${count_i} leftmouseup
        } else {
        |---If the item in the packslot is a container, search it.
        /for e 1 to ${Me.Inventory[pack${count_i}].Container}
        /if (${SoulStoneToUse.Equal[${Me.Inventory[pack${count_i}].Item[${e}]}]}) {
        /if (!${Window[pack${count_i}].Open}) /itemnotify pack${count_i} rightmouseup
        /delay 1s ${Window[pack${count_i}].Open}
        /itemnotify in pack${count_i} ${e} leftmouseup
        }
        /next e
        }
        /next count_i
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
/RETURN

SUB buysoulstone
/if (${HaveSoulStone}) {
/RETURN
}
:tardisciple
/Target "A Disciple of Luclin"
/if (${Target.ID}) {
|got a target
} else {
/goto :tardisciple
}
:opendisciplewindow
/click right target
/delay 10 ${Window[MerchantWnd].Open}
/if (!${Window[MerchantWnd].Open}) {
/goto ::opendisciplewindow
} else {
/echo ERROR: Failed to open the soulstone merchant.
}
/varset merchantTotal -1
:populateMerchant
/if (${merchantTotal}!=${Window[MerchantWnd].Child[ItemList].Items}) {
/varset merchantTotal ${Window[MerchantWnd].Child[ItemList].Items}
/delay 2
/goto :populateMerchant
}
/nomodkey /notify MerchantWnd ItemList listselect ${Window[MerchantWnd].Child[ItemList].List[=${SoulStoneToUse},2]}
/delay 2
/if (!${FindItemCount[${SoulStoneToUse}]}) /notify MerchantWnd MW_Buy_Button leftmouseup
/delay 2
|If i failed to buy the soulstone.
/if (!${FindItemCount[${SoulStoneToUse}]}) {
/if (${retryTimer}) {
/goto :buyLoop
} else {
/echo ERROR: Failed to buy a soulstone!
}
} else {
/nomodkey /notify MerchantWnd MW_Done_Button leftmouseup
}

/RETURN

SUB movetosummoner
/echo moving to summoner
:movetosummonerloop
/moveto loc 321 270
/delay 1s
/if (${Math.Distance[321,270,2]} < 16) {
    :summonerclose
    /Target "A Priest of Luclin"
    /stick 8
    /delay 1s
    /if (${Target.Distance} < 9) {
    /keypress forward
    /echo moved to summoner
    } else {
    /goto :summonerclose
    }
} else {
/goto :movetosummonerloop
}
/RETURN

SUB movetocomponentsven
/if (${HaveSoulStone}) {
/RETURN
}
/echo moving to disciple
:movetocomponentsvenloop
/moveto loc 350 191
/delay 1s
/if (${Math.Distance[350,191,2]} < 16) {
    :discipleclose
    /Target "A Disciple of Luclin"
    /stick 8
    /delay 1s
    /if (${Target.Distance} < 9) {
    /keypress forward
    /echo moved to disciple
    } else {
    /goto :discipleclose
    }
} else {
    /goto :movetocomponentsvenloop
}
/RETURN
   
SUB movetobanker
/if (${Me.Platinum} > ${SoulStoneCost}) {
/RETURN
}
/if (${HaveSoulStone}) {
/RETURN
}
/echo moving to banker
:movetobankerloop
/moveto loc 477 190
/delay 1s
/if (${Math.Distance[477,190,2]} < 16) {
    :bankercloser
    /Target "A Shrewd Banker"
    /stick 8
    /delay 1s
    /if (${Target.Distance} < 9) {
    /keypress forward
    /echo moved to banker
    }    else {
    /goto :bankercloser
    }
} else {
    /goto :movetobankerloop
}
/RETURN

SUB getmoneyfrombanker
/if (${Me.Platinum} > ${SoulStoneCost}) {
/RETURN
}
/if (${HaveSoulStone}) {
/RETURN
}
:startwithdraw
/Target "A Shrewd Banker"
/if (${Target.ID}) {
|got a target
} else {
/goto :startwithdraw
}
:openbanker
/if (!${Window[BigBankWnd].Open}) {
/click right target
} else {
/goto :openbanker
}
:ammountofplat
/if (!${Window[QuantityWnd].Open}) {
    /notify BigBankWnd BIGB_Money0 leftmouseup
    }
    /delay 5 ${Window[QuantityWnd].Open}
    /for count_i 1 to 9
        /keypress backspace chat
        /delay 3
    /next count_i
    /varset count_i 0
    /for count_i 1 to ${SoulStoneCost.Length}
        /keypress ${SoulStoneCost.Mid[${count_i},1]} chat
        /delay 3
    /next count_i
    /notify QuantityWnd QTYW_Accept_Button leftmouseup
    /delay 2
    /autoinventory
else {
/echo QuantityWnd is open
}
/if (${Me.Platinum} < ${SoulStoneCost}) {
/goto :startwithdraw
}
/RETURN

SUB runtomiddlepoint
:loopmiddlep
/moveto loc 400 250
|/echo ${Math.Distance[400,250,1]}
/if (${Math.Distance[400,250,1]} < 20) {
/echo all done moved to middle
/RETURN
} else {
/delay 5
/goto :loopmiddlep
}
/RETURN

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
    /squelch /alias /summoncorpses /bc Use Corpse Summoner
    /squelch /alias /summoncorpse /echo Use Corpse Summoner
/return
|-----------------------------------------
Sub corpseSummon_Background_Events
    /doevents UseCorpseSummoner
/return