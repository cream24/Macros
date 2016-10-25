|******************************************************************************************************************************
| MQ2Cast_Spell_Routines.inc v3.03
|
| HISTORY
| - Written by A_Enchanter_00
| - Modified 2008811 by Kroak
|   - Changed /casting line to use the pipe format to correct memorization issues.
| - v3.00 2014-01-26 by woobs
|   - Adjusted for new MQ2Cast v10.0, which no longer handles item swapping/bandoliers.
|   - Add automatic Item-Swapping for items that are must-equip.
|   - Add Bandolier processing. Will read Bandolier sets saved by MQ2Bandolier, but will do the 
|     actual swapping in this macro. This is done to make sure swapping(s) are complete before casting,
|     otherwise the macro will continue while the swap(s) are still going on. Since you can't swap
|     items while casting, this doesn't work out very well. When casting is done, the item(s) are 
|     swapped back.
|   - Adjust logic after Exchange to wait for item swapping to complete.
|   - Miscellaneous fixes for old coding (ie, reformat references to InvSlot)
| - v3.01 2014-01-29 by woobs
|   - Removed /while loop until this MQ2 feature is ready for prime-time.
|   - Added detection of an integer as a valid gem type in case you forget "gem", ie. "8", "12" instead of "gem8" or "gem12"
|     (thanks PeteSampras).
|   - Added SRFindBookName to attempt auto detection of spells in your book so you don't need the Rk II/III in the name.
|     Used a Subroutine to enable you to call it directly from your macro, if you wish, as follows:
|         Usage:   /call SRFindBookName "Blezon's Mending"
|         Returns: The Spell name found in your book in ${Macro.Return}.  Empty string if not found. 
| - v3.03 2014-11-06 by TreeHuginDruid
|   - Fix to ensure FOCUS aa of a spell is not cast but the actual spell itself, based on aa type check.
|
| This MQ2Cast_Spell_Routines.inc is much like the normal Spell_Routines.inc by Rusty. This uses the
| MQ2Cast Plugin by s0rcier.
|
| This macro also requires MQ2Exchange for bandolier sets and worn item swapping.
|
| Most of the actual Casting logic is done in the MQ2Cast Plugin.
| 
| - Allows you to use items in bags that are must-equip. Equips item, normal cast, then returns it to its previous location.
| - Allows you to use Bandolier sets for swapping in focus items, etc.
| - Lets you set how long you want to keep trying to cast the spell (defaults to 0)
| - Lets you call a custom subroutine while waiting for spell to finish casting
| - This file also includes a sub named Interrupt. You can call this to interrupt any spell you're casting instantly.
| - Note: if you don't want this to cast spells while you're invis, in your main macro have this at the top:
|      /declare noInvis bool outer TRUE
| - Note: if you don't want to stand up when Feigning do the following:
|      /declare noFeigning bool outer TRUE
|   This will make it return CAST_INVISIBLE if you're invis.
| - Added: defaultGem so that spells can be casted using only /call MQ2Cast "Howl of Tashan"
|      /declare defaultGem string outer gem1
|	Note: If spell is not memed and and NOT seting /call MQ2Cast "Howl of Tashan" gem#
|   Set /declare defaultGem string outer gem# to change default gem#
| - Added: MQ2Cast Plugin features:
|   - "-targetid|#####" - will find target and cast a MQ2Cast plugin feature.
|   - "-kill" - will keep casting spell untill target is dead
|   - "-recast|#" - recast spells # number of times
|   - "-maxtries|#" - retry casting spell # number of times
| - Added: -invis for single spell invis checks.
| - Changed: -bandolier|setname OR -setin|setname to swap items using local code and MQ2Exchange.  You don't need MQ2Bandolier
|   to be loaded, but the sets need to be in the standard MQ2Bandolier file/format.
|*******************************************************************************************************************************
|  MQ2Cast: the main subroutine that casts spells or items for you
|     Usage:
|        /call MQ2Cast ["spell name"|"item name"|"AA name"|"AA#"] [[item|slotname]|alt|gem#] [give up time][s|m] [custom subroutine name] [-targetid|###] [-maxtries|#] [-recast|#] [-setin|setname] [-bandolier|setname]
|     Examples:
|
|     To cast Howl of Tashan and mem it in slot 3 if not memmed:
|       /call MQ2Cast "Howl of Tashan" gem3
|
|     To cast Arcane Rune and keep trying for 7 seconds, in case of interrupts.
|       /call MQ2Cast "Arcane Rune" gem5 7s
|
|     To click Grim Aura earring that's in a bag:
|       /call MQ2Cast "Shrunken Goblin Skull Earring" item
|
|     To click Grim Aura earring that's in a bag:
|       /call MQ2Cast "Shrunken Goblin Skull Earring" slotname
|
|     To use AA ability Eldritch Rune:
|       /call MQ2Cast "Eldritch Rune" alt
|         or
|       /call MQ2Cast "173" alt
|
|     To call a subroutine that interrupts CH if target gets healed before it lands:
|       /call MQ2Cast "Complete Healing" gem1 0 CheckHP
|     Then in your macro have somewhere:
|       Sub CheckHP
|          /if ( ${Target.PctHPs}>=80 ) /call Interrupt
|       /return
|
|		3000 = 3 sec
|		Sub CheckHP
|		   /if ( ${Me.Casting.Name.Equal[Complete Healing]} && ${Target.PctHPs} < 40 && ${Cast.Timing} > 3000 ) ) /call Interrupt
|		/return
|
| Returns these values: ${Macro.Return} and ${Cast.Result} and ${castReturn}
|----------------------+----------------------------------------------------------------------+
| CAST_CANCELLED       | Spell was cancelled by ducking (either manually or because mob died) |
| CAST_CANNOTSEE       | You can't see your target                                            |
| CAST_IMMUNE          | Target is immune to this spell                                       |
| CAST_INTERRUPTED     | Casting was interrupted and exceeded the given time limit            |
| CAST_INVIS           | You were invis, and noInvis is set to true                           |
| CAST_NOTARGET        | You don't have a target selected for this spell                      |
| CAST_NOTMEMMED       | Spell is not memmed and you gem to mem was not specified             |
| CAST_NOTREADY        | AA ability or spell is not ready yet                                 |
| CAST_OUTOFMANA       | You don't have enough mana for this spell!                           |
| CAST_OUTOFRANGE      | Target is out of range                                               |
| CAST_RESIST          | Your spell was resisted!                                             |
| CAST_SUCCESS         | Your spell was cast successfully! (yay)                              |
| CAST_UNKNOWN         | Spell/Item/Ability was not found                                     |
| CAST_COLLAPSE        | Gate Colapsed                                                        |
| CAST_TAKEHOLD        | Spell not hold                                                       |
| CAST_FIZZLE          | Spell Fizzle                                                         |
| CAST_INVISIBLE       | NOT Casting Invis                                                    |
| CAST_RECOVER	       | Spell not Recovered yet!                                             |
| CAST_STUNNED	       | Stunned                                                              |
| CAST_STANDING	       | Not Standing                                                         |
| CAST_DISTRACTED      | To Distracted ( spell book open )                                    |
| CAST_COMPONENTS	   | Missing Component													  |
|----------------------+----------------------------------------------------------------------+
**************************************************************************************************|

