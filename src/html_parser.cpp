#include "include/html_parser.hpp"

#define DBG(_x) std::cout << _x
#define DBGLN(_x) DBG(_x) << std::endl
#define DBGSP(_x) DBGSP(_x) << ' '

#define DBGV(_x) std::cout << #_x << " is " << _x
#define DBGVLN(_x) DBGV(_x) << std::endl
#define DBGVSP(_x) DBGV(_x) << ' '
#define EXIT_IF_FILE_ENDED(read, err_message) \
  if (read == EOF) { \
    std::cerr << err_message << "\n"; \
    std::cerr << "\tat line " << line_number << ":" << character_in_a_line << '\n'; \
    exit(-1); \
  }

#define ERR_MSG(condition, err_message) \
  if (!(condition)) { \
    std::cerr << err_message << "\n"; \
    std::cerr << "\tat line " << line_number << ":" << character_in_a_line << '\n'; \
  }

dom_element* html_parser::read_file() {
  dom_element *dom = new dom_element(nullptr);
  while (read != EOF) {
    read = read_char();
    switch(read) {
      case '<':
        dom_element *child = read_tags(dom);
        if (child && child->tag.size()) {
          dom->child_nodes.push_back(child);
          dom->children.push_back(dom->child_nodes.back());
        }
        break;
    }
  }
  return dom;
}

void html_parser::read_tag_name(dom_element *dom, const char was_prev_read) {
  if (read == '!') {
    // DBGLN("Comment incoming");
    dom->tag = "#comment";
    dom->is_comment = true;
    // check if dashes exist
    bool is_comment_terminated = false;
    read = read_char();
    while (!is_comment_terminated) {
      if (read == '-') {
        // dash exist, check for another dash
        read = read_char();
        // if dash again, then comment started
        // read till two dashes and one '>' sign comes up.
        if (read == '-') {
          // this is kept as a separate loop, since we don't want
          // to start at the top loop for two more dashes
          while (!is_comment_terminated) {
            read = read_char();
            // read till dash
            while (read != EOF && read != '-') {
              dom->innertext.push_back(read);
              read = read_char();
            }
            EXIT_IF_FILE_ENDED(read, "File ended without closing comment '-->'");
            // dash exist
            read = read_char();
            // if dash again, scan for next character
            if (read != EOF && read == '-') {
              read = read_char();

              if (read != EOF && read == '>') {
                // closing comment found, stop scanning here.
                read = read_char();
                is_comment_terminated = true;
              } else {
                // part of the comment, keep scanning
                dom->innertext += "--";
              }
            } else {
              // part of the comment, keep scanning
              dom->innertext.push_back('-');
            }
          }
        } else {
          // second char is not dash, greedy approach, keep scanning till 
          // '>' is scanned
          while (read != EOF && read != '>') {
            dom->innertext.push_back(read);
            read = read_char();
          }
          EXIT_IF_FILE_ENDED(read, "File ended without closing comment '->'");
          // closing tag found, skip it
          read = read_char();
          is_comment_terminated = true;
        }
      } else {
        // first char is not dash, greedy approach, keep scanning till 
        // '>' is scanned
        while (read != EOF && read != '>') {
          dom->innertext.push_back(read);
          read = read_char();
        }
        EXIT_IF_FILE_ENDED(read, "File ended without closing comment '>'");
        // closing tag found, skip it
        read = read_char();
        is_comment_terminated = true;
      }
    }
  }
  else {
    // std::cout << " tag: " << read << " " << dom->tag << " ~~ ";
    dom->is_comment = false;
    dom->tag.clear();
    while (is_alpha_num(read) || read == '-' || read == '_' || read == ':') {
      dom->tag.push_back(char_to_lowercase(read));
      read = read_char();
    }
    if (!head_dom_hit) {
      dom->is_head = head_dom_hit = !head_dom_hit && dom->tag == "head";
    }
    if (!body_dom_hit) {
      dom->is_body = body_dom_hit = !body_dom_hit && dom->tag == "body";
    }
  }
}

