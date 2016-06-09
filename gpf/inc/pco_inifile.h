/* 
+----------------------------------------------------------------------------- 
|  Project :  PCO2
|  Modul   :  inc\pco_inifile.h
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Contains declarations of the class IniFile. (Based on 
|             File from Adam Clauss)
+----------------------------------------------------------------------------- 
*/ 

#ifndef _PCO_INIFILE_H_
#define _PCO_INIFILE_H_

/*==== INCLUDES ===================================================*/

#pragma warning (disable: 4786) /* ignore warning about to long template names */

#include <iostream>
#include <string>
#include <vector>
#include "cms.h"

using namespace std;

/*==== PROTOTYPES ==================================================*/

//all Keys are of this type
struct Key
{
	//list of values in key
	vector<string> values; 

	//corresponding list of value names
	vector<string> names;

  //corresponding list of modified state
  vector<bool> modified; 
};

class IniFile  
{
	//all private variables
private:

	//stores pathname of ini file to read/write
	string path;
	
	//list of Keys in ini
	vector<Key> keys; 

	//corresponding list of Keynames
	vector<string> names; 

  // semaphore to protect file reading/writing
  CMS_HANDLE m_file_sema;
	
	//all private functions
private:
	//overloaded to take string
	istream & getline( istream & is, string & str );

	//returns index of specified value, in the specified Key, or -1 if not found
	int FindValue(int Keynum, string valuename) const;

	//returns index of specified Key, or -1 if not found
	int FindKey(string Keyname) const;


	//public variables
public:

	//will contain error info if one occurs
	//ended up not using much, just in ReadFile and GetValue
	mutable string error;
  
	//will contain warning info if one occurs
	//ended up not using much, just in ReadFile 
  mutable string warning;

	//public functions
public:
	//default constructor
	IniFile();

	//constructor, can specify pathname here instead of using SetPath later
	IniFile(string inipath);

	//default destructor
	virtual ~IniFile();

	//sets path of ini file to read and write from
	void SetPath(string newpath);

	//returns path of currently selected ini file
  const char* GetPath() const { return path.c_str();}

	//reads ini file specified using IniFile::SetPath()
	//returns true if successful, false otherwise
  //may contain warnings in warning-variable
	bool ReadFile(bool refresh=false);

	//writes data stored in class to ini file
	bool WriteFile(bool refresh_first=true); 

	//deletes all stored ini data
	void Reset();

	//returns number of Keys currently in the ini
	int GetNumKeys() const;

	//returns number of values stored for specified Key
	int GetNumValues(string Keyname) const;

	//gets value of [Keyname] valuename = 
	//overloaded to return string, int, and double,
	//returns "", or 0 if Key/value not found.  Sets error member to show problem
	string GetValue(string Keyname, string valuename) const; 
	int GetValueI(string Keyname, string valuename) const; 
	double GetValueF(string Keyname, string valuename) const;

	//sets value of [Keyname] valuename =.
	//specify the optional paramter as false (0) if you do not want it to create
	//the Key if it doesn't exist. Returns true if data entered, false otherwise
	//overloaded to accept string, int, and double
	bool SetValue(string Key, string valuename, string value, bool create = 1);
	bool SetValueI(string Key, string valuename, int value, bool create = 1);
	bool SetValueF(string Key, string valuename, double value, bool create = 1);

  bool modified(string keyname, string valuename);
  bool set_modified(string keyname, string valuename, bool modified=true);

	//deletes specified value
	//returns true if value existed and deleted, false otherwise
	bool DeleteValue(string Keyname, string valuename);

	//deletes specified Key and all values contained within
	//returns true if Key existed and deleted, false otherwise
	bool DeleteKey(string Keyname);
};

#endif /* _PCO_INIFILE_H_ */