Sub MQ2Cast(string spellName,string spellType,timer giveUpTimer,string mySub,string MQ2Feature1,string MQ2Feature2,string MQ2Feature3,string MQ2Feature4,string MQ2Feature5,string MQ2Feature6,string MQ2Feature7,string MQ2Feature8)
	
    /if (!${Plugin[MQ2Cast].Name.Length}) {
        /squelch /plugin MQ2Cast noauto
        /if (!${Plugin[MQ2Cast].Name.Length}) {
            /echo You must have MQ2Cast plugin to use this macro!!
            /endmacro
        }
    }
    
    /if (!${Plugin[MQ2Exchange].Name.Length}) {
        /squelch /plugin MQ2Exchange noauto
        /if (!${Plugin[MQ2Exchange].Name.Length}) {
            /echo You must have MQ2Exchange plugin to use this macro!!
            /endmacro
        }
    }

    /if (!${Defined[castReturn]}) /declare castReturn string outer NULL
    /if (!${Defined[defaultGem]}) /declare defaultGem string outer gem8
    /if (!${Defined[spellType]}) /declare spellType string local NULL
    /if (!${Defined[giveUpTimer]}) /declare giveUpTimer timer local 0
    /if (!${Defined[MQ2Feature1]}) /declare MQ2Feature1 string local
    /if (!${Defined[MQ2Feature2]}) /declare MQ2Feature2 string local
    /if (!${Defined[MQ2Feature3]}) /declare MQ2Feature3 string local
    /if (!${Defined[MQ2Feature4]}) /declare MQ2Feature4 string local
    /if (!${Defined[MQ2Feature5]}) /declare MQ2Feature5 string local
    /if (!${Defined[MQ2Feature6]}) /declare MQ2Feature6 string local
    /if (!${Defined[MQ2Feature7]}) /declare MQ2Feature7 string local
    /if (!${Defined[MQ2Feature8]}) /declare MQ2Feature8 string local
    /declare i int local
    /declare bandSet string local
    /declare oldItem string local
    /if (!${Defined[equippedArray]}) /declare equippedArray[23] string outer
    /if (!${Defined[slotNames]}) /declare slotNames[23]    string outer

    /varset slotNames[1] charm
    /varset slotNames[2] leftear
    /varset slotNames[3] head
    /varset slotNames[4] face
    /varset slotNames[5] rightear
    /varset slotNames[6] neck
    /varset slotNames[7] shoulder
    /varset slotNames[8] arms
    /varset slotNames[9] back
    /varset slotNames[10] leftwrist
    /varset slotNames[11] rightwrist
    /varset slotNames[12] ranged
    /varset slotNames[13] hands
    /varset slotNames[14] mainhand
    /varset slotNames[15] offhand
    /varset slotNames[16] leftfinger
    /varset slotNames[17] rightfinger
    /varset slotNames[18] chest
    /varset slotNames[19] legs
    /varset slotNames[20] feet
    /varset slotNames[21] waist
    /varset slotNames[22] powersource
    /varset slotNames[23] ammo
    
    /if (!${Defined[interruptFlag]}) {
        /declare interruptFlag bool outer FALSE
    } else {
        /varset interruptFlag FALSE
    }

    /if ( ${mySub.Find[-targetid|]} || ${mySub.Find[-maxtries|]} || ${mySub.Find[-kill]} || ${mySub.Find[-recast|]} || ${mySub.Find[-setin]} || ${mySub.Find[-setout]} || ${mySub.Find[-bandolier|]} || ${mySub.Find[-invis]}) {
        /varset MQ2Feature8 ${MQ2Feature7}
        /varset MQ2Feature7 ${MQ2Feature6}
        /varset MQ2Feature6 ${MQ2Feature5}
        /varset MQ2Feature5 ${MQ2Feature4}
        /varset MQ2Feature4 ${MQ2Feature3}
        /varset MQ2Feature3 ${MQ2Feature2}
        /varset MQ2Feature2 ${MQ2Feature1}
        /varset MQ2Feature1 ${mySub}
        /varset mySub
    }
        
