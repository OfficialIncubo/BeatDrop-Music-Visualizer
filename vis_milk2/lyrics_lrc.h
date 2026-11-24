#pragma once

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>

// A deliberately small, dependency-free LRC reader.  Keeping the timed-text
// format separate from the network and Direct3D code makes seeking and editor
// behaviour deterministic and easy to regression-test.
struct BeatDropLyricLine
{
    double startSeconds = 0.0;
    std::wstring text;
};

namespace BeatDropLrc
{
    inline std::wstring Trim(const std::wstring& value)
    {
        const std::wstring::size_type first = value.find_first_not_of(L" \t\r\n");
        if (first == std::wstring::npos)
            return std::wstring();
        const std::wstring::size_type last = value.find_last_not_of(L" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    inline bool ParseUnsigned(const std::wstring& text, unsigned long long& value)
    {
        if (text.empty())
            return false;

        unsigned long long parsed = 0;
        for (wchar_t ch : text)
        {
            if (ch < L'0' || ch > L'9')
                return false;
            const unsigned digit = static_cast<unsigned>(ch - L'0');
            if (parsed > (0xffffffffffffffffULL - digit) / 10ULL)
                return false;
            parsed = parsed * 10ULL + digit;
        }
        value = parsed;
        return true;
    }

    // Parses the contents of a tag, for example "03:14.250".  LRC uses a
    // dot as its decimal separator regardless of the Windows regional locale.
    inline bool ParseTimestamp(const std::wstring& tag, double& seconds)
    {
        const std::wstring::size_type colon = tag.find(L':');
        if (colon == std::wstring::npos || tag.find(L':', colon + 1) != std::wstring::npos)
            return false;

        unsigned long long minutes = 0;
        if (!ParseUnsigned(tag.substr(0, colon), minutes))
            return false;

        const std::wstring fractionAndSeconds = tag.substr(colon + 1);
        const std::wstring::size_type dot = fractionAndSeconds.find(L'.');
        const std::wstring secondsPart = fractionAndSeconds.substr(0, dot);
        unsigned long long wholeSeconds = 0;
        if (!ParseUnsigned(secondsPart, wholeSeconds) || wholeSeconds >= 60)
            return false;

        double fraction = 0.0;
        if (dot != std::wstring::npos)
        {
            const std::wstring fractionPart = fractionAndSeconds.substr(dot + 1);
            unsigned long long fractionalValue = 0;
            if (fractionPart.empty() || fractionPart.size() > 3 ||
                !ParseUnsigned(fractionPart, fractionalValue))
                return false;

            double divisor = 1.0;
            for (std::wstring::size_type i = 0; i < fractionPart.size(); ++i)
                divisor *= 10.0;
            fraction = static_cast<double>(fractionalValue) / divisor;
        }

        seconds = static_cast<double>(minutes) * 60.0 +
            static_cast<double>(wholeSeconds) + fraction;
        return seconds >= 0.0;
    }

    inline bool ParseOffsetMilliseconds(const std::wstring& tag, long long& milliseconds)
    {
        const std::wstring prefix = L"offset:";
        if (tag.size() <= prefix.size())
            return false;
        for (std::wstring::size_type i = 0; i < prefix.size(); ++i)
            if (std::towlower(tag[i]) != prefix[i])
                return false;

        std::wstring number = tag.substr(prefix.size());
        bool negative = false;
        if (!number.empty() && (number[0] == L'+' || number[0] == L'-'))
        {
            negative = number[0] == L'-';
            number.erase(0, 1);
        }

        unsigned long long magnitude = 0;
        if (!ParseUnsigned(number, magnitude) || magnitude > 2147483647ULL)
            return false;
        milliseconds = negative ? -static_cast<long long>(magnitude) :
            static_cast<long long>(magnitude);
        return true;
    }

    inline std::vector<BeatDropLyricLine> Parse(const std::wstring& source)
    {
        std::vector<BeatDropLyricLine> lines;
        std::wstring text = source;
        if (!text.empty() && text[0] == 0xfeff)
            text.erase(0, 1);

        long long offsetMilliseconds = 0;
        std::wstring::size_type lineStart = 0;
        while (lineStart <= text.size())
        {
            const std::wstring::size_type lineEnd = text.find(L'\n', lineStart);
            std::wstring line = text.substr(lineStart, lineEnd == std::wstring::npos
                ? std::wstring::npos : lineEnd - lineStart);
            if (!line.empty() && line.back() == L'\r')
                line.pop_back();

            std::vector<double> timestamps;
            std::wstring::size_type cursor = 0;
            std::wstring::size_type lastTagEnd = std::wstring::npos;
            while (cursor < line.size() && line[cursor] == L'[')
            {
                const std::wstring::size_type close = line.find(L']', cursor + 1);
                if (close == std::wstring::npos)
                    break;

                const std::wstring tag = line.substr(cursor + 1, close - cursor - 1);
                double timestamp = 0.0;
                long long parsedOffset = 0;
                if (ParseTimestamp(tag, timestamp))
                    timestamps.push_back(timestamp);
                else if (ParseOffsetMilliseconds(tag, parsedOffset))
                    offsetMilliseconds = parsedOffset;
                else
                    break; // metadata tags are only meaningful at the start of a line

                lastTagEnd = close;
                cursor = close + 1;
            }

            if (!timestamps.empty())
            {
                const std::wstring lyric = lastTagEnd == std::wstring::npos
                    ? std::wstring() : Trim(line.substr(lastTagEnd + 1));
                for (double timestamp : timestamps)
                    lines.push_back({timestamp, lyric}); // empty text intentionally creates a silence event
            }

            if (lineEnd == std::wstring::npos)
                break;
            lineStart = lineEnd + 1;
        }

        const double offsetSeconds = static_cast<double>(offsetMilliseconds) / 1000.0;
        for (BeatDropLyricLine& line : lines)
            line.startSeconds = (std::max)(0.0, line.startSeconds + offsetSeconds);

        std::stable_sort(lines.begin(), lines.end(), [](const BeatDropLyricLine& a, const BeatDropLyricLine& b) {
            return a.startSeconds < b.startSeconds;
        });
        return lines;
    }
}
