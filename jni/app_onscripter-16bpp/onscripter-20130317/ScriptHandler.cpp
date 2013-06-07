/* -*- C++ -*-
 *
 *  ScriptHandler.cpp - Script manipulation class
 *
 *  Copyright (c) 2001-2012 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ScriptHandler.h"
#include "onslocale.h"
#include "Crypt.h"
#include "ScriptParser.h"

#define TMP_SCRIPT_BUF_LEN 4096
#define STRING_BUFFER_LENGTH 2048

#define SKIP_SPACE(p) while ( getchar3(p) == ' ' || getchar3(p) == '\t' ) (p)++

bool encrypt;
bool useencrypt;
bool usemode=true;

ScriptHandler::ScriptHandler()
{
    num_of_labels = 0;
    script_buffer = NULL;
    kidoku_buffer = NULL;
    log_info[LABEL_LOG].filename = "NScrllog.dat";
    log_info[FILE_LOG].filename  = "NScrflog.dat";
    clickstr_list = NULL;
    
    string_buffer       = new char[STRING_BUFFER_LENGTH];
    str_string_buffer   = new char[STRING_BUFFER_LENGTH];
    saved_string_buffer = new char[STRING_BUFFER_LENGTH];

    variable_data = NULL;
    extended_variable_data = NULL;
    num_extended_variable_data = 0;
    max_extended_variable_data = 1;
    root_array_variable = NULL;
    
    screen_width  = 640;
    screen_height = 480;
    variable_range = 0;
    global_variable_border = 0;
}

ScriptHandler::~ScriptHandler()
{
    reset();
    
    if ( script_buffer ) delete[] script_buffer;
    if ( kidoku_buffer ) delete[] kidoku_buffer;

    delete[] string_buffer;
    delete[] str_string_buffer;
    delete[] saved_string_buffer;
    
    if (variable_data) delete[] variable_data;
}

void ScriptHandler::reset()
{
    for (int i=0 ; i<variable_range ; i++)
        variable_data[i].reset(true);

    if (extended_variable_data) delete[] extended_variable_data;
    extended_variable_data = NULL;
    num_extended_variable_data = 0;
    max_extended_variable_data = 1;

    ArrayVariable *av = root_array_variable;
    while(av){
        ArrayVariable *tmp = av;
        av = av->next;
        delete tmp;
    }
    root_array_variable = current_array_variable = NULL;

    // reset log info
    resetLog( log_info[LABEL_LOG] );
    resetLog( log_info[FILE_LOG] );
    
    // reset number alias
    Alias *alias;
    alias = root_num_alias.next;
    while (alias){
        Alias *tmp = alias;
        alias = alias->next;
        delete tmp;
    };
    last_num_alias = &root_num_alias;
    last_num_alias->next = NULL;

    // reset string alias
    alias = root_str_alias.next;
    while (alias){
        Alias *tmp = alias;
        alias = alias->next;
        delete tmp;
    };
    last_str_alias = &root_str_alias;
    last_str_alias->next = NULL;

    // reset misc. variables
    end_status = END_NONE;
    kidokuskip_flag = false;
    text_flag = true;
    linepage_flag = false;
    english_mode = false;
    textgosub_flag = false;
    skip_enabled = false;
    if (clickstr_list){
        delete[] clickstr_list;
        clickstr_list = NULL;
    }
    internal_current_script = NULL;
}

FILE *ScriptHandler::fopen( const char *path, const char *mode )
{
    char * file_name = new char[strlen(archive_path)+strlen(path)+1];
    sprintf( file_name, "%s%s", archive_path, path );

    FILE *fp = ::fopen( file_name, mode );
    delete[] file_name;

    return fp;
}

void ScriptHandler::setKeyTable( const unsigned char *key_table )
{
    int i;
    if (key_table){
        key_table_flag = true;
        for (i=0 ; i<256 ; i++) this->key_table[i] = key_table[i];
    }
    else{
        key_table_flag = false;
        for (i=0 ; i<256 ; i++) this->key_table[i] = i;
    }
}

// basic parser function
const char *ScriptHandler::readToken()
{
    current_script = next_script;
	wait_script = NULL;
    char *buf = current_script;
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    text_flag = false;

    SKIP_SPACE( buf );
    markAsKidoku( buf );

  readTokenTop:
    string_counter = 0;
    char ch = getchar3(buf);
//    char ch=*buf;
    if (ch == ';'){ // comment
        addStringBuffer( ch );
        do{
            ch = getchar3(++buf);
            addStringBuffer( ch );
        } while ( ch != 0x0a && ch != '\0' );
    }
    else if (ch & 0x80 ||
             (ch >= '0' && ch <= '9') ||
             ch == '@' || ch == '\\' || ch == '/' ||
             ch == '%' || ch == '?' || ch == '$' ||
             ch == '[' || ch == '(' || ch == '<' ||
#ifndef ENABLE_1BYTE_CHAR
             ch == '`' ||
#endif             
             (!english_mode && ch == '>') ||
             ch == '!' || ch == '#' || ch == ',' || ch == '"'){ // text
        while(1){
            if ( IS_TWO_BYTE(ch) ){
                addStringBuffer( ch );
                ch = getchar3(++buf);
                if (ch == 0x0a || ch == '\0') break;
                addStringBuffer( ch );
                buf++;
            }
            else{
                if (ch == '%' || ch == '?'){
                    addIntVariable(&buf);
                    SKIP_SPACE(buf);
                }
                else if (ch == '$'){
                    addStrVariable(&buf);
                    SKIP_SPACE(buf);
                }
                else{
                    if (ch == 0x0a || ch == '\0') break;
                    addStringBuffer( ch );
                    buf++;
					if (!wait_script && ch == '@') wait_script = buf;
                }
            }
            ch = getchar3(buf);
        }

        text_flag = true;
    }
#ifdef ENABLE_1BYTE_CHAR
    else if (ch == '`'){
        ch = getchar3(++buf);
        while (ch != '`' && ch != 0x0a && ch !='\0'){
            if ( IS_TWO_BYTE(ch) ){
                addStringBuffer( ch );
                ch = getchar3(++buf);
            }
            addStringBuffer( ch );
            ch = getchar3(++buf);
        }
        if (ch == '`') buf++;
        
        text_flag = true;
        end_status |= END_1BYTE_CHAR;
    }
#endif
    else if (english_mode && ch == '>'){
        ch = getchar3(++buf);
        while (1){
            if (ch == 0x0a || ch =='\0') break;

            if (ch != '\t') 
                addStringBuffer( ch );
            ch = getchar3(buf);
        }
        
        text_flag = true;
        end_status |= END_1BYTE_CHAR;
    }
    else if ((ch >= 'a' && ch <= 'z') || 
             (ch >= 'A' && ch <= 'Z') ||
             ch == '_'){ // command
        do{
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            addStringBuffer( ch );
            ch = getchar3(++buf);
        }
        while((ch >= 'a' && ch <= 'z') || 
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_');
    }
    else if (ch == '*'){ // label
        return readLabel();
    }
    else if (ch == '~' || ch == 0x0a || ch == ':'){
        addStringBuffer( ch );
        markAsKidoku( buf++ );
    }
    else if (ch != '\0'){
        fprintf(stderr, "readToken: skip unknown heading character %c (%x)\n", ch, ch);
        buf++;
        goto readTokenTop;
    }

    next_script = checkComma(buf);

    //printf("readToken [%s] len=%d [%c(%x)] %p\n", string_buffer, strlen(string_buffer), ch, ch, next_script);

    return string_buffer;
}

const char *ScriptHandler::readLabel()
{
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    string_counter = 0;
    char ch = getchar3(buf);
    if (ch == '$'){
        addStrVariable(&buf);
    }
    else if ((ch >= 'a' && ch <= 'z') || 
        (ch >= 'A' && ch <= 'Z') ||
        ch == '_' || ch == '*'){
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
        addStringBuffer( ch );
        buf++;
        if (ch == '*') SKIP_SPACE(buf);
        
        ch = getchar3(buf);
        while((ch >= 'a' && ch <= 'z') || 
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_'){
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            addStringBuffer( ch );
            ch = getchar3(++buf);
        }
    }
    addStringBuffer( '\0' );
    
    next_script = checkComma(buf);

    return string_buffer;
}

const char *ScriptHandler::readStr()
{
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    string_buffer[0] = '\0';
    string_counter = 0;

    while(1){
        parseStr(&buf);
        buf = checkComma(buf);
        string_counter += strlen(str_string_buffer);
        if (string_counter+1 >= STRING_BUFFER_LENGTH)
            errorAndExit("readStr: string length exceeds 2048 bytes.");
        strcat(string_buffer, str_string_buffer);
        if (getchar3(buf) != '+') break;
        buf++;
    }
    next_script = buf;
    
    return string_buffer;
}

int ScriptHandler::readInt()
{
    string_counter = 0;
    string_buffer[string_counter] = '\0';

    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;
    int ret = parseIntExpression(&buf);
    
    next_script = checkComma(buf);

    return ret;
}

void ScriptHandler::skipToken()
{
    SKIP_SPACE( current_script );
    char *buf = current_script;

    bool quat_flag = false;
    bool text_flag = false;
    while(1){
        if ( getchar3(buf) == 0x0a || getchar3(buf) == 0 ||
             (!quat_flag && !text_flag && (getchar3(buf) == ':' || getchar3(buf) == ';') ) ) break;
        if ( getchar3(buf) == '"' ) quat_flag = !quat_flag;
        if ( IS_TWO_BYTE(getchar3(buf)) ){
            buf += 2;
            if ( !quat_flag ) text_flag = true;
        }
        else
            buf++;
    }
    if (text_flag && getchar3(buf) == 0x0a) buf++;
    
    next_script = buf;
}

// string access function
char *ScriptHandler::saveStringBuffer()
{
    strcpy( saved_string_buffer, string_buffer );
    return saved_string_buffer;
}

// script address direct manipulation function
void ScriptHandler::setCurrent(char *pos)
{
    current_script = next_script = pos;
}

void ScriptHandler::pushCurrent( char *pos )
{
    pushed_current_script = current_script;
    pushed_next_script = next_script;

    current_script = pos;
    next_script = pos;
}

void ScriptHandler::popCurrent()
{
    current_script = pushed_current_script;
    next_script = pushed_next_script;
}

void ScriptHandler::enterExternalScript(char *pos)
{
    internal_current_script = current_script;
    current_script = pos;
    internal_next_script = next_script;
    next_script = pos;
    internal_end_status = end_status;
    internal_current_variable = current_variable;
    internal_pushed_variable = pushed_variable;
}

void ScriptHandler::leaveExternalScript()
{
    current_script = internal_current_script;
    internal_current_script = NULL;
    next_script = internal_next_script;
    end_status = internal_end_status;
    current_variable = internal_current_variable;
    pushed_variable = internal_pushed_variable;
}

bool ScriptHandler::isExternalScript()
{
    return (internal_current_script != NULL);
}

int ScriptHandler::getOffset( char *pos )
{
    return pos - script_buffer;
}

char *ScriptHandler::getAddress( int offset )
{
    return script_buffer + offset;
}

int ScriptHandler::getLineByAddress( char *address )
{
    LabelInfo label = getLabelByAddress( address );

    char *addr = label.label_header;
    int line = 0;
    while ( address > addr ){
        if ( getchar3(addr) == 0x0a ) line++;
        addr++;
    }
    return line;
}

char *ScriptHandler::getAddressByLine( int line )
{
    LabelInfo label = getLabelByLine( line );
    
    int l = line - label.start_line;
    char *addr = label.label_header;
    while ( l > 0 ){
        while( getchar3(addr) != 0x0a ) addr++;
        addr++;
        l--;
    }
    return addr;
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByAddress( char *address )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_address > address )
            return label_info[i];
    }
    return label_info[i];
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByLine( int line )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_line > line )
            return label_info[i];
    }
    return label_info[i];
}

bool ScriptHandler::isName( const char *name )
{
    if (string_buffer[0] == '_')
        return (strncmp( name, string_buffer+1, strlen(name) )==0)?true:false;
    return (strncmp( name, string_buffer, strlen(name) )==0)?true:false;
}

bool ScriptHandler::isText()
{
    return text_flag;
}

bool ScriptHandler::compareString(const char *buf)
{
    SKIP_SPACE(next_script);
    unsigned int i, num = strlen(buf);
    for (i=0 ; i<num ; i++){
        unsigned char ch = getchar3(&next_script[i]);
        if ('A' <= ch && 'Z' >= ch) ch += 'a' - 'A';
        if (ch != buf[i]) break;
    }
	
    return (i==num)?true:false;
}

void ScriptHandler::skipLine( int no )
{
    for ( int i=0 ; i<no ; i++ ){
        while ( getchar3(current_script) != 0x0a ) current_script++;
        current_script++;
    }
    next_script = current_script;
}

void ScriptHandler::setLinepage( bool val )
{
    linepage_flag = val;
}

// function for kidoku history
bool ScriptHandler::isKidoku()
{
    return skip_enabled;
}

void ScriptHandler::markAsKidoku( char *address )
{
    if (!kidokuskip_flag || internal_current_script != NULL) return;

    int offset = current_script - script_buffer;
    if ( address ) offset = address - script_buffer;
    //printf("mark (%c)%x:%x = %d\n", *current_script, offset /8, offset%8, kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)));
    if ( kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)) )
        skip_enabled = true;
    else
        skip_enabled = false;
    kidoku_buffer[ offset/8 ] |= ((char)1 << (offset % 8));
}

void ScriptHandler::setKidokuskip( bool kidokuskip_flag )
{
    this->kidokuskip_flag = kidokuskip_flag;
}

void ScriptHandler::saveKidokuData()
{
    FILE *fp;

    if ( ( fp = fopen( "kidoku.dat", "wb" ) ) == NULL ){
        fprintf( stderr, "can't write kidoku.dat\n" );
        return;
    }

    fwrite( kidoku_buffer, 1, script_buffer_length/8, fp );
    fclose( fp );
}

void ScriptHandler::loadKidokuData()
{
    FILE *fp;

    setKidokuskip( true );
    kidoku_buffer = new char[ script_buffer_length/8 + 1 ];
    memset( kidoku_buffer, 0, script_buffer_length/8 + 1 );

    if ( ( fp = fopen( "kidoku.dat", "rb" ) ) != NULL ){
        fread( kidoku_buffer, 1, script_buffer_length/8, fp );
        fclose( fp );
    }
}

void ScriptHandler::addIntVariable(char **buf)
{
    char num_buf[20];
    int no = parseInt(buf);

    int len = getStringFromInteger( num_buf, no, -1 );
    for (int i=0 ; i<len ; i++)
        addStringBuffer( num_buf[i] );
}

void ScriptHandler::addStrVariable(char **buf)
{
    (*buf)++;
    int no = parseInt(buf);
    VariableData &vd = getVariableData(no);
    if ( vd.str ){
        for (unsigned int i=0 ; i<strlen( vd.str ) ; i++){
            addStringBuffer( vd.str[i] );
        }
    }
}

void ScriptHandler::enableTextgosub(bool val)
{
    textgosub_flag = val;
}

void ScriptHandler::setClickstr(const char *list)
{
    if (clickstr_list) delete[] clickstr_list;
    clickstr_list = new char[strlen(list)+2];
    memcpy( clickstr_list, list, strlen(list)+1 );
    clickstr_list[strlen(list)+1] = '\0';
}

int ScriptHandler::checkClickstr(const char *buf, bool recursive_flag)
{
    if ( getchar3(buf) == '@' || getchar3(buf) == '\\' ) return 1;

    if (clickstr_list == NULL) return 0;

    bool double_byte_check = true;
    char *click_buf = clickstr_list;
    while(click_buf[0]){
#ifdef ENABLE_1BYTE_CHAR
        if (click_buf[0] == '`'){
            click_buf++;
            double_byte_check = false;
            continue;
        }
#endif
        if (double_byte_check){
            if ( click_buf[0] == getchar3(buf) && click_buf[1] == getchar3(buf+1)){
                if (!recursive_flag && checkClickstr(buf+2, true) > 0) return 0;
                return 2;
            }
            click_buf += 2;
        }
        else{
            if ( click_buf[0] == getchar3(buf) ){
                if (!recursive_flag && checkClickstr(buf+1, true) > 0) return 0;
                return 1;
            }
            click_buf++;
        }
    }

    return 0;
}

int ScriptHandler::getIntVariable( VariableInfo *var_info )
{
    if ( var_info == NULL ) var_info = &current_variable;
    
    if ( var_info->type == VAR_INT )
        return getVariableData(var_info->var_no).num;
    else if ( var_info->type == VAR_ARRAY )
        return *getArrayPtr( var_info->var_no, var_info->array, 0 );
    return 0;
}

void ScriptHandler::readVariable( bool reread_flag )
{
    end_status = END_NONE;
    current_variable.type = VAR_NONE;
    if ( reread_flag ) next_script = current_script;
    current_script = next_script;
    char *buf = current_script;

    SKIP_SPACE(buf);

    bool ptr_flag = false;
    if ( getchar3(buf) == 'i' || getchar3(buf) == 's' ){
        ptr_flag = true;
        buf++;
    }

    if ( getchar3(buf) == '%' ){
        buf++;
        current_variable.var_no = parseInt(&buf);
        current_variable.type = VAR_INT;
    }
    else if ( getchar3(buf) == '?' ){
        ArrayVariable av;
        current_variable.var_no = parseArray( &buf, av );
        current_variable.type = VAR_ARRAY;
        current_variable.array = av;
    }
    else if ( getchar3(buf) == '$' ){
        buf++;
        current_variable.var_no = parseInt(&buf);
        current_variable.type = VAR_STR;
    } 

    if (ptr_flag) current_variable.type |= VAR_PTR;

    next_script = checkComma(buf);
}

void ScriptHandler::setInt( VariableInfo *var_info, int val, int offset )
{
    if ( var_info->type & VAR_INT ){
        setNumVariable( var_info->var_no + offset, val );
    }
    else if ( var_info->type & VAR_ARRAY ){
        *getArrayPtr( var_info->var_no, var_info->array, offset ) = val;
    }
    else{
        errorAndExit( "setInt: no variables." );
    }
}

void ScriptHandler::pushVariable()
{
    pushed_variable = current_variable;
}

void ScriptHandler::pushBufVariable()
{
    switch_buf = current_variable;
}


void ScriptHandler::setNumVariable( int no, int val )
{
    VariableData &vd = getVariableData(no);
    if ( vd.num_limit_flag ){
        if      ( val < vd.num_limit_lower ) val = vd.num_limit_lower;
        else if ( val > vd.num_limit_upper ) val = vd.num_limit_upper;
    }
    vd.num = val;
}

int ScriptHandler::getStringFromInteger( char *buffer, int no, int num_column, bool is_zero_inserted )
{
    int i, num_space=0, num_minus = 0;
    if (no < 0){
        num_minus = 1;
        no = -no;
    }
    int num_digit=1, no2 = no;
    while(no2 >= 10){
        no2 /= 10;
        num_digit++;
    }

    if (num_column < 0) num_column = num_digit+num_minus;
    if (num_digit+num_minus <= num_column)
        num_space = num_column - (num_digit+num_minus);
    else{
        for (i=0 ; i<num_digit+num_minus-num_column ; i++)
            no /= 10;
        num_digit -= num_digit+num_minus-num_column;
    }

#if defined(ENABLE_1BYTE_CHAR) && defined(FORCE_1BYTE_CHAR)
    if (num_minus == 1) no = -no;
    char format[6];
    if (is_zero_inserted)
        sprintf(format, "%%0%dd", num_column);
    else
        sprintf(format, "%%%dd", num_column);
    sprintf(buffer, format, no);
    
    return num_column;
#else
    int c = 0;
    if (is_zero_inserted){
        for (i=0 ; i<num_space ; i++){
			if (!useencrypt)
			{
				buffer[c++] = ((char*)"£°")[0];
				buffer[c++] = ((char*)"£°")[1];
			}
			else
			{
				buffer[c++] = ((char*)"°«")[0];
				buffer[c++] = ((char*)"°«")[1];
			}
        }
    }
    else{
        for (i=0 ; i<num_space ; i++){
			if (!useencrypt)
			{
				buffer[c++] = ((char*)"¡¡")[0];
				buffer[c++] = ((char*)"¡¡")[1];
			}
			else
			{
				buffer[c++] = ((char*)"¡«")[0];
				buffer[c++] = ((char*)"¡«")[1];
			}
        }
    }
    if (num_minus == 1){
        buffer[c++] = "|"[0];
        buffer[c++] = "|"[1];
    }
    c = (num_column-1)*2;

	if (!useencrypt)
	{
		char num_str[]= "£°£±£²£³£´£µ£¶£·£¸£¹";
		for (i=0 ; i<num_digit ; i++){
        buffer[c]   = num_str[ no % 10 * 2];
        buffer[c+1] = num_str[ no % 10 * 2 + 1];
        no /= 10;
        c -= 2;
		}
	}
	else
	{
		char num_str2[]= "°«±“²Ÿ³®´¨µ§¶ª·¨¸ ¹«";
		for (i=0 ; i<num_digit ; i++){
        buffer[c]   = num_str2[ no % 10 * 2];
        buffer[c+1] = num_str2[ no % 10 * 2 + 1];
        no /= 10;
        c -= 2;
		}
	}

    buffer[num_column*2] = '\0';

    return num_column*2;
#endif    
}

int ScriptHandler::openScript(char *path)
{
    if (readScript(path) < 0) return -1;
    readConfiguration();
    variable_data = new VariableData[variable_range];
    return labelScript();
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabel( const char *label )
{
    int i = findLabel( label );

    findAndAddLog( log_info[LABEL_LOG], label_info[i].name, true );
    return label_info[i];
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabelNext( const char *label )
{
    int i = findLabel( label );
    if ( i+1 < num_of_labels ){
        findAndAddLog( log_info[LABEL_LOG], label_info[i+1].name, true );
        return label_info[i+1];
    }

    return label_info[num_of_labels];
}

ScriptHandler::LogLink *ScriptHandler::findAndAddLog( LogInfo &info, const char *name, bool add_flag )
{
    char capital_name[256];
    for ( unsigned int i=0 ; i<strlen(name)+1 ; i++ ){
        capital_name[i] = name[i];
        if ( 'a' <= capital_name[i] && capital_name[i] <= 'z' ) capital_name[i] += 'A' - 'a';
        else if ( capital_name[i] == '/' ) capital_name[i] = '\\';
    }
    
    LogLink *cur = info.root_log.next;
    while( cur ){
        if ( !strcmp( cur->name, capital_name ) ) break;
        cur = cur->next;
    }
    if ( !add_flag || cur ) return cur;

    LogLink *link = new LogLink();
    link->name = new char[strlen(capital_name)+1];
    strcpy( link->name, capital_name );
    info.current_log->next = link;
    info.current_log = info.current_log->next;
    info.num_logs++;
    
    return link;
}

void ScriptHandler::resetLog( LogInfo &info )
{
    LogLink *link = info.root_log.next;
    while( link ){
        LogLink *tmp = link;
        link = link->next;
        delete tmp;
    }

    info.root_log.next = NULL;
    info.current_log = &info.root_log;
    info.num_logs = 0;
}

ScriptHandler::ArrayVariable *ScriptHandler::getRootArrayVariable(){
    return root_array_variable;
}

void ScriptHandler::addNumAlias( const char *str, int no )
{
    Alias *p_num_alias = new Alias( str, no );
    last_num_alias->next = p_num_alias;
    last_num_alias = last_num_alias->next;
}

void ScriptHandler::addStrAlias( const char *str1, const char *str2 )
{
    Alias *p_str_alias = new Alias( str1, str2 );
    last_str_alias->next = p_str_alias;
    last_str_alias = last_str_alias->next;
}

void ScriptHandler::errorAndExit( const char *str )
{
	fprintf( stderr, " **** Script error, %s [%s] ***\n", str, string_buffer );
    exit(-1);

}

void ScriptHandler::addStringBuffer( char ch )
{
    if (string_counter+1 == STRING_BUFFER_LENGTH)
        errorAndExit("addStringBuffer: string length exceeds 2048.");
    string_buffer[string_counter++] = ch;
    string_buffer[string_counter] = '\0';
}

ScriptHandler::VariableData &ScriptHandler::getVariableData(int no)
{
	if (localon_flag){
        if (sp->last_nest_info->previous && sp->last_nest_info->is_func && no>=0 && no < 15)
            return sp->last_nest_info->vd[no];
    } 

    if (no >= 0 && no < variable_range)
        return variable_data[no];

    for (int i=0 ; i<num_extended_variable_data ; i++)
        if (extended_variable_data[i].no == no) 
            return extended_variable_data[i].vd;
        
    num_extended_variable_data++;
    if (num_extended_variable_data == max_extended_variable_data){
        ExtendedVariableData *tmp = extended_variable_data;
        extended_variable_data = new ExtendedVariableData[max_extended_variable_data*2];
        if (tmp){
            memcpy(extended_variable_data, tmp, sizeof(ExtendedVariableData)*max_extended_variable_data);
            delete[] tmp;
        }
        max_extended_variable_data *= 2;
    }

    extended_variable_data[num_extended_variable_data-1].no = no;

    return extended_variable_data[num_extended_variable_data-1].vd;
}

// ----------------------------------------
// Private methods

int ScriptHandler::readScript( char *path )
{
    archive_path = new char[strlen(path) + 1];
    strcpy( archive_path, path );

    FILE *fp = NULL;
    char filename[10];
    int i, encrypt_mode = 0;
    if ((fp = fopen("0.txt", "rb")) != NULL){
        encrypt_mode = 0;
    }
    else if ((fp = fopen("00.txt", "rb")) != NULL){
        encrypt_mode = 0;
    }
    else if ((fp = fopen("nscr_sec.dat", "rb")) != NULL){
        encrypt_mode = 2;
    }
    else if ((fp = fopen("nscript.___", "rb")) != NULL){
        encrypt_mode = 3;
    }
	else if ((fp = fopen("onscript.nt3", "rb")) != NULL){
		if ( fgetc(fp) == 'P' && fgetc(fp) == 'K' ) 
		{
			encrypt_mode = 18; 
		}
		else
		{
			encrypt_mode = 17;
			encrypt = true;
			useencrypt = true;
		}
    }
    else if ((fp = fopen("nscript.dat", "rb")) != NULL){
        encrypt_mode = 1;
    }
	else if ((fp = fopen("onscript.nt", "rb")) != NULL){
        encrypt_mode = 15;
		encrypt = true;
		useencrypt = true;
    }
	else if ((fp = fopen("onscript.nt2", "rb")) != NULL){
        encrypt_mode = 16;
    }
	

    if (fp == NULL){
        fprintf( stderr, "can't open any of 0.txt, 00.txt, nscript.dat and nscript.___\n" );
        return -1;
    }
    
    fseek( fp, 0, SEEK_END );
    int estimated_buffer_length = ftell( fp ) + 1;

    if (encrypt_mode == 0){
        fclose(fp);
        for (i=1 ; i<100 ; i++){
            sprintf(filename, "%d.txt", i);
            if ((fp = fopen(filename, "rb")) == NULL){
                sprintf(filename, "%02d.txt", i);
                fp = fopen(filename, "rb");
            }
            if (fp){
                fseek( fp, 0, SEEK_END );
                estimated_buffer_length += ftell(fp)+1;
                fclose(fp);
            }
        }
    }

    if ( script_buffer ) delete[] script_buffer;
    script_buffer = new char[ estimated_buffer_length ];

    char *p_script_buffer;
    current_script = p_script_buffer = script_buffer;
    
    tmp_script_buf = new char[TMP_SCRIPT_BUF_LEN];
    if (encrypt_mode > 0){
        fseek( fp, 0, SEEK_SET );
        readScriptSub( fp, &p_script_buffer, encrypt_mode );
        fclose( fp );
    }
    else{
        for (i=0 ; i<100 ; i++){
            sprintf(filename, "%d.txt", i);
            if ((fp = fopen(filename, "rb")) == NULL){
                sprintf(filename, "%02d.txt", i);
                fp = fopen(filename, "rb");
            }
            if (fp){
                readScriptSub( fp, &p_script_buffer, 0 );
                fclose(fp);
            }
        }
    }
    delete[] tmp_script_buf;

    script_buffer_length = p_script_buffer - script_buffer;


    return 0;
    
}

int count=0,count2=0,count3=0,count4=0,count5=0,count6=0,count7=0,count8=0,count9=0,count10=0,m_count=0,m_count2=0,m_count3=0,sss=0;

int choose()
{
	m_count++;
	if(m_count<10)
	{
		switch(m_count)
		{
		case 1:
			count++;
			
			if(count <=8)
			{
				
			switch(count)
			{
				case 0:
					return 9+55;
					break;
				case 1:
					return 8+24;
					break;
				case 2:
					return 3+85;
					break;
				case 3:
					return 7+56;
					break;
				case 4:
					return 1+43;
					break;
				case 5:
					return 2+46;
					break;
				case 6:
					return 4+35;
					break;
				case 7:
					return 5+65;
					break;
				case 8:
					return 6+33;
					break;
				default:
					return 0+26;
					break;
				}
			}
			else
			{
				count = 0;
				return 0+26;
			}
			break;
		case 2:
			count2++;
			
			if(count2 <=8)
			{
				
			switch(count2)
			{
				case 0:
					return 9+52;
					break;
				case 1:
					return 8+26;
					break;
				case 2:
					return 3+87;
					break;
				case 3:
					return 7+51;
					break;
				case 4:
					return 1+48;
					break;
				case 5:
					return 2+43;
					break;
				case 6:
					return 4+34;
					break;
				case 7:
					return 5+69;
					break;
				case 8:
					return 6+35;
					break;
				default:
					return 0+21;
					break;
				}
			}
			else
			{
				count2 = 0;
				return 0+33;
			}
			break;
		case 3:
			count3++;
			
			if(count3 <=8)
			{
				
			switch(count3)
			{
				case 0:
					return 9+36;
					break;
				case 1:
					return 8+65;
					break;
				case 2:
					return 3+76;
					break;
				case 3:
					return 7+24;
					break;
				case 4:
					return 1+63;
					break;
				case 5:
					return 2+44;
					break;
				case 6:
					return 4+52;
					break;
				case 7:
					return 5+14;
					break;
				case 8:
					return 6+35;
					break;
				default:
					return 0+22;
					break;
				}
			}
			else
			{
				count3 = 0;
				return 0+84;
			}
			break;
		
		case 4:
			count4++;
			
			if(count4 <=8)
			{
				
			switch(count4)
			{
				case 0:
					return 9+38;
					break;
				case 1:
					return 8+42;
					break;
				case 2:
					return 3+69;
					break;
				case 3:
					return 7+39;
					break;
				case 4:
					return 1+24;
					break;
				case 5:
					return 2+75;
					break;
				case 6:
					return 4+68;
					break;
				case 7:
					return 5+55;
					break;
				case 8:
					return 6+45;
					break;
				default:
					return 0+27;
					break;
				}
			}
			else
			{
				count4 = 0;
				return 0+47;
			}
			break;
		case 5:
			count5++;
			
			if(count5 <=8)
			{
				
			switch(count5)
			{
				case 0:
					return 9+35;
					break;
				case 1:
					return 8+75;
					break;
				case 2:
					return 3+46;
					break;
				case 3:
					return 7+81;
					break;
				case 4:
					return 1+57;
					break;
				case 5:
					return 2+38;
					break;
				case 6:
					return 4+36;
					break;
				case 7:
					return 5+76;
					break;
				case 8:
					return 6+77;
					break;
				default:
					return 0+73;
					break;
				}
			}
			else
			{
				count5 = 0;
				return 0+31;
			}
			break;
		case 6:
			count6++;
			
			if(count6 <=8)
			{
				
			switch(count6)
			{
				case 0:
					return 9+31;
					break;
				case 1:
					return 8+74;
					break;
				case 2:
					return 3+48;
					break;
				case 3:
					return 7+21;
					break;
				case 4:
					return 1+17;
					break;
				case 5:
					return 2+88;
					break;
				case 6:
					return 4+66;
					break;
				case 7:
					return 5+46;
					break;
				case 8:
					return 6+27;
					break;
				default:
					return 0+23;
					break;
				}
			}
			else
			{
				count6 = 0;
				return 0+51;
			}
			break;
		case 7:
			count7++;
			
			if(count7 <=8)
			{
				
			switch(count7)
			{
				case 0:
					return 9+31;
					break;
				case 1:
					return 8+44;
					break;
				case 2:
					return 3+41;
					break;
				case 3:
					return 7+75;
					break;
				case 4:
					return 1+82;
					break;
				case 5:
					return 2+10;
					break;
				case 6:
					return 4+6;
					break;
				case 7:
					return 5+36;
					break;
				case 8:
					return 6+87;
					break;
				default:
					return 0+29;
					break;
				}
			}
			else
			{
				count7 = 0;
				return 0+55;
			}
			break;
		case 8:
			count8++;
			
			if(count8 <=8)
			{
				
			switch(count8)
			{
				case 0:
					return 9+31;
					break;
				case 1:
					return 8+12;
					break;
				case 2:
					return 3+17;
					break;
				case 3:
					return 7+22;
					break;
				case 4:
					return 1+18;
					break;
				case 5:
					return 2+58;
					break;
				case 6:
					return 4+46;
					break;
				case 7:
					return 5+66;
					break;
				case 8:
					return 6+27;
					break;
				default:
					return 0+33;
					break;
				}
			}
			else
			{
				count8 = 0;
				return 0+11;
			}
			break;
		default:
			count9++;
			
			if(count9 <=8)
			{
				
			switch(count9)
			{
				case 0:
					return 9+31;
					break;
				case 1:
					return 8+14;
					break;
				case 2:
					return 3+68;
					break;
				case 3:
					return 7+81;
					break;
				case 4:
					return 1+37;
					break;
				case 5:
					return 2+38;
					break;
				case 6:
					return 4+76;
					break;
				case 7:
					return 5+64;
					break;
				case 8:
					return 6+71;
					break;
				default:
					return 0+33;
					break;
				}
			}
			else
			{
				count9 = 0;
				return 0+57;
			}
			break;
		
		}
		
	}
	else
	{
		m_count = 0;
		count10++;
		
		if(count10 <=8)
		{
			
		switch(count10)
		{
			case 0:
				return 9+31;
				break;
			case 1:
				return 8+54;
				break;
			case 2:
				return 3+38;
				break;
			case 3:
				return 7+11;
				break;
			case 4:
				return 1+14;
				break;
			case 5:
				return 2+58;
				break;
			case 6:
				return 4+36;
				break;
			case 7:
				return 5+86;
				break;
			case 8:
				return 6+47;
				break;
			default:
				return 0+93;
				break;
			}
		}
		else
		{
			count10 = 0;
			return 0+91;
		}
	}
}


int sum(int s)
{
	return 1;
}

int myxor()
{
	m_count3++;
	if(m_count3<10)
	{
		switch(m_count3)
		{
		case 1:
			return 0x81&0x96;
			break;
		case 2:
			return 0x84&0x93;
			break;
		case 3:
			return 0x85&0x96;
			break;
		
		case 4:
			return 0x80&0x96;
			break;
		case 5:
			return 0x88&0x96;
			break;
		case 6:
			return 0x84&0x92;
			break;
		case 7:
			return 0x84&0x91;
			break;
		case 8:
			return 0x82&0x93;
			break;
		default:
			return 0x81&0x91;
			break;
		
		}
		
	}
	else
	{
		m_count3 = 0;
		return 0x84&0x96;
	}
}

unsigned long getfilesize(FILE* File_ptr)
{
	unsigned long PosCur=0;
	unsigned long PosBegin=0;
	unsigned long PosEnd=0;
	PosCur=ftell(File_ptr);
	fseek(File_ptr,0L,SEEK_SET);
	PosBegin=ftell(File_ptr);
	fseek(File_ptr,0L,SEEK_END);
	PosEnd=ftell(File_ptr);
	fseek(File_ptr,PosCur,SEEK_SET);
	return PosEnd-PosBegin;
}

int ScriptHandler::decodeNT3(char **buf,size_t size,FILE *fp)
{
	bool newline_flag = true;
    int key;
    unsigned char version;
    if (size <= 2336) errorAndExit("NT3Decoder: invalid nt3 script .") ;
    size -= 2336;
    fseek(fp,2332,SEEK_SET);
    fread(&key,4,1,fp); 
    //fread(&version,1,1,fp);
    int main_key;
    //if (version = 0x23)
	main_key=0x5D588B65;
   // else errorAndExit("NT3Decoder: invalid nt3 script .") ; 

    size = fread(*buf, 1, size, fp);
     int i;

     for (i=1;i<=size;i++){
         key=**buf^key;
         key+=main_key+(**buf)*(size-i+1);
         **buf^=key;
         
         
        if ( getchar3(*buf) == '*' && newline_flag ) num_of_labels++;
        if ( getchar3(*buf) == 0x0a )
            newline_flag = true;
        else{
            if ( getchar3(*buf)  != ' ' && getchar3(*buf) != '\t' )
                newline_flag = false;
        }
    (*buf)++;
    }
    setchar((*buf)++,0x0a);


    return 0;
} 


int ScriptHandler::readScriptSub( FILE *fp, char **buf, int encrypt_mode )
{
    unsigned char magic[5] = {0x79, 0x57, 0x0d, 0x80, 0x04 };
    int  magic_counter = 0;
    bool newline_flag = true;
    bool cr_flag = false;


	int size = getfilesize(fp);

    if (encrypt_mode == 3 && !key_table_flag)
        errorAndExit("readScriptSub: the EXE file must be specified with --key-exe option.");

    size_t len=0, count=0;
	if (encrypt_mode == 18) 
	{
		decodeNT3(buf,size,fp); 
	}
    while(1){
        if (len == count){
            len = fread(tmp_script_buf, 1, TMP_SCRIPT_BUF_LEN, fp);
            if (len == 0){
                if (cr_flag) setchar((*buf)++,0x0a);
                break;
            }
            count = 0;
        }
        char ch = tmp_script_buf[count++];
        if      ( encrypt_mode == 1 ) ch ^= 0x84;
		else if ( encrypt_mode == 15 )
		{
			ch ^=0x85&0x97;
			ch -=1; 
		}
		else if ( encrypt_mode == 16 )
		{

				ch ^=0x85&0x97;
				ch -=1; 

		}
		else if ( encrypt_mode == 17 )
		{
				if(size < 4000)
				{
					ch ^=24^0x80&0x93;
					encrypt = false;
					useencrypt = false;
				}
				else
				{
					ch ^=choose()^ myxor();
					ch -=sum(ch);
				}
				

		}

        else if ( encrypt_mode == 2 ){
            ch = (ch ^ magic[magic_counter++]) & 0xff;
            if ( magic_counter == 5 ) magic_counter = 0;
        }
        else if ( encrypt_mode == 3){
            ch = key_table[(unsigned char)ch] ^ 0x84;
        }

        if ( cr_flag && getchar2(ch) != 0x0a ){
            setchar((*buf)++,0x0a);
            newline_flag = true;
            cr_flag = false;
        }
    
        if ( getchar2(ch) == '*' && newline_flag ) num_of_labels++;
        if ( getchar2(ch) == 0x0d ){
            cr_flag = true;
            continue;
        }
        if ( getchar2(ch) == 0x0a ){
            setchar((*buf)++,0x0a);
            newline_flag = true;
            cr_flag = false;
        }
        else{
            setchar((*buf)++,ch);
            if ( getchar2(ch) != ' ' && getchar2(ch) != '\t' )
                newline_flag = false;
        }
    }

    setchar((*buf)++ , 0x0a);


    return 0;
}

void ScriptHandler::readConfiguration()
{
    variable_range = 4096;
    global_variable_border = 200;

    if (getchar3(script_buffer) != ';')
	{
		usemode=false;
		return;
	}
    
    char *buf = script_buffer+1;

    bool config_flag = false;
    if (getchar3(buf) == '$'){
        config_flag = true;
        buf++;
    }

    while (getchar3(buf) && getchar3(buf) != 0x0a){
        SKIP_SPACE(buf);
        if (!strncmp( buf, getptr("mode"), 4 )){
            buf += 4;
			 if      (!strncmp( buf, getptr("1024"), 3 )){
                screen_width  = 1024;
                screen_height = 768;
                buf += 3;
            }
            if      (!strncmp( buf, getptr("800"), 3 )){
                screen_width  = 800;
                screen_height = 600;
                buf += 3;
            }
            else if (!strncmp( buf, getptr("400"), 3 )){
                screen_width  = 400;
                screen_height = 300;
                buf += 3;
            }
            else if (!strncmp( buf, getptr("320"), 3 )){
                screen_width  = 320;
                screen_height = 240;
                buf += 3;
            }
            else
				
                break;
        }
        else if (!strncmp( buf, getptr("value"), 5 ) ||
                 getchar3(buf) == 'g' || getchar3(buf) == 'G'){
            if (getchar3(buf) == 'g' || getchar3(buf) == 'G') buf++;
            else                            buf += 5;
            SKIP_SPACE(buf);
            global_variable_border = 0;
            while ( getchar3(buf) >= '0' && getchar3(buf) <= '9' )
                global_variable_border = global_variable_border*10 + getchar3(buf++) - '0';
        }
        else if (getchar3(buf) == 'v' || getchar3(buf) == 'V'){
            buf++;
            SKIP_SPACE(buf);
            variable_range = 0;
            while (getchar3(buf) >= '0' && getchar3(buf) <= '9')
                variable_range = variable_range*10 + getchar3(buf++) - '0';
        }
        else if (getchar3(buf) == 's' || getchar3(buf) == 'S'){
            buf++;
            if (!(getchar3(buf) >= '0' && getchar3(buf) <= '9')) break;
            screen_width = 0;
            while (getchar3(buf) >= '0' && getchar3(buf) <= '9')
                screen_width = screen_width*10 + getchar3(buf++) - '0';
            while (getchar3(buf) == ',' || getchar3(buf) == ' ' || getchar3(buf) == '\t') buf++;
            screen_height = 0;
            while (getchar3(buf) >= '0' && getchar3(buf) <= '9')
                screen_height = screen_height*10 + getchar3(buf++) - '0';
        }
        else if (getchar3(buf) == 'l' || getchar3(buf) == 'L'){
            buf++;
            SKIP_SPACE(buf);
            while (getchar3(buf) >= '0' && getchar3(buf) <= '9') buf++;
        }
        else if (getchar3(buf) != ',')
            break;

        SKIP_SPACE(buf);
        if (!config_flag && getchar3(buf) != ',') break;
        if (getchar3(buf) == ',') buf++;
    }
}

int ScriptHandler::labelScript()
{
    int label_counter = -1;
    int current_line = 0;
    char *buf = script_buffer;
    label_info = new LabelInfo[ num_of_labels+1 ];

    while ( buf < script_buffer + script_buffer_length ){
        SKIP_SPACE( buf );
        if ( getchar3(buf) == '*' ){
            setCurrent( buf );
            readLabel();
            label_info[ ++label_counter ].name = new char[ strlen(string_buffer) ];
            strcpy( label_info[ label_counter ].name, string_buffer+1 );
            label_info[ label_counter ].label_header = buf;
            label_info[ label_counter ].num_of_lines = 1;
            label_info[ label_counter ].start_line   = current_line;
            buf = getNext();
            if ( getchar3(buf) == 0x0a ){
                buf++;
                current_line++;
            }
            label_info[ label_counter ].start_address = buf;
        }
        else{
            if ( label_counter >= 0 )
                label_info[ label_counter ].num_of_lines++;
            while( getchar3(buf) != 0x0a ) buf++;
            buf++;
            current_line++;
        }
    }

    label_info[num_of_labels].start_address = NULL;
    
    return 0;
}

int ScriptHandler::findLabel( const char *label )
{
    int i;
    char capital_label[256];

    for ( i=0 ; i<(int)strlen( label )+1 ; i++ ){
        capital_label[i] = label[i];
        if ( 'A' <= capital_label[i] && capital_label[i] <= 'Z' ) capital_label[i] += 'a' - 'A';
    }
    for ( i=0 ; i<num_of_labels ; i++ ){
        if ( !strcmp( label_info[i].name, capital_label ) )
            return i;
    }

    char *p = new char[ strlen(label) + 32 ];
    sprintf(p, "Label \"%s\" is not found.", label);
    errorAndExit( p );
    
    return -1; // dummy
}

char *ScriptHandler::checkComma( char *buf )
{
    SKIP_SPACE( buf );
    if (getchar3(buf) == ','){
        end_status |= END_COMMA;
        buf++;
        SKIP_SPACE( buf );
    }
    
    return buf;
}

void ScriptHandler::parseStr( char **buf )
{
    SKIP_SPACE( *buf );

    if ( getchar3(*buf) == '(' ){
        (*buf)++;
        parseStr(buf);
        SKIP_SPACE( *buf );
        if ( getchar3(*buf) != ')' ) errorAndExit("parseStr: missing ')'.");
        (*buf)++;

        if ( findAndAddLog( log_info[FILE_LOG], str_string_buffer, false ) ){
            parseStr(buf);
            char *tmp_buf = new char[ strlen( str_string_buffer ) + 1 ];
            strcpy( tmp_buf, str_string_buffer );
            parseStr(buf);
            strcpy( str_string_buffer, tmp_buf );
            delete[] tmp_buf;
        }
        else{
            parseStr(buf);
            parseStr(buf);
        }
        current_variable.type |= VAR_CONST;
    }
    else if ( getchar3(*buf) == '$' ){
        (*buf)++;
        int no = parseInt(buf);
        VariableData &vd = getVariableData(no);

        if ( vd.str )
            strcpy( str_string_buffer, vd.str );
        else
            str_string_buffer[0] = '\0';
        current_variable.type = VAR_STR;
        current_variable.var_no = no;
    }
    else if ( getchar3(*buf) == '"' ){
        int c=0;
        (*buf)++;
        while ( getchar3(*buf) != '"' && getchar3(*buf) != 0x0a )
            str_string_buffer[c++] = getchar3((*buf)++);
        str_string_buffer[c] = '\0';
        if ( getchar3(*buf) == '"' ) (*buf)++;
        current_variable.type |= VAR_CONST;
    }
#ifdef ENABLE_1BYTE_CHAR
    else if ( getchar3(*buf) == '`' ){
        int c=0;
        str_string_buffer[c++] = getchar3((*buf)++);
        while ( getchar3(*buf) != '`' && getchar3(*buf) != 0x0a )
            str_string_buffer[c++] = getchar3((*buf)++);
        str_string_buffer[c] = '\0';
        if ( getchar3(*buf) == '`' ) (*buf)++;
        current_variable.type |= VAR_CONST;
        end_status |= END_1BYTE_CHAR;
    }
#endif    
    else if ( getchar3(*buf) == '#' ){ // for color
        for ( int i=0 ; i<7 ; i++ )
            str_string_buffer[i] = getchar3((*buf)++);
        str_string_buffer[7] = '\0';
        current_variable.type = VAR_NONE;
    }
    else if ( getchar3(*buf) == '*' ){ // label
        int c=0;
        str_string_buffer[c++] = getchar3((*buf)++);
        SKIP_SPACE(*buf);
        char ch = getchar3(*buf);
        while((ch >= 'a' && ch <= 'z') || 
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_'){
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            str_string_buffer[c++] = ch;
            ch = getchar3(++(*buf));
        }
        str_string_buffer[c] = '\0';
        current_variable.type |= VAR_CONST;
    }
    else{ // str alias
        char ch, alias_buf[512];
        int alias_buf_len = 0;
        bool first_flag = true;
        
        while(1){
            if ( alias_buf_len == 511 ) break;
            ch = getchar3(*buf);
            
            if ( (ch >= 'a' && ch <= 'z') || 
                 (ch >= 'A' && ch <= 'Z') || 
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( first_flag ) errorAndExit("parseStr: number is not allowed for the first letter of str alias.");
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else break;
            (*buf)++;
        }
        alias_buf[alias_buf_len] = '\0';
        
        if ( alias_buf_len == 0 ){
            str_string_buffer[0] = '\0';
            current_variable.type = VAR_NONE;
            return;
        }
        
        Alias *p_str_alias = root_str_alias.next;

        while( p_str_alias ){
            if ( !strcmp( p_str_alias->alias, (const char*)alias_buf ) ){
                strcpy( str_string_buffer, p_str_alias->str );
                break;
            }
            p_str_alias = p_str_alias->next;
        }
        if ( !p_str_alias ){
            printf("can't find str alias for %s...\n", alias_buf );
            exit(-1);
        }
        current_variable.type |= VAR_CONST;
    }
}

int ScriptHandler::parseInt( char **buf)
{
	
    int ret = 0;
    
    SKIP_SPACE( *buf );

    if ( getchar3(*buf) == '%' ){
        (*buf)++;
        current_variable.var_no = parseInt(buf);
        current_variable.type = VAR_INT;
        return getVariableData(current_variable.var_no).num;
    }
    else if ( **buf == '(' ){
        (*buf)++;
        current_variable.var_no = parseIntExpression(buf);
        current_variable.type = VAR_INT;
        SKIP_SPACE( *buf );
        if ( (*buf)[0] != ')' ) errorAndExit("parseInt: missing ')'.");
        (*buf)++;
        return current_variable.var_no;
    }
    else if ( getchar3(*buf) == '?' ){
        ArrayVariable av;
        current_variable.var_no = parseArray( buf, av );
        current_variable.type = VAR_ARRAY;
        current_variable.array = av;
        return *getArrayPtr( current_variable.var_no, current_variable.array, 0 );
    }
    else if (getchar3(*buf) == '\0')
    errorAndExit("parseInt():alias wanted got \"\"");
    else{
        char ch, alias_buf[256];
        int alias_buf_len = 0, alias_no = 0;
        bool direct_num_flag = false;
        bool num_alias_flag = false;

        char *buf_start = *buf;
        while( 1 ){
            ch = getchar3(*buf);
            
            if ( (ch >= 'a' && ch <= 'z') || 
                 (ch >= 'A' && ch <= 'Z') || 
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                if ( direct_num_flag ) break;
                num_alias_flag = true;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( !num_alias_flag ) direct_num_flag = true;
                if ( direct_num_flag ) 
                    alias_no = alias_no * 10 + ch - '0';
                else
                    alias_buf[ alias_buf_len++ ] = ch;
            }
            else break;
            (*buf)++;
        }

        if ( *buf - buf_start  == 0 ){
            current_variable.type = VAR_NONE;
            return 0;
        }
        
        /* ---------------------------------------- */
        /* Solve num aliases */
        if ( num_alias_flag ){
            alias_buf[ alias_buf_len ] = '\0';
            Alias *p_num_alias = root_num_alias.next;

            while( p_num_alias ){
                if ( !strcmp( p_num_alias->alias,
                              (const char*)alias_buf ) ){
                    alias_no = p_num_alias->num;
                    break;
                }
                p_num_alias = p_num_alias->next;
            }
            if ( !p_num_alias ){
                //printf("can't find num alias for %s... assume 0.\n", alias_buf );
                current_variable.type = VAR_NONE;
                *buf = buf_start;
                return 0;
            }
        }
        current_variable.type = VAR_INT | VAR_CONST;
        ret = alias_no;
    }

    SKIP_SPACE( *buf );

    return ret;
}

