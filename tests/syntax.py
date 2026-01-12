#!/usr/bin/env python3
import subprocess
import sys

def check(cmd_list, expected_line):
    try:
        output = subprocess.run(cmd_list, capture_output=True, text=True)
        result = output.stderr + output.stdout
    except Exception as e:
        result = str(e)

    if expected_line in result:
        print("PASS")
    else:
        print("FAIL")
        print(f"Expected somewhere: {expected_line}")
        print("Got output:\n" + result)
        sys.exit(1)

def check_repl(cmd_list, expected_line):
    try:
        output = subprocess.run(
            cmd_list,
            capture_output=True,
            text=True
        )
        result = output.stderr + output.stdout
    except Exception as e:
        result = str(e)

    # شوف الناتج الخام (مهم وقت debugging)
    # print("RAW OUTPUT:\n", repr(result))

    lines = result.splitlines()

    # خذ فقط الأسطر اللي:
    # - مو فاضية
    # - مو prompt
    filtered = [
        line for line in lines
        if line.strip() != "" and not line.strip().startswith(">>>")
    ]

    filtered_output = "\n".join(filtered)

    if expected_line in filtered_output:
        print("PASS")
    else:
        print(f"FAIL {cmd_list}")
        print(f"Expected somewhere: {expected_line}")
        print("Got output:\n" + filtered_output)
        sys.exit(1)
        
print("Strings\n") 
check(["build/noon", "-c", "'"], "<string>:1:1: error: unclosed char `'`")
check(["build/noon", "-c", '"'], "<string>:1:1: error: unclosed string `\"`")
check(["build/noon", "-c", '"\'"'], "")
check(["build/noon", "-c", "'\"'"], "")

print("\nComments\n") 
check(["build/noon", "-c", "1#++"], "")
check(["build/noon", "-c", "/**/"], "")
check(["build/noon", "-c", "/*\*/*/"], "")
check(["build/noon", "-c", "/*8383*/"], "")
check(["build/noon", "-c", "/*"], "<string>:1:1: error: unclosed comment `/*`")
check(["build/noon", "-c", "*/"], "<string>:1:1: error: unmatched comment `*/`")

print("\nBrackets\n") 
check(["build/noon", "-c", "("], "<string>:1:1: error: unclosed bracket `(`")
check(["build/noon", "-c", "{"], "<string>:1:1: error: unclosed curly `{`")
check(["build/noon", "-c", "["], "<string>:1:1: error: unclosed square `[`")
check(["build/noon", "-c", ")"], "<string>:1:1: error: unmatched bracket `)`")
check(["build/noon", "-c", "}"], "<string>:1:1: error: unmatched curly `}`")
check(["build/noon", "-c", "]"], "<string>:1:1: error: unmatched square `]`")
check(["build/noon", "-c", "[]"], "")
check(["build/noon", "-c", "{}"], "") 
check(["build/noon", "-c", "()"], "")

print("\nComplex Strings\n")
check(["build/noon", "-c", "\"Hello 'world'\""], "")
check(["build/noon", "-c", "'Hello \"world\"'"], "")
check(["build/noon", "-c", "\"Unclosed 'inner\""], "")
check(["build/noon", "-c", "'Unclosed \"inner'"], "")

print("\nStrings in Comments\n")
check(["build/noon", "-c", "# This is 'a comment'"], "")
check(["build/noon", "-c", "/* Comment with \"quotes\" */"], "")
check(["build/noon", "-c", "/* Unclosed 'string */"], "")

print("\nBrackets with Strings\n")
check(["build/noon", "-c", "(\"Hello\")"], "")
check(["build/noon", "-c", "{'World'}"], "")
check(["build/noon", "-c", "[\"Unclosed]"], "<string>:1:1: error: unclosed square `[`")
check(["build/noon", "-c", "(/* comment */)"], "")
check(["build/noon", "-c", "({/* nested */})"], "")

print("\nNested Complexity\n")
check(["build/noon", "-c", "({\"String\" #comment})"], "")
check(["build/noon", "-c", "([/* comment */ 'Char'])"], "")
check(["build/noon", "-c", "({/* 'inner' */})"], "")
check(["build/noon", "-c", "(\"Outer /* inner */\")"], "")

print("\nNumbers\n")
check(["build/noon", "-c", "35534444"], "")
check(["build/noon", "-c", "1.5"], "")
check(["build/noon", "-c", "1.1.1"], "<string>:1:4: error: invalid syntax `.`")
check(["build/noon", "-c", "1__1"], "<string>:1:1: error: consecutive underscore in numeric literal `1__1`")
check(["build/noon", "-c", "123_"], "<string>:1:1: error: trailing underscore in numeric literal `123_`")
check(["build/noon", "-c", "1e"], "<string>:1:1: error: invalid decimal literal")
check(["build/noon", "-c", "1e+"], "<string>:1:1: error: invalid decimal literal")
check(["build/noon", "-c", "1e_10"], "<string>:1:1: error: invalid decimal literal")
check(["build/noon", "-c", "1._1"], "<string>:1:1: error: invalid decimal literal")
check(["build/noon", "-c", ".1e"], "<string>:1:1: error: invalid decimal literal")
check(["build/noon", "-c", "(1)"], "")
check(["build/noon", "-c", "1."], "")
check(["build/noon", "-c", ".0"], "<string>:1:1: error: expected expression")
check(["build/noon", "-c", "1.0.0"], "<string>:1:4: error: invalid syntax `.`")

