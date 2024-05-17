#! /usr/bin/awk -f

BEGIN { FS = "[ ]*;[ ]*"; }

{
  gsub(/#.*$/, "");
  gsub(/^ */, "");
  gsub(/ *$/, "");
}

$2 == "XID_Start" && n = split($1, u, /\.\./) {
  s = s sprintf( "\U%s%s", substr("0000000", length(u[1])), u[1]);
  if (n == 1)
    next;
  s = s sprintf("-\U%s%s", substr("0000000", length(u[2])), u[2]);
}

$2 == "XID_Continue" && n = split($1, u, /\.\./) {
  c = c sprintf( "\U%s%s", substr("0000000", length(u[1])), u[1]);
  if (n == 1)
    next;
  c = c sprintf("-\U%s%s", substr("0000000", length(u[2])), u[2]);
}

END {
  printf "XID_Start = [%s];\n\nXID_Continue = [%s];\n", s, c;
}