int ScriptHandler::parseIntExpression( char **buf )
{
    int num[3], op[2]; // internal buffer

    SKIP_SPACE( *buf );

    readNextOp( buf, NULL, &num[0] );

    readNextOp( buf, &op[0], &num[1] );
    if ( op[0] == OP_INVALID )
        return num[0];

    while(1){
        readNextOp( buf, &op[1], &num[2] );
        if ( op[1] == OP_INVALID ) break;

        if ( !(op[0] & 0x04) && (op[1] & 0x04) ){ // if priority of op[1] is higher than op[0]
            num[1] = calcArithmetic( num[1], op[1], num[2] );
        }
        else{
            num[0] = calcArithmetic( num[0], op[0], num[1] );
            op[0] = op[1];
            num[1] = num[2];
        }
    }
    return calcArithmetic( num[0], op[0], num[1] );
}

/*
 * Internal buffer looks like this.
 *   num[0] op[0] num[1] op[1] num[2]
 * If priority of op[0] is higher than op[1], (num[0] op[0] num[1]) is computed,
 * otherwise (num[1] op[1] num[2]) is computed.
 * Then, the next op and num is read from the script.
 * Num is an immediate value, a variable or a bracketed expression.
 */
void ScriptHandler::readNextOp( char **buf, int *op, int *num )
{
    bool minus_flag = false;
    SKIP_SPACE(*buf);
    char *buf_start = *buf;
    
    if ( op ){
        if      ( getchar3(*buf) == '+' ) *op = OP_PLUS;
        else if ( getchar3(*buf) == '-' ) *op = OP_MINUS;
        else if ( getchar3(*buf) == '*' ) *op = OP_MULT;
        else if ( getchar3(*buf) == '/' ) *op = OP_DIV;
        else if ( getchar3(*buf) == 'm' &&
                  getchar3(*buf+1) == 'o' &&
                  getchar3(*buf+2) == 'd' &&
                  ( getchar3(*buf+3) == ' '  ||
                    getchar3(*buf+3) == '\t' ||
                    getchar3(*buf+3) == '$' ||
                    getchar3(*buf+3) == '%' ||
                    getchar3(*buf+3) == '?' ||
                    ( getchar3(*buf+3) >= '0' && getchar3(*buf+3) <= '9') ))
            *op = OP_MOD;
        else{
            *op = OP_INVALID;
            return;
        }
        if ( *op == OP_MOD ) *buf += 3;
        else                 (*buf)++;
        SKIP_SPACE(*buf);
    }
    else{
        if ( getchar3(*buf) == '-' ){
            minus_flag = true;
            (*buf)++;
            SKIP_SPACE(*buf);
        }
    }

    if ( getchar3(*buf) == '(' ){
        (*buf)++;
        *num = parseIntExpression( buf );
        if (minus_flag) *num = -*num;
        SKIP_SPACE(*buf);
        if ( getchar3(*buf) != ')' ) errorAndExit("Missing ')' in expression");
        (*buf)++;
    }
    else{
        *num = parseInt( buf );
        if (minus_flag) *num = -*num;
        if ( current_variable.type == VAR_NONE ){
            if (op) *op = OP_INVALID;
            *buf = buf_start;
        }
    }
}

