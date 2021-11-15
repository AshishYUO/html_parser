#include "include/dom_element.hpp"
#include <iostream>
dom_element::dom_element(dom_element *parent): parent(parent), is_comment(false) { }

void dom_element::__construct_innerHTML(std::string &buffop, const uint16_t depth) {
  if (!parent) {
    for (auto &x: child_nodes) {
      x->__construct_innerHTML(buffop, depth + 1);
    }
    return;
  }
  if (is_comment) {
    if (innertext != "DOCTYPE html" && innertext != "doctype html") {
      buffop += "<!--";
      buffop += innertext;
      buffop += "-->";
    } else {
      buffop += "<!";
      buffop += innertext;
      buffop += ">";
    }
    return;
  }
  if (is_text_node) {
    buffop += innertext;
    return;
  }
  buffop.push_back('<');
  buffop += tag;
  for (auto iter = attr.begin(), end_iter = attr.end(); iter != end_iter; ++iter) {
    buffop.push_back(' ');
    buffop += iter->first;
    buffop += "=\"";
    buffop += iter->second;
    buffop.push_back('"');
  }
  if (is_non_terminating) {
    buffop += " />";
    return;
  }
  buffop.push_back('>');
  for (auto &x: child_nodes) {
    x->__construct_innerHTML(buffop, depth + 1);
  }
  buffop += "</";
  buffop += tag;
  buffop.push_back('>');
}

std::string dom_element::get_attribute_value(const std::string &attribute_name) {
  const std::unordered_map<std::string, std::string>::iterator iter = attr.find(attribute_name);
  if (iter != attr.end()) {
    return iter->second;
  }
  return "";
}

// void dom_element::hierarchy() {
//   dom_element *traverse = this;
//   while (traverse != nullptr) {
//     std::cout << traverse->tag << '(' << traverse->child_nodes.size() << ')';
//     if (traverse->parent) {
//       std::cout << " -> ";
//     } else {
//       std::cout << '\n';
//     }
//     traverse = traverse->parent;
//   }
// }

dom_element* dom_element::get_element_by_id(const std::string &id) const {
  for (auto &x: children) {
    if (x->id == id) {
      // if ID found, return it immediately.
      return x;
    } else {
      // if not text element, it's evident that the
      // child_nodes exists, search within the DOM recursively.
      // and return the DOM
      dom_element *ptr = x->get_element_by_id(id);
      if (ptr) {
        // if found return.
        return ptr;
      }
    }
  }
  return nullptr;
}

std::vector<dom_element *> dom_element::get_elements_by_class_name(const std::string &classname) const {
  std::vector<dom_element *> dom;
  // printf("here\n");
  for (auto &x: children) {
    // Check if it classname exists in the list
    if (x->has_classname(classname)) {
      dom.push_back(x);
    } else {
      // unsure of keeping `else` behavior, but if not found
      // find recursively and add it to DOM list.
      auto class_list_ptr = x->get_elements_by_class_name(classname);
      for (const auto &x: class_list_ptr) {
        dom.push_back(x);
      }
    }
  }
  return dom;
}

dom_element *dom_element::delete_dom_from_document(dom_element *input) {
  if (this == input) {
    if (this->parent != nullptr) {
      return this->parent->delete_dom_from_document(input);
    }
    return this;
  }
  for (uint32_t i = 0; i < child_nodes.size(); ++i) {
    if (child_nodes[i] == input) {
      dom_element *dom_reference = child_nodes[i];
      if (!child_nodes[i]->is_text_node) {
        uint32_t idx = 4294967295U;
        for (uint32_t i = 0; i < children.size(); ++i) {
          if (children[i] == input) {
            idx = i + 1;
            break;
          }
        }
        if (idx < children.size()) { 
          for (; idx < children.size(); ++idx) {
            children[idx - 1] = children[idx];
          }
          children.pop_back();
        }
      }
      for (uint32_t j = i + 1; j < child_nodes.size(); ++j) {
        child_nodes[j - 1] = child_nodes[j];
      }
      child_nodes.pop_back();
      return dom_reference;
    }
  }
  return nullptr;
}

std::vector<dom_element *> dom_element::get_elements_by_tag_name(const std::string &tagname) const {
  std::vector<dom_element *> dom;
  for (const auto &x: children) {
    // Check if tag is same
    if (x->tag == tagname) {
      dom.push_back(x);
    }
    // unsure of this behavior, but if not found
    // find recursively and add it to DOM list.
    const auto tag_list_ptr = x->get_elements_by_tag_name(tagname);
    for (auto &x: tag_list_ptr) {
      dom.push_back(x);
    }
  }
  return dom;
}

std::string dom_element::innerText() {
  // Return if text is a node.
  if (is_text_node) {
    return innertext;
  }
  // Parse DOM list and return inner text.
  std::string value = "";
  for (const auto &x: child_nodes) {
    value += x->innerText();
  }
  return value;
}

std::string dom_element::innerHTML() {
  std::string output = "";
  __construct_innerHTML(output, 0);
  return output;
}

dom_element::~dom_element () {
  if (child_nodes.size()) {
    for (;!child_nodes.empty();) {
      delete child_nodes.back();
      child_nodes.back() = nullptr;
      child_nodes.pop_back();
    }
    for (;!children.empty();) {
      children.back() = nullptr;
      children.pop_back();
    }
    attr.clear();
    tag.clear();
    _class.clear();
    id.clear();
  }
}
