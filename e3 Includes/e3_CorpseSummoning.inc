|--------------------------------------------------------------------------------------------------------|
|- e3_Macro_CorpseSummoning.inc - designed to work with e3.mac )                                        -|
|- Contains functions to automatically use the (West side only) corpse summoners in the guild lobby.    -|
|--------------------------------------------------------------------------------------------------------|
#event UseCorpseSummoner "<#1#> Use Corpse Summoner#*#"
#event UseCorpseSummoner "[#1#] Use Corpse Summoner#*#"
|#event UseCorpseSummoner "Use Corpse Summoner"
#event UseCorpseSummoner "<#1#> Summon Corpses#*#"
#event UseCorpseSummoner "[#1#] Summon Corpses#*#"
SUB event_UseCorpseSummoner(line, ChatSender)
|/varset Debug_CorpseSummoning TRUE
/if (${Debug} || ${Debug_CorpseSummoning}) /echo |- EVENT_UseCorpseSummoner ==> 
    /if (!${checkEventArgs[${ChatSender},${line},UZR,""]}) /return

    /if (!${Defined[SoulStoneCost]}) /declare SoulStoneCost string outer
    /if (!${Defined[SoulStoneToUse]}) /declare SoulStoneToUse string outer
    /declare HaveSoulStone int local
    /declare equippedItemCount int local
    /declare summonCorpseTimer timer local 30s
    |/declare bankPlatinumWindow string local
    |/declare bankPlatinumWindowAmount int local
    |${Me.PlatinumShared}
    | enhancement, fallback to shared bank plat if not enough bank platinum

    | Check to see if you need to summon a corpse
    /declare i int local 0

    /if (${Spawn[${Me}'s].ID}) {
        /if (${Debug} || ${Debug_CorpseSummoning}) /echo I already have a corpse in the zone, exiting corpseSummoning
        /goto :skip
    }
    
    :retrySummonCorpse
    /varset equippedItemCount 0
    /varset HaveSoulStone 0

    |recover from restarts etc by closing any open windows
    /if (${Window[BigBankWnd].Open}) /nomodkey /notify BigBankWnd DoneButton leftmouseup
    /if (${Window[MerchantWnd].Open}) /nomodkey /notify MerchantWnd MW_Done_Button leftmouseup

    /for i 1 to 30
        /if (${Bool[${InvSlot[${i}].Item}]}) /varcalc equippedItemCount ${equippedItemCount} + 1
    /next i
    |not all low level are fully geared, make this configurable though
    /if (${equippedItemCount} >= 27 || (${Me.Level} < 50 && ${equippedItemCount} >= 10)) {
        |always print this echo for clarity
        /echo I have ${equippedItemCount} items equipped, I dont need to summon a corpse
        /goto :skip
    }
    | If I'm in The Guild Lobby.
    /if (${Zone.ID} == 344) {
        /if (${Debug} || ${Debug_CorpseSummoning}) /echo I am in the Guild Lobby
    } else {
        /if (${Debug} || ${Debug_CorpseSummoning}) /echo I am not in the Guild Lobby, exiting corpseSummoning
        /goto :skip
    }
    |stop doing other stuff
    /call AssistOff |this should not be needed
    /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
	/call StopForAction
    |determine which stone to use based on level
    /call pickSoulstone 
    
    |count soulstone in inventory base slots, dont consider others
    /if (${FindItemCount[${SoulStoneToUse}]}) {
        /for i 1 to 10
            /if (${Me.Inventory[pack${i}].Name.Find[${SoulStoneToUse}]}) /varcalc HaveSoulStone ${HaveSoulStone}+${Me.Inventory[pack${i}].Stack}
        /next i   
    }
    /if (${Debug} || ${Debug_CorpseSummoning}) /echo Already have ${HaveSoulStone} Soulstones in base inventory  

    /if (${HaveSoulStone}==0) {
        |do i have plat, if not get from banker
        /if (${Me.Platinum} < ${SoulStoneCost}) {
            /if (${Me.PlatinumBank} < ${SoulStoneCost}) {
                /echo I do not have enough money in bank to summon corpse, exiting corpseSummon
                /goto :skip
            }
            /nav locxyz 250 420 0
            /delay 5s ${Math.Distance[420,250,0]} < 16
            |/call MoveToLoc 400 250 30 15 |runtomiddlepoint Y X 3sec 15 arrival dist
            /delay 2
            |/call MoveToLoc 477 190 30 15 |move to A Shrewd Banker Y X 3sec 15 arrival dist
            |/delay 2
            /target "A Shrewd Banker"
            /delay 1s ${Target.DisplayName.Equal[A Shrewd Banker]}
            /nav target
            /delay 5s ${Math.Distance[477,190,2]} < 16
            /if (${Math.Distance[477,190,2]} > 16 && ${summonCorpseTimer}) /goto :retrySummonCorpse
            
            /if (!${Window[BigBankWnd].Open}) {
                /click right target
                /delay 1s ${Window[BigBankWnd].Open}
                /nomodkey /notify BigBankWnd BIGB_Money0 leftmouseup          
                /delay 1s ${Window[QuantityWnd].Open}
                /delay 4
                /for i 1 to 9
                    /keypress backspace chat
                    /delay 5
                /next i

                /for i 1 to ${SoulStoneCost.Length}
                    /keypress ${SoulStoneCost.Mid[${i},1]} chat
                    /delay 5
                /next i

                /nomodkey /notify QuantityWnd QTYW_Accept_Button leftmouseup
                /delay 1s !${Window[QuantityWnd].Open}
                /delay 5
                /autoinventory                  
            } 
            /if (${Me.Platinum} < ${SoulStoneCost}) {              
                /if (${summonCorpseTimer}) {
                    /if (${Debug} || ${Debug_CorpseSummoning}) /echo I failed to get platinum from bank, trying again ${Time}  
                    /goto :retrySummonCorpse
                } else {
                    /if (${Debug} || ${Debug_CorpseSummoning}) /echo CorpseSummon failed: I couldnt get platinum from bank ${Time}
                }
            }
        }
        |i have enough platinum, go to soulstone vendor
        |/call MoveToLoc 400 250 30 15 |runtomiddlepoint Y X 3sec 15 arrival dist
        |/delay 2    
        |/call MoveToLoc 350 191 30 15 |move to Disciple Y X 3sec 15 arrival dist
        /delay 2
        /target "A Disciple of Luclin"
        /delay 1s ${Target.DisplayName.Equal[A Disciple of Luclin]}
        /nav target
        |/stick 5
        /delay 5s ${Math.Distance[350,191,2]} < 16
        /if (${Math.Distance[350,191,2]} > 16 && ${summonCorpseTimer}) /goto :retrySummonCorpse
        /delay 2
        /click right target
        /delay 2
        /delay 2s ${Window[MerchantWnd].Open}
        /delay 2
        /nomodkey /notify MerchantWnd ItemList listselect ${Window[MerchantWnd].Child[ItemList].List[=${SoulStoneToUse},2]}
        /delay 5
        /nomodkey /notify MerchantWnd MW_Buy_Button leftmouseup
        /delay 5
        |count soulstone in inventory base slots, dont consider others
        /if (${FindItemCount[${SoulStoneToUse}]}) {
            /varset HaveSoulStone 0
            /for i 1 to 10
                /if (${Me.Inventory[pack${i}].Name.Find[${SoulStoneToUse}]}) /varcalc HaveSoulStone ${HaveSoulStone}+${Me.Inventory[pack${i}].Stack}
            /next i   
        }
        /if (${HaveSoulStone}==0) {
            /if (${summonCorpseTimer}) {
                /if (${Debug} || ${Debug_CorpseSummoning}) /echo I failed to buy a soulstone, trying again ${Time}  
                /goto :retrySummonCorpse
            } else {
                /if (${Debug} || ${Debug_CorpseSummoning}) /echo CorpseSummon failed: I couldnt buy a soulstone ${Time}
            }
        }   
        /if (${Window[MerchantWnd].Open}) /nomodkey /notify MerchantWnd MW_Done_Button leftmouseup
    }
    |i have a soulstone in base inventory slot, go to corpse summoner
    |/call MoveToLoc 400 250 30 15 |runtomiddlepoint Y X 3sec 15 arrival dist
    /delay 2    
    |/call MoveToLoc 321 270 30 15 |move to Disciple Y X 3sec 15 arrival dist
    |/delay 2
    /target "A Priest of Luclin"
    /delay 1s ${Target.DisplayName.Equal[A Priest of Luclin]}
    |/stick 5
    /nav target
    /delay 5s ${Math.Distance[321,270,2]} < 16
    /if (${Math.Distance[321,270,2]} > 16 && ${summonCorpseTimer}) /goto :retrySummonCorpse

    |reuse event_itemTurnin(line, ChatSender, itemName, msgTarget)
    /call event_itemTurnin "itemTurnin ${SoulStoneToUse} to ${Target.ID}" "${Me}" "${SoulStoneToUse}" "${Target.ID}"
    /delay 1s
    /delay 3s ${Spawn[${Me}'s].ID}

    /if (!${Spawn[${Me}'s].ID}) {
        /if (${summonCorpseTimer}) {
            /if (${Debug} || ${Debug_CorpseSummoning}) /echo My corpse did not appear after soulstone turnin, trying again ${Time}  
            /goto :retrySummonCorpse
        } else {
            /if (${Debug} || ${Debug_CorpseSummoning}) /echo CorpseSummon failed: corpse did not appear. this should never happen ${Time}
        }
    }
    /call MoveToLoc 400 250 30 15 |runtomiddlepoint Y X 3sec 15 arrival dist

    :skip
    /if (${Debug} || ${Debug_CorpseSummoning}) /echo <== EVENT_UseCorpseSummoner -|   
/return


SUB pickSoulstone
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
/return 

SUB corpseSummon_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug CorpseSummoning (On/Off)" Debug_CorpseSummoning bool
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