
Sub #LogInit
  /declare LogMacroName string outer ${If[${Macro.Name.Equal[NULL]},nomacro,${Macro.Name}]}
  /declare LogSectionNumber int local ${Ini[log.ini,Sections,${LogMacroName},0]}
  /declare LogSection string outer ${LogMacroName}_${Me.Name}_${LogSectionNumber}
  /declare LogLineCounter int outer 0
  /ini log.ini Sections "${LogMacroName}" "${Math.Calc[${LogSectionNumber}+1].Int}"
/return

Sub #Log(Msg)
  /ini "log.ini" "${LogSection}" "Log${LogLineCounter}" "${Msg}"
  /varcalc LogLineCounter ${LogLineCounter}+1
/return

Sub #BuildItemLink(ItemName)
  /call #CreateVars string outer ItemLinkReturn ItemIDPart ExtraPart NamePart AugPart
  /call #GetItemLink "${ItemName}" ItemLinkReturn
  /varset NamePart ${ItemName}
  /varset ItemIDPart ${ItemLinkReturn.Mid[2,6]}
  /varset ExtraPart ${ItemLinkReturn.Mid[32,19]}
  
  /declare argCount int local 1
  :NextArg
    /if (${Defined[Param${argCount}]}) {
      /varcalc argCount ${argCount}+1
      /goto :NextArg
    }
    
  /declare i int local
  /declare tempInt int local
  /for i 1 to ${Math.Calc[${argCount}-1]}
    /varset tempInt ${Param${Math.Calc[${i}].Int}}
    /varset AugPart ${AugPart}${PadL[${tempInt.Hex},5,0]}
  /next i
  /if (${argCount}<6) {
    /for i 1 to ${Math.Calc[5-${argCount}+1]}
      /varset AugPart ${AugPart}${PadL[,5,0]}
    /next i
  }
  /varset ItemLinkReturn ${ItemIDPart}${AugPart}${ExtraPart}${NamePart}
  /varset ${Param${Math.Calc[${argCount}-1].Int}} ${ItemLinkReturn}
/return ${ItemLinkReturn}

#Event VysLibMakeTimer "<#1#> MakeTimer #2#"
#Event VysLibMakeTimer "[#1#] MakeTimer #2#"
Sub Event_VysLibMakeTimer(line, chatSender, timerName)
  /call #verifyEvent "${chatSender}" "${line}"
  /if (!${Macro.Return}) /return
  /call #CreateVars timer global ${timerName}
/return

Sub #RequirePlugin(PluginName)
  /if (!${Plugin[${PluginName}].Name.Length}) /plugin ${PluginName} noauto
  /if (!${Plugin[${PluginName}].Name.Length}) {
    /return 0
  }
/return 1

Sub #RequirePlugins
  /declare i int local 0
  :NextArg
  /if (!${Plugin[${Param${i}}].Name.Length}) /plugin ${Param${i}} noauto
  /if (!${Plugin[${Param${i}}].Name.Length}) {
    /return 0
  }
  /varcalc i ${i}+1
  /if (${Defined[Param${i}]}) /goto :NextArg
/return 1

Sub #GetItemAugIDs(FromInvSlot, destVar)
  /declare i int local
  /varset ${destVar}
  /if (${InvSlot[${FromInvSlot}].Item.Augs}) {
    /for i 1 to ${InvSlot[${FromInvSlot}].Item.Augs}
      /varset ${destVar} ${${destVar}} ${InvSlot[${FromInvSlot}].Item.Item[${i}].ID}
    /next i
  } 
/return

Sub #CreateArrays(ofType, scope)
  /declare currArg int local 1
  /declare argValue string local
  /declare argName string local
  /declare argSize int local
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /varset argValue ${Param${currArg}}
    /varset argName ${argValue.Left[${Math.Calc[${argValue.Find[[]}-1]}]}
    /varset argSize ${argValue.Mid[${Math.Calc[${argValue.Find[[]}+1]},${Math.Calc[${argValue.Find[]]}-1]}]}
    /if (${Defined[${argName}]}) /deletevar ${argName}
    /declare ${argName}[${argSize}] ${ofType} ${scope}
    /goto :NextArg
  }
/return

Sub #CreateVars(ofType, scope)
  /declare currArg int local 1
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /if (${Defined[${Param${currArg}}]}) /deletevar ${Param${currArg}}
    /declare ${Param${currArg}} ${ofType} ${scope}
    /goto :NextArg
  } 
