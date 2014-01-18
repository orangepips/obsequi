#include "protoson.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

using namespace std;

using google::protobuf::Message;
using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::Reflection;

// TODO: make certain functions visible for unit testing
#if TESTING
#define TESTABLE
#include "protoson-test.h"
#else  // TESTING
#define TESTABLE static
#endif

namespace protoson {

// ##################################################################
// escape/unescape string, read/write numbers
// ##################################################################
TESTABLE string escape_string(const char* start, size_t len) {
  string tmp;
  tmp.reserve(len*2);

  size_t prev = 0;
  for (size_t i = 0; i < len; i++) {
    const char* s = NULL;
    switch (start[i]) {
      case '"' : s = "\\\""; break;
      case '\\': s = "\\\\"; break;
      case '\b': s = "\\b"; break;
      case '\f': s = "\\f"; break;
      case '\n': s = "\\n"; break;
      case '\r': s = "\\r"; break;
      case '\t': s = "\\t"; break;
      default: continue;
    }
    tmp.append(start + prev, i - prev);
    tmp.append(s);
    prev = i + 1;
  }
  tmp.append(start + prev, len - prev);
  return tmp;
}

// TODO: Handles most utf-8 strings, doesn't handle \u escaping.
TESTABLE string unescape_string(const char* start, size_t len) {
  string tmp;
  tmp.reserve(len);

  size_t prev = 0;
  for (size_t i = 0; i < len; i++) {
    if (start[i] == '\\') {
      tmp.append(start + prev, i - prev);
      char ch;
      switch(start[i+1]) {
        case '"': case '\\': case '/': ch = start[i+1]; break;
        case 'b': ch = '\b'; break;
        case 'f': ch = '\f'; break;
        case 'n': ch = '\n'; break;
        case 'r': ch = '\r'; break;
        case 't': ch = '\t'; break;
        default: assert(0);  // Should this just be ch = start[i+1]?
      }
      tmp.append(1, ch);
      i++;
      prev = i + 1;
    }
  }
  tmp.append(start + prev, len - prev);
  return tmp;
}

// ##################################################################
// parse_json
// ##################################################################
enum JsonValueType {
  JSON_V_OBJECT,
  JSON_V_ARRAY,
  JSON_V_NUMBER,
  JSON_V_STRING,
  JSON_V_FALSE,
  JSON_V_TRUE,
  JSON_V_NULL,
};

struct JsonValue {
  JsonValueType type;

  bool separator;
  std::vector<JsonValue> values;

  const char* start;  // Not owned
  int len;

  JsonValue* parent;  // Not owned

  bool AddNameSeparator() {
    if (type == JSON_V_OBJECT) {
      if (separator) return false;
      if (values.size() % 2 != 1) return false;
    } else {
      return false;
    }
    separator = true;
    return true;
  }

  bool AddValueSeparator() {
    if (separator) return false;
    if (type == JSON_V_OBJECT) {
      if (values.size() % 2 != 0) return false;
    } else if (type != JSON_V_ARRAY) {
      return false;
    }
    separator = true;
    return true;
  }

  bool Add(const JsonValue& value) {
    if (!separator) return false;
    if (type == JSON_V_OBJECT) {
      if (values.size() % 2 == 0 && value.type == JSON_V_STRING) {
        values.push_back(value);
      } else if (values.size() % 2 == 1) {
        values.push_back(value);
      } else {
        return false;
      }
    } else if (type == JSON_V_ARRAY) {
      values.push_back(value);
    } else {
      return false;
    }
    separator = false;
    return true;
  }

  bool Close() {
    if (values.size() == 0) return true;
    if (separator) return false;
    return (type != JSON_V_OBJECT) || (values.size() % 2 == 0);
  }

  string Name() const {
    return unescape_string(start, len);
  }

