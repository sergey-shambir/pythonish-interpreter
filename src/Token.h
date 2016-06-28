#pragma once

struct SToken
{
    unsigned line;
    unsigned column;
    union {
        // Id in CStringPool object. Always 0 for most tokens.
        unsigned stringId;
        double value;
        bool boolValue;
    };
};