void html_parser::read_attributes(dom_element *dom) {
  while (read != EOF && read != '>' && read != '/') {
    // read attr_key
    std::string key = "", value = "";
    // starts with negate sign, then boolean value
    if (read == '!') {
      value = "false";
      read = read_char();
      while (is_alpha_num(read) || read == '-' || read == '_' || read == ':') {
        key.push_back(char_to_lowercase(read));
        read = read_char();
        dom->attr.emplace(key, "false");
      }
      EXIT_IF_FILE_ENDED(read, "File end without complete tag read: " + key);
    } else {
      // value might be a string, check
      while (read != EOF && is_alpha_num(read) || read == '-' || read == '_' || read == ':') {
        key.push_back(char_to_lowercase(read));
        read = read_char();
      }
      EXIT_IF_FILE_ENDED(read, "File end without reading attribute key: " + key);
      // skip whitespaces
      skip_whitespaces();
      // if another attribute is mentioned, then value is true
      if (is_alpha_num(read) || read == '-' || read == '_' || read == ':' || read == '!') {
        // DBGLN("No value, set it as true");
        dom->attr.emplace(key, "true");
      }
      // else check if it has value
      else if (read == '='){
        // value is readable, either with or without inverted commas
        read = read_char();
        skip_whitespaces();
        auto iter = dom->attr.emplace(key, "").first;
        // DBGLN("Find value for key: " + key);
        // if inverted commas, read the value, else read till whitespace.
        if (read == '\'' || read == '"') {
          // Using double inverted commas for storing the attribute.
          // making sure that the char read are properly escaped.
          const char inv = read;
          bool valid_attribute = false;
          while (!valid_attribute) {
            read = read_char();
            while (read != EOF && read != inv && read != '"') {
              iter->second.push_back(read);
              read = read_char();
            }
            EXIT_IF_FILE_ENDED(read, "File end without attribute inverted comma close")
            // if inverted comma ends with ", we will verify whether it
            // has delimiter or not.
            if (inv == '"') {
              if (iter->second.back() != '\\') {
                // the inverted comma is not escaped, that means it ends here.
                valid_attribute = true;
              } else {
                // the character is escaped
                iter->second.push_back(inv);
              }
            } else {
              if (read == '"') {
                // This is single inverted comma.
                // We are using double inverted comma for simplicity.
                // Push delimiter for keeping the string valid.
                iter->second.push_back('\\');
              } else {
                if (iter->second.back() == '\\') {
                  // Delimiter exists
                  iter->second.pop_back();
                } else {
                  valid_attribute = true;
                }
              }
            }
          }
          // skip the inverted comma
          read = read_char();
          // DBGLN(iter->first + "=" + iter->second);
        } else {
          // std::cout << "Next char: " << read << ' ';
          while (!is_a_whitespace(read) && read != '>') {
            iter->second.push_back(read);
            read = read_char();
          }
        }
      }
    }
    // escape values - no need if value after equal is taken as exactly as mentioned.
    // attr[key] = value;
    if (key.size() == 5 && key == "class") {
      dom->_class = value;
      construct_class_list(dom, value);
    } else if(key.size() == 2 && key == "id") {
      dom->id = value;
    }
    skip_whitespaces();
  }
}

void html_parser::construct_class_list(dom_element *dom, const std::string &value) {
  uint32_t i = 0;
  const uint32_t sz = value.size();
  while (i < sz) {
    // skip whitespace.
    while (i < sz && is_a_whitespace(value[i])) { ++i; }
    dom->class_list.emplace_back("");
    std::string &classname = dom->class_list.back();
    // get class name till whitespace character occurs. 
    while (i < sz && !is_a_whitespace(value[i])) { classname.push_back(value[i++]); }
  }
}

