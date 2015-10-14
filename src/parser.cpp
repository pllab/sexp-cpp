// SExp - A S-Expression Parser for C++
// Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//               2015 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sstream>
#include <stdexcept>
#include <string.h>
#include <iostream>

#include "sexp/parser.hpp"

namespace sexp {

Value
Parser::from_string(std::string const& str)
{
  std::istringstream is(str);
  Parser parser(is);
  return parser.read();
}

Value
Parser::from_stream(std::istream& stream)
{
  Parser parser(stream);
  return parser.read();
}

Parser::Parser(std::istream& stream) :
  lexer(std::make_unique<Lexer>(stream)),
  token(lexer->getNextToken())
{
}

Parser::~Parser()
{
}

void
Parser::parse_error(const char* msg) const
{
  std::stringstream emsg;
  emsg << "Parse Error at line " << lexer->getLineNumber()
       << ": " << msg;
  throw std::runtime_error(emsg.str());
}

Value
Parser::read()
{
  Value result;

  switch(token)
  {
    case Lexer::TOKEN_EOF:
      parse_error("Unexpected EOF.");
      break;

    case Lexer::TOKEN_CLOSE_PAREN:
      parse_error("Unexpected ')'.");
      break;

    case Lexer::TOKEN_DOT:
      parse_error("Unexpected '.'.");
      break;

    case Lexer::TOKEN_OPEN_PAREN:
      token = lexer->getNextToken();
      if(token == Lexer::TOKEN_CLOSE_PAREN)
      {
        result = Value::nil();
      }
      else
      {
        result = Value::cons(read(), Value::nil());
        Value* cur = &result;
        while(token != Lexer::TOKEN_CLOSE_PAREN)
        {
          if (token == Lexer::TOKEN_DOT)
          {
            token = lexer->getNextToken();
            cur->set_cdr(read());
            if (token != Lexer::TOKEN_CLOSE_PAREN)
            {
              parse_error("Expected ')'");
            }
            break;
          }
          else
          {
            cur->set_cdr(Value::cons(read(), Value::nil()));
            cur = &cur->get_cdr();
          }
        }
      }
      break;

    case Lexer::TOKEN_SYMBOL:
      result = Value::symbol(lexer->getString());
      break;

    case Lexer::TOKEN_STRING:
      result = Value::string(lexer->getString());
      break;

    case Lexer::TOKEN_INTEGER:
      result = Value::integer(stoi(lexer->getString()));
      break;

    case Lexer::TOKEN_REAL:
      result = Value::real(stof(lexer->getString()));
      break;

    case Lexer::TOKEN_TRUE:
      result = Value::boolean(true);
      break;

    case Lexer::TOKEN_FALSE:
      result = Value::boolean(false);
      break;

    default:
      assert(false && "this should never happen");
      break;
  }

  token = lexer->getNextToken();
  return result;
}

} // namespace sexp

/* EOF */