/return

Sub #CreateVarsIfNeeded(ofType, scope)
  /declare currArg int local 1
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /if (!${Defined[${Param${currArg}}]}) /declare ${Param${currArg}} ${ofType} ${scope}
    /goto :NextArg
  }
/return

| Creating variables within a VarGroup makes it easier to delete them all at once when finished with them
| so all the variables don't clutter the outer/global scope and slow down macro every time a variable is referenced
Sub #CreateVarGroup(GroupName, ofType, scope)
  /declare currArg int local 2
  /if (${Defined[${GroupName}_Group]}) /deletevar ${GroupName}_Group
  /declare ${GroupName}_Group string ${ofType}
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /if (${Defined[${Param${currArg}}]}) /deletevar ${Param${currArg}}
    /declare ${Param${currArg}} ${ofType} ${scope}
    /varset ${GroupName}_Group ${${GroupName}_Group} ${Param${currArg}}
    /goto :NextArg
  }
/return

Sub #DeleteVarGroup(GroupName)
  /if (${Defined[${GroupName}_Group]}) {
    /declare i int local
    /for i 1 to ${Math.Calc[${${GroupName}_Group.Count[ ]}+1]}
      /if (${Defined[${${GroupName}_Group.Arg[${i}, ]}]}) /deletevar ${${GroupName}_Group.Arg[${i}, ]}
    /next i
    /deletevar ${GroupName}_Group
  }
/return

Sub #DeleteVars
  /declare currArg int local -1
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /if (${Defined[${Param${currArg}}]}) /deletevar ${Param${currArg}}
    /goto :NextArg
  } 
/return

Sub #GetItemLink(FromName, destVar)
  /declare tempLink string local
  /varset tempLink ${LinkDB[=${FromName}]}
  /if (!${tempLink.Find[${FromName}]}) /varset tempLink ${LinkDB[${FromName}]}
  /if (${Defined[destVar]}) /varset ${destVar} ${tempLink}
/return ${tempLink}

Sub #UnlinkAllItems(input, destVar)
  /call #CreateVars string outer ItemNameReturn
  /varset ${destVar}
  /declare i int local 0
  /for i 1 to ${Math.Calc[${input.Count[|]}+1]}
    /call #StripItemLink "${input.Arg[${i},|]}" ItemNameReturn
    /varset ${destVar} ${${destVar}}${If[${i}>1,|,]}${ItemNameReturn}
  /next i
/return

