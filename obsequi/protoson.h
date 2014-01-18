// Utility for converting json to protobufs and protobufs to json.
//
// The goals of this utility are:
// - Good quality, standards conformant, handling of protobuf and json.
// - Easy to use in any C++ project. (Minimal source files and dependencies.)
// - Fast.
// - Full UTF-8 support, no plans to support UTF-16/32.
//
// To use just copy the files protoson.{cc,h} into your project and compile.
// (The only dependency is the protobuf library for obvious reasons.)

#ifndef PROTOSON_HEADER
#define PROTOSON_HEADER

#include <string>

namespace google {
namespace protobuf {
class Message;
}
}

namespace protoson {

enum FillType {
  FILL_STRICT,  // Expect all json elements to map to a Message field.
  FILL_COMPATIBLE,  // Ignore json elements that don't map to a Message field.
  FILL_DEBUG,  // Same as COMPATIBLE, but spits out lots of INFO logs.
};

enum LogType {
  LOG_ERROR,
  LOG_WARNING,
  LOG_INFO,
};

typedef void (*Logger)(LogType log_type, const std::string& msg);

// Returns false if json is invalid or if unable to populate Message.
bool fill_message(google::protobuf::Message* msg,
                  const char* json, int size,
                  FillType fill_type = FILL_COMPATIBLE,
                  Logger logger_func = NULL);
bool fill_message(google::protobuf::Message* msg,
                  const std::string& json,
                  FillType fill_type = FILL_COMPATIBLE,
                  Logger logger_func = NULL);

// Return json representation of Message.
std::string get_json(const google::protobuf::Message &msg);

}  // namespace protoson

#endif  // PROTOSON_HEADER
