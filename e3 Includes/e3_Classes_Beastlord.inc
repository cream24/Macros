|----------------------------------------------------------------------------|
|- e3_Classes_Beastlord.inc v3.0 ( originally designed to work with e3.mac )-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
Sub BST_Setup
  /if (${Ini[${Character_Ini},Pets,Pet Buff Combat (On/Off)].Length}) /call INItoVar "${Character_Ini},Pets,Pet Growl Combat (On/Off)" petGrowlCombat bool outer
  /call #INItoArray "${Character_Ini},Pets,Pet Growl#" petGrowl
  /if (${petGrowl.Size}>0) /call BuildSpellArray "petGrowl"	"petGrowl2D"
/return

|----------------------------------------------------------------------------|
SUB BST_MacroSettings
/if (${Debug}) /echo |- BST_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#2" "check_Debuffs" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#3" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#4" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#5" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#6" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#7" "check_Pets" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#8" "check_DoTs" 1
/if (${Debug}) /echo <== BST_MacroSettings -|
/RETURN

|----------------------------------------------------------------------------|
Sub pet_Growl
/if (${Debug} || ${Debug_Pets}) /echo |- petGrowl ==>
  /declare s int local
  /for s 1 to ${petGrowl2D.Size[1]}
  /if (!${Bool[${Me.PetBuff[${petGrowl2D[${s},${iSpellName}]}]}]} && !${Bool[${Me.PetBuff[${petGrowl2D[${s},${iCheckFor}]}]}]}) {
    /call check_Ready "petGrowl2D" ${s}
    /if (${Macro.Return}) {
      /if (${Me.Pet.Distance} < ${petGrowl2D[${s},${iMyRange}]}) {
        /call check_Mana "petGrowl2D" ${s}
        /if (${Macro.Return}) {
          /call e3_Cast ${Me.Pet.ID} "petGrowl2D" ${s}
        }
      }
    }
  }
  /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_Pets}) /echo <== petGrowl -|
/return

|----------------------------------------------------------------------------|
#event AE_POS "#*# tells you, 'AEPOS'"
sub Event_AE_POS(string line)
  /if (${Me.Class.ShortName.Equal[BST]}) {
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Paragon of Spirit]}) {
     /bc MGB Paragon of Spirit inc...
     /casting "Mass Group Buff|alt" "-maxtries|3"
     /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
     /delay 5
     /casting "Paragon of Spirit|alt" "-maxtries|3"
     /delay 5 !${Me.AltAbilityReady[Paragon of Spirit]}
     /casting "Paragon of Spirit|alt" "-maxtries|3"
     /rs MGB Paragon of Spirit inc...
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Paragon of Spirit is not available...
  }
/return

|----------------------------------------------------------------------------|
Sub BST_Background_Events
  /doevents AE_POS
/return

SUB BST_CharacterSettings
  /call WriteToIni "${Character_Ini},Pets,Pet Growl"
  /call WriteToIni "${Character_Ini},Pets,Pet Growl Combat (On/Off)" On
/RETURN

Sub BST_Aliases
/return