dom_element* html_parser::read_tags(dom_element *parent_dom, const char was_prev_read) {
  // skip whitespaces
  bool valid = false;
  dom_element* dom = new dom_element(parent_dom);
  while (read != EOF && !valid) {
    if (read != EOF && read == '<') {
      read = read_char();
    }
    if (read == '/') {
      // why did it come here?
      // anyways, remove this thing from my face.
      while (read != EOF && read != '>') {
        read = read_char();
      }
      read = read_char();
    } else {
      valid = true;
      // skip whitespaces: this case is not possible, but still
      skip_whitespaces();
      // first tags
      read_tag_name(dom, was_prev_read);
      // if (is_comment) {
      //   DBGLN("comment found");
      // }
      if (dom->is_comment) {
        return dom;
      }
    }
  }
  // skip whitespaces
  skip_whitespaces();
  if (is_alpha_num(read) || read == '_' || read == '!') {
    // read the attributes
    read_attributes(dom);
  }
  // skip non-terminating read character, now that it contains '>'
  // svg tags might be different to parse now.
  if (read == '/') {
    dom->is_non_terminating = true;
    read = read_char();
  } else {
    dom->is_non_terminating = false;
  }
  // skip read character, now that it contains '>'
  // std::cout << "close >\n";
  read = read_char();
  // read innerhtml
  // DBGLN("Tag: " << dom->tag);
  dom->is_text_node = false;
  if (!dom->is_non_terminating) {
    dom->is_non_terminating = is_non_terminating_str(dom->tag);
  }
  if (dom->is_non_terminating) {
    return dom;
  }
  else if (is_pure_text_tag(dom->tag)) {
    // handle this differently
    pure_text_tag_parser(dom);
  } else {
    read_innerhtml(dom);
  }
  return dom;
}

void html_parser::read_innerhtml(dom_element *dom) {
  bool is_tag_closed = false;
  while (read != EOF && !is_tag_closed) {
    if (dom->is_head) {
      skip_whitespaces();
    }
    switch(read) {
      case '<': {
        // check if ending tag or not.
        read = read_char();
        if (read == '/') {
          // std::cout << "closing tag\n";
          // just to verify that this tag is ended
          read = read_char();
          skip_whitespaces();
          std::string tag_ends = "";
          while (is_alpha_num(read) || read == '-' || read == '_' || read == ':') {
            tag_ends.push_back(char_to_lowercase(read));
            read = read_char();
          }
          ERR_MSG(tag_ends == dom->tag, "DOM Mismatch between " + tag_ends + " and " + dom->tag + " with html " + dom->innerHTML());
          // std::cout << tag_ends << " closed with " << dom->tag << ": ";
          // for (auto &x: dom->attr) {
          //   std::cout << x.first << " " << x.second << " ";
          // }
          // std::cout << "\n";
          skip_whitespaces();
          // assert(read == '>' && tag_ends == tag);
          read = read_char();
          is_tag_closed = true;
          break;
        } else {
          // std::cout << "new tag: ";
          // Can be a tag, can be a comment as well
          if (!is_a_whitespace(read) && is_alpha_num(read) || read == '!') {
            // Adding new tag name
            // DBGLN("Adding new tag instantly");
            dom_element *d = read_tags(dom, read);
            dom->child_nodes.emplace_back(d);
            dom->children.emplace_back(d);
          }
        }
        break;
      }
      default: {
        // read the text till tag does not appear in the tag
        // std::cout << "text tag\n";
        dom_element *child_node = new dom_element(dom);
        child_node->is_text_node = true;
        dom->child_nodes.emplace_back(child_node);
        std::string &innertext_ref = dom->child_nodes.back()->innertext = "";
        while (read != '<' && read != EOF) {
          innertext_ref.push_back(read);
          read = read_char();
        }
        break;
      }
    }
  }
}