:wait_for_stop
    /if (${Me.Casting.ID}) {
        /goto :wait_for_stop
    } else /if (${Corpse.Open}) {
        /notify LootWnd DoneButton leftmouseup
        /goto :wait_for_stop
    }

    /if (!${noFeigning} && ${Me.Feigning}) {
        /stand
    }

    /if (${Window[SpellBookWnd].Open}) {
        /keypress spellbook
    }

    /if (${spellName.Find[|]}) {
        /varset spellType ${spellName.Right[${spellName.Length:Dec[]}]}
        /varset spellName ${spellName.Left[${spellName.Find[|]:Dec}]}
    }

:cast_spell
    /if (${FindItem[=${spellName}].ID}) {
        /if (${FindItem[=${spellName}].EffectType.Equal[Click Worn]}) {
            /call SREquipItem "${spellName}"
            /varset oldItem ${Macro.Return}
        }
    } else /if (${Me.AltAbility[${spellName}].ID} && ${Me.AltAbility[${spellName}].Type}!=5 && ${spellName.NotEqual[twincast]}) { 
        /varset spellType alt
    } else /if (${Spell[${spellName}].ID}) {
        /if (${Int[${spellType}]}) { 
            /varset spellType gem${spellType}
        }
        /if (!${spellType.Find[gem]}) {
            /varset spellType ${defaultGem}
        }
        /if (!${Me.Book[${spellName}]}) {
            /call SRFindBookName "${spellName}"
            /varset spellName ${Macro.Return}
        }
    }

    /if (${Stick.Status.Equal[on]}) {
        /squelch /stick pause
        /delay 5 !${Me.Moving}
    }

    /if (${Defined[FollowFlag]}) {
        /if (${AdvPath.Following} && !${AdvPath.Paused} && ${FollowFlag} && ${FollowFlag}<2) {
            /varset FollowFlag 3
            /squelch /afollow pause
        }
    }

    /for i 1 to 8
        /if (${MQ2Feature${i}.Find[-bandolier|]} || ${MQ2Feature${i}.Find[-setin|]}) {
            /varset bandSet ${MQ2Feature${i}.Arg[2,|]}
        }
    /next i

    /if (${bandSet.Length}) {
        /call SRBandIn ${bandSet}
    }
    
    /if (${noInvis}) {
        /squelch /casting "${spellName}|${spellType}" "-invis" "${MQ2Feature1}" "${MQ2Feature2}" "${MQ2Feature3}" "${MQ2Feature4}" "${MQ2Feature5}" "${MQ2Feature6}" "${MQ2Feature7}" "${MQ2Feature8}"
    } else {
        /squelch /casting "${spellName}|${spellType}" "${MQ2Feature1}" "${MQ2Feature2}" "${MQ2Feature3}" "${MQ2Feature4}" "${MQ2Feature5}" "${MQ2Feature6}" "${MQ2Feature7}" "${MQ2Feature8}"
    }

    /if (${Cast.Status.Find[M]}) {
        /delay 3s !${Cast.Status.Find[M]}
    }

