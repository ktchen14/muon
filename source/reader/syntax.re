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

!include "literal.re";

// ================================ Unknown ====================================

<normal> [^] { return YYerror; }
<normal> * { return YYerror; }
