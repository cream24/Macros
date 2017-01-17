|------------------------------------------------------------------------|
|- e3_Classes_Monk.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
Sub MNK_Setup
/return

Sub MNK_Background_Events
/return

SUB MNK_MacroSettings
/if (${Debug}) /echo |- MNK_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},MNK Functions,MNK Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},MNK Functions,MNK Function" "check_Buffs" 1
/if (${Debug}) /echo <== MNK_MacroSettings -|
/RETURN

SUB MNK_CharacterSettings
/RETURN

Sub MNK_Aliases
/return