:cast_still_pending
    /if (${Cast.Status.Find[C]}) {
        /if (!${interruptFlag} && ${mySub.Length}) { 
            /call ${mySub}
        }
        /goto :cast_still_pending
    }

    /if (${Select[${Cast.Result},CAST_FIZZLE,CAST_STUNNED,CAST_INTERRUPTED]}) {
        /if (!${interruptFlag} && (${FindItem[${spellName}].ID} && ${giveUpTimer} && !${FindItem[${spellName}].Timer}) || (${Me.AltAbility[${spellName}].ID} && ${Me.AltAbilityReady[${spellName}]}) || (${Spell[${spellName}].ID} && ${giveUpTimer} > ${Spell[${spellName}].RecoveryTime})) {
            /goto :cast_spell
        }
    }

    /varset castReturn ${Cast.Result}

    /if (${Stick.Status.Equal[paused]}) {
        /squelch /stick unpause
    }

    /if (${Defined[FollowFlag]}) {
        /if (${AdvPath.Paused} && ${FollowFlag}>2) {
            /varset FollowFlag 1
            /squelch /afollow unpause
        }
    }

    /if (${bandSet.Length}) {
        /call SRBandOut
    }
    
    /if (${oldItem.Length}) {
        /call SREquipItem ${oldItem}
    }
/return ${Cast.Result}

Sub Interrupt
    /varset interruptFlag TRUE
    /interrupt
/return

Sub SRBandIn(setName)
    /if (!${Defined[bandIni]}) /declare bandIni string local ..\MQ2Bandolier_${Me.Name}.ini
    /declare equipSlots string local
    /declare itemSlot string local
    /declare itemNum string local
    /declare i int local
        
    /varset equipSlots ${Ini["${bandIni}",${setName}]}

    /if (${equipSlots.Length} && ${equipSlots.Count[|]} > 1) {
        /for i 1 to 23
            /if (${Me.Inventory[${Math.Calc[${i}-1]}].ID}) {
                /varset equippedArray[${i}] ${Me.Inventory[${Math.Calc[${i}-1]}].Name}
            } else {
                /varset equippedArray[${i}]
            }
        /next i 
        /for i 1 to ${equipSlots.Count[|]} - 1
            /varset itemSlot ${equipSlots.Arg[${i},|]}
            /if (${itemSlot.Length}) {
                /varset itemNum ${Ini["${bandIni}",${setName},${itemSlot}]}
                /if (${itemNum.Length}) {
                    /if (${Me.Inventory[${itemSlot}].ID} != ${itemNum}) {
                        /call SRSwapItemNum ${itemNum} ${itemSlot}
                    }
                }
            }
        /next i
    }