void html_parser::javascript_parser(dom_element *dom) {
  bool valid_final_tag = false;
  dom->child_nodes.push_back(new dom_element(dom));
  std::string &innertext_ref = dom->child_nodes.back()->innertext = "";
  dom->child_nodes.back()->is_text_node = true;
  while (!valid_final_tag) {
    // these characters can impact the nature of parsing the
    // html file.
    while (read != '<' && read != '\'' && read != '"' && read != '`' && read != '/') {
      innertext_ref.push_back(read);
      read = read_char();
    }
    // DBGLN("Break, found " << read);
    switch(read) {
      // It might be the case of:
      // A: a closing script tag
      // B: template, detect the closing or template
      // and check if script is closed.
      case '<': {
        std::string check_tag = "";
        // currently read is '<', skip and check if it ends
        read = read_char();
        if (read == '/') {
          // check that the ending tag is valid
          read = read_char();
          while (read != EOF && is_alpha_num(read) && read != '>') {
            check_tag.push_back(read);
            read = read_char();
          }
          skip_whitespaces();
          // if read ended and final tag is the same
          if (dom->tag == check_tag) {
            valid_final_tag = true;
            read = read_char();
          } else {
            // the tag is not the same, consider string.
            // we skipped < and / so adding those too.
            innertext_ref += "</";
            innertext_ref += check_tag;
          }
        } else {
          // else consider as a string, and read as it is
          innertext_ref.push_back('<');
        }
        break;
      }
      // ultimately the same case for '', "" && `` characters
      // the string can contain variety of character, and 
      // it might contains html tags as well, so parse them
      // differently.
      case '\'':
      case '"':
      case '`': {
        // closing case, set as true if condition satisfies.
        bool valid_string = false;
        const char string_quotes = read;
        innertext_ref.push_back(read);
        while (read != EOF && !valid_string) {
          read = read_char();
          // read till the end of quote.
          while (read != EOF && read != string_quotes) {
            innertext_ref.push_back(read);
            read = read_char();
          }
          EXIT_IF_FILE_ENDED(read, "Error in <script> reading, file end without closing inv comma")
          // if there is not a delimiter, then stop
          // otherwise continue the steps
          if (innertext_ref.back() != '\\') {
            innertext_ref.push_back(string_quotes);
            valid_string = true;
            // skip the character
            read = read_char();
          } else {
            innertext_ref.push_back(string_quotes);
          }
        }
        break;
      }
      // Comments can have different tags (if script exists, exit from it.) or quotation as well.
      // figure single line/multiline comment and proceed with the read
      case '/': {
        // comment if the previous character
        // is not a delimiter.
        if (innertext_ref.back() != '\\') {
          // skip the character.
          read = read_char();
          if (read == '/') {
            // Single line comment, parse everything till the end of the line.
            innertext_ref += "/";
            bool done = false;
            while (!done) {
              while (read != EOF && read != '\n' && read != '<') {
                innertext_ref.push_back(read);
                read = read_char();
              }
              EXIT_IF_FILE_ENDED(read, "Error: file end while reading single line comment in script");
              // add newline character, but exit from the logic.
              if (read == '<') {
                std::string check_tag = "";
                // currently read is '<', skip and check if it ends
                read = read_char();
                if (read == '/') {
                  // check that the ending tag is valid
                  read = read_char();
                  while (read != EOF && is_alpha_num(read) && read != '>') {
                    check_tag.push_back(read);
                    read = read_char();
                  }
                  skip_whitespaces();
                  // if read ended and final tag is the same
                  if (dom->tag == check_tag) {
                    done = valid_final_tag = true;
                    read = read_char();
                  } else {
                    // the tag is not the same, consider string.
                    // we skipped < and / so adding those too.
                    innertext_ref += "</";
                    innertext_ref += check_tag;
                  }
                } else {
                  // else consider as a string, and read as it is
                  innertext_ref.push_back('<');
                }
              } else {
                innertext_ref.push_back(read);
                read = read_char();
              }
            }
          } else if(read == '*') {
            // multiline comment, parse till we find */
            innertext_ref += "/*";
            read = read_char();
            bool end_of_comment = false;
            while (!end_of_comment) {
              while (read != EOF && read != '*' && read != '<') {
                innertext_ref.push_back(read);
                read = read_char();
              }
              EXIT_IF_FILE_ENDED(read, "Error: file end while reading multiline comment in script");
              if (read == '*') {
                read = read_char();
                if (read == '/') {
                  // we finally got the end of multiline comment, terminate.
                  innertext_ref += "*/";
                  read = read_char();
                  end_of_comment = true;
                  break;
                } else {
                  innertext_ref += "*";
                }
              } else if(read == '<') {
                std::string check_tag = "";
                // currently read is '<', skip and check if it ends
                read = read_char();
                if (read == '/') {
                  // check that the ending tag is valid
                  read = read_char();
                  while (read != EOF && is_alpha_num(read) && read != '>') {
                    check_tag.push_back(read);
                    read = read_char();
                  }
                  skip_whitespaces();
                  // if read ended and final tag is the same
                  if (dom->tag == check_tag) {
                    end_of_comment = valid_final_tag = true;
                    read = read_char();
                  } else {
                    // the tag is not the same, consider string.
                    // we skipped < and / so adding those too.
                    innertext_ref += "</";
                    innertext_ref += check_tag;
                  }
                }
              }
            }
          } else {
            innertext_ref.push_back('/');
          }
          break;
        } else {
          innertext_ref.push_back('/');
          read = read_char();
        }
      }
    }
  }
}

