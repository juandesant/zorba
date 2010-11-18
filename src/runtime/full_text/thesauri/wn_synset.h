/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ZORBA_FULL_TEXT_WORDNET_SYNSET_H
#define ZORBA_FULL_TEXT_WORDNET_SYNSET_H

#include <stdint.h>
#include <vector>

#include "wn_types.h"

namespace zorba {
namespace wordnet {

/**
 * TODO
 */
class synset {
public:

  struct ptr {
    part_of_speech::type pos_;
    pointer::type type_;
    synset_id synset_id_;
    uint8_t source_;
    uint8_t target_;
  };

  typedef std::vector<lemma_id> lemma_id_list;
  typedef std::vector<ptr> ptr_list;

  part_of_speech::type pos_;
  lemma_id_list lemma_ids_;
  ptr_list ptr_list_;
};

} // namespace wordnet
} // namespace zorba

#endif /* ZORBA_FULL_TEXT_WORDNET_SYNSET_H */
/* vim:set et sw=2 ts=2: */