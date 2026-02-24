//=============================================================================
// json_helpers.h - Thin C++ wrappers around yyjson for Hyperliquid data
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: yyjson.h
// THREAD SAFETY: Stateless functions, safe to call from any thread
//
// Handles HL's quirk of encoding numbers as JSON strings:
//   "accountValue":"511.478065"  (string, not number)
//=============================================================================

#pragma once

#include "yyjson.h"
#include <cstdlib>
#include <cstring>

namespace hl {
namespace json {

/// Get a double from a yyjson object, handling both number and string types.
/// Returns 0.0 if obj is NULL, key not found, or value is null/empty.
inline double getDouble(yyjson_val* obj, const char* key) {
    if (!obj) return 0.0;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item) return 0.0;
    if (yyjson_is_num(item)) return yyjson_get_real(item);
    if (yyjson_is_str(item)) return atof(yyjson_get_str(item));
    return 0.0;
}

/// Get a string from a yyjson object. Returns false if not found or not a string.
inline bool getString(yyjson_val* obj, const char* key, char* out, size_t outSize) {
    if (!obj || !out || outSize == 0) return false;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item || !yyjson_is_str(item)) return false;
    strncpy_s(out, outSize, yyjson_get_str(item), _TRUNCATE);
    return true;
}

/// Get a 64-bit integer from a yyjson object (handles both number and string).
inline long long getInt64(yyjson_val* obj, const char* key) {
    if (!obj) return 0;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item) return 0;
    if (yyjson_is_int(item)) return (long long)yyjson_get_sint(item);
    if (yyjson_is_real(item)) return (long long)yyjson_get_real(item);
    if (yyjson_is_str(item)) return _atoi64(yyjson_get_str(item));
    return 0;
}

/// Navigate into a nested object: obj["key"]. Returns NULL if not found or not an object.
inline yyjson_val* getObject(yyjson_val* obj, const char* key) {
    if (!obj) return nullptr;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item || !yyjson_is_obj(item)) return nullptr;
    return item;
}

/// Get array from object. Returns NULL if not found or not an array.
inline yyjson_val* getArray(yyjson_val* obj, const char* key) {
    if (!obj) return nullptr;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item || !yyjson_is_arr(item)) return nullptr;
    return item;
}

/// Check if an item is null, missing, or the string "null" (HL returns "null" for some fields).
inline bool isNull(yyjson_val* item) {
    if (!item || yyjson_is_null(item)) return true;
    if (yyjson_is_str(item)) {
        return strcmp(yyjson_get_str(item), "null") == 0;
    }
    return false;
}

/// Get a double directly from a yyjson_val (not by key lookup).
/// Handles HL's string-encoded numbers.
inline double valToDouble(yyjson_val* item) {
    if (!item) return 0.0;
    if (yyjson_is_num(item)) return yyjson_get_real(item);
    if (yyjson_is_str(item)) return atof(yyjson_get_str(item));
    return 0.0;
}

/// Get a boolean from a yyjson object. Returns defaultVal if not found.
inline bool getBool(yyjson_val* obj, const char* key, bool defaultVal = false) {
    if (!obj) return defaultVal;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item) return defaultVal;
    if (yyjson_is_bool(item)) return yyjson_get_bool(item);
    return defaultVal;
}

/// Get a string pointer directly from a yyjson_val (not by key lookup).
/// Returns nullptr if item is NULL or not a string. Pointer valid while doc lives.
inline const char* valToString(yyjson_val* item) {
    if (!item || !yyjson_is_str(item)) return nullptr;
    return yyjson_get_str(item);
}

/// Get a string pointer from a yyjson object by key.
/// Returns nullptr if not found or not a string. Pointer valid while doc lives.
inline const char* getStringPtr(yyjson_val* obj, const char* key) {
    if (!obj) return nullptr;
    yyjson_val* item = yyjson_obj_get(obj, key);
    if (!item || !yyjson_is_str(item)) return nullptr;
    return yyjson_get_str(item);
}

} // namespace json
} // namespace hl