void html_parser::pure_text_tag_parser(dom_element *dom) {
  // currently, read has skipped the > sign, 
  bool valid_final_tag = false;
  if (dom->tag.size() == 6 && dom->tag == "script") {
    // parse the inner text differently
    return javascript_parser(dom);
  } else {
    dom->child_nodes.push_back(new dom_element(dom));
    std::string &innertext_ref = dom->child_nodes.back()->innertext = "";
    dom->child_nodes.back()->is_text_node = true;
    while (!valid_final_tag) {
      while (read != EOF && read != '<') {
        innertext_ref.push_back(read);
        read = read_char();
      }
      std::string check_tag = "";
      // currently read is '<', skip and check if it ends
      read = read_char();
      if (read == '/') {
        // check that the ending tag is valid
        read = read_char();
        while (is_alpha_num(read) && read != '>') {
          check_tag.push_back(read);
          read = read_char();
        }

        // if read ended and final tag is the same
        if (dom->tag == check_tag) {
          valid_final_tag = true;
          read = read_char();
        } else {
          // the tag is not the same, consider string.
          // we skipped < and / so adding those too.
          innertext_ref += "</" + check_tag;
        }
      } else {
        // else consider as a string, and read as it is
        innertext_ref.push_back('<');
      }
    }
  }
}

html_parser::html_parser(const char *path) {
  read = '\0';
  total_character = character_in_a_line = line_number = 0;
  head_dom_hit = body_dom_hit = false;
  FILE *iptr = fopen(path, "rb");
  if (!iptr) {
    printf("Error while reading file %s", path);
    perror("");
    exit(-1);
  }
  rd = new reader <FILE*>(iptr, 0);
  document = read_file();
}

dom_element *html_parser::parse_html(const char *path) {
  read = '\0';
  if (document) {
    delete document;
    document = nullptr;
  }
  if (rd) {
    delete rd;
    rd = nullptr;
  }
  total_character = character_in_a_line = line_number = 0;
  head_dom_hit = body_dom_hit = false;
  FILE *iptr = fopen(path, "rb");
  if (!iptr) {
    printf("Error while reading file %s\n", path);
    perror("");
    exit(-1);
  }
  rd = new reader <FILE*>(iptr, 0);
  return document = read_file();
}

/**
 * @brief build the set from array<string>
 * @param op array of strings
 * @returns an unordered_set of elements in array (no duplicates.)
 */
std::unordered_set<std::string> _build_st(const std::vector<std::string> &op) {
  std::unordered_set<std::string> st_;
  for (auto &x: op) {
    st_.emplace(x);
  }
  return st_;
}

const std::unordered_set<std::string> html_parser::st = 
        _build_st(std::vector<std::string>({"br", "hr", "img", "input", 
                      "area", "base", "col", "command", "keygen", 
                      "link", "meta", "param", "source", "track", "wbr"}));

const std::unordered_set<std::string> html_parser::inline_elem = 
  _build_st(std::vector<std::string>({"b", "strong", "i", "em", "u", "span",
    "sub", "sup", "bdo", "img", "button", "input", "option", "textarea", 
    "select", "abbr", "code", "script", "label","big", "small", "a", "map",
    "object", "q", "kbd", "samp", "acronym", "dfn"}));

const std::unordered_set<std::string> html_parser::p_text_tag = 
  _build_st(std::vector<std::string>({"title", "textarea", "script", 
                                      "style"}));

#undef DBG
#undef DBGLN
#undef DBGSP
#undef DBGV
#undef DBGVLN
#undef DBGVSP
#undef EXIT_IF_FILE_ENDED
#undef ERR_MSG
