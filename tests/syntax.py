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

print("\nRepl\n")