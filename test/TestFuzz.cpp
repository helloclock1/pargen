#include <cstddef>
#include <cstdint>

#include "BNFParser.h"
#include "Entities.h"
#include "GrammarAnalyzer.h"
#include "TableBuilder.h"
#include "TestHelpers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    std::string input(reinterpret_cast<const char *>(data), size);
    try {
        GrammarParser gp(MakeStream(input));
        gp.Parse();
        Grammar g = gp.Get();

        GrammarAnalyzer ga(g);
        ParserTables tables(g, ga);
        tables.Generate();
    } catch (const GrammarParserError &e) {
        return 0;
    } catch (const TableGeneratorError &e) {
        return 0;
    }
    return 0;
}