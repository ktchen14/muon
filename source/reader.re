<normal> "constant" { return CONSTANT; }
<normal> "instance" { return INSTANCE; }
<normal> "type" { return TYPE; }

<normal> [ \t] { return ' '; }

<normal> "0" {
  yylval.integer = 0;
  return INTEGER;
}

<normal> [1-9][0-9]* {

  return true;
}

<normal> "\"" => string {
  printf("\"");
}

<string> "\\\"" {
  printf("\\\"");
}

<string> "\"" => normal {
  printf("\"");
}

<normal> * {
  return false;
}
