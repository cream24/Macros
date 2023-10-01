| AAPurchase.inc Revamped
| Rewritten by 3PO with ideas and code snippets from smattowers
| Date 11/11/2008
|
| This is a complete revamp of the AAPicker routine.
| It now parses text from the AA Window to determine the Level required to purchase an
| AA as well as the current level and max level of the AA.
| If your level is below the level required to purchae the AA it will skip it.
| It will detect and skip an AA that you do not have the expansion for.
|    This is performed by first making sure you are the right level to purchase it and
|    then if your banked AA is equal to or higher than that needed it will skip it.
| It will also detect if your class is not able to purchase a skill defined if you
| accidentally add a wrong ability and skip it.
| It will tell you when all your abilities are maxed, will warn you if your banked AAs
| are over 27, and if you reach 30 banked it will switch you to full Level XP.
| Can choose to restrict how many levels of an ability to purchase by putting |# where
| where # would equal the max skill you want to purchase. M would be used to purchase
| the to the max possible level of the ability.
| You can also have multiple setcions in the ini file now. AAtoBuy is the default but
| you can now call the init routine like this "/call AAInit inisection" and setup a special
| section to purchase skills for a shroud without having to lose your section for your
| characters Alt Ability skills.
|
| Todo:
|   Add ability to purchase skills evenly.
|     For instance Combat Agility and Stability you might want to purchase them evenly through all levels.
|
| Usage:
| Setup your ini file.
| For example:
|
|   Your file is AA_Frank.INI
|   [AAtoBuy]
|   AACount=2
|   AA1=Combat Agility|2
|   AA2=Combat Stability|M
|
| Put "/call AAInit" or "/call AAInit inisection" into the initialization section of your macro.
| Put "/call AAPicker" into your main loop.
|
| If you want to use a special purchase section other than AAtoBuy in your ini. For instance to define AA
| you want to purchase when in a shroud change "/call AAInit" to "/call AAInit yoursectionnamehere"
| Kroak: Added third argument. '|S'  Which will make you skip to the next possible AA in the list if you don't have enough AA to purchase.
| This version will also auto-create an .ini file for you at AA_${Me.Name}.ini without setting quantities to buy if you don't have one.
#event AAEarned "#*#You have gained#*#ability point#*#"
| Change this to change which channel you send your notifications to.
| Ex: /bc , /echo , /mqlog
#define def_channel_ /echo  |"/mqlog ${Me.Name}"
Sub AAInit(IniSection)
| Declare local variables
   /declare ListNum string outer
   /declare AAIniFile string local AA_${Me.Name}.ini
   /declare a int local
   /if (!${Defined[IniSection]}) /declare IniSection string local AAtoBuy
   /if (${IniSection.Equal[NULL]}) /varset IniSection AAtoBuy
   /if (!${Ini["${AAIniFile}","${IniSection}"].Length} ) {
   /echo Creating AA_${Me.Name}.ini file
   /call AADump
  }
| Determine the max AACount.
   /call LoadINIVar2 ${IniSection} AACount 1 AACount ${AAIniFile} int
| If AACount is not 0
   /if (${AACount}) {
| Create the AA array
      /declare AA[${AACount}] string outer
| Start for/next loop.
      /for a 1 to ${AACount}
| Load the AAs into the array.
         /call LoadINIVar2 ${IniSection} AA${a} " " AA[${a}] ${AAIniFile}
| End for/next loop.
      /next a
|   /call AAPicker  
   }
/return
Sub AAPicker
| Declare local variables
   /declare x int local
   /declare Cost int local
   /declare AALevelStr string local
   /declare curAALevel int local
   /declare maxAALevel int local
   /declare maxPurchaseLevel int local
   /declare nameAA string local
   /declare requiredLevel int local
   /declare requiredExpansion string local
   /declare maxCounter int local 0
| Declare and set lvlDigits to 2
   /declare lvlDigits int local 2
| Change lvlDigits to 1 if your level is less than 10 (usefull for using this with a shroud)
   /if (${Me.Level}<10) /varset lvlDigits 1
| Start for/next loop.
   /for x 1 to ${AACount}
| Sel ListNum to the page the AA is on.
      /varset nameAA ${AA[${x}].Arg[1,"|"]}
      /if (!${AA[${x}].Arg[2,"|"].Length}) /next x
       /if (${x} > ${AACount}) /return
      /varset ListNum List${AltAbility[${nameAA}].Type}
| Bookmark            
      :BuyAdditional
| If Can Purchase is selected unselect it.
      /if (${Window[AAWindow].Child[CanPurchaseFilter].Checked}) /nomodkey /notify AAWindow CanPurchaseFilter leftmouseup
