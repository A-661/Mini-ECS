// RapidYAML Utils library. https://github.com/A-661/cpp-strategy
#pragma once
#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <ryml.hpp>

#include "ECS/Math/Vector3.h"

namespace RymlUtils
{
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    inline std::string ToString(ryml::csubstr value)
    {
        return std::string(value.str, value.len);
    }

    inline ryml::csubstr ToCSubstr(std::string_view value)
    {
        return ryml::csubstr(value.data(), value.size());
    }

    inline std::string Trim(const std::string& value)
    {
        size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
        {
            ++begin;
        }

        size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
        {
            --end;
        }

        return value.substr(begin, end - begin);
    }

    inline std::string ToLower(std::string value)
    {
        for (char& ch : value)
        {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        return value;
    }

    class ScopedErrorHandler
    {
    public:
        ScopedErrorHandler()
            : _previous(ryml::get_callbacks())
        {
            ryml::Callbacks callbacks = _previous;
            callbacks
                .set_user_data(this)
                .set_error_basic(&ScopedErrorHandler::OnBasicError)
                .set_error_parse(&ScopedErrorHandler::OnParseError)
                .set_error_visit(&ScopedErrorHandler::OnVisitError);

            ryml::set_callbacks(callbacks);
        }

        ~ScopedErrorHandler()
        {
            ryml::set_callbacks(_previous);
        }

    private:
        [[noreturn]] static void Throw(ryml::csubstr message)
        {
            throw Error(ToString(message));
        }

        [[noreturn]] static void OnBasicError(ryml::csubstr message, ryml::ErrorDataBasic const&, void*)
        {
            Throw(message);
        }

        [[noreturn]] static void OnParseError(ryml::csubstr message, ryml::ErrorDataParse const&, void*)
        {
            Throw(message);
        }

        [[noreturn]] static void OnVisitError(ryml::csubstr message, ryml::ErrorDataVisit const&, void*)
        {
            Throw(message);
        }

    private:
        ryml::Callbacks _previous;
    };

    inline bool HasChild(ryml::ConstNodeRef node, std::string_view key)
    {
        return node.readable() && node.is_map() && node.has_child(ToCSubstr(key));
    }

    inline ryml::ConstNodeRef RequireChild(ryml::ConstNodeRef node, std::string_view key, const std::string& context)
    {
        if (!node.readable() || !node.is_map() || !node.has_child(ToCSubstr(key)))
        {
            throw Error(context + " is missing required field '" + std::string(key) + "'");
        }

        return node[ToCSubstr(key)];
    }

    inline void RequireMap(ryml::ConstNodeRef node, const std::string& context)
    {
        if (!node.readable() || !node.is_map())
        {
            throw Error(context + " must be a YAML map");
        }
    }

    inline void RequireSequence(ryml::ConstNodeRef node, const std::string& context)
    {
        if (!node.readable() || !node.is_seq())
        {
            throw Error(context + " must be a YAML sequence");
        }
    }

    inline std::string ReadScalar(ryml::ConstNodeRef node, const std::string& context)
    {
        if (!node.readable() || !node.has_val())
        {
            throw Error(context + " must be a scalar value");
        }

        return ToString(node.val());
    }

    inline std::string ReadRequiredStringChild(ryml::ConstNodeRef node, std::string_view key, const std::string& context)
    {
        return ReadScalar(RequireChild(node, key, context), context + "." + std::string(key));
    }

    inline bool TryParseFloat(const std::string& text, float& outValue)
    {
        std::string normalized = Trim(text);
        if (normalized.empty())
        {
            return false;
        }

        if (normalized.find('.') == std::string::npos)
        {
            std::replace(normalized.begin(), normalized.end(), ',', '.');
        }

        const char* begin = normalized.data();
        const char* end = begin + normalized.size();
        const std::from_chars_result result = std::from_chars(begin, end, outValue);
        return result.ec == std::errc{} && result.ptr == end;
    }

    inline float ReadRequiredFloatChild(ryml::ConstNodeRef node, std::string_view key, const std::string& context)
    {
        const std::string text = ReadRequiredStringChild(node, key, context);

        float value = 0.0f;
        if (!TryParseFloat(text, value))
        {
            throw Error(context + "." + std::string(key) + " must be a number");
        }

        return value;
    }

    inline float ReadOptionalFloatChild(ryml::ConstNodeRef node, std::string_view key, float fallback, const std::string& context)
    {
        if (!HasChild(node, key))
        {
            return fallback;
        }

        float value = 0.0f;
        if (!TryParseFloat(ReadScalar(node[ToCSubstr(key)], context + "." + std::string(key)), value))
        {
            throw Error(context + "." + std::string(key) + " must be a number");
        }

        return value;
    }

    inline bool ReadOptionalBoolChild(ryml::ConstNodeRef node, std::string_view key, bool fallback, const std::string& context)
    {
        if (!HasChild(node, key))
        {
            return fallback;
        }

        const std::string value = ToLower(ReadScalar(node[ToCSubstr(key)], context + "." + std::string(key)));
        if (value == "true" || value == "1")
        {
            return true;
        }

        if (value == "false" || value == "0")
        {
            return false;
        }

        throw Error(context + "." + std::string(key) + " must be true or false");
    }

    inline uint8_t ReadOptionalPriorityChild(ryml::ConstNodeRef node, std::string_view key, uint8_t fallback, const std::string& context)
    {
        if (!HasChild(node, key))
        {
            return fallback;
        }

        const std::string text = Trim(ReadScalar(node[ToCSubstr(key)], context + "." + std::string(key)));
        int value = 0;
        const char* begin = text.data();
        const char* end = begin + text.size();
        const std::from_chars_result result = std::from_chars(begin, end, value);

        if (result.ec != std::errc{} || result.ptr != end || value < 0 || value > static_cast<int>(std::numeric_limits<uint8_t>::max()))
        {
            throw Error(context + "." + std::string(key) + " must be an integer in range 0..255");
        }

        return static_cast<uint8_t>(value);
    }

    inline Vector3 ReadVector3Child(ryml::ConstNodeRef node, std::string_view key, const std::string& context)
    {
        ryml::ConstNodeRef vectorNode = RequireChild(node, key, context);
        if (!vectorNode.readable() || !vectorNode.is_seq() || vectorNode.num_children() != 3)
        {
            throw Error(context + "." + std::string(key) + " must contain exactly 3 numbers");
        }

        Vector3 result;
        float* values[3] = { &result.x, &result.y, &result.z };
        size_t index = 0;

        for (ryml::ConstNodeRef item : vectorNode.children())
        {
            if (!TryParseFloat(ReadScalar(item, context + "." + std::string(key)), *values[index]))
            {
                throw Error(context + "." + std::string(key) + " contains a non-numeric value");
            }

            ++index;
        }

        return result;
    }

    inline std::string FormatFloat(float value)
    {
        if (value == 0.0f)
        {
            value = 0.0f;
        }

        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::fixed << std::setprecision(3) << value;
        std::string result = stream.str();

        while (result.size() > 2 && result.back() == '0' && result[result.size() - 2] != '.')
        {
            result.pop_back();
        }

        if (!result.empty() && result.back() == '.')
        {
            result.push_back('0');
        }

        return result;
    }

    inline bool IsPlainYamlScalarSafe(const std::string& value)
    {
        if (value.empty())
        {
            return false;
        }

        const std::string lower = ToLower(value);
        if (lower == "true" || lower == "false" || lower == "null" || value == "[]" || value == "{}")
        {
            return false;
        }

        for (const char ch : value)
        {
            const unsigned char uch = static_cast<unsigned char>(ch);
            if (!std::isalnum(uch) && ch != '_' && ch != '-' && ch != '.' && ch != '/')
            {
                return false;
            }
        }

        return true;
    }

    inline std::string EscapeYamlScalar(const std::string& value)
    {
        if (IsPlainYamlScalarSafe(value))
        {
            return value;
        }

        std::string escaped = "\"";
        for (const char ch : value)
        {
            if (ch == '"' || ch == '\\')
            {
                escaped.push_back('\\');
                escaped.push_back(ch);
            }
            else if (ch == '\n')
            {
                escaped += "\\n";
            }
            else if (ch == '\t')
            {
                escaped += "\\t";
            }
            else
            {
                escaped.push_back(ch);
            }
        }

        escaped.push_back('"');
        return escaped;
    }
}
