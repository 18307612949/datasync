#include "StdAfx.h"
#include "BuildFinally.h"
#include <MyException.h>
CBuildFinally::~CBuildFinally(void)
{
}

CBuildFinally::CBuildFinally(string &jsonData):m_json_data(jsonData){
	
}

static string normalizeFloatingPointStr(double value) {
  char buffer[32];
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__)
  sprintf_s(buffer, sizeof(buffer), "%.16g", value);
#else
  snprintf(buffer, sizeof(buffer), "%.16g", value);
#endif
  buffer[sizeof(buffer) - 1] = 0;
  JSONCPP_STRING s(buffer);
  JSONCPP_STRING::size_type index = s.find_last_of("eE");
  if (index != JSONCPP_STRING::npos) {
    JSONCPP_STRING::size_type hasSign =
        (s[index + 1] == '+' || s[index + 1] == '-') ? 1 : 0;
    JSONCPP_STRING::size_type exponentStartIndex = index + 1 + hasSign;
    JSONCPP_STRING normalized = s.substr(0, exponentStartIndex);
    JSONCPP_STRING::size_type indexDigit =
        s.find_first_not_of('0', exponentStartIndex);
    JSONCPP_STRING exponent = "0";
    if (indexDigit !=
        JSONCPP_STRING::npos) // There is an exponent different from 0
    {
      exponent = s.substr(indexDigit);
    }
    return normalized + exponent;
  }
  return s;
}

string CBuildFinally::JsonValueToString(Json::Value &jsonValue) {
	string retStr = "";
	switch(jsonValue.type()) {
		case Json::stringValue: {
			retStr = jsonValue.asString();
			retStr = "\"" + retStr + "\"";
			break;
		}
		case Json::intValue: {
			retStr = Json::valueToString(jsonValue.asLargestInt());
			break;
		}
		case Json::uintValue: {
			retStr = Json::valueToString(jsonValue.asLargestUInt());
			break;
		}
		case Json::realValue: {
			retStr = normalizeFloatingPointStr(jsonValue.asDouble());
			break;
		}
		case Json::nullValue: {
			retStr = "null";
			break;
		}
		default:{
			retStr = "null";
			//THROW_EXCEPTION(ExceptionParsejson, "unknown json type");
			break;
		}
	}
	return retStr;
}

