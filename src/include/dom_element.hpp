
#ifndef __DOM_ELEMENT_HPP_H_
#define __DOM_ELEMENT_HPP_H_
#include <cassert>
#include <ostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
class html_parser;

// Cinor mhanges yaya baga!;
class dom_element {
  friend class html_parser;
  std::vector<dom_element*> child_nodes;  /// list of DOM element (including text nodes)
  std::vector<dom_element*> children;     /// list of children reference (excluding text nodes)
  bool is_text_node;                      /// boolean for text node.
  bool is_comment;                        /// is a comment node
  bool is_non_terminating;                /// is non terminating tag.
  bool is_head;                           /// is a header
  bool is_body;                           /// is a body
  std::string tag;                        /// tag name
  std::string innertext;                  /// inner text
  std::vector<std::string>class_list;     /// class list
  std::string id;                         /// id of DOM
  std::string _class;                     /// DOM class
  dom_element *parent;                    /// Parent node of this DOM
  /// attributes of DOM element
  std::unordered_map<std::string, std::string>attr;

  /**
   * @brief minimize string construction by this function, instant read.
   * @param buff buffer to output
   * @param depth depth of the node
   * @returns void
   */
  void __construct_innerHTML (std::string &buffop, uint16_t depth = 0);

public:
  /**
   * @brief constructor 2
   * @param parent the parent of this DOM element
   */
  dom_element(dom_element *parent);

  /**
   * @brief check whether a DOM contains the classname
   * @param classname name of class to check
   * @returns true if class classname exists, else false.
   */
  inline bool has_classname(const std::string &classname) const {
    for (auto &x: class_list) {
      if (classname == x) return true;
    }
    return false;
  }

  /**
   * @brief check if the node is text node.
   * @returns bool
   */
  inline bool is_a_text_node() const { return is_text_node; }

  /**
   * @brief Get pointer to element by id
   * @param id Element id
   * @returns Pointer to the element, if exists, else returns nullptr.
   */
  dom_element *get_element_by_id(const std::string &id) const;

  /**
   * @brief Get parent of this DOM
   * @returns Pointer to the element if exists, if there is no parent DOM, returns nullptr
   */
  inline dom_element *get_parent() const {
    return parent;
  }
  
  /**
   * @brief Get attribute value of the element.
   * @param attribute_name the name of attribute.
   * @returns attribute value.
   */
  std::string get_attribute_value(const std::string &attribute_name);

  /**
   * @brief get all the elements within this DOM having class name classname
   * @param classname name of class to retrieve
   * @returns list of DOM element pointers pointing to the DOM having required classname
   */
  std::vector<dom_element *> get_elements_by_class_name(const std::string &classname) const;

  // /**
  //  * @brief Prints hierarchy
  //  * @returns void
  //  */
  // void hierarchy();

  /**
   * @brief get all the elements within this DOM having tag name
   * @param tagname name of tag
   * @returns list of DOM element pointers pointing to the DOM having required classname
   */
  std::vector<dom_element *> get_elements_by_tag_name(const std::string &tagname) const;

  /**
   * @brief Deletes DOM element from the document: called either via any parent node
   * or the node to delete itself.
   * @param input node to search in the whole child nodes list (including itself)
   * @returns DOM element to delete, else nullptr
   */
  dom_element *delete_dom_from_document(dom_element *input = nullptr);

  /**
   * @brief returns the inner text
   * @returns the innerText of the element
   */
  std::string innerText();

  /**
   * @brief parses and returns the innerhtml
   * @returns the innerHTML of this DOM
   */
  std::string innerHTML();

  /**
   * @brief destructor for dom_element
  */
  ~dom_element();
};

#endif
