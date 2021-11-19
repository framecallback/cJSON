#ifndef CppJSON_hpp_
#define CppJSON_hpp_

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "cJSON.h"

// Move copy & not thread safe. Must use reference or pointer when being arguments.
class CppJSON {
public:
  struct iterator {
    cJSON* p;
    iterator() : p(NULL) {}
    iterator(cJSON* p) : p(p) {}
    operator bool() { return !!p; }
    iterator& operator++() { p = p->next; return *this; }
    const iterator operator++(int) { iterator ret(p); p = p->next; return ret; }
    iterator& operator--() { p = (p->prev->next == NULL ? NULL : p->prev); return *this; }
    const iterator operator--(int) { iterator ret(p); p = (p->prev->next == NULL ? NULL : p->prev); return ret; }
    CppJSON operator*() { return p; }
    bool operator==(const iterator& other) { return p == other.p; }
  };

  static CppJSON CreateNull() { return cJSON_CreateNull(); }
  static CppJSON CreateBool(bool v = false) { return cJSON_CreateBool(v); }
  static CppJSON CreateNumber(double v = 0) { return cJSON_CreateNumber(v); }
  static CppJSON CreateString(const std::string& v = std::string()) { return cJSON_CreateString(v.c_str()); }
  static CppJSON CreateArray() { return cJSON_CreateArray(); }
  static CppJSON CreateObject() { return cJSON_CreateObject(); }
  
