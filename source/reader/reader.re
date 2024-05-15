<> => normal {
  cursor->line = cursor->column = 1;
  symbol->line = symbol->column = 1;
  goto yyc_normal;
}

stop = [\x00];

<normal> "constant" { return CONSTANT; }
<normal> "instance" { return INSTANCE; }
<normal> "type" { return TYPE; }

// Whitespace
<normal> [ \t] { return ' '; }

// Newline
<normal> [\r\n]+ [ \t\r\n]* { return '\n'; }

// Integer
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

// Boolean

<normal> "true" {
  return yylval->boolean = 1, BOOLEAN;
}

<normal> "false" {
  return yylval->boolean = 0, BOOLEAN;
}

// String

<normal> "\"" :=> string

<normal> stop { break; }

<normal> [^] { return YYerror; }
<normal> * { return YYerror; }

<string> "\\\"" {
  printf("\\\"");
}

<string> * {
}

<string> "\"" :=> normal

<string> stop { return YYerror; }