int ScriptHandler::calcArithmetic( int num1, int op, int num2 )
{
    int ret=0;
    
    if      ( op == OP_PLUS )  ret = num1+num2;
    else if ( op == OP_MINUS ) ret = num1-num2;
    else if ( op == OP_MULT )  ret = num1*num2;
    else if ( op == OP_DIV )   ret = num1/num2;
    else if ( op == OP_MOD )   ret = num1%num2;

    current_variable.type = VAR_INT | VAR_CONST;

    return ret;
}

int ScriptHandler::parseArray( char **buf, struct ArrayVariable &array )
{
    SKIP_SPACE( *buf );
    
    (*buf)++; // skip '?'
    int no = parseInt( buf );

    SKIP_SPACE( *buf );
    array.num_dim = 0;
    while ( getchar3(*buf) == '[' ){
        (*buf)++;
        array.dim[array.num_dim] = parseIntExpression(buf);
        array.num_dim++;
        SKIP_SPACE( *buf );
        if ( getchar3(*buf) != ']' ) errorAndExit( "parseArray: missing ']'." );
        (*buf)++;
    }
    for ( int i=array.num_dim ; i<20 ; i++ ) array.dim[i] = 0;

    return no;
}

int *ScriptHandler::getArrayPtr( int no, ArrayVariable &array, int offset )
{
    ArrayVariable *av = root_array_variable;
    while(av){
        if (av->no == no) break;
        av = av->next;
    }
    if (av == NULL) errorAndExit( "Array No. is not declared." );
    
    int dim = 0, i;
    for ( i=0 ; i<av->num_dim ; i++ ){
        if ( av->dim[i] <= array.dim[i] ) errorAndExit( "dim[i] <= array.dim[i]." );
        dim = dim * av->dim[i] + array.dim[i];
    }
    if ( av->dim[i-1] <= array.dim[i-1] + offset ) errorAndExit( "dim[i-1] <= array.dim[i-1] + offset." );

    return &av->data[dim+offset];
}

void ScriptHandler::declareDim()
{
    current_script = next_script;
    char *buf = current_script;

    if (current_array_variable){
        current_array_variable->next = new ArrayVariable();
        current_array_variable = current_array_variable->next;
    }
    else{
        root_array_variable = new ArrayVariable();
        current_array_variable = root_array_variable;
    }

    ArrayVariable array;
    current_array_variable->no = parseArray( &buf, array );

    int dim = 1;
    current_array_variable->num_dim = array.num_dim;
    for ( int i=0 ; i<array.num_dim ; i++ ){
        current_array_variable->dim[i] = array.dim[i]+1;
        dim *= (array.dim[i]+1);
    }
    current_array_variable->data = new int[dim];
    memset( current_array_variable->data, 0, sizeof(int) * dim );

    next_script = buf;
}

//defined by taigacon
char *ScriptHandler::getptr(const char *str){
  int len = strlen(str);
  char *back = new char[len+1];
  for (int i = 0;i<len;i++) 
      setchar(back+i,str[i]);
  back[len+1]='\0';
  return back;
}