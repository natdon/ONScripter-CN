/*
* Copyright (c) 2008-2011, Helios (helios.vmg@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, 
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*     * Products derived from this software may not be called "ONSlaught" nor
*       may "ONSlaught" appear in their names without specific prior written
*       permission from the author. 
*
* THIS SOFTWARE IS PROVIDED BY HELIOS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Common.h"

const char *errorMessages[]={
	//NONS_NO_ERROR
	"",
	//NONS_INVALID_PARAMETER
	"Invalid parameter.",
	//NONS_INVALID_ARCHIVE
	"Invalid archive.",
	//NONS_ARCHIVE_UNINIT
	"Archive uninitialized.",
	//NONS_NOT_IMPLEMENTED
	"Command not implemented.",
	//NONS_NOT_ALLOWED_IN_DEFINE_MODE
	"The command is illegal in the *define block.",
	//NONS_DUPLICATE_CONSTANT_DEFINITION
	"Constant redefinition.",
	//NONS_ALREADY_INITIALIZED
	"The NSA archive had already been initialized.",
	//NONS_NO_SUCH_BLOCK
	"No such block.",
	//NONS_NO_ERROR_BUT_BREAK
	"",
	//NONS_EMPTY_CALL_STACK
	"A return was found while the call stack was empty.",
	//NONS_UNDEFINED_SYNTAX_ERROR
	"Syntax error in expression.",
	//NONS_DIVISION_BY_ZERO
	"Division by zero.",
	//NONS_NON_INTEGRAL_VARIABLE_IN_EXPRESSION
	"Non integral variable in expression.",
	//NONS_UNDEFINED_CONSTANT
	"Undefined constant.",
	//NONS_UNRECOGNIZED_COMMAND
	"Unrecognized command.",
	//NONS_UNMATCHING_OPERANDS
	"The operands do not match.",
	//NONS_INVALID_ID_NAME
	"Invalid identifier name. Only [A-Za-z_][A-Za-z_0-9]*. For constants, names that collide with commands are restricted.",
	//NONS_INSUFFICIENT_PARAMETERS
	"Not enough parameters passed to the command.",
	//NONS_FILE_NOT_FOUND
	"File not found.",
	//NONS_NO_MUSIC_LOADED
	"No music loaded.",
	//NONS_NO_SOUND_EFFECT_LOADED
	"No sound effect loaded.",
	//NONS_INTERNAL_INVALID_PARAMETER
	"Invalid parameter.",
	//NONS_DUPLICATE_EFFECT_DEFINITION
	"Effect redefinition.",
	//NONS_INVALID_RUN_TIME_PARAMETER_VALUE
	"Invalid run time parameter value.",
	//NONS_UNMATCHED_BRAKETS
	"Unmatched [].",
	//NONS_UNRECOGNIZED_OPERATOR
	"Unrecognized token.",
	//NONS_ARRAY_INDEX_OUT_OF_BOUNDS
	"Array index out of bounds. Defaulting to index 0 and continuing evaluation.",
	//NONS_MISSING_Q_IN_ARRAY_DECLARATION
	"Missing ? in array declaration.",
	//NONS_MISSING_B_IN_ARRAY_DECLARATION
	"Missing [] in array declaration.",
	//NONS_EXPECTED_NUMERIC_VARIABLE
	"A numeric variable was expected.",
	//NONS_TOO_MANY_PARAMETERS
	"Too many parameters passed. Some will be ignored.",
	//NONS_NO_JUMPS
	"No jumps matching the condition found.",
	//NONS_UNMATCHED_QUOTES
	"Unmatched quotes.",
	//NONS_ZERO_VALUE_IN_SKIP
	"Zero value in skip.",
	//NONS_EFFECT_CODE_OUT_OF_RANGE
	"Effect code is out of range.",
	//NONS_EMPTY_STRING
	"Empty string.",
	//NONS_INVALID_CHANNEL_INDEX
	"Invalid channel index.",
	//NONS_SCREEN_UNINIT
	"Screen is uninitialized.",
	//NONS_SCRIPT_NOT_FOUND
	"Could not open script file.",
	//NONS_INI_SECTION_NOT_FOUND
	"INI section not found.",
	//NONS_INI_KEY_NOT_FOUND
	"INI key not found.",
	//NONS_INVALID_HEX
	"Invalid hex integer.",
	//NONS_UNIMPLEMENTED_COMMAND
	"This command left unimplemented.",
	//NONS_NO_EFFECT
	"Effect not implemented.",
	//NONS_UNDEFINED_EFFECT
	"Undefined effect.",
	//NONS_UNEXPECTED_NEXT
	"Next or break found without a for inside the current block.",
	//NONS_NO_NEXT
	"No next for open for.",
	//NONS_TRANSPARENCY_METHOD_UNIMPLEMENTED
	"Transparency method not implemented.",
	//NONS_NO_TRAP_SET
	"No trap was set.",
	//NONS_MENU_UNINITIALIZED
	"Menu not initialized.",
	//NONS_NO_BUTTON_IMAGE
	"There is no source image for the button.",
	//NONS_NO_BUTTONS_DEFINED
	"There are no defined buttons at the moment.",
	//NONS_SELECT_TOO_BIG
	"The button layer doesn't fit on the screen.",
	//NONS_NO_START_LABEL
	"The *start block was not found.",
	//NONS_GOSUB
	"",
	//NONS_NO_SPRITE_LOADED_THERE
	"No sprite is loaded at that index.",
	//NONS_INVALID_TRANSPARENCY_METHOD
	"Invalid transparency method.",
	//NONS_UNSUPPORTED_SAVEGAME_VERSION
	"Unsupported savegame version.",
	//NONS_UNDOCUMENTED_COMMAND
	"Undocumented command. The developer doesn't know at this time what this command does with sufficient detail. "
		"If you do, please contact him and explain in as much detail as possible.",
	//NONS_EXPECTED_VARIABLE
	"A variable was expected, but a constant was passed.",
	//NONS_EXPECTED_STRING_VARIABLE
	"A string variable was expected.",
	//NONS_EXPECTED_SCALAR
	"A scalar variable was expected, but an array was passed.",
	//NONS_EXPECTED_ARRAY
	"An array was expected, but a scalar variable was passed.",
	//NONS_VARIABLE_OUT_OF_RANGE
	"The variable or array index is out of range. Variable indices may only be in the range -1073741824 to 1073741823.",
	//NONS_UNDEFINED_ARRAY
	"Undefined array.",
	//NONS_OUT_OF_BOUNDS
	"The subindex is outside the boundaries of the array.",
	//NONS_NO_DEFINE_LABEL
	"The *define block was not found.",
	//NONS_INSUFFICIENT_DIMENSIONS
	"Attempting to get the integer value of an array or too few dimensions.",
	//NONS_TOO_MANY_DIMENSIONS
	"Attempting to use the [] operator on a scalar or too many dimensions.",
	//NONS_ILLEGAL_ARRAY_SPECIFICATION
	"Attempting to dereference with \'?\' something other than a literal or a variable (see section 1.8).",
	//NONS_NEGATIVE_INDEX_IN_ARRAY_DECLARATION
	"Negative index in array declaration.",
	//NONS_LEXICALLY_UNCASTABLE
	"The string cannot be converted to an integer.",
	//NONS_DUPLICATE_LABEL
	"Duplicate label found.",
	//NONS_INVALID_COMMAND_NAME
	"Invalid command name. Only [A-Za-z_][A-Za-z_0-9]*",
	//NONS_NOT_ENOUGH_LINES_TO_SKIP
	"There aren't enough lines to skip. The skip will be ignored.",
	//NONS_BAD_MATRIX
	"The matrix cannot be applied.",
	//NONS_NOT_ENOUGH_VARIABLE_INDICES
	"There aren't enough upper indices to complete the mov operation.",
	//NONS_NO_SUCH_SAVEGAME
	"The savegame doesn't exist.",
	//NONS_HASH_DOES_NOT_MATCH
	"The savegame doesn't appear to belong to this script.",
	//NONS_DUPLICATE_COMMAND_DEFINITION_BUILTIN
	"The command already exists as a built-in command.",
	//NONS_DUPLICATE_COMMAND_DEFINITION_USER
	"The command already exists as a user command.",
	//NONS_NOT_IN_A_USER_COMMAND_CALL
	"Currently not in a user command call.",
	//NONS_NEGATIVE_GOTO_INDEX
	"Goto index is negative.",
	//NONS_NOT_ENOUGH_LABELS
	"The goto index is larger than the goto table.",
	//NONS_NOT_ENOUGH_LOG_PAGES
	"Not enough pages in the log.",
	//NONS_LIBRARY_NOT_FOUND
#if NONS_SYS_WINDOWS
	"DLL not found.",
#elif NONS_SYS_UNIX
	"Shared object (.so) not found.",
#endif
	//NONS_FUNCTION_NOT_FOUND
	"Function not found. Invalid dynamic library.",
	//NONS_NOT_A_DEREFERENCE
	"Expected a variable dereference at the top level of the expression, but found an operation.",
	//NONS_EXPECTED_INTEGRAL_VALUE
	"Expected integral value.",
	//NONS_EXPECTED_STRING_VALUE
	"Expected string value.",
	//NONS_NOT_ALLOWED_IN_RUN_MODE
	"The command is illegal in the *start block.",
	//NONS_CUSTOMSEL_NOT_DEFINED
	"*customsel block not defined.",
	//NONS_NOT_IN_CSEL_CALL
	"Not currently in *customsel call.",
	//NONS_NOT_ENOUGH_PARAMETERS_TO_CSEL
	"Not enough parameters were passed to csel.",
	//NONS_NOT_IN_TEXTGOSUB_CALL
	"Not currently in textgosub call.",
	//NONS_CORRUPTED_SAVEGAME
	"Savegame file is corrupted.",
	//NONS_DUPLICATE_ARRAY_DEFINITION
	"The array has already been defined.",
	//NONS_BAD_MEMBER
	"There is no member of the object.",
	//NONS_NOT_ENOUGH_MEMBERS
	"The stack does not have enough members.",
	//NONS_SAVEOFF_ERROR
	"Saveoff is not in the Textgosub Block.",
	//NONS_SAVEON_ERROR
	"Saveon is not in the Textgosub Block or there is no Saveoff command.",
	0
};
