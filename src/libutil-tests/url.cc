#include "nix/util/url.hh"
#include <gtest/gtest.h>

namespace nix {

/* ----------- tests for url.hh --------------------------------------------------*/

std::string print_map(StringMap m)
{
    StringMap::iterator it;
    std::string s = "{ ";
    for (it = m.begin(); it != m.end(); ++it) {
        s += "{ ";
        s += it->first;
        s += " = ";
        s += it->second;
        s += " } ";
    }
    s += "}";
    return s;
}

TEST(parseURL, parsesSimpleHttpUrl)
{
    auto s = "http://www.example.org/file.tar.gz";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "www.example.org",
        .path = "/file.tar.gz",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsesSimpleHttpsUrl)
{
    auto s = "https://www.example.org/file.tar.gz";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "https",
        .authority = "www.example.org",
        .path = "/file.tar.gz",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsesSimpleHttpUrlWithQueryAndFragment)
{
    auto s = "https://www.example.org/file.tar.gz?download=fast&when=now#hello";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "https",
        .authority = "www.example.org",
        .path = "/file.tar.gz",
        .query = (StringMap) {{"download", "fast"}, {"when", "now"}},
        .fragment = "hello",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsesSimpleHttpUrlWithComplexFragment)
{
    auto s = "http://www.example.org/file.tar.gz?field=value#?foo=bar%23";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "www.example.org",
        .path = "/file.tar.gz",
        .query = (StringMap) {{"field", "value"}},
        .fragment = "?foo=bar#",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsesFilePlusHttpsUrl)
{
    auto s = "file+https://www.example.org/video.mp4";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "file+https",
        .authority = "www.example.org",
        .path = "/video.mp4",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, rejectsAuthorityInUrlsWithFileTransportation)
{
    auto s = "file://www.example.org/video.mp4";
    ASSERT_THROW(parseURL(s), Error);
}

TEST(parseURL, parseIPv4Address)
{
    auto s = "http://127.0.0.1:8080/file.tar.gz?download=fast&when=now#hello";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "127.0.0.1:8080",
        .path = "/file.tar.gz",
        .query = (StringMap) {{"download", "fast"}, {"when", "now"}},
        .fragment = "hello",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parseScopedRFC4007IPv6Address)
{
    auto s = "http://[fe80::818c:da4d:8975:415c\%enp0s25]:8080";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "[fe80::818c:da4d:8975:415c\%enp0s25]:8080",
        .path = "",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parseIPv6Address)
{
    auto s = "http://[2a02:8071:8192:c100:311d:192d:81ac:11ea]:8080";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "[2a02:8071:8192:c100:311d:192d:81ac:11ea]:8080",
        .path = "",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parseEmptyQueryParams)
{
    auto s = "http://127.0.0.1:8080/file.tar.gz?&&&&&";
    auto parsed = parseURL(s);
    ASSERT_EQ(parsed.query, (StringMap) {});
}

TEST(parseURL, parseUserPassword)
{
    auto s = "http://user:pass@www.example.org:8080/file.tar.gz";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "http",
        .authority = "user:pass@www.example.org:8080",
        .path = "/file.tar.gz",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parseFileURLWithQueryAndFragment)
{
    auto s = "file:///none/of//your/business";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "file",
        .authority = "",
        .path = "/none/of//your/business",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsedUrlsIsEqualToItself)
{
    auto s = "http://www.example.org/file.tar.gz";
    auto url = parseURL(s);

    ASSERT_TRUE(url == url);
}

TEST(parseURL, parseFTPUrl)
{
    auto s = "ftp://ftp.nixos.org/downloads/nixos.iso";
    auto parsed = parseURL(s);

    ParsedURL expected{
        .scheme = "ftp",
        .authority = "ftp.nixos.org",
        .path = "/downloads/nixos.iso",
        .query = (StringMap) {},
        .fragment = "",
    };

    ASSERT_EQ(parsed, expected);
}

TEST(parseURL, parsesAnythingInUriFormat)
{
    auto s = "whatever://github.com/NixOS/nixpkgs.git";
    auto parsed = parseURL(s);
}

TEST(parseURL, parsesAnythingInUriFormatWithoutDoubleSlash)
{
    auto s = "whatever:github.com/NixOS/nixpkgs.git";
    auto parsed = parseURL(s);
}

TEST(parseURL, emptyStringIsInvalidURL)
{
    ASSERT_THROW(parseURL(""), Error);
}

/* ----------------------------------------------------------------------------
 * decodeQuery
 * --------------------------------------------------------------------------*/

TEST(decodeQuery, emptyStringYieldsEmptyMap)
{
    auto d = decodeQuery("");
    ASSERT_EQ(d, (StringMap) {});
}

TEST(decodeQuery, simpleDecode)
{
    auto d = decodeQuery("yi=one&er=two");
    ASSERT_EQ(d, ((StringMap) {{"yi", "one"}, {"er", "two"}}));
}

TEST(decodeQuery, decodeUrlEncodedArgs)
{
    auto d = decodeQuery("arg=%3D%3D%40%3D%3D");
    ASSERT_EQ(d, ((StringMap) {{"arg", "==@=="}}));
}

TEST(decodeQuery, decodeArgWithEmptyValue)
{
    auto d = decodeQuery("arg=");
    ASSERT_EQ(d, ((StringMap) {{"arg", ""}}));
}

/* ----------------------------------------------------------------------------
 * percentDecode
 * --------------------------------------------------------------------------*/

TEST(percentDecode, decodesUrlEncodedString)
{
    std::string s = "==@==";
    std::string d = percentDecode("%3D%3D%40%3D%3D");
    ASSERT_EQ(d, s);
}

TEST(percentDecode, multipleDecodesAreIdempotent)
{
    std::string once = percentDecode("%3D%3D%40%3D%3D");
    std::string twice = percentDecode(once);

    ASSERT_EQ(once, twice);
}

TEST(percentDecode, trailingPercent)
{
    std::string s = "==@==%";
    std::string d = percentDecode("%3D%3D%40%3D%3D%25");

    ASSERT_EQ(d, s);
}

/* ----------------------------------------------------------------------------
 * percentEncode
 * --------------------------------------------------------------------------*/

TEST(percentEncode, encodesUrlEncodedString)
{
    std::string s = percentEncode("==@==");
    std::string d = "%3D%3D%40%3D%3D";
    ASSERT_EQ(d, s);
}

TEST(percentEncode, keepArgument)
{
    std::string a = percentEncode("abd / def");
    std::string b = percentEncode("abd / def", "/");
    ASSERT_EQ(a, "abd%20%2F%20def");
    ASSERT_EQ(b, "abd%20/%20def");
}

TEST(percentEncode, inverseOfDecode)
{
    std::string original = "%3D%3D%40%3D%3D";
    std::string once = percentEncode(original);
    std::string back = percentDecode(once);

    ASSERT_EQ(back, original);
}

TEST(percentEncode, trailingPercent)
{
    std::string s = percentEncode("==@==%");
    std::string d = "%3D%3D%40%3D%3D%25";

    ASSERT_EQ(d, s);
}

TEST(percentEncode, yen)
{
    // https://en.wikipedia.org/wiki/Percent-encoding#Character_data
    std::string s = reinterpret_cast<const char *>(u8"円");
    std::string e = "%E5%86%86";

    ASSERT_EQ(percentEncode(s), e);
    ASSERT_EQ(percentDecode(e), s);
}

TEST(nix, isValidSchemeName)
{
    ASSERT_TRUE(isValidSchemeName("http"));
    ASSERT_TRUE(isValidSchemeName("https"));
    ASSERT_TRUE(isValidSchemeName("file"));
    ASSERT_TRUE(isValidSchemeName("file+https"));
    ASSERT_TRUE(isValidSchemeName("fi.le"));
    ASSERT_TRUE(isValidSchemeName("file-ssh"));
    ASSERT_TRUE(isValidSchemeName("file+"));
    ASSERT_TRUE(isValidSchemeName("file."));
    ASSERT_TRUE(isValidSchemeName("file1"));
    ASSERT_FALSE(isValidSchemeName("file:"));
    ASSERT_FALSE(isValidSchemeName("file/"));
    ASSERT_FALSE(isValidSchemeName("+file"));
    ASSERT_FALSE(isValidSchemeName(".file"));
    ASSERT_FALSE(isValidSchemeName("-file"));
    ASSERT_FALSE(isValidSchemeName("1file"));
    // regex ok?
    ASSERT_FALSE(isValidSchemeName("\nhttp"));
    ASSERT_FALSE(isValidSchemeName("\nhttp\n"));
    ASSERT_FALSE(isValidSchemeName("http\n"));
    ASSERT_FALSE(isValidSchemeName("http "));
}

} // namespace nix
