--- LPeg (Parsing Expression Grammar for Lua)
lpeg = require "lpeg"

-- expand lpeg: append following patterns
-- * alnum  : alphabets + numerics
-- * alpha  : alphabets
-- * cntrl  : control characters
-- * digit  : decimal numbers
-- * graph  : displaying characters (without spaces, crlf)
-- * lower  : lower alpabets
-- * print  : displaying characters (without crlf)
-- * punct  : delimiters
-- * space  : spaces
-- * upper  : upper alphabets
-- * xdigit : hexadecimal numbers
lpeg.locale(lpeg)
lpeg.crlf = lpeg.P"\r"^-1 * lpeg.P"\n" + lpeg.P"\r"
lpeg.utf8 =   lpeg.R"\0\127"                                                            -- 1 byte char
            + lpeg.R"\194\223" * lpeg.R"\128\191"                                       -- 2 bytes char
            + lpeg.R"\224\239" * lpeg.R"\128\191" * lpeg.R"\128\191"                    -- 3 bytes char
            + lpeg.R"\224\239" * lpeg.R"\128\191" * lpeg.R"\128\191" * lpeg.R"\128\191" -- 4 bytes char

--[[
    Matching pattern for balaced parentheses
    -- capture inner string in parentheses --
    @param  head: head of parentheses    e.g. `(` etc
    @param  tail: head of parentheses    e.g. `)` etc
    @option skip: skip pattern in the parentheses    e.g. when matching `"..."`, you may want to skip `\"`
    @option head_inner: inner pattern in head of parenthes    e.g. use to match `<html>` tag, etc
    @option inner_tail: inner pattern in tail of parenthes    e.g. `</html>`
    @return captured innerstring (if you don't need, designate `/0`)
--]]
lpeg.Bp = function(head, tail, skip, head_inner, inner_tail)
    head, tail = lpeg.P(head), lpeg.P(tail)
    local inner = skip and (skip + 1 - head - tail) or (1 - head - tail)
    
    if head_inner then
        head = head * head_inner
    end
    if inner_tail then
        tail = inner_tail * tail
    end
    return lpeg.P { head * lpeg.C((inner + lpeg.V(1))^0) * tail }
end

-- Matching pattern for quotation ("...", '...', etc)
-- @option quotation: quotation mark (default: `"`)
-- @return: captured innerstring (if you don't need, designate `/0`)
lpeg.Qs = function(quotation)
    local q = quotation or '"'
    return lpeg.Bp(q, q, lpeg.P'\\' * lpeg.P(q))
end

-- Matching pattern for Lua long-string: [=*[...]=*]
-- @return: captured innerstring (if you don't need, designate `/0`)
lpeg.Ls = function()
    local equals = lpeg.P"="^0
    local open = "[" * lpeg.Cg(equals, "LuaLongStringInit") * "[" * lpeg.P"\n"^-1
    local close = "]" * lpeg.C(equals) * "]"
    local closeeq = lpeg.Cmt(close * lpeg.Cb("LuaLongStringInit"), function(s, i, a, b) return a == b end)
    return open * lpeg.C((lpeg.P(1) - closeeq)^0) * close
end

-- Pattern that match other than the beginning of the sentence
-- @param pattern: the pattern made by lpeg
lpeg.A = function(pattern)
    return lpeg.P{ pattern + 1 * lpeg.V(1) }
end

-- Function that match other than the beginning of the sentence
-- @param  pattern: the pattern made by lpeg
-- @param  subject: the target string for matching
-- @option init:    start index of subject (default: 1)
lpeg.search = function(pattern, subject, init)
    return lpeg.A(pattern):match(subject, init)
end

-- Function that splits the string
-- @param  sep: delimiters (string or lpeg pattern)
-- @param  subject: the target string for splitting
-- @option skip: skip pattern in parentheses
-- @return table of splitted strings
lpeg.split = function(sep, subject, skip)
    sep = lpeg.P(sep)
    local elem = skip and (skip + lpeg.C((1-sep)^0)) or lpeg.C((1-sep)^0)
    local p = lpeg.Ct(elem * (sep * elem)^0)
    return p:match(subject)
end

--[[
    Function that analyzes string character by character
    -- for example, use AST parser --
    @param  pattern: the pattern made by lpeg
                    - you designate next analyzing index to argument 1
                    - if you designate argument 2 (string), you can replace [mathed index] - [next analyzing index] into the designated string
                        - in this case, next analyzing index is [matched index]
                        - if you designate argument 3 (number), next analyzing index is the designated number
    @param  subject: the target string for analyzing
    @return analyzed string
]]
lpeg.parse = function(pattern, subject)
    local cur, len = 1, subject:len()
    local p = lpeg.P(pattern) + lpeg.utf8
    
    while cur <= len do
        local fin, repl, next = p:match(subject, cur)
        
        if repl then -- replace source string if the argument 2 is designated
            subject = subject:replace(cur, fin-1, repl)
            len = subject:len()
            if next then -- set next analyzing index to the designated argument 3
                cur = next
            end
        else
            cur = fin
        end
    end
    return subject
end

--[[
    -- Sample of the original script --

-- command: "\msg(message)" -> Win32.showMessageBox(message)
-- macro: "@" -> "\msg", "#" -> "Hello"
local esc = false -- escaping flag

-- Process of escape sequence
local function procESC(fin)
    if not (esc) then -- remove escape symbol when switching to escape-mode
        esc = true
        return fin, ""
    else
        esc = false -- unlock escape-mode
        return fin
    end
end

-- Process of MessageBox
local function procMSG(cur, message, fin)
    if esc then -- show MessageBox if in escape-mode
        Win32.showMessageBox(message)
        return fin, "" -- remove command string
    else
        return cur + 3 -- skip "msg" for analyzing inner string in "(...)"
    end
end

-- Process of character
local function procCHAR(c)
    esc = false -- unlock escape-mode
end

-- Grammers of this script
-- lpeg.Cp() will return current position
local grammar = (lpeg.P"\\" * lpeg.Cp() / procESC)
    + (lpeg.Cp() * "msg" * lpeg.space^0 * lpeg.Bp("(", ")") / 1 * lpeg.Cp() / procMSG)
    + (lpeg.utf8 / procCHAR)

-- Grammers for definition of macros
grammar = lpeg.P"@"*lpeg.Cp() / def(fin){ return fin, "\\msg" } -- "@" -> "\\msg"
    + lpeg.P"#"*lpeg.Cp() / def(fin){ return fin, "Hello" } -- "#" -> "Hello"
    + grammar

-- source script
local script = "#MessageBox:\\msg(Hello), NotMessageBox:\\msg(#), End:@(See, you)"

-- analyze
print(grammar:parse(script))
-- action --> Win32.showMessageBox"Hello", Win32.showMessageBox"See, you"
-- console --> "HelloMessageBox:, NotMessageBox:\msg(Hello), End:"
]]
