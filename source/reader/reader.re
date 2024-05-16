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

// ================================ Boolean ====================================

<normal> "true" {
  return yylval->boolean = 1, BOOLEAN;
}

<normal> "false" {
  return yylval->boolean = 0, BOOLEAN;
}

// ================================ Integer ====================================

<normal> "0" {
  return yylval->integer = 0, INTEGER;
}

<normal> [1-9][0-9]* {
  char text[20];
  memcpy(text, &buffer[symbol->offset], cursor->offset - symbol->offset);
  text[19] = '\0';
  long long i = strtoll(text, NULL, 10);

  return yylval->integer = i, INTEGER;
}

// ================================= String ====================================

<normal> "\"" => string { return '"'; }
<string> "\"" => normal { return '"'; }

<string> "\\\"" {
  return yylval->string.c = "\"", yylval->string.length = 1, STRING;
}

<string> "\\t" {
  return yylval->string.c = "\t", yylval->string.length = 1, STRING;
}

<string> "\\n" {
  return yylval->string.c = "\n", yylval->string.length = 1, STRING;
}

<string> "\\r" {
  return yylval->string.c = "\r", yylval->string.length = 1, STRING;
}

<string> ([^] \ ("\\" | "\"" | stop))+ {
  yylval->string.c = (char *) &buffer[symbol->offset];
  yylval->string.length = cursor->offset - symbol->offset;
  return STRING;
}

<string> stop { return YYerror; }

<string> * { return YYerror; }

// ================================ Unknown ====================================

<normal> [^] { return YYerror; }
<normal> * { return YYerror; }
