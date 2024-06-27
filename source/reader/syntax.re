<> => normal {
  cursor->line = cursor->column = 1;
  symbol->line = symbol->column = 1;
  goto yyc_normal;
}

stop = [\x00];

<normal> [ \t]              { return ' '; }
<normal> [\n\r]+ [ \t\n\r]* { return '\n'; }
<normal> stop               { break; }

// ================================ Keyword ====================================

<normal> "constant" { return CONSTANT; }
<normal> "instance" { return INSTANCE; }
<normal> "type"     { return TYPE; }
<normal> "Integer"  { return INTEGER; }

// ================================ Operator ====================================

<normal> ","                { return ','; }
<normal> "="                { return '='; }
<normal> "["                { return '['; }
<normal> "]"                { return ']'; }

// ================================ Boolean ====================================

<normal> "true" {
  return yylval->boolean = 1, BOOLEAN_LITERAL;
}

<normal> "false" {
  return yylval->boolean = 0, BOOLEAN_LITERAL;
}

// ================================ Integer ====================================

<normal> "0" {
  return yylval->integer = 0, INTEGER_LITERAL;
}

<normal> [1-9][0-9]* {
  char text[20];
  memcpy(text, &buffer[symbol->offset], cursor->offset - symbol->offset);
  text[19] = '\0';
  long long i = strtoll(text, NULL, 10);

  return yylval->integer = i, INTEGER_LITERAL;
}

// ================================= String ====================================

<normal> "\"" => string { return '"'; }
<string> "\"" => normal { return '"'; }

<string> "\\\"" {
  return yylval->text.c = UTF8("\""), yylval->text.length = 1, STRING;
}

<string> "\\t" {
  return yylval->text.c = UTF8("\t"), yylval->text.length = 1, STRING;
}

<string> "\\n" {
  return yylval->text.c = UTF8("\n"), yylval->text.length = 1, STRING;
}

<string> "\\r" {
  return yylval->text.c = UTF8("\r"), yylval->text.length = 1, STRING;
}

<string> ([^] \ ("\\" | "\"" | stop))+ {
  yylval->text.c = &buffer[symbol->offset];
  yylval->text.length = cursor->offset - symbol->offset;
  return STRING;
}

<string> stop { return YYerror; }

<string> * { return YYerror; }

// ================================== Name =====================================

/* !include "unicode.re"; */

XID_Start = [A-Za-z];
XID_Continue = [A-Za-z0-9_];

<normal> XID_Start XID_Continue* {
  yylval->text.c = &buffer[symbol->offset];
  yylval->text.length = cursor->offset - symbol->offset;
  return NAME;
}

// ================================ Unknown ====================================

<normal> [^] { return YYerror; }
<normal> * { return YYerror; }
