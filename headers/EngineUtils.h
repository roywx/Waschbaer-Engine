#ifndef ENGINEUTILS_H
#define ENGINEUTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include <type_traits>
#include <algorithm>

#include "rapidjson/document.h"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class EngineUtils{
public:
    static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document){
        FILE* file_pointer = nullptr;
#ifdef _WIN32
	    fopen_s(&file_pointer, path.c_str(), "rb");
#else
	    file_pointer = fopen(path.c_str(), "rb");
#endif
        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);
        std::fclose(file_pointer);

        if (out_document.HasParseError()) {
        //   rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
            std::cout << "error parsing json at [" << path << "]" << std::endl;
            exit(0);
        }
    };
    
    static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase){
        size_t pos = input.find(phrase);
        if(pos == std::string::npos) return "";
        pos += phrase.length();
        while(pos < input.size() && std::isspace(input[pos])){
            ++pos;
        }
        if(pos == input.size()) return "";
        
        size_t endPos = pos;
        while(endPos < input.size() && !std::isspace(input[endPos])){
            ++endPos;
        }
        return input.substr(pos, endPos - pos);
    };

    static void ReportError(const std::string& actor_name, const luabridge::LuaException& e){
        std::string error_message = e.what();
        std::replace(error_message.begin(), error_message.end(), '\\', '/');
        std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
    };

    static void OpenURL(const std::string& url){
        std::string cmd;
#ifdef _WIN32
        cmd += "start " + url;
#elif __APPLE__
        cmd += "open " + url;
#else
        cmd += "xdg-open " + url;
#endif
        std::system(cmd.c_str());
    };
   
    // TODO: May need edge case testing
    template <typename T> 
    static bool json_try_get_value(const rapidjson::Value& obj, const char* key, T& target){
        auto it = obj.FindMember(key);
        if (it == obj.MemberEnd()) return false;
        const auto& val = it->value;

        if constexpr (std::is_same_v<T, std::string>) {
            if (!val.IsString()) return false;
            target = val.GetString();
            return true;
        }else if constexpr (std::is_same_v<T, bool>) {
            if (!val.IsBool()) return false;
            target = val.GetBool();
            return true;
        }else if constexpr (std::is_arithmetic_v<T>) {
            /* Didn't structure this branch as is_floating_point getFloatingPoint(), etc. because
            if target is floating point and JSON value is declared as int (valid declaration), 
            it would not grab the JSON value. Additionally, needed support for uint8 types, etc.
            */
            if (!val.IsNumber()) return false;
    
            if constexpr (std::is_floating_point_v<T>) {
                target = static_cast<T>(val.GetDouble());
            } else if constexpr (std::is_unsigned_v<T>) {
                uint64_t v = val.GetUint64();
                if (v > (std::numeric_limits<T>::max)()) return false;
                target = static_cast<T>(v);
            }else {
                int64_t v = val.GetInt64();
                assert(v >= static_cast<int64_t>(std::numeric_limits<T>::min()));
                assert(v <= std::numeric_limits<T>::max());
                target = static_cast<T>(v);
            }
            return true;
        }
        return false;
    }
};



#endif