/return

Sub SRBandOut
    /declare i int local
    /for i 1 to 23
        /if (${equippedArray[${i}].Length} && !${equippedArray[${i}].Equal[${Me.Inventory[${Math.Calc[${i}-1]}].Name}]}) {
            /call SRSwapItem "${equippedArray[${i}]}" "${slotNames[${i}]}"
        }
    /next i 
/return

Sub SRClearCursor
    /declare i int local
:check_cursor
    /if (${Cursor.ID}) {
|        /if (${Cursor.Container}) {
|            /for i 1 to 10    
|                /if (!${Me.Inventory[pack${i}].Container}) {
|                    /nomodkey /itemnotify pack${i} leftmouseup
|                }
|        |    /next i
|        } 
|		/if (!${Cursor.Container} || !${Cursor.Items}) {
            /timed 5 /autoinventory
|        }
    }
    /goto :check_cursor
/return

Sub SREquipItem(WhatWhere)
    /declare destName string local
    /declare itemName string local ${WhatWhere.Arg[1,|]}
    /declare slotName string local ${WhatWhere.Arg[2,|]}
    
    /if (${slotName.Equal[NULL]}) {
        /varset slotName ${FindItem[=${itemName}].WornSlot[1].Name}
    }
    /if (${FindItem[=${itemName}].InvSlot}<23 || !${FindItem[=${itemName}].WornSlot[${slotName}]}) {
        /return
    }
    /if (${Me.Inventory[${slotName}].ID}) {
        /varset destName "${Me.Inventory[${slotName}].Name}|${slotName}"
    }
    /call SRSwapItem "${itemName}" "${slotName}"
/return ${destName}

Sub SRSwapItem(itemName,slotName)
    /declare i int local
    /if (${Cursor.ID}) /call SRClearCursor
    /exchange "${itemName}" ${slotName}
    /delay 5s ${Me.Inventory[${slotName}].Name.Equal[${itemName}]}
    /delay 5s !${Cursor.ID}
    /if (${Cursor.ID}) /call SRClearCursor
/return

Sub SRSwapItemNum(itemNum,slotNum)
    /declare i int local
    /if (${Cursor.ID}) /call SRClearCursor
    /exchange ${itemNum} ${slotNum}
    /delay 5s ${Me.Inventory[${slotNum}].ID}==${itemNum}
    /delay 5s !${Cursor.ID}
    /if (${Cursor.ID}) /call SRClearCursor
/return

Sub SRFindBookName(sName)
    /declare returnName string local
    /declare tName string local
    /declare a[2] string local
    /declare b[2] string local
    /declare i int local
    /declare j int local
    
    /if (${Me.Book[${sName}]}) {
        /varset returnName ${sName}
        /goto :found_book_name
    }

    /varset a[1] Rk.
    /varset a[2] Rk

    /varset b[1] III
    /varset b[2] II
    
    /for i 1 to 2
        /for j 1 to 2
            /varset tName ${sName} ${a[${i}]} ${b[${j}]}
            /if (${Me.Book[${tName}]}) {
                /varset returnName ${tName}
                /goto :found_book_name
            }
            /varset tName ${sName} ${a[${i}]}${b[${j}]}
            /if (${Me.Book[${tName}]}) {
                /varset returnName ${tName}
                /goto :found_book_name
            }
            /varset tName ${sName} ${a[${i}]} ${b[${j}]} *
            /varset tName ${tName.Left[-1]}
            /if (${Me.Book[${tName}]}) {
                /varset returnName ${tName}
                /goto :found_book_name
            }
            /varset tName ${sName} ${a[${i}]}${b[${j}]} *
            /varset tName ${tName.Left[-1]}
            /if (${Me.Book[${tName}]}) {
                /varset returnName ${tName}
                /goto :found_book_name
            }
        /next j
    /next i
        
:found_book_name
/return ${returnName}