| Check if the skill is available to your class.
      /if (!${AltAbility[${nameAA}].ID}==NULL) {
| Grab the level information from the AAWindow so that
         /varset AALevelStr ${Window[AAWindow].Child[${ListNum}].List[${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}, 2]}
| we can determine the current AA level
         /varset curAALevel ${AALevelStr.Arg[1,"/"]}
| and the max AA level of the AA.
         /varset maxAALevel ${AALevelStr.Arg[2,"/"]}
| Check if we are leveling to max skill
         /if (${AA[${x}].Arg[2,"|"].Equal[M]}) {
            /varset maxPurchaseLevel ${maxAALevel}
| or if we want to level evenly
         } else /if (${AA[${x}].Arg[2,"|"].Equal[E]}) {
            def_channel_ Error. ${nameAA} is set to equal but this feature is not active yet.
            /return
| or set to the max skill we want to train to
         } else {
            /varset maxPurchaseLevel ${AA[${x}].Arg[2,"|"]}
| and if the max skill we want to train is valid.
            /if (${maxPurchaseLevel}<=0) {
               /if (${Macro.Name.NotEqual[Modbot.Mac]}) def_channel_ Error. ${nameAA} is set to an invalid level to train to. Skipping.
               /next x
            }
         }
| Select the AA so that we can
         /call AASelect "${nameAA}"
| parse the level information from the text window.
         /varset requiredLevel ${Window[AAWindow].FirstChild.Next.Text.Mid[${Math.Calc[${Window[AAWindow].FirstChild.Next.Text.Find[Level:]} + 7]},${lvlDigits}]}
| Set requiredExpansion to the expansion listed in the AAWindow.
         /varset requiredExpansion ${Window[AAWindow].Child[${ListNum}].List[${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}, 4]}
| If AA required level is less than or equal to your level.
         /if (${requiredLevel}<=${Me.Level}) {
| If the AA Cost is not equal to zero length.
            /if (${curAALevel}<${maxPurchaseLevel}) {
| Define Cost be the cost listed in the AAWindow.
               /varset Cost ${Window[AAWindow].Child[${ListNum}].List[${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}, 3]}
| Click the can purchase button.
               /nomodkey /notify AAWindow CanPurchaseFilter leftmouseup
| If the Cost of the ability is less than or equal to your banked AA and if the AA still shows in the list.
               /if (${Cost}<=${Me.AAPoints} && !${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}==NULL) {
| Select and purcahse the AA.
                  /call AASelect "${nameAA}"
                  /call AAPurchase
                  /varcalc curAALevel ${curAALevel}+1
                  def_channel_ ${Time} Purchased level ${curAALevel}/${maxAALevel} of ${nameAA} for ${Cost} points.
                  /delay 2s
| If I still have points after purchasing try to purchase again.
                  /if (${Me.AAPoints}>0) /goto :BuyAdditional
                  /return
| Else, if Cost is greater than banked AA.
               } else /if (${Cost}>${Me.AAPoints}) {
                  def_channel_ Unable to purchase ${nameAA} at this time. You have ${Me.AAPoints} AA and need ${Cost} AA.
                 /if (${AA[${x}].Arg[3,"|"].Equal[S]}) /multiline ; def_channel_ Checking if other AA are available.;/next x
                  /return
| Else, you dont have the expansion.
               } else {
                  def_channel_ You have ${Me.AAPoints} AA and ${nameAA} costs ${Cost} AA. You must not have ${requiredExpansion} or you have not met the prerequisites. Skipping.
                  /varcalc maxCounter ${maxCounter}+1
               }
| Else, the skill is maxed.
            } else {
               /if (${curAALevel}==${maxAALevel} && ${maxPurchaseLevel}==${maxAALevel}) def_channel_ ${nameAA} is Maxed
               /varcalc maxCounter ${maxCounter}+1
            }
| Else, you are not the right level.
         } else {
            def_channel_ You are ${Me.Level} and ${nameAA} requires ${requiredLevel}. Skipping.
         }
| Else, that skill is not available to your class.
      } else /if (${AltAbility[${nameAA}].ID}==NULL) {
         def_channel_ ${nameAA} is not a ${Me.Class} Ability.
         /varcalc maxCounter ${maxCounter}+1
      }
| End for/next loop.
   /next x
| If all skills are maxed or you do not have the right expansions.
   /if (${maxCounter}==${AACount}) {
      def_channel_ All defined skills maxed or unable to be purchased due to expansion/prerequisite issues. You should define new ones.
| Else, if not, wait for next round.
   } else {
      def_channel_ Cycled through all AA and nothing to purchase this round.
   }
| If banked AA is greater than or equal to 27 and less than 30, issue a warning.  This is not an issue on Emu.
|    /if (${Me.AAPoints}>=27 && ${Me.AAPoints}<30) def_channel_ WARNING!!! You are at ${Me.AAPoints} banked AA.
| | If banked AA equals 30, turn off alt exp.
|    /if (${Me.AAPoints}>=30) {
|       def_channel_ You have maxed your AAs and are unable to purchase any of your defined AAs.
|       def_channel_ Changing you to full level XP. Hopefully you are not max level yet.
|       /alt off
   }
