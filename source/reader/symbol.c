#include "symbol.h"

#include "syntax.h"

#include <stdio.h>

void symbol_debug(
    FILE *stream,
    yytoken_kind_t kind,
    const YYSTYPE *yylval,
    const YYLTYPE *yylloc) {
  if (kind > YYEOF && kind < YYerror) {
    int c = kind;
    switch (c) {
      case '\n':
        printf("LITERAL '\\n'");
        break;
      case '\t':
        printf("LITERAL '\\t'");
        break;
      case '\r':
        printf("LITERAL '\\r'");
        break;
      default:
        printf("LITERAL '%c'", c);
        break;
    }
  } else switch (kind) {
    case YYEOF:
      printf("EOF\n");
      break;
    case YYerror:
      printf("Error\n");
      break;
    case CONSTANT:
      printf("\"constant\"");
      break;
    case INSTANCE:
      printf("\"instance\"");
      break;
    case TYPE:
      printf("\"type\"");
      break;
    case INTEGER:
      printf("INTEGER %lli", yylval->integer);
      break;
    case STRING:
      printf("STRING");
      break;
    default:
      printf("Unknown %d", kind);
      break;
  }

  printf(" <offset=%zu length=%zu line=%u:%u>\n", yylloc->offset, yylloc->length, yylloc->line, yylloc->column);
}
