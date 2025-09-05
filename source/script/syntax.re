// Like an implicit @a ... @z around each rule. Then assign region and text.
<*> !entry := YYSTAGP(a);
<*> !pre_rule {
  if (z.offset == 0)
    YYSTAGP(z);

  *yylloc = (YYLTYPE) {
    .offset = a.offset,
    .length = z.offset - a.offset,
    .line = a.line,
    .column = a.column,
  };
}

<> => normal {
  cursor->line = cursor->column = 1;
  goto yyc_normal;
}

NL   = [\n\r]+ [ \t\n\r]*;
_    = [ \t]+;
sentinel = [\x00];

<normal> NL       { return '\n'; }
<normal> _        { return ' '; }
<normal> sentinel { break; }
<normal> "#" [ \t] [^\n\r\x00]* [\n\r]+ [ \t\n\r]* { continue; }

// ================================ Keyword ====================================

<normal> @a "case" @z    { return CASE; }
<normal> "datatype"      { return DATATYPE; }
<normal> "define"        { return DEFINE; }
<normal> "instance"      { return INSTANCE; }
<normal> "lambda" | "λ"  { return LAMBDA; }
<normal> "switch"        { return SWITCH; }

<normal> "Boolean"       { return BOOLEAN; }
<normal> "Integer"       { return INTEGER; }

// ================================ Operator ====================================

<normal> "(" @z NL?      { return '('; }
<normal> ")"             { return ')'; }
<normal> ","             { return ','; }
<normal> "->" | "→"      { return TO; }
<normal> "."             { return '.'; }
<normal> ":"             { return ':'; }
<normal> "::" | "∷"      { return CAST; }
<normal> "<:"            { return IS_SUBTYPE_OF; }
<normal> "="             { return '='; }
<normal> "[" NL?         { return '['; }
<normal> "]"             { return ']'; }
<normal> "|"             { return '|'; }

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
  memcpy(text, &buffer[a.offset], z.offset - a.offset);
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

<string> ([^] \ ("\\" | "\"" | sentinel))+ {
  yylval->text.c = &buffer[a.offset];
  yylval->text.length = z.offset - a.offset;
  return STRING;
}

<string> sentinel {
  fprintf(stderr, "Unexpected end of script\n");
  return YYerror;
}

<string> * { return YYerror; }

// ================================== Name =====================================

/* !include "unicode.re"; */

XID_Start = [A-Za-z];
XID_Continue = [A-Za-z0-9_];

<normal> XID_Start XID_Continue* {
  yylval->text.c = &buffer[a.offset];
  yylval->text.length = z.offset - a.offset;
  return NAME;
}

// ================================ Unknown ====================================

<normal> [^] {
  int length = z.offset - a.offset;
  const char *text = &buffer[a.offset];
  fprintf(stderr, "Unexpected character %.*s\n", length, text);
  return YYerror;
}

<normal> * {
  fprintf(stderr, "Unexpected character %c\n", buffer[a.offset]);
  return YYerror;
}