print("\nOperators\n")
check(["build/noon", "-c", "+1"], "")
check(["build/noon", "-c", "-1"], "")
check(["build/noon", "-c", "++1"], "")
check(["build/noon", "-c", "--1"], "")
check(["build/noon", "-c", "+"], "<string>:1:1: error: expected expression")
check(["build/noon", "-c", "+++"], "<string>:1:1: error: expected expression")
check(["build/noon", "-c", "1+"], "<string>:1:2: error: expected value after operator `+`")
check(["build/noon", "-c", "*1"], "<string>:1:1: error: expected value before operator `*`")
check(["build/noon", "-c", "1+'6'"], "<string>:1:3: error: operator `+` not supported between integer and char")
check(["build/noon", "-c", "1+\"1\""], "<string>:1:3: error: operator `+` not supported between integer and string")

print("\nNumbers\n")
 
check_repl(["build/noon", "-rp", "-c", "0"], "0");
check_repl(["build/noon", "-rp", "-c", "1"], "1");
check_repl(["build/noon", "-rp", "-c", "-1"], "-1");
check_repl(["build/noon", "-rp", "-c", "+1"], "1");
 
check_repl(["build/noon", "-rp", "-c", "1+1"], "2");
check_repl(["build/noon", "-rp", "-c", "5-3"], "2");
check_repl(["build/noon", "-rp", "-c", "4*5"], "20");
check_repl(["build/noon", "-rp", "-c", "8/4"], "2");
 
check_repl(["build/noon", "-rp", "-c", "1+2*3"], "7");
check_repl(["build/noon", "-rp", "-c", "10-2*3"], "4");
check_repl(["build/noon", "-rp", "-c", "18/3+2"], "8");
check_repl(["build/noon", "-rp", "-c", "4+6/2"], "7");
 
check_repl(["build/noon", "-rp", "-c", "(1+2)*3"], "9");
check_repl(["build/noon", "-rp", "-c", "2*(3+(4*5))"], "46");
check_repl(["build/noon", "-rp", "-c", "((2+3)*4)-5"], "15");
check_repl(["build/noon", "-rp", "-c", "(10-(2+3))*2"], "10");
 
check_repl(["build/noon", "-rp", "-c", "-5"], "-5");
check_repl(["build/noon", "-rp", "-c", "--5"], "4");
check_repl(["build/noon", "-rp", "-c", "++5"], "6");
check_repl(["build/noon", "-rp", "-c", "-(2+3)"], "-5");

check_repl(["build/noon", "-rp", "-c", "2**3"], "8");
check_repl(["build/noon", "-rp", "-c", "2**3**2"], "512");
check_repl(["build/noon", "-rp", "-c", "10%%3"], "3");
check_repl(["build/noon", "-rp", "-c", "20%%4"], "5");
 
check_repl(["build/noon", "-rp", "-c", "10%3"], "1");
check_repl(["build/noon", "-rp", "-c", "25%7"], "4");
 
check_repl(["build/noon", "-rp", "-c", "1<<3"], "8");
check_repl(["build/noon", "-rp", "-c", "8>>1"], "4");
check_repl(["build/noon", "-rp", "-c", "5&3"], "1");
check_repl(["build/noon", "-rp", "-c", "5|2"], "7");
check_repl(["build/noon", "-rp", "-c", "5^1"], "4");
check_repl(["build/noon", "-rp", "-c", "~1"], "-2");
 
check_repl(["build/noon", "-rp", "-c", "5>3"], "true");
check_repl(["build/noon", "-rp", "-c", "5<3"], "false");
check_repl(["build/noon", "-rp", "-c", "5==5"], "true");
check_repl(["build/noon", "-rp", "-c", "5!=5"], "false");
check_repl(["build/noon", "-rp", "-c", "5>=5"], "true");
check_repl(["build/noon", "-rp", "-c", "4<=3"], "false");
 
check_repl(["build/noon", "-rp", "-c", "1&&1"], "true");
check_repl(["build/noon", "-rp", "-c", "1&&0"], "false");
check_repl(["build/noon", "-rp", "-c", "0||1"], "true");
check_repl(["build/noon", "-rp", "-c", "0||0"], "false");
check_repl(["build/noon", "-rp", "-c", "!0"], "true");
check_repl(["build/noon", "-rp", "-c", "!5"], "false");
 
check_repl(["build/noon", "-rp", "-c", "5>3&&2<4"], "true");
check_repl(["build/noon", "-rp", "-c", "5>3&&2>4"], "false");
check_repl(["build/noon", "-rp", "-c", "(2+3)*4==20"], "true");
check_repl(["build/noon", "-rp", "-c", "!(5>3)"], "false");
check_repl(["build/noon", "-rp", "-c", "((1+2)*3)+(4<<1)-5"], "12");

print("\nStrings\n")
check_repl(["build/noon", "-rp", "-c", "\"Ya\"+\"sser\""], "Yasser");
check_repl(["build/noon", "-rp", "-c", "\"ha\"*3"], "hahaha");
check_repl(["build/noon", "-rp", "-c", "3*\"ha\""], "hahaha");
check_repl(["build/noon", "-rp", "-c", "(\"a\"+\"b\")*4"], "abababab");
check_repl(["build/noon", "-rp", "-c", "\"a\"==\"a\""], "true");
check_repl(["build/noon", "-rp", "-c", "\"a\"!=\"b\""], "true");
check_repl(["build/noon", "-rp", "-c", "\"ab\"*(2+3)"], "ababababab");
check_repl(["build/noon", "-rp", "-c", "(\"x\"*2)+(\"y\"*3)"], "xxyyy");
check_repl(["build/noon", "-rp", "-c", "(\"ha\"*3)==\"hahaha\""], "true");
check_repl(["build/noon", "-rp", "-c", "(\"\"*5)==\"\""], "true");