BOOL CBuildFinally::LoadJson(){
	Json::Reader reader;
	Json::Value root;
	if(reader.parse(m_json_data, root) == false) {
		THROW_EXCEPTION(ExceptionParsejson, "reader parse json failed");
		return FALSE;
	}
	Json::Value vTables;
	if(!root.isMember("tables")){
		THROW_EXCEPTION(ExceptionParsejson, "tables is not exist");
		return FALSE;
	}
	vTables = root["tables"];
	if(vTables.type() != Json::arrayValue) {
		THROW_EXCEPTION(ExceptionParsejson, "tables type is not arrayvalue");
		return FALSE;
	}
	Json::ArrayIndex size = vTables.size();
	string strSelect, strInsert, strUpdate, strColumn, strValues;
	for(Json::ArrayIndex index = 0; index < size; ++index) {
		strColumn = " (";
		strValues = " (";
		strSelect = "select * from";
		strInsert = "insert into";
		strUpdate = "update";
		Json::Value vTableItem = vTables[index];
		if(vTableItem.type() == Json::nullValue)
			return FALSE;
		//表名
		if(!vTableItem.isMember("name")){
			THROW_EXCEPTION(ExceptionParsejson, "tables name is not exist");
			return FALSE;
		}
		Json::Value vTableName = vTableItem["name"];
		if(vTableName.type() != Json::stringValue) {
			THROW_EXCEPTION(ExceptionParsejson, "tables name type is not stringvalue");
			return FALSE;
		}
		strSelect += " ";
		strSelect += vTableName.asString();
		strInsert += " ";
		strInsert += vTableName.asString();
		strUpdate += " ";
		strUpdate += vTableName.asString();

		//列名 数据
		if(!vTableItem.isMember("column") || !vTableItem.isMember("values") || !vTableItem.isMember("mapkey")){
			THROW_EXCEPTION(ExceptionParsejson, "tables column|values|mapkey is not exist");
			return FALSE;
		}
		Json::Value vColumn = vTableItem["column"];
		Json::Value vValues = vTableItem["values"];
		Json::Value vMapKey = vTableItem["mapkey"];
		if(vMapKey.type() != Json::objectValue 
			|| vColumn.type() != Json::arrayValue 
			|| vValues.type() != Json::arrayValue 
			|| vColumn.size() != vValues.size()) {
				THROW_EXCEPTION(ExceptionParsejson, "tables column|values|mapkey type is not enable or size error");
				return FALSE;
		}

		//tx
		/*if(!vMapKey.isMember("key") || !vMapKey.isMember("value")) {
			THROW_EXCEPTION(ExceptionParsejson, "mapkey key|value is not exist");
			return FALSE;
		}
		if(vMapKey["key"].type() != Json::stringValue || vMapKey["value"].type() == Json::nullValue) {
			THROW_EXCEPTION(ExceptionParsejson, "mapkey key|value type is not enable");
			return FALSE;
		}*/

		strUpdate += " set ";
		Json::ArrayIndex columnSize = vColumn.size();
		for(Json::ArrayIndex colIndex = 0; colIndex < columnSize - 1; ++colIndex) {
			//update 数据
			strUpdate += vColumn[colIndex].asString();
			strUpdate += "=";
			strUpdate += JsonValueToString(vValues[colIndex]);
			strUpdate += ", ";

			//insert data
			strColumn += vColumn[colIndex].asString();
			strColumn += ",";
			strValues += JsonValueToString(vValues[colIndex]);
			strValues += ",";
		}
		//update data
		strUpdate += vColumn[columnSize - 1].asString();
		strUpdate += "=";
		strUpdate += JsonValueToString(vValues[columnSize - 1]);
		strUpdate += ", hm_update_timestamp=0";

		//insert data
		strColumn += vColumn[columnSize - 1].asString();
		strColumn += ", hm_update_timestamp)";
	
		strValues += JsonValueToString(vValues[columnSize - 1]);
		strValues += ", 0)";

		//select where  tx
		/*strSelect += " where ";
		strSelect += vMapKey["key"].asString();
		strSelect += "=";
		strSelect += JsonValueToString(vMapKey["value"]);
		
		//update where
		strUpdate += " where ";
		strUpdate += vMapKey["key"].asString();
		strUpdate += "=";
		string strTmp = "";
		if(vMapKey["value"].type() != Json::stringValue) 
		strTmp = "\"" + JsonValueToString(vMapKey["value"]) + "\"";
		else
		strTmp = JsonValueToString(vMapKey["value"]);
		strUpdate += strTmp;*/

		//whc begin 
		strSelect += " where ";
		int iSize = vMapKey.size();
		if(iSize == 0) {
			THROW_EXCEPTION(ExceptionParsejson, "mapkey key size is zero");
			return FALSE;
		}
		Json::Value::Members vMembers= vMapKey.getMemberNames();
		for (int nIndex =0;nIndex<iSize;nIndex++)
		{
			if(!vMapKey.isMember(vMembers[nIndex])) {
				string strerro = vMembers[nIndex]+" is not exist";
				THROW_EXCEPTION(ExceptionParsejson, strerro);
				return FALSE;
			}

			strSelect += vMembers[nIndex];
			strSelect += "=";
			strSelect += JsonValueToString(vMapKey[vMembers[nIndex]]);
			if (nIndex == (iSize -1))
			{
				break;
			}
			strSelect += " and ";
		}

		/*strSelect += vMembersSelect[iSize -1];
		strSelect += "=";
		strSelect += JsonValueToString(vMapKey[vMembersSelect[iSize -1]]);*/
		
		//update where
		strUpdate += " where ";
		for (int nIndex =0;nIndex<iSize;nIndex++)
		{
			strUpdate += vMembers[nIndex];
			strUpdate += "=";

			string strTmp = "";
			if(vMapKey[vMembers[nIndex]].type() != Json::stringValue) 
				strTmp = "\"" + JsonValueToString(vMapKey[vMembers[nIndex]]) + "\"";
			else
				strTmp = JsonValueToString(vMapKey[vMembers[nIndex]]);
			strUpdate += strTmp;

			if (nIndex == (iSize -1))
			{
				break;
			}
			strUpdate += " and ";
		}
		/*strUpdate += vMembersUpdate[iSize -1];
		strUpdate += "=";
		strUpdate += JsonValueToString(vMapKey[vMembersUpdate[iSize -1]]);*/
		
		// end whc

		//insert data
		strInsert += strColumn;
		strInsert += " values ";
		strInsert += strValues;
		
		m_sql_select.push_back(strSelect);
		m_sql_insert.push_back(strInsert);
		m_sql_update.push_back(strUpdate);
	}
	if(m_sql_select.size() != m_sql_insert.size() ||
		m_sql_select.size() != m_sql_update.size() ||
		m_sql_insert.size() != m_sql_update.size()) {
			THROW_EXCEPTION(ExceptionParsejson, "vector select|update|insert size error");
			return FALSE;
	}
	return TRUE;
}