  CppJSON() : m_json(NULL) {}
  explicit CppJSON(int type) { m_json = cJSON_CreateNode(type); }
  explicit CppJSON(bool v) { m_json = cJSON_CreateBool(v); }
  explicit CppJSON(double v) { m_json = cJSON_CreateNumber(v); }
  explicit CppJSON(const std::string& v) { m_json = cJSON_CreateString(v.c_str()); }
  CppJSON(const bool* p, int count) { m_json = cJSON_CreateBoolArray((const cJSON_bool*)p, count); }
  template<class T>
  CppJSON(const T* p, int count) {
    m_json = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
      cJSON_AddItemToArray(m_json, cJSON_CreateNumber((double)p[i]));
    }
  }
  CppJSON(const char** p, int count) { m_json = cJSON_CreateStringArray(p, count); }
  CppJSON(const std::string* p, int count) {
    m_json = cJSON_CreateArray();
    for (int i = 0; i < count; i++) {
      cJSON_AddItemToArray(m_json, cJSON_CreateString(p[i].c_str()));
    }
  }

  // !!! Move copy !!!
  CppJSON(const CppJSON& other) { m_json = other.m_json; const_cast<CppJSON&>(other).m_json = NULL;}
  CppJSON& operator=(const CppJSON& other) { m_json = other.m_json; const_cast<CppJSON&>(other).m_json = NULL;}

  ~CppJSON() { Clear(); }

  bool Parse(const std::string& str) {
    assert(m_json == NULL);
    m_json = cJSON_Parse(str.c_str());
    return m_json != NULL;
  }

  // info
  int Type() const { return m_json ? m_json->type & 0xFF : 0; }
  const char* Name() const { return (m_json && m_json->name) ? m_json->name : ""; }

  // copy
  CppJSON Duplicate() const { return cJSON_Duplicate(m_json, false); }
  CppJSON DuplicateRecursive() const { return cJSON_Duplicate(m_json, true); }

  // detach, and will be deleted in destruction
  void Detach() { m_json = cJSON_DetachItemViaPointer(m_json); }
  // clear may not delete internal m_json, 
  void Clear() { if (m_json) { cJSON_Delete(m_json); m_json = NULL;} }
  // release current resource 
  void Delete() { Detach(); Clear(); }

  // type
  bool IsValid() const { return !cJSON_IsInvalid(m_json); }
  bool IsEmpty() const { return !m_json; }
  bool IsNull() const { return cJSON_IsNull(m_json); }
  bool IsBool() const { return cJSON_IsBool(m_json); }
  bool IsNumber() const { return cJSON_IsNumber(m_json); }
  bool IsString() const { return cJSON_IsString(m_json); }
  bool IsArray() const { return cJSON_IsArray(m_json); }
  bool IsObject() const { return cJSON_IsObject(m_json); }
  bool IsReference() const { return m_json->type & cJSON_IsReference; }

  // bool
  bool GetBool() const { assert(IsBool()); return cJSON_GetBool(m_json); }
  bool Set(bool v) const { assert(IsBool()); return cJSON_SetBool(m_json, v); }

  // number
  double GetNumber() const { assert(IsNumber()); return cJSON_GetNumber(m_json); }
  bool Set(double v) const { assert(IsNumber()); return cJSON_SetNumber(m_json, v); }

  // string
  const char* GetString() const { assert(IsString()); return cJSON_GetString(m_json); }
  bool Set(const std::string& str) const { assert(IsString()); return cJSON_SetString(m_json, str.c_str()); }

  // array
  int GetArraySize() const { assert(IsArray()); return cJSON_GetArraySize(m_json); }
  CppJSON GetArrayItem(int index) const { assert(IsArray()); return cJSON_GetArrayItem(m_json, index); }
  inline CppJSON operator[] (int index) const { return GetArrayItem(index); }
  bool Add(const CppJSON& obj) { if (IsEmpty()) m_json = cJSON_CreateArray(); assert(IsArray()); return cJSON_AddItemToArray(m_json, obj.m_json); }
  bool Insert(size_t index, const CppJSON& obj) { assert(IsArray()); return cJSON_InsertItemInArray(m_json, index, obj.m_json); }
  inline void Delete(size_t index) { assert(IsArray()); GetArrayItem(index).Delete(); }

  inline bool Add(bool value) { return Add(CppJSON(value)); }
  template<class T>
  inline bool Add(T value) { return Add(CppJSON((double)value)); }
  inline bool Add(const char* value) { return Add(CppJSON(value)); }
  inline bool Add(const std::string& value) { return Add(CppJSON(value)); }
  inline bool AddNull() { return Add(CppJSON(cJSON_NULL)); }
  inline bool AddEmptyObject() { return Add(CppJSON(cJSON_Object)); }
  inline bool AddEmptyArray() { return Add(CppJSON(cJSON_Array)); }

  inline bool Insert(size_t index, bool value) { return Insert(index, CppJSON(value)); }
  template<class T>
  inline bool Insert(size_t index, T value) { return Insert(index, CppJSON((double)value)); }
  inline bool Insert(size_t index, const char* value) { return Insert(index, CppJSON(value)); }
  inline bool Insert(size_t index, const std::string& value) { return Insert(index, CppJSON(value)); }
  inline bool InsertNull(size_t index) { return Insert(index, CppJSON(cJSON_NULL)); }
  inline bool InsertEmptyObject(size_t index) { return Insert(index, CppJSON(cJSON_Object)); }
  inline bool InsertEmptyArray(size_t index) { return Insert(index, CppJSON(cJSON_Array)); }

  // Obj
  bool HasObject(const std::string& name) const { assert(IsObject()); return cJSON_HasObjectItem(m_json, name.c_str()); }
  CppJSON GetObject(const std::string& name) const { assert(IsObject()); return cJSON_GetObjectItem(m_json, name.c_str()); }
  // Support names like "child[4][key]", "zoo[big][tiger][3]", ...
  CppJSON GetObjectRecursive(const std::string& name) const {
    if (name.empty()) return m_json; // current item
    if (name[0] == '[') {
      size_t pos = name.find_first_of(']', 1);
      if (pos == std::string::npos) return CppJSON();
      std::string index = name.substr(1, pos - 1);
      if (IsArray()) {
        size_t v = atoi(index.c_str());
        if (v >= GetArraySize()) return CppJSON();
        return GetArrayItem(v).GetObjectRecursive(name.substr(pos+1));
      } else {
        if (!HasObject(index)) return CppJSON();
        return GetObject(index).GetObjectRecursive(name.substr(pos+1));
      }
    }
    // find parent
    size_t pos = name.find_first_of('[');
    if (pos != std::string::npos) {
      std::string index = name.substr(0, pos);
      if (IsArray()) {
        size_t k = atoi(index.c_str());
        if (k >= GetArraySize()) return CppJSON();
        return GetArrayItem(k).GetObjectRecursive(name.substr(pos));
      } else if (IsObject()) {
        if (!HasObject(index)) return CppJSON();
        return GetObject(index).GetObjectRecursive(name.substr(pos));
      } else {
        return CppJSON();
      }
    }
    // current key
    return GetObject(name);
  }
  inline CppJSON operator[] (const std::string& name) const { return GetObjectRecursive(name); }
  bool Add(const std::string& name, const CppJSON& json) { if (IsEmpty()) m_json = cJSON_CreateObject(); assert(IsObject()); return cJSON_AddItemToObject(m_json, name.c_str(), json.m_json); }
  inline void Delete(const std::string& name) { assert(IsObject()); GetObject(name).Delete(); }

  inline bool Add(const std::string& name, bool value) { return Add(name, CppJSON(value)); }
  template<class T>
  inline bool Add(const std::string& name, T value) { return Add(name, CppJSON((double)value)); }
  inline bool Add(const std::string& name, const char* value) { return Add(name, CppJSON(value)); }
  inline bool Add(const std::string& name, const std::string& value) { return Add(name, CppJSON(value)); }
  inline bool AddNull(const std::string& name) { return Add(name, CppJSON(cJSON_NULL)); }
  inline bool AddEmptyObject(const std::string& name) { return Add(name, CppJSON(cJSON_Object)); }
  inline bool AddEmptyArray(const std::string& name) { return Add(name, CppJSON(cJSON_Array)); }

  // iterator, only array and object may have iterator
  iterator begin() { return IsEmpty() ? NULL : m_json->child; }
  iterator end() { return NULL; }

  // format string
  std::string ToString() const { char* p = cJSON_PrintUnformatted(m_json); std::string ret(p); cJSON_free(p); return ret; }
  std::string ToFormattedString() const { char* p = cJSON_Print(m_json); std::string ret(p); cJSON_free(p); return ret; }
  // fmt must be C format without any other chars, like "%05d", "%.3f"
  char* ToString(const std::string& fmt, char* buffer) const {
    assert(fmt.length() > 0);
    assert(fmt[0] == '%');
    if (IsNull()) {
      sprintf(buffer, fmt.c_str(), "null");
    } else if (IsBool() || IsNumber()) {
      switch(tolower(*fmt.rbegin())) {
        case 'f': case 'e': case 'g': case 'a':
          sprintf(buffer, fmt.c_str(), m_json->number);
          break;
        case 'd': case 'i': case 'u': case 'o': case 'x': case 'c': case 'p':
          sprintf(buffer, fmt.c_str(), (long long int)m_json->number);
          break;
        default:
          strcpy(buffer, "{wrong fmt}");
      }
    } else if (IsString()) {
      sprintf(buffer, fmt.c_str(), m_json->valuestring);
    } else {
      *buffer = '\0';
    }
    return buffer;
  }

private:
  CppJSON(cJSON* js) : m_json(js) {}

  cJSON* m_json;
};

#endif  // CppJSON_hpp_