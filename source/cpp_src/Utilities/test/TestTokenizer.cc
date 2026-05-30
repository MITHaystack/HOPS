#include <iostream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_Tokenizer.hh"

using namespace hops;

static int check_tokens(const std::vector<std::string>& actual,
                        const std::vector<std::string>& expected)
{
    if(actual.size() != expected.size())
    {
        std::cerr << "FAIL: token count " << actual.size()
                  << " != expected " << expected.size()
                  << " @ " << __FILE__ << ":" << __LINE__ << std::endl;
        return 1;
    }
    for(std::size_t i = 0; i < actual.size(); i++)
    {
        if(actual[i] != expected[i])
        {
            std::cerr << "FAIL: token[" << i << "] \"" << actual[i]
                      << "\" != expected \"" << expected[i]
                      << "\" @ " << __FILE__ << ":" << __LINE__ << std::endl;
            return 1;
        }
    }
    return 0;
}

static int get_and_check(MHO_Tokenizer& tok, const std::string* input,
                         const std::vector<std::string>* expected)
{
    tok.SetString(input);
    std::vector<std::string> result;
    tok.GetTokens(&result);
    return check_tokens(result, *expected);
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_Tokenizer tokenizer;

    //  Case 1: Default delimiter (space), no empty tokens
    {
        std::string input = "This is a string separated by spaces.";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("This");
        exp.push_back("is");
        exp.push_back("a");
        exp.push_back("string");
        exp.push_back("separated");
        exp.push_back("by");
        exp.push_back("spaces.");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 2: Single-char delimiter '|'
    {
        std::string input = "This|is|a|string|separated|by|pipes.";
        tokenizer.SetDelimiter("|");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("This");
        exp.push_back("is");
        exp.push_back("a");
        exp.push_back("string");
        exp.push_back("separated");
        exp.push_back("by");
        exp.push_back("pipes.");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 3: Single-char delimiter SET "| \t\r\n"
    {
        std::string input = "This \t is a\tstring|separated by a\nmix.";
        tokenizer.SetDelimiter("| \t\r\n");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("This");
        exp.push_back("is");
        exp.push_back("a");
        exp.push_back("string");
        exp.push_back("separated");
        exp.push_back("by");
        exp.push_back("a");
        exp.push_back("mix.");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 4: Multi-character delimiter "<d>"
    {
        std::string input = "This<d>is<d>a<d>string<d>by<d>a<d>multi-character<d>delimiter.";
        tokenizer.SetDelimiter("<d>");
        tokenizer.SetUseMulticharacterDelimiterTrue();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("This");
        exp.push_back("is");
        exp.push_back("a");
        exp.push_back("string");
        exp.push_back("by");
        exp.push_back("a");
        exp.push_back("multi-character");
        exp.push_back("delimiter.");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 5: Repeated/adjacent delimiters, IncludeEmptyTokens TRUE
    {
        std::string input = "a,,b,";
        tokenizer.SetDelimiter(",");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensTrue();
        std::vector<std::string> exp;
        exp.push_back("a");
        exp.push_back("");
        exp.push_back("b");
        exp.push_back(""); // trailing comma emits empty token after final delimiter
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 6: Repeated/adjacent delimiters, IncludeEmptyTokens FALSE
    {
        std::string input = "a,,b,";
        tokenizer.SetDelimiter(",");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("a");
        exp.push_back("b");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 7: Whitespace trimming with single-char delim
    {
        std::string input = "  alpha , beta ,gamma  ";
        tokenizer.SetDelimiter(",");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("alpha");
        exp.push_back("beta");
        exp.push_back("gamma");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 8: Quote preservation, matched quotes
    {
        std::string input = "This is \"quoted \t text\" here";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesTrue();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("This");
        exp.push_back("is");
        exp.push_back("\"quoted \t text\"");
        exp.push_back("here");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 9: Quote preservation, UNMATCHED quote (odd '"')
    {
        std::string input = "a \"b c";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesTrue();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("a");
        exp.push_back("\"b");
        exp.push_back("c");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 10: Empty input string
    {
        std::string input = "";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp; // empty
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 11: Delimiter-only input (3 spaces, space delim)
    {
        std::string input = "   ";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp; // empty
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 12: Assignment-style delimiter "=;" with trimming
    {
        std::string input = "    mean_motion = 0.; ";
        tokenizer.SetDelimiter("=;");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("mean_motion");
        exp.push_back("0.");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 13: Pol-product split "XX+YY" on "+"
    {
        std::string input = "XX+YY";
        tokenizer.SetDelimiter("+");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        std::vector<std::string> exp;
        exp.push_back("XX");
        exp.push_back("YY");
        REQUIRE(get_and_check(tokenizer, &input, &exp) == 0);
    }

    //  Case 14: TrimLeadingAndTrailingWhitespace (static) direct
    {
        std::string r1 = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace("  \t hello world \t ");
        REQUIRE(r1 == "hello world");
        std::string r2 = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace("   ");
        REQUIRE(r2 == "");
        std::string r3 = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace("");
        REQUIRE(r3 == "");
    }

    //  Case 15: SplitString() free function - empty delimiter (per-char)
    {
        std::vector<std::string> result = hops::SplitString("abc");
        std::vector<std::string> exp;
        exp.push_back("a");
        exp.push_back("b");
        exp.push_back("c");
        REQUIRE(check_tokens(result, exp) == 0);
    }

    //  Case 16: SplitString() free function - explicit delimiter
    {
        std::vector<std::string> result = hops::SplitString(" x , y , z ", ",");
        std::vector<std::string> exp;
        exp.push_back("x");
        exp.push_back("y");
        exp.push_back("z");
        REQUIRE(check_tokens(result, exp) == 0);
    }

    //  Case 17: Object reuse / idempotency
    // Reuse the same tokenizer instance across 3 different configurations.
    {
        // Re-run case 2 config
        std::string input2 = "This|is|a|string|separated|by|pipes.";
        tokenizer.SetDelimiter("|");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetPreserveQuotesFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
        tokenizer.SetIncludeEmptyTokensFalse();
        {
            std::vector<std::string> exp;
            exp.push_back("This");
            exp.push_back("is");
            exp.push_back("a");
            exp.push_back("string");
            exp.push_back("separated");
            exp.push_back("by");
            exp.push_back("pipes.");
            REQUIRE(get_and_check(tokenizer, &input2, &exp) == 0);
        }
        // Reconfigure for case 4 (multi-char)
        std::string input4 = "This<d>is<d>a<d>string<d>by<d>a<d>multi-character<d>delimiter.";
        tokenizer.SetDelimiter("<d>");
        tokenizer.SetUseMulticharacterDelimiterTrue();
        {
            std::vector<std::string> exp;
            exp.push_back("This");
            exp.push_back("is");
            exp.push_back("a");
            exp.push_back("string");
            exp.push_back("by");
            exp.push_back("a");
            exp.push_back("multi-character");
            exp.push_back("delimiter.");
            REQUIRE(get_and_check(tokenizer, &input4, &exp) == 0);
        }
        // Back to default-space config (case 1)
        std::string input1 = "This is a string separated by spaces.";
        tokenizer.SetDelimiter(" ");
        tokenizer.SetUseMulticharacterDelimiterFalse();
        {
            std::vector<std::string> exp;
            exp.push_back("This");
            exp.push_back("is");
            exp.push_back("a");
            exp.push_back("string");
            exp.push_back("separated");
            exp.push_back("by");
            exp.push_back("spaces.");
            REQUIRE(get_and_check(tokenizer, &input1, &exp) == 0);
        }
    }

    return 0;
}