Sub #SplitInclusionArgs(input, baseDestVar, inclusionArgsDestVar, extra)

  /declare endOfBaseInput int local ${input.Length}
  /declare i int local 0
  /if (${input.Find[/only|]} || ${input.Find[/not|]} || ${input.Find[/include|]} || ${input.Find[/exclude|]} || ${input.Find[/true|]} || ${input.Find[/false|]} || ${input.Find[/if|]} || ${input.Find[/unless|]} || ${input.Find[/pipeto|]} || ${extra.Length}) {
    /if (${input.Find[/only|]} && ${input.Find[/only|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/only|]}
    /if (${input.Find[/not|]} && ${input.Find[/not|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/not|]}
    /if (${input.Find[/include|]} && ${input.Find[/include|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/include|]}
    /if (${input.Find[/exclude|]} && ${input.Find[/exclude|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/exclude|]}
    /if (${input.Find[/true|]} && ${input.Find[/true|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/true|]}
    /if (${input.Find[/false|]} && ${input.Find[/false|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/false|]}
    /if (${input.Find[/if|]} && ${input.Find[/if|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/if|]}
    /if (${input.Find[/unless|]} && ${input.Find[/unless|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/unless|]}
    /if (${input.Find[/pipeto|]} && ${input.Find[/pipeto|]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[/pipeto|]}
    /if (${Defined[extra]} && ${extra.Length}) {
      /for i 1 to ${Math.Calc[${extra.Count[ ]}+1]}
        /if (${input.Find[${extra.Arg[${i}, ]}]} && ${input.Find[${extra.Arg[${i}, ]}]} < ${endOfBaseInput}) /varset endOfBaseInput ${input.Find[${extra.Arg[${i}, ]}]}
      /next i
    }
  }

  /if (${endOfBaseInput}<${input.Length}) {
    /if (${endOfBaseInput}==1) {
      /varset ${baseDestVar}
      /varset ${inclusionArgsDestVar} ${input}
    } else {
      /varset ${baseDestVar} ${input.Left[${Math.Calc[${endOfBaseInput}-2]}]}
      /varset ${inclusionArgsDestVar} ${input.Right[-${Math.Calc[${endOfBaseInput}-1]}]}
    }
    |/call #PrintVars ${baseDestVar} ${inclusionArgsDestVar}
  } else {
    /varset ${baseDestVar} ${input}
    /varset ${inclusionArgsDestVar}
  }
  
/return

Sub #SplitTruthArgs(input, truthArgSelection, destVar, additionalFlags)

  /declare flags string local /not|/only|/include|/exclude|/pipeto${If[${Defined[additionalFlags]},|${additionalFlags},]}
  /if (${truthArgSelection.Find[true]} || ${truthArgSelection.Find[if]}) /varset flags ${flags}|/false|/unless
  /if (${truthArgSelection.Find[false]} || ${truthArgSelection.Find[unless]}) /varset flags ${flags}|/true|/if
  /declare i int local
  |/echo input: [${input}] @ ${input.Find[${truthArgSelection}]}
  /declare startOfTruthArgs int local ${input.Find[${truthArgSelection}]}
  /declare remainingInput string local ${input.Right[-${startOfTruthArgs}]}
  /declare endOfTruthArgs int local ${remainingInput.Length}
  |/echo remainingInput: [${remainingInput}]
  /declare thisFlag string local
  /for i 1 to ${Math.Calc[${flags.Count[|]}+1]}
    /varset thisFlag ${flags.Arg[${i},|]}
    |/echo thisFlag: ${thisFlag}
    /if (${remainingInput.Find[${thisFlag}|]} && ${remainingInput.Find[${thisFlag}|]} < ${endOfTruthArgs}) /varset endOfTruthArgs ${Math.Calc[${remainingInput.Find[${thisFlag}|]}-2]}
  /next i
  |/echo endOfTruthArgs: ${endOfTruthArgs} returning: ${remainingInput.Left[${endOfTruthArgs}].Arg[2,|]}
  /varset ${destVar} ${remainingInput.Left[${endOfTruthArgs}].Arg[2,|]}
  
/return

Sub #check_selectedBots(selectedBots, ChatSender)
	/declare botSelected bool local FALSE
	/if (${selectedBots.Find[ ]}) /varset selectedBots ${selectedBots.Replace[ ,,]}
  |/echo selectedBots: ${selectedBots}
  /if (${Select[me,${selectedBots}]} && ${ChatSender.Equal[${Me.Name}]}) /varset botSelected TRUE
  /if (${Select[melee,${selectedBots}]} && ${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) /varset botSelected TRUE
  /if (${Select[casters,${selectedBots}]} && ${Select[${Me.Class.ShortName},ENC,MAG,NEC,WIZ]}) /varset botSelected TRUE
  /if (${Select[hybrids,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD,RNG,BST]}) /varset botSelected TRUE
	/if ((${Select[cloth,${selectedBots}]} || ${Select[silk,${selectedBots}]}) && ${Select[${Me.Class.ShortName},NEC,WIZ,MAG,ENC]}) /varset botSelected TRUE
  /if (${Select[leather,${selectedBots}]} && ${Select[${Me.Class.ShortName},DRU,MNK,BST]}) /varset botSelected TRUE
  /if (${Select[chain,${selectedBots}]} && ${Select[${Me.Class.ShortName},BER,ROG,RNG,SHM]}) /varset botSelected TRUE
  /if (${Select[plate,${selectedBots}]} && ${Select[${Me.Class.ShortName},WAR,CLR,PAL,SHD,BRD]}) /varset botSelected TRUE
  /if (${Select[knights,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD]}) /varset botSelected TRUE
  /if (${Select[tanks,${selectedBots}]} && ${Select[${Me.Class.ShortName},PAL,SHD,WAR]}) /varset botSelected TRUE
  /if ((${Select[priests,${selectedBots}]} || ${Select[healers,${selectedBots}]}) && ${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) /varset botSelected TRUE
  /if (${Select[all,${selectedBots}]} || ${Select[${Me.Class},${selectedBots}]} || ${Select[${Me.Class.ShortName},${selectedBots}]} || ${Select[${Me.CleanName},${selectedBots}]}) /varset botSelected TRUE
  /if (${Select[group,${selectedBots}]} && ${NetBots[${ChatSender}].InGroup}) /varset botSelected TRUE
  /if (${Select[raid,${selectedBots}]} && ${Raid.Members} > 0 && ${Bool[${Raid.Member[${ChatSender}]}]}) /varset botSelected TRUE
  /if (${Select[zone,${selectedBots}]} && ${SpawnCount[pc =${ChatSender}]}) /varset botSelected TRUE
  /if (${Select[leaders,${selectedBots}]} && ${Group}) /if (${Group.Leader.Name.Equal[${Me.Name}]}) /varset botSelected TRUE
/return ${botSelected}

Sub #verifyEventInZoneAndRange(ChatSender, eventLine)
  /if (!${SpawnCount[pc ${ChatSender} radius ${MaxResponseDist}]}) /return 0
  /call #verifyEvent "${ChatSender}" "${eventLine}"
/return ${Macro.Return}

Sub #verifyEvent(ChatSender, eventLine)

	/declare userValidated bool local FALSE
	/declare includeBot bool local TRUE
	/declare inZone bool local FALSE
	/declare inRange bool local FALSE
	/declare inAssistRange bool local FALSE

	| -Reset ChatSender.
	/if (${ChatSender.Left[1].Compare[ ]}<0) /varset ChatSender ${ChatSender.Right[-2].Left[-1]}
	/if (${ChatSender.Equal[You]}) /varset ChatSender ${Me.CleanName}
	/if (${ChatSender.Equal[MQ2]}) /varset ChatSender ${Me.CleanName}
	/if (${ChatSender.Right[5].Equal[(msg)]}) /varset ChatSender ${ChatSender.Left[-5]}
	| -Check for /only|
	/if (${eventLine.Find[/only|]}) {
		/varset includeBot FALSE
		/call #argueString only| "${eventLine}"
		/call #check_selectedBots "${Macro.Return}" "${ChatSender}"
		/if (${Macro.Return}) /varset includeBot TRUE
	}
		
	| -Check for /exclude|
	/if (${eventLine.Find[/exclude|]}) {
		/call #argueString exclude| "${eventLine}"
		/call #check_selectedBots "${Macro.Return}" "${ChatSender}"
		/if (${Macro.Return}) /varset includeBot FALSE
	}
	
	| -Check for /not|
	/if (${eventLine.Find[/not|]}) {
		/call #argueString not| "${eventLine}"
		/call #check_selectedBots "${Macro.Return}" "${ChatSender}"
		/if (${Macro.Return}) /varset includeBot FALSE
	}
	
	| -Check for /include|
	/if (${eventLine.Find[/include|]}) {
		/call #argueString include| "${eventLine}"
		/call #check_selectedBots "${Macro.Return}" "${ChatSender}"
		/if (${Macro.Return}) /varset includeBot TRUE
	}
	
  /if (${eventLine.Find[/PipeTo|]} && ${Defined[PipeToChannel]}) {
    /call #argueString pipeto| "${eventLine}"
    /varset PipeToChannel ${Macro.Return}
  }

/return ${includeBot}

Sub #argueString(WhatToFind, givenData)

	/declare i int local 1
	/declare foundData string local NULL
	
	:nextArg
	/if (${givenData.Find[/]}) {
		
		/if (${givenData.Arg[${i},/].Left[${WhatToFind.Length}].Equal[${WhatToFind}]}) {
		
			/varset foundData ${givenData.Arg[${i},/].Arg[2,|]}

		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i},/].Length}) /goto :nextArg
		}
	} else {
		/if (${givenData.Arg[${i}, ].Left[${WhatToFind.Length}].Equal[${WhatToFind}]}) {
		
			/varset foundData ${givenData.Arg[${i}, ].Arg[2,|]}

		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i}, ].Length}) /goto :nextArg
		}
	}

/return ${foundData}

Sub #GetArgumentOrElse(WhatToFind, GivenData, OrElse, DestVar)
  /if (${GivenData.Find[${WhatToFind}]}) {
    /call #argueString "${WhatToFind}" "${GivenData}"
    /if (${Macro.Return.Equal[NULL]}) {
      /if (${Defined[${DestVar}]}) /varset ${DestVar} ${OrElse}
      /return ${OrElse}
    }
  }
  /if (${Defined[${DestVar}]}) /varset ${DestVar} ${Macro.Return}
/return ${Macro.Return}

| ArgumentList is actual data passed in
| BaseParameters is a list of regular expected parameters
Sub #GetExtraParameters(ArgumentList, BaseParameters, DestVar)
  /declare i int local 1
  /declare currentArg string local
  /varset ${DestVar}
  /varset BaseParameters |${BaseParameters.Replace[ ,|]}|
  /if (!${ArgumentList.Find[/]}) /return
  /declare firstSlash string local ${ArgumentList.Find[/]}
  :NextArg
  /if (${ArgumentList.Arg[${i},/].Find[|]} && !${ArgumentList.Arg[${i},/].Arg[1,|].Find[ ]}) {
    /varset currentArg ${ArgumentList.Arg[${i},/].Arg[1,|]}
    /if (!${BaseParameters.Find[|${currentArg}|]}) {
      /varset ${DestVar} ${${DestVar}} ${currentArg}
    }
  } else /if (${ArgumentList.Arg[${i},/].Length} && ${ArgumentList.Find[${ArgumentList.Arg[${i},/]}]} >= ${firstSlash}) {
    /varset currentArg ${ArgumentList.Arg[${i},/]}
    /if (!${BaseParameters.Find[|${currentArg}|]}) {
      /varset ${DestVar} ${${DestVar}} ${Trim[${currentArg}]}
    }
  }
  /varcalc i ${i} + 1
  /if (${ArgumentList.Arg[${i},/].Length}) /goto :NextArg
/return

Sub #IsItemLink(input, destVar)
  /if (${input.Find[]} && ${input.Length} > 40) {
    /varset ${destVar} 1
  } else {
    /varset ${destVar} 0
  }
/return

Sub #StripItemLink(input, destVar)
  /if (${input.Find[]}) {
    /varset ${destVar} ${input.Right[-51].Left[-1]}
  } else {
    /varset ${destVar} ${input}
  }
/return

Sub #StripNameLink(input, destVar)
  /if (${input.Find[]}) {
    /varset ${destVar} ${input.Right[-2].Left[-1]}
  } else {
    /varset ${destVar} ${input}
  }
/return

Sub #RemoveAll(toRemove, input, destVar)
  /declare i int local
  /for i 1 to ${toRemove.Length}
    /if (!${toRemove.Mid[${i},1].Equal[,]} && ${input.Find[${toRemove.Mid[${i},1]}]}) /varset input ${input.Replace[${toRemove.Mid[${i},1]},]}
  /next i
  /if (${toRemove.Find[,]}) {
    /call #RemoveCommas "${input}"
    /varset input ${Macro.Return}
  }
  /varset ${destVar} ${input}
/return 

Sub #RemoveCommas(input)
  /declare tempReturn string local ${input}
  /declare commaLoc int local 0
  /declare leftString string local
  /declare rightString string local
  :Loop
  /varset commaLoc ${tempReturn.Find[,]}
  /varset leftString
  /varset rightString
  /if (${commaLoc}) {
    /if (${commaLoc}>1) /varset leftString ${tempReturn.Left[${Math.Calc[${commaLoc}-1]}]}
    /varset rightString |${tempReturn.Right[-${commaLoc}]}
    /varset tempReturn ${leftString}${rightString.Right[-1]}
    /goto :Loop
  }
/return ${tempReturn}


| SortBy can be excluded if the whole value of the array entry is an integer.
| Otherwise, use it to pull the numeric value out of each entry like this:
|   /declare TestArray[3] string outer
|   /varset TestArray[1] Whatever|3|Other-non-sorty-stuff
|   /varset TestArray[2] Whatever|1|Yeah
|   /varset TestArray[3] Whatever|178|Thanks, QuickSort
|   /call #QuickSort TestArray 1 3 .Arg[2,|]

Sub #QuickSort(ArrayName, LowIndex, HighIndex, SortBy)
  /if (!${Defined[SortBy]}) /declare SortBy string local
  /if (${LowIndex} < ${HighIndex}) {
    /call #QS:Partition ${ArrayName} ${LowIndex} ${HighIndex} "${SortBy}"
    /declare pivot int local ${Macro.Return}
    /call #QuickSort ${ArrayName} ${LowIndex} ${Math.Calc[${pivot}-1]} "${SortBy}"
    /call #QuickSort ${ArrayName} ${Math.Calc[${pivot}+1]} ${HighIndex} "${SortBy}"
  }
/return

Sub #QS:Partition(ArrayName, LowIndex, HighIndex, SortBy)
  /declare pivot int local ${${ArrayName}[${LowIndex}]${SortBy}}
  /declare left int local ${LowIndex}
  /declare i int local
  /for i ${Math.Calc[${LowIndex}+1]} to ${HighIndex}
    /if (${${ArrayName}[${i}]${SortBy}} < ${pivot}) {
      /varcalc left ${left}+1
      /call #QS:Swap ${ArrayName} ${i} ${left}
    }
  /next i
  /call #QS:Swap ${ArrayName} ${LowIndex} ${left}
/return ${left}

Sub #QS:Swap(ArrayName, IndexA, IndexB)
  /declare Temp string local ${${ArrayName}[${IndexA}]}
  /varset ${ArrayName}[${IndexA}] ${${ArrayName}[${IndexB}]}
  /varset ${ArrayName}[${IndexB}] ${Temp}
/return

#Event CombineArrayTest "#*#CombineArrayTest"
Sub Event_CombineArrayTest
  /declare ArrayOne[3] string outer UNDEFINED
  /declare ArrayTwo[5] string outer
  /declare ArrayThree[1] string outer ThirdArray
  
  /declare i int local 0
  /for i 1 to 5
    /varset ArrayTwo[${i}] Entry ${i}
  /next i
  /call #PrintArray ArrayOne
  /call #PrintArray ArrayTwo
  /call #PrintArray ArrayThree
  /call #CombineArrays CombinedArray ArrayOne ArrayTwo ArrayThree
  /echo returned: CombinedArray[${CombinedArray.Size}]
  /call #PrintArray CombinedArray
  
  /call #ArrayToString TempStr CombinedArray " "
  /echo CombinedArray ToString with Spaces: ${TempStr}
  
  /call #ArrayToString TempStr2 CombinedArray
  /echo CombinedArray ToString: ${TempStr2}
/return

Sub #CombineArrays(DestVar)
  /declare arrayEntryCount int local 0
  /declare currentParam int local 0
  
  :NextParam
  /varcalc currentParam ${currentParam}+1
  /if (!${Defined[Param${currentParam}]}) /goto :FinishedParams
  /varcalc arrayEntryCount ${arrayEntryCount}+${${Param${currentParam}}.Size}
  /goto :NextParam
  
  :FinishedParams
  /if (${Defined[${DestVar}]}) /deletevar ${DestVar}
  /declare ${DestVar}[${arrayEntryCount}] string outer
  /declare i int local 0
  /declare currentInputArray int local 0
  /declare currentArrayPos int local 1
  /varset currentParam 0
  
  :NextInputArray
  /varcalc currentInputArray ${currentInputArray}+1
  /if (!${Defined[Param${currentInputArray}]}) /goto :End
  /if (!${${Param${currentInputArray}}.Size}) /goto :NextInputArray
  /for i 1 to ${${Param${currentInputArray}}.Size}
    /varset ${DestVar}[${currentArrayPos}] ${${Param${currentInputArray}}[${i}]}
    /varcalc currentArrayPos ${currentArrayPos}+1
  /next i
  
  /goto :NextInputArray
  :End
  
/return

Sub #AppendToArray(ArrayName)

/return


Sub #ArrayToString(DestVar, ArrayName)
  /declare separator string local |
  /declare spaceAsSeparator int local 0
  /if (${Defined[Param2]}) {
    /if (${Param2.Equal[ ]}) {
      /varset spaceAsSeparator 1
    } else {
      /varset separator ${Param2}
    }
  }
  /declare i int local 0
  /declare ${DestVar} string outer
  /for i 1 to ${${ArrayName}.Size}
    /if (${i}==1) {
      /varset ${DestVar} ${${ArrayName}[${i}]}
    } else {
      /varset ${DestVar} ${${DestVar}}${If[${spaceAsSeparator}, ,${separator}]}${${ArrayName}[${i}]}
    }
  /next i
  
/return

Sub #PrintArray(ArrayName)
  /declare i int local 0
  /for i 1 to ${${ArrayName}.Size}
    /echo ${ArrayName}[${i}]: ${${ArrayName}[${i}]}
  /next i
/return

Sub #PrintVars
  /declare currArg int local -1
  :NextArg
  /varcalc currArg ${currArg}+1
  /if (${Defined[Param${currArg}]}) {
    /echo ${Param${currArg}} = '${${Param${currArg}}}'
    /goto :NextArg
  }
/return

Sub #SetArray(ArrayName)
  /declare i int local 
  /if (!${${ArrayName}.Size}) /return
  /for i 1 to ${${ArrayName}.Size}
    /varset ${ArrayName}[${i}] ${Param${i}}
  /next i
/return

Sub #RemoveBuff
  /call #RemoveBuffs "${Param0}"
/return

| Accepts a list of buffs, by name or ID
Sub #RemoveBuffs
  /declare currArg int local -1
  /declare buffList string local
  /declare currentBuff string local
  :NextArg
  /varcalc currArg ${currArg}+1
  | only want to process buffs if you have them up, otherwise it might remove blocked buffs that you want to stay blocked
  /if (${Defined[Param${currArg}]}) {
    /if (${Me.Buff[${Spell[${Param${currArg}}].Name}].ID}) {
      /varset buffList ${buffList} ${Me.Buff[${Spell[${Param${currArg}}].Name}].Spell.ID}
    } else /if (${Me.Song[${Spell[${Param${currArg}}].Name}].ID}) {
      /varset buffList ${buffList} ${Me.Song[${Spell[${Param${currArg}}].Name}].Spell.ID}
    }
    /goto :NextArg
  }
  /if (${buffList.Length}) {
    /blockspell add me ${buffList}
    /blockspell remove me ${buffList}
  }
/return

| this isn't done
Sub #DropInvis
  /if (${Me.Class.ShortName.Equal[BRD]}) {
    /twist off
    /if (${Macro.Name.Equal[e3.mac]}) {
      /if (${Defined[DefaultMelody]}) {
        /call Event_StartMelody "NULL" "${Me.Name}" "${DefaultMelody}"
      } else {
        /call Event_StopMelody "NULL" "${Me.Name}" ""
      }
    }
  }
  /if (${Me.Class.ShortName.Equal[ROG]}) {
    /if (${Bool[${Me.Sneaking}]}) {
      /doability Sneak
    }
    /if (${Bool[${Me.Invis}]}) {
      /doability Hide
    }
  }

  :DropInvis
  /if ( ${Me.GetSelfBuffBySPA[12].ID} ) {
    /call #RemoveBuffs "${Me.GetSelfBuffBySPA[12].ID}"

    /if ( ${Me.GetSelfBuffBySPA[12].ID} ) /goto :DropInvis
  }
/return

Sub #IsInteger(Input, DestVar)
  /if (${Input.Length} == 0) /goto :Fail
  /declare IntegerCheck string local 0123456789
  /declare i int local
  /for i 1 to ${Input.Length}
    /if (!${IntegerCheck.Find[${Input.Mid[${i},1]}]}) /goto :Fail
  /next i
  /if (${Defined[${DestVar}]}) /varset ${DestVar} 1
  /return 1
  :Fail
  /if (${Defined[${DestVar}]}) /varset ${DestVar} 0
/return 0

Sub #IniToArray_Deprecated(IniKey, ArrayName)
  /declare i int local 1
  |/declare IniKey ${File},${Section},${Key}
  /declare ArrayScope string local ${If[${Defined[Param2]},${Param2},outer]}
  
  /if (${Defined[${ArrayName}]}) /deletevar ${ArrayName}
  /if (${Ini[${IniKey}1].Length}) {
    /declare count int local 0
    :NextKey
      /if (${Ini[${IniKey}${i}].Length}) {
        /varset count ${i}
        /varcalc i ${i}+1
        /goto :NextKey
      }
    /declare ${ArrayName}[${count}] string ${ArrayScope}
    /for i 1 to ${count}
      /varset ${ArrayName}[${i}] ${Ini[${IniKey}${i}]}
    /next i
  } else {
    /declare ${ArrayName}[0] string ${ArrayScope}
  }
/return

Sub #IniToArray(IniKey, ArrayName)
  /declare i int local 1
  /declare j int local 1
  /declare array_size int local 0
  /declare ArrayScope string local ${If[${Defined[Param2]},${Param2},outer]}
  /call #RequirePlugin MQ2IniExt
  /if (${Macro.Return}) {
    /if (${IniKey.Right[1].Equal[#]} && ${IniExt[${IniKey}1].Length}) {
      /call #IniToArray_Deprecated "${IniKey}" "${ArrayName}" "${ArrayScope}"
      /return
    }
    /if (${IniKey.Right[1].Equal[#]}) /varset IniKey ${IniKey.Left[-1]}
    |/echo IniKey = ${IniKey} count = ${IniExt[${IniKey}].Count}
    /if (${Defined[${ArrayName}]}) /deletevar ${ArrayName}
    /if (${IniExt[${IniKey}].Index[1].Length}) {
      /for i 1 to ${IniExt[${IniKey}].Count}
      /if (${IniExt[${IniKey}].Index[${i}].Length}) {
        /varcalc array_size ${array_size}+1
      }
      /next i
      |/declare ${ArrayName}[${IniExt[${IniKey}].Count}] string ${ArrayScope}
      /declare ${ArrayName}[${array_size}] string ${ArrayScope}
      /if (${IniExt[${IniKey}].Count}) {
        /for i 1 to ${IniExt[${IniKey}].Count}
        |/echo ${ArrayName}[${i}] ${IniExt[${IniKey}].Index[${i}]} Length: ${IniExt[${IniKey}].Index[${i}].Length}
        /if (${IniExt[${IniKey}].Index[${i}].Length}) {
          /varset ${ArrayName}[${j}] ${IniExt[${IniKey}].Index[${i}]}
          /varcalc j ${j}+1
        }
        /next i
      }
    }
  } else {
    /call #IniToArray_Deprecated "${IniKey}" "${ArrayName}" "${ArrayScope}"
  }
/return

Sub #IniToVar_Deprecated(Ini_Key, VarToMake, VarType, VarScope)
  /if (${IniExt[${Ini_Key}].Length}) {
    /if (${VarType.Equal[bool]}) {
      /declare ${VarToMake} bool ${VarScope} ${If[${Select[${IniExt[${Ini_Key}]},TRUE,ON,YES,1]},TRUE,FALSE]}
    } else {
      /declare ${VarToMake} ${VarType} ${VarScope} ${IniExt[${Ini_Key}]}
    }
  }
/return ${${VarToMake}}

Sub #IniToVar(Ini_Key, VarToMake, VarType, VarScope)
  /if (${Defined[${VarToMake}]}) /deletevar ${VarToMake}
  /call #RequirePlugin MQ2IniExt
  /if (${Macro.Return}) {
    /if (${IniExt[${Ini_Key}].Length}) {
      /if (${VarType.Equal[bool]}) {
        /declare ${VarToMake} bool ${VarScope} ${If[${Select[${IniExt[${Ini_Key}]},TRUE,ON,YES,1]},TRUE,FALSE]}
      } else {
        /declare ${VarToMake} ${VarType} ${VarScope} ${IniExt[${Ini_Key}]}
      }
    }
  } else {
    /call #IniToVar_Deprecated "${Ini_Key}" "${VarToMake}" "${VarType}" "${VarScope}"
  }
/return ${${VarToMake}}


Sub #TestNewArray
  /call #RequirePlugin mq2iniext
  /declare macroVersion string outer 5.1.21 (r6)
  /declare MacroData_Ini string outer e3 Macro Inis\e3 Data.ini
  /declare advSettings_Ini string outer ${IniExt[${MacroData_Ini},File Paths,Advanced Settings]}
  /echo advSettings_Ini = ${advSettings_Ini}
  /declare genSettings_Ini string outer ${IniExt[${MacroData_Ini},File Paths,General Settings]}
  |/mqp
  /if (${Bool[${IniExt[${genSettings_Ini}]}]} && !${IniExt[${MacroData_Ini},e3 Build,Version].Equal[${macroVersion}]}) {
    /echo versions don't match
  }
  /echo advSettings_Ini = ${advSettings_Ini}
  /call #IniToArray "${advSettings_Ini},${Me.Class.ShortName} Functions,${Me.Class.ShortName} Function#" mainLoop_Array
  /call #PrintArray mainLoop_Array
  /call #IniToArray "${advSettings_Ini},SHM Functions,SHM Function#" shmArray
  /call #PrintArray shmArray
/return