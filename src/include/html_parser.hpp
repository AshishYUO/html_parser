#ifndef  __HTML_PARSER_HPP_H_
#define  __HTML_PARSER_HPP_H_

#include "dom_element.hpp"
#include "reader.hpp"

// Minor changes, baba yaga!
class html_parser {
  char read;                                            /// read char
  dom_element *document;                                /// DOM element to return
  static const std::unordered_set<std::string> st;      /// shared st instance
  static const std::unordered_set<std::string> p_text_tag;    /// shared pure text tags instance
  static const std::unordered_set<std::string> inline_elem;   /// shared inline elem instance
  uint32_t line_number;
  uint32_t character_in_a_line;
  uint32_t total_character;
  bool head_dom_hit;
  bool body_dom_hit;
  reader <FILE *>*rd;
  
  /**
   * @brief read character, but more:
   * @returns character from file.
   */
  inline char read_char() {
    char c = rd->read_next_char();
    ++total_character;
    line_number += (c == '\n');
    character_in_a_line = ((c == '\n') ? 0 : character_in_a_line + 1);
    return c;
  }

  /**
   * @brief inner tools to check if a whitespace or not [\\s\\n\\t]
   * @param read character
   * @returns true or false
   */
  inline bool is_a_whitespace(const char read) const { return read == ' ' || read == '\n' || read == '\t'; }

  /**
   * @brief checks whether character is alphabet [a-zA-Z]
   * @param read character
   * @returns true or false
   */
  inline bool is_alpha(const char read) const { return (read >= 'a' && read <= 'z') || (read >= 'A' && read <= 'Z'); }

  /**
   * @brief checks whether character is alphanumeric [a-zA-Z0-9]
   * @param read character
   * @returns true or false
   */
  inline bool is_alpha_num(const char read) const { return is_alpha(read) || (read >= '0' && read <= '9'); }

  /**
   * @brief check if the tag is terminating string or not.
   * @param p string to check
   * @returns true if the tag is non terminating else false.
   */
  inline bool is_non_terminating_str(const std::string &p) const {
    return (st.find(p) != st.end());
  }

  /**
   * @brief convert the incoming alphabetical character to lowercase
   * @param character incoming character
   * @returns lowercase of given alphabet. for other types, it returns as is
   */
  inline char char_to_lowercase(const char character) const { return is_alpha(character) ? (character | (32)) : character; }

  /**
   * @brief check if the tags contains only texts and not dom elements.
   * @param p tag to check
   * @returns true if tag is a pure text tag, else false.
   */
  inline bool is_pure_text_tag(const std::string &p) const {
    return p_text_tag.find(p) != p_text_tag.end();
  }

  /**
   * @brief skips whitespace
   * @returns void
   */
  inline void skip_whitespaces() {
    while (read != EOF && is_a_whitespace(read)) {
      read = read_char();
    }
  }

  /**
   * @brief read the entire file
   * @returns void
   */
  dom_element* read_file();

  /**
   * @brief Read tag name: this was separated for simplicity
   * @param dom Pointer to the parent element
   * @param was_prev_read previously read character that needs to be read
   *                      by this element
   * @returns void
   */
  void read_tag_name(dom_element *dom, const char was_prev_read = '\0');

  /**
   * @brief read attributes of a tag.
   * @param dom DOM pointer to store attributes
   * @returns void
   */
  void read_attributes(dom_element *dom);

  /**
   * @brief constructs classlist for the DOM element
   * @param dom DOM element pointer to construct class list
   * @param value a string of space separated class names (if multiple exists)
   * @returns void
   */
  void construct_class_list(dom_element *dom, const std::string &value);

  /**
   * @brief read a tag if occurs.
   * @param parent_dom the parent DOM that contains this element.
   * @param was_prev_read the character that read previously by parent dom but needs 
   *                      to be included in this DOM
   * @returns element
   */
  dom_element* read_tags(dom_element *parent_dom, const char was_prev_read = '\0');

  /**
   * @brief reads inner html of the tag.
   * @param dom parent dom of the innerhtml which will be parsed.
   * @returns void
   */
  void read_innerhtml(dom_element *dom);

  /**
   * @brief Parsing specifically for javascript.
   * @param dom parent dom of the innertext which will be parsed.
   * @returns void
   */
  void javascript_parser(dom_element *dom);

  /**
   * @brief parsing tags that does not contain any tags.
   * @param dom parent dom of the innertext which will be parsed.
   * @returns void
   */
  void pure_text_tag_parser(dom_element *dom);

public:
  /**
   * @brief default constructor;
   */
  html_parser(): document(nullptr), rd(nullptr) {}

  /**
   * @brief Initialize DOM via file.
   * @param path path to an HTML file.
   */
  html_parser(const char *path);

  /**
   * @brief returns the main DOM element
   * @returns dom_element pointer.
   */
  inline dom_element *main_element() const {
    return document;
  }

  /**
   * @brief Return the parsed file
   * @param path path to an HTML file.
   * @returns a dom_element from a file
   */
  dom_element *parse_html(const char *path);

  friend std::unordered_set<std::string> _build_st(const std::vector<std::string> &op);

};

#endif
