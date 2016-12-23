
Sub #ItemIDFromLink(ItemLink, IDReturn)
  /if (${Defined[IDReturn]}) /varset ${IDReturn} ${ItemLink.Mid[2,6]}
/return ${ItemLink.Mid[2,6]}

Sub #ItemLinkToClickable(ItemLink, LinkReturn)
  /if (${Defined[LinkReturn]}) /varset ${LinkReturn} ${ItemLink.Mid[3,49]}
/return ${ItemLink.Mid[3,49]}


#Event ClickLink "[MQ2] clicklink #1#"
Sub Event_ClickLink(line, itemName)
  /call #CreateVars string outer ItemLinkReturn StatReturn
  /call #GetItemLink "${itemName}" ItemLinkReturn
  /call #ItemLinkToClickable "${ItemLinkReturn}" ItemLinkReturn
  /cleanup
  /delay 10 !${Window[ItemDisplay].Open}
  /notify ChatWindow CW_ChatOutput link ${ItemLinkReturn}
  /delay 10 ${Window[ItemDisplay].Open}
  /call #DisplayStatTotalForItem DisplayItem
  /cleanup
/return

Sub #DisplayStatTotalForItem(itemRef)
  /call #CalculateStatTotalForItem ${itemRef} StatReturn
  /echo [${DisplayItem.Name}] Stat Total: ${StatReturn}
/return

Sub #CalculateStatTotalForItem(itemRef, DestVar, UseTheseWeights)
  /varset ${DestVar} 0
  /declare WeightsToUse string local
  /if (${Defined[UseTheseWeights]}) {
    /varset WeightsToUse ${UseTheseWeights}
  } else {
    /call #CheckStatWeights
    /varset WeightsToUse ${LoadedStatWeights}
  }
  /if (!${WeightsToUse.Count[|]}) /return 0
  
  /declare i int local
  /declare CurrentStat string local 
  /declare CurrentWeight int local
  /declare total int local 0
  /for i 1 to ${WeightsToUse.Count[|]}
    /varset CurrentStat ${WeightsToUse.Arg[${i},|].Arg[1,:]}
    /varset CurrentWeight ${WeightsToUse.Arg[${i},|].Arg[2,:]}
    /if (${Select[${CurrentStat},Ratio,ShieldAC,AC]}) {
      /if (${CurrentStat.Equal[Ratio]} && ${${itemRef}.ItemDelay}>0) {
        /varcalc total ${total}+(${${itemRef}.Damage}/${${itemRef}.ItemDelay})*${CurrentWeight}
      } else /if (${CurrentStat.Equal[ShieldAC]} && ${${itemRef}.Type.Equal[Shield]}) {
        /varcalc total ${total}+${${itemRef}.AC}*${CurrentWeight}
      } else /if (${CurrentStat.Equal[AC]} && !${${itemRef}.Type.Equal[Shield]}) {
        /varcalc total ${total}+${${itemRef}.AC}*${CurrentWeight}
      }
    } else /if (${Bool[${${itemRef}.${CurrentStat}}]}) {
      /varcalc total ${total}+${${itemRef}.${CurrentStat}}*${CurrentWeight}
    }
  /next i
  /varset ${DestVar} ${total}
/return ${total}

#Event TestWeights "[MQ2] statweights #1#"
Sub Event_TestWeights(line, name)
  /call #CreateVars string outer LoadedStatWeights
  /call #LoadStatWeights "${name}" LoadedStatWeights
  /echo ${LoadedStatWeights}
/return

Sub #CheckStatWeights
  /if (!${Defined[LoadedStatWeights]}) {
    /call #CreateVars string outer LoadedStatWeights
    /call #LoadDefaultStatWeights
  }
/return

Sub #LoadDefaultStatWeights
  /call #LoadStatWeights "${Me.Class.ShortName}" LoadedStatWeights
/return

Sub #LoadStatWeights(Name, DestVar)
  /declare StatTypes string local HP|AC|ShieldAC|Mana|HealAmount|Ratio|SpellDamage|Shielding|Accuracy|Attack|Avoidance|Clairvoyance|CombatEffects|DamageShieldMitigation|DamShield|DoTShielding|HPRegen|ManaRegen|SpellShield|StrikeThrough|StunResist|Haste|
  /declare StatTemp string local 
  /declare CurrentStat string local
  /declare i int local
  /varset ${DestVar}
  /if (${Ini[stat_weights.ini,${Name}].Length}) {
    /for i 1 to ${StatTypes.Count[|]}
      /varset CurrentStat ${StatTypes.Arg[${i},|]}
      /if (${Ini[stat_weights.ini,${Name},${CurrentStat}].Length}) {
        /varset StatTemp ${StatTemp}${CurrentStat}:${Ini[stat_weights.ini,${Name},${CurrentStat}]}|
      }
    /next i
  }
  /varset ${DestVar} ${StatTemp}
/return