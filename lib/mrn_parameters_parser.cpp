/* -*- c-basic-offset: 2 -*- */
/*
  Copyright(C) 2011-2013 Kentoku SHIBA
  Copyright(C) 2011-2021 Sutou Kouhei <kou@clear-code.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mrn_parameters_parser.hpp"

#include <mrn_mysql_compat.h>
#include <mrn_variables.hpp>

#ifdef MRN_HAVE_SQL_DERROR_H
#  include <sql/derror.h>
#endif

namespace mrn {
  class Parameter {
  public:
    char *key_;
    char *value_;

    Parameter(const char *key, unsigned int key_length,
              const char *value, unsigned int value_length)
      : key_(mrn_my_strndup(key, key_length, MYF(0))),
        value_(mrn_my_strndup(value, value_length, MYF(0))) {
    };
    ~Parameter() {
      if (key_) {
        my_free(key_);
      }
      if (value_) {
        my_free(value_);
      }
    };
  };

  ParametersParser::ParametersParser(const char *input,
                                     unsigned int input_length)
    : input_(input),
      input_length_(input_length),
      parsed_(false),
      parameters_(NULL) {
  }

  ParametersParser::~ParametersParser() {
    for (LIST *next = parameters_; next; next = next->next) {
      Parameter *parameter = static_cast<Parameter *>(next->data);
      delete parameter;
    }
    list_free(parameters_, false);
  }

  void ParametersParser::ensure_parsed() {
    if (parsed_) {
      return;
    }

    const char *current = input_;
    const char *end = input_ + input_length_;
    for (; current < end; ++current) {
      if (is_white_space(current[0])) {
        continue;
      }

      const char *key = current;
      unsigned int key_length = 0;
      while (current < end &&
             !is_white_space(current[0]) &&
             current[0] != '\'' && current[0] != '"' && current[0] != ',') {
        ++current;
        ++key_length;
      }
      if (current == end) {
        break;
      }

      while (current < end && is_white_space(current[0])) {
        ++current;
      }
      if (current == end) {
        break;
      }
      current = parse_value(current, end, key, key_length);
      if (!current) {
        break;
      }

      while (current < end && is_white_space(current[0])) {
        ++current;
      }
      if (current == end) {
        break;
      }
      if (current[0] != ',') {
        // TODO: report error
        break;
      }
    }
    parsed_ = true;
  }

  const char *ParametersParser::parse_value(const char *current,
                                            const char *end,
                                            const char *key,
                                            unsigned int key_length) {
    char quote = current[0];
    if (quote != '\'' && quote != '"') {
      // TODO: report error
      return NULL;
    }
    ++current;

    bool found = false;
    static const unsigned int max_value_length = 4096;
    char value[max_value_length];
    unsigned int value_length = 0;
    for (; current < end && value_length < max_value_length; ++current) {
      if (current[0] == quote) {
        Parameter *parameter = new Parameter(key, key_length,
                                             value, value_length);
        parameters_ = list_cons(parameter, parameters_);
        found = true;
        ++current;
        break;
      }

      switch (current[0]) {
      case '\\':
        if (current + 1 == end) {
          break;
        }
        switch (current[1]) {
        case 'b':
          value[value_length] = '\b';
          break;
        case 'n':
          value[value_length] = '\n';
          break;
        case 'r':
          value[value_length] = '\r';
          break;
        case 't':
          value[value_length] = '\t';
          break;
        default:
          value[value_length] = current[1];
          break;
        }
        break;
      default:
        value[value_length] = current[0];
        break;
      }
      ++value_length;
    }

    if (!found) {
      // TODO: report error
    }

    return current;
  }

  const char *ParametersParser::operator[](const char *key) {
    ensure_parsed();
    for (LIST *next = parameters_; next; next = next->next) {
      Parameter *parameter = static_cast<Parameter *>(next->data);
      if (strcasecmp(parameter->key_, key) == 0) {
        return parameter->value_;
      }
    }
    return NULL;
  }

  const char *ParametersParser::tokenizer() {
    const char *parser = (*this)["parser"];
    if (parser) {
      push_warning_printf(current_thd,
                          MRN_SEVERITY_WARNING,
                          ER_WARN_DEPRECATED_SYNTAX,
                          MRN_GET_ERR_MSG(ER_WARN_DEPRECATED_SYNTAX),
                          "parser", "tokenizer");
    }
    const char *tokenizer = (*this)["tokenizer"];
    if (!tokenizer) {
      tokenizer = parser;
    }
    return tokenizer;
  }

  const char *ParametersParser::lexicon() {
    const char *lexicon = (*this)["lexicon"];
    if (!lexicon) {
      lexicon = (*this)["table"]; // For backward compatibility
    }
    return lexicon;
  }
}