  int Int32() const { return atoi(start); }
};

static int parse_string(const char* str, int len) {
  for (int i = 0; i < len; i++) {
    if (str[i] == '\\') i++;
    else if (str[i] == '"') return i;
  }
  return -1;
}

static int parse_number(const char* str, int len) {
  return strspn(str, "-+eE.0123456789.");
}

static bool parse_json(JsonValue* json_value, const char* str, int len) {
  json_value->values.clear();
  json_value->separator = true;
  json_value->start = NULL;
  json_value->len = 0;
  json_value->parent = NULL;

  // Special case to handle the first character
  int i = 0;
  for (bool first = true; i < len && first; i++) {
    char ch = str[i];
    if (ch == '\r' || ch == '\t' || ch == '\n' || ch == ' ') continue;
    if (ch == '[') {
      json_value->type = JSON_V_ARRAY;
      first = false;
    } else if (ch == '{') {
      json_value->type = JSON_V_OBJECT;
      first = false;
    } else {
      return false;
    }
  }

  JsonValue* curr = json_value;
  for (; i < len; i++) {
    char ch = str[i];

    JsonValue value;
    value.separator = true;
    value.start = NULL;
    value.len = 0;
    value.parent = curr;

    if (ch == '\r' || ch == '\t' || ch == '\n' || ch == ' ') continue;

    switch (ch) {
      case '[':
        value.type = JSON_V_ARRAY;
        if (!curr->Add(value)) return false;
        curr = &curr->values[curr->values.size() - 1];
        break;
      case '{':
        value.type = JSON_V_OBJECT;
        if (!curr->Add(value)) return false;
        curr = &curr->values[curr->values.size() - 1];
        break;
      case ']':
        if (curr->type != JSON_V_ARRAY) return false;
        if (!curr->Close()) return false;
        curr = curr->parent;
        if (curr == NULL) return true;
        break;
      case '}':
        if (curr->type != JSON_V_OBJECT) return false;
        if (!curr->Close()) return false;
        curr = curr->parent;
        if (curr == NULL) return true;
        break;
      case ':':
        if (!curr->AddNameSeparator()) return false;
        break;
      case ',':
        if (!curr->AddValueSeparator()) return false;
        break;
      case 'f':
        value.type = JSON_V_FALSE;
        if (strncmp(str + i, "false", 5) != 0) return false;
        i += 4;  // strlen - 1
        if (!curr->Add(value)) return false;
        break;
      case 't':
        value.type = JSON_V_TRUE;
        if (strncmp(str + i, "true", 4) != 0) return false;
        i += 3;  // strlen - 1
        if (!curr->Add(value)) return false;
        break;
      case 'n':
        value.type = JSON_V_NULL;
        if (strncmp(str + i, "null", 4) != 0) return false;
        i += 3;  // strlen - 1
        if (!curr->Add(value)) return false;
        break;
      case '"':
        value.type = JSON_V_STRING;
        value.start = str + i + 1;
        value.len = parse_string(str + i + 1, len - i - 1);
        if (value.len == -1) return false;
        i += value.len + 1;  // strlen + " + " - 1
        if (!curr->Add(value)) return false;
        break;
      default:
        value.type = JSON_V_NUMBER;
        value.start = str + i;
        value.len = parse_number(str + i, len - i);
        if (value.len == 0) return false;
        i += value.len - 1;  // strlen - 1
        if (!curr->Add(value)) return false;
        break;
    }
  }
  return false;
}

// TESTABLE, actually this is only for testing purposes.
bool parse_json_valid(const char* json, int len) {
  JsonValue v;
  return parse_json(&v, json, len);
}


// ##################################################################
// fill_message
// ##################################################################
static bool handle_json_object(Message* msg, const JsonValue& node);

static bool handle_json_value(
    Message* msg, const FieldDescriptor* field, const JsonValue& value) {
  const Reflection* ref = msg->GetReflection();
  assert(ref);
  const bool repeated = field->is_repeated(); 
      
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      if (repeated) ref->AddInt32(msg, field, value.Int32());
      else ref->SetInt32(msg, field, value.Int32());
      break;
    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_UINT64:
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_BOOL:
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      handle_json_object(((repeated) ?
                          ref->AddMessage(msg, field) :
                          ref->MutableMessage(msg, field)),
                         value);
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      if (repeated) ref->AddString(msg, field, value.Name());
      else ref->SetString(msg, field, value.Name());
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      {
        const EnumValueDescriptor* ev = 
            field->enum_type()->FindValueByName(value.Name());
        if (!ev) return true;  // TODO: strict error.
        if (repeated) ref->AddEnum(msg, field, ev);
        else ref->SetEnum(msg, field, ev);
      }
      break;
  }
  return true;
}