/return
Sub AASelect(string AAName)
| Code to select the AA passed into it by AAName.
   /nomodkey /notify AAWindow AAW_Subwindows tabselect ${AltAbility[${AAName}].Type}
   /nomodkey /notify AAWindow ${ListNum} listselect ${Window[AAWindow].Child[${ListNum}].List[=${AAName}]}
   /nomodkey /notify AAWindow ${ListNum} leftmouse ${Window[AAWindow].Child[${ListNum}].List[=${AAName}]}
/return
Sub AAPurchase
| The actual purchase code.
   /nomodkey /notify AAWindow TrainButton leftmouseup
   /delay 2s
| If fast AA purchase is not on, accept the purchase.
   /if (!${Window[OptionsWindow].Child[OptionsGeneralPage].Child[OGP_AANoConfirmCheckbox].Checked}) {
      
      :ConfirmWait
      /delay 10s ${Window[ConfirmationDialogBox].Open}
      /if (!${Window[ConfirmationDialogBox].Open}) /goto :ConfirmWait
      /delay 2s
      /nomodkey /notify ConfirmationDialogBox Yes_Button leftmouseup
      /delay 2s
   }
/return
Sub Event_AAEarned
| Triggered  From event statement at the top
   /call AAPicker
/return
Sub LoadINIVar2(IniSection,IniVar,IniValue,MacroVar,MyIni,VarType)
| INI loading code from ModBot, used with permission.
   /if (!${Defined[${MacroVar}]} && ${Defined[VarType]}) /declare ${MacroVar} ${VarType} outer
   /declare IniString string local ${Ini["${MyIni}",${IniSection},${IniVar},NOTFOUND]}
   /varset ${MacroVar} ${IniString}
   /if (${IniString.Equal["NOTFOUND"]}) {
      /varset ${MacroVar} ${IniValue}
      /ini "${MyIni}" "${IniSection}" "${IniVar}" "${${MacroVar}}"
   }
/return
Sub AADump
/declare a int local 1
/declare b int local 3
/declare AAcnt int local 1
/declare AAIniFile string local AA_${Me.Name}.ini
/ini ${AAIniFile} AAtoBuy AACount 0
/declare AALevelStr string local
/if (${Window[AAWindow].Child[CanPurchaseFilter].Checked}) /nomodkey /notify AAWindow CanPurchaseFilter leftmouseup
/delay 10
:Loop
  /if (${Window[AAWindow].Child[List${b}].List[${a}].Length}) {
    /varset AALevelStr ${Window[AAWindow].Child[List${b}].List[${a}, 2]}
    /if (${AALevelStr.Arg[1,"/"]}<${AALevelStr.Arg[2,"/"]}) {
      /ini "${AAIniFile}" AAtoBuy AA${AAcnt} "${Window[AAWindow].Child[List${b}].List[${a}]}|"
      /varcalc AAcnt ${AAcnt}+1
    }
    /varcalc a ${a}+1
    /goto :Loop
  }
  /if (${b}>1) {
    /varcalc b ${b}-1
    /varset a 1
    /goto :Loop
  }
/ini "${AAIniFile}" AAtoBuy AACount ${Int[${Math.Calc[${AAcnt} -1]}]}
/echo Finished dump to file: ${AAIniFile}
/return
Sub AASave
/declare a int local 1
/declare b int local 3
/declare AAcnt int local 1
/declare AAIniFile string local AA_${Me.Name}.ini
/ini ${AAIniFile} AAtoBuy AACount 0
/declare AALevelStr string local
/if (${Window[AAWindow].Child[CanPurchaseFilter].Checked}) /nomodkey /notify AAWindow CanPurchaseFilter leftmouseup
/delay 10
:Loop
  /if (${Window[AAWindow].Child[List${b}].List[${a}].Length}) {
    /varset AALevelStr ${Window[AAWindow].Child[List${b}].List[${a}, 2]}
    /if (${AALevelStr.Arg[1,"/"]} > 0) {
      /ini "${AAIniFile}" AAtoBuy AA${AAcnt} "${Window[AAWindow].Child[List${b}].List[${a}]}|${AALevelStr.Arg[1,"/"]}"
      /varcalc AAcnt ${AAcnt}+1
    }
    /varcalc a ${a}+1
    /goto :Loop
  }
  /if (${b}>1) {
    /varcalc b ${b}-1
    /varset a 1
    /goto :Loop
  }
/ini "${AAIniFile}" AAtoBuy AACount ${Int[${Math.Calc[${AAcnt} -1]}]}
/echo Finished dump to file: ${AAIniFile}
/return