static bool handle_json_object(Message* msg, const JsonValue& node) {
  const Descriptor* d = msg->GetDescriptor();
  assert(d);

  for (size_t i = 1; i < node.values.size(); i += 2) {
    const JsonValue& key = node.values[i-1];
    string name(key.start, key.len);
    const JsonValue& value = node.values[i];

    const FieldDescriptor* field = d->FindFieldByName(name);
    if (!field) continue;  // TODO: strict error.

    if (field->is_repeated()) {
      if (value.type != JSON_V_ARRAY) return false;  // TODO: always an error.
      for (size_t j = 0; j < value.values.size(); j++)
        if (!handle_json_value(msg, field, value.values[j])) return false;
    } else {
      if (!handle_json_value(msg, field, value)) return false;
    }
  }
  return true;
}

bool fill_message(google::protobuf::Message* msg,
    const char* json, int size, FillType fill_type, Logger logger_func) {
  JsonValue v;
  if (!parse_json(&v, json, size)) {
    // TODO: should log an ERROR with the last 40 characters of the string.
    return false;
  }

  return handle_json_object(msg, v);
}

bool fill_message(google::protobuf::Message* msg,
    const string& json, FillType fill_type, Logger logger_func) {
  return fill_message(msg, json.c_str(), json.size(), fill_type, logger_func);
}


// ##################################################################
// get_json
// ##################################################################
static void get_json_object(const Message& msg, stringstream& stream);

static void get_json_value(const Message& msg, const FieldDescriptor* field,
    int index, stringstream& stream) {
  const Reflection* ref = msg.GetReflection();
  const bool repeated = field->is_repeated();

  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      if (repeated) stream << ref->GetRepeatedInt32(msg, field, index);
      else stream << ref->GetInt32(msg, field);
      break;
    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_UINT64:
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_BOOL:
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      {
        const Message& mf = (repeated) ?
            ref->GetRepeatedMessage(msg, field, index) :
            ref->GetMessage(msg, field);
        get_json_object(mf, stream);
      }
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      {
        const std::string &value = (repeated)?
            ref->GetRepeatedString(msg, field, index):
            ref->GetString(msg, field);
        stream << "\"" << escape_string(value.c_str(), value.size()) << "\"";
      }
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      {
        const EnumValueDescriptor* ef = (repeated)?
        ref->GetRepeatedEnum(msg, field, index):
        ref->GetEnum(msg, field);

        stream << "\"" << escape_string(ef->name().c_str(), ef->name().size()) << "\"";
      }
      break;
  }
}

static void get_json_object(const Message& msg, stringstream& stream) {
  const Descriptor* d = msg.GetDescriptor();
  assert(d);
  const Reflection* ref = msg.GetReflection();
  assert(ref);

  std::vector<const FieldDescriptor*> fields;
  ref->ListFields(msg, &fields);

  stream << "{";
  bool first = true;
  for (size_t i = 0; i < fields.size(); i++) {
    const FieldDescriptor* field = fields[i];
    if (!first) stream << ",";
    first = false;

    stream << "\"" << field->name() << "\":";

    if (field->is_repeated()) {
      size_t count = ref->FieldSize(msg, field);
      if (!count) continue;

      stream << "[";
      bool first_array = true;
      for (size_t j = 0; j < count; j++) {
        if (!first_array) stream << ",";
        first_array = false;
        get_json_value(msg, field, j, stream);
      }
      stream << "]";
    } else if (ref->HasField(msg, field)) {
      get_json_value(msg, field, 0, stream);
    } else {
      continue;
    }
  }
  stream << "}";
}

std::string get_json(const google::protobuf::Message &msg) {
  stringstream stream;
  get_json_object(msg, stream);
  return stream.str();
}

}  // namespace protoson
