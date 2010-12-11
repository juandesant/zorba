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

#define DEBUG_FT_THESAURUS 1

#if DEBUG_FT_THESAURUS
#include <iostream>
#endif
#include <algorithm>                    /* for equal_range() */
#include <cassert>
#include <cstring>                      /* for strcmp(3) */
#include <limits>

#include <zorba/util/path.h>

#include "util/ascii_util.h"
#include "util/less.h"
#include "zorbaerrors/error_manager.h"

#include "decode_base128.h"
#include "wn_db_segment.h"
#include "wn_synset.h"
#include "wn_thesaurus.h"
#include "wn_types.h"

using namespace std;

namespace zorba {
namespace wordnet {

/**
 * Used to ensure the endianness of the WordNet data file matches that of the
 * CPU we're running on.
 */
uint32_t const Magic_Number = 42;       // same as TIFF -- why not?

////////// Helper functions ///////////////////////////////////////////////////

#define THROW_VERSION_EXCEPTION(FILE_VERSION,OUR_VERSION) {           \
  ostringstream oss;                                                  \
  oss << '"' << FILE_VERSION                                          \
      << "\": wrong WordNet file version; should be \""               \
      << OUR_VERSION << '"';                                          \
  ZORBA_ERROR_DESC( XQP8402_THESAURUS_VERSION_MISMATCH, oss.str() );  \
}

#define THROW_ENDIANNESS_EXCEPTION() \
  ZORBA_ERROR_DESC( XQP8403_THESAURUS_DATA_ERROR, "wrong endianness" )

/**
 * Adds two pointer "directions".
 *
 * @param di The first direction.
 * @param dj The second direction.
 * @return Returns the "sum" of the two directions.
 */
static ptr_direction_t add_ptr_directions( ptr_direction_t di,
                                           ptr_direction_t dj ) {
  //
  // Addition of two directions is defined by this table:
  //
  //   di dj result
  //   -- -- ------
  //    0  0  0
  //    0  1  1
  //    1  0  1
  //    1  1  1
  //    0 -1 -1
  //   -1  0 -1
  //   -1 -1 -1
  //    1 -1 ERROR
  //   -1  1 ERROR
  //
  if ( !(di || dj) )
    return 0;
  ptr_direction_t const dsum = di + dj;
  assert( dsum );
  return dsum < -1 ? -1 : +1;
}

/**
 * Checks whether two WordNet pointer directions are "congruous", i.e., they do
 * not point in opposite directions.
 *
 * @param di The first direction.
 * @param dj The second direction.
 * @return Returns \c true only if the two directions are congruous.
 */
inline bool congruous( ptr_direction_t di, ptr_direction_t dj ) {
  //
  // Congruity of two directions is defined by this table:
  //
  //   di dj result
  //   -- -- ------
  //    0  0  T
  //    0  1  T
  //    1  0  T
  //    1  1  T
  //    0 -1  T
  //   -1  0  T
  //   -1 -1  T
  //    1 -1  F
  //   -1  1  F
  //
  return (di || dj) ? di + dj != 0 : true;
}

/**
 * Gets a reference to a singleton instance of the WordNet thesaurus database
 * file.
 *
 * @return Returns said reference.
 */
mmap_file const& get_wordnet_file() {
  typedef char version_type[4];
  static char const our_version[] = "ZW01";

  static mmap_file wordnet_file;
  if ( !wordnet_file ) {
    zstring const &dir = thesaurus::get_directory();
    assert( !dir.empty() );

    zstring wordnet_path = dir;
    if ( !ascii::ends_with( wordnet_path,
                            filesystem_path::get_path_separator() ) )
      wordnet_path += filesystem_path::get_path_separator();
    wordnet_path += "wordnet.zth";
    wordnet_file.open( wordnet_path.c_str() );

    // check version
    char file_version[ sizeof( version_type ) + 1 ];
    char const *byte_ptr = wordnet_file.begin();
    ::strncpy( file_version, byte_ptr, sizeof( version_type ) );
    file_version[ sizeof( version_type ) ] = '\0';
    if ( ::strcmp( file_version, our_version ) != 0 )
      THROW_VERSION_EXCEPTION( file_version, our_version );

    // check endian-ness
    byte_ptr += sizeof( uint32_t );
    uint32_t const file_endian = *reinterpret_cast<uint32_t const*>( byte_ptr );
    if ( file_endian != Magic_Number )
      THROW_ENDIANNESS_EXCEPTION();
  }
  return wordnet_file;
}

/**
 * Gets a reference to a singleton instance of a WordNet thesaurus database
 * segment.
 *
 * @return Returns said segment.
 */
template<db_segment::id_t SegID>
static db_segment const& get_segment() {
  static db_segment const segment( get_wordnet_file(), SegID );
  return segment;
}

#define GET_SEGMENT(SEG_ID) get_segment<db_segment::SEG_ID>()
#define LEMMAS              GET_SEGMENT( lemma )
#define SYNSETS             GET_SEGMENT( synset )

/**
 * Attempts to find a lemma within the WordNet thesaurus.
 *
 * @param phrase The phrase to search for.
 * @return Returns said lemma or \c NULL if not found.
 */
static char const* find_lemma( zstring const &phrase ) {
  typedef pair<db_segment::const_iterator,db_segment::const_iterator>
    lemma_range;

  db_segment const &lemmas = LEMMAS;
  char const *const c_phrase = phrase.c_str();
  less<char const*> const comparator;

  lemma_range const range =
    ::equal_range( lemmas.begin(), lemmas.end(), c_phrase, comparator );
  if ( range.first == lemmas.end() || comparator( c_phrase, *range.first ) )
    return NULL;
  return *range.first;
}

/**
 * "Fixes" the "at most" parameter.  The Full Text specification section 3.4.3
 * saus in part:
 *
 *    FTThesaurusID specifies the relationship sought between tokens and
 *    phrases written in the query and terms in the thesaurus and the number of
 *    levels to be queried in hierarchical relationships by including an
 *    FTRange "levels". If no levels are specified, the default is to query all
 *    levels in hierarchical relationships.
 *
 * The problem with defaulting to "all levels" is that it makes queries too
 * broad, hence if at_most specifies "all levels" (max int), clamp it at 2
 * (which seems to work well in practice).
 *
 * For more information, see:
 * http://www.w3.org/Bugs/Public/show_bug.cgi?id=11444
 */
inline ft_int fix_at_most( ft_int at_most ) {
  return at_most == numeric_limits<ft_int>::max() ? 2 : at_most;
}

/**
 * Checks whether a WordNet synset pointer should be followed.
 *
 * @param ptr_type The pointer's type.
 * @return Returns \c true only if the pointer should be followed.
 */
static bool follow_ptr( pointer::type ptr_type ) {
  switch ( ptr_type ) {
    case pointer::antonym:
    case pointer::attribute:
    case pointer::cause:
    case pointer::derivationally_related_form:
    case pointer::derived_from_adjective:
    case pointer::domain_of_synset_region:
    case pointer::domain_of_synset_topic:
    case pointer::domain_of_synset_usage:
    case pointer::entailment:
    case pointer::member_holonym:
    case pointer::member_meronym:
    case pointer::member_of_domain_region:
    case pointer::member_of_domain_topic:
    case pointer::member_of_domain_usage:
    case pointer::part_holonym:
    case pointer::part_meronym:
    case pointer::participle_of_verb:
    case pointer::pertainym:
    case pointer::substance_holonym:
    case pointer::substance_meronym:
      //
      // These pointer types are skipped because it's thought that nobody would
      // want/expect such words as synonyms.
      //
      return false;

    case pointer::also_see:
    case pointer::hypernym:
    case pointer::hyponym:
    case pointer::instance_hypernym:
    case pointer::instance_hyponym:
    case pointer::similar_to:
    case pointer::verb_group:
      //
      // These pointer types are thought to be what most would want/expect
      // synonyms to be.
      //
      return true;

    default:
      assert( false );                  // ensures all cases are handled
  }
  return false;                         // suppesses warning -- never gets here
}

/**
 * Attempts to map an XQuery thesaurus relationship to a WordNet pointer type.
 *
 * @param relationship The XQuery thesaurus relationship.
 * @return Returns the corresponding Wordnet pointer type.
 */
static pointer::type map_xquery_rel( zstring const &relationship ) {
  zstring relationship_lower;
  ascii::to_lower( relationship, &relationship_lower );
  if ( iso2788::rel_type iso_rel = iso2788::find_rel( relationship_lower ) )
    return pointer::map_iso_rel( iso_rel );
  return pointer::find( relationship_lower );
}

/**
 * Gets the "direction" of a WordNet pointer, i.e., whether a pointer leads to
 * a more specific, neutral, or more general lemma.
 *
 * @param ptr_type The WordNet pointer type to get the direction of.
 * @return Returns -1, 0, or +1 if \a ptr_type leads to a more specific,
 * neutral, or more general lemma, respectively.
 */
static ptr_direction_t ptr_direction( pointer::type ptr_type ) {
  switch ( ptr_type ) {
    case pointer::attribute:
    case pointer::derivationally_related_form:
    case pointer::domain_of_synset_region:
    case pointer::domain_of_synset_topic:
    case pointer::domain_of_synset_usage:
    case pointer::entailment:
    case pointer::hypernym:
    case pointer::instance_hypernym:
    case pointer::member_holonym:
    case pointer::part_holonym:
    case pointer::substance_holonym:
      return +1;

    case pointer::derived_from_adjective:
    case pointer::hyponym:
    case pointer::instance_hyponym:
    case pointer::member_meronym:
    case pointer::member_of_domain_region:
    case pointer::member_of_domain_topic:
    case pointer::member_of_domain_usage:
    case pointer::part_meronym:
    case pointer::substance_meronym:
      return -1;

    case pointer::also_see:
    case pointer::antonym:
    case pointer::cause:
    case pointer::participle_of_verb:
    case pointer::pertainym:
    case pointer::similar_to:
    case pointer::unknown:
    case pointer::verb_group:
      return 0;

    default:
      assert( false );                  // ensures all cases are handled
  }
  return 0;                             // suppesses warning -- never gets here
}

#if DEBUG_FT_THESAURUS
class oseparator {
public:
  explicit oseparator( char const *sep = ", " ) : sep_( sep ), print_( false ) {
  }

  string const& sep() const {
    return sep_;
  }

  void sep( char const *s ) {
    sep_ = s;
  }

  template<class StringType>
  void sep( StringType const &s ) {
    sep_ = s;
  }

  void reset() {
    print_ = false;
  }

  friend ostream& operator<<( ostream &os, oseparator &sep ) {
    if ( sep.print_ )
      os << sep.sep_;
    else
      sep.print_ = true;
    return os;
  }

private:
  string sep_;
  bool print_;
};
#endif

///////////////////////////////////////////////////////////////////////////////

thesaurus::synset_queue::value_type const thesaurus::LevelMarker =
  make_pair( ~0u, 0 );

thesaurus::thesaurus( zstring const &phrase, zstring const &relationship,
                      ft_int at_least, ft_int at_most ) :
  query_ptr_type_( map_xquery_rel( relationship ) ),
  at_least_( at_least ), at_most_( fix_at_most( at_most ) ), level_( 0 )
{
# if DEBUG_FT_THESAURUS
  cout << "==================================================" << endl;
  cout << "query phrase: " << phrase << endl;
  cout << "query ptr_type=" << pointer::string_of[ query_ptr_type_ ] << endl;
# endif
  if ( char const *p = find_lemma( phrase ) ) {
    while ( *p++ ) ;                    // skip past lemma
    //
    // Load the synset_queue_ will all the synsets for the lemma.
    //
    for ( unsigned num_synsets = decode_base128( &p ); num_synsets-- > 0; ) {
      synset_id_t const synset_id = decode_base128( &p );
      synset_queue_.push_back( make_pair( synset_id, 0 ) );
    }
    //
    // The initial synset IDs constitute a "level", so add the sentinel to the
    // queue to increment the level.
    //
    synset_queue_.push_back( LevelMarker );
  }
}

thesaurus::~thesaurus() {
  // do nothing
}

bool thesaurus::next( zstring *synonym ) {
  while ( synonym_queue_.empty() ) {
#   if DEBUG_FT_THESAURUS
    cout << "--------------------------------------------------" << endl;
    cout << "synonym_queue is empty" << endl;
#   endif

    if ( synset_queue_.empty() ) {
#     if DEBUG_FT_THESAURUS
      cout << "synset_queue is empty --> no more synonyms" << endl;
#     endif
      return false;
    }

    synset_queue::value_type const synset_entry( pop_front( synset_queue_ ) );
    synset_id_t const synset_id = synset_entry.first;

    if ( synset_id == LevelMarker.first ) {
#     if DEBUG_FT_THESAURUS
      cout << "+ found LevelMarker" << endl;
#     endif
      if ( ++level_ > at_most_ ) {
#       if DEBUG_FT_THESAURUS
        cout << "+ level (" << level_ << ") > at_most (" << at_most_
             << ") --> no more synonyms" << endl;
#       endif
        return false;
      }

      //
      // We've just incremented the level, so all synsets that have been added
      // to the queue since the last time we were here constitute a "level",
      // therefore add the level marker so we know when to increment the level
      // next time.
      //
      // Note that we do this only if the queue isn't empty, otherwise the
      // queue would never become empty.
      //
      if ( !synset_queue_.empty() )
        synset_queue_.push_back( LevelMarker );

      if ( query_ptr_type_ == pointer::antonym ) {
        //
        // Antonyms are a special case.  When the initial batch of synset
        // pointers are followed, we obviously follow only those whose type is
        // antonym.
        //
        // After that, however, we can no longer follow antonym pointers
        // otherwise we'd get antonyms of the antonyms (i.e., synonyms of the
        // original word).  Instead, we need synonyms of the antonyms.  Hence,
        // after the initial batch of antonym synset pointers are followed, we
        // reset query_ptr_type_ to unknown.
        //
        // For example, given the word "poor", we initially follow an antonym
        // pointer to the word "rich".  After that, we don't want antonyms of
        // rich, but synonyms, e.g., "affluent", "wealthy", "well to do", etc.
        //
#       if DEBUG_FT_THESAURUS
        cout << "$ resetting query_ptr_type_ to unknown" << endl;
#       endif
        query_ptr_type_ = pointer::unknown;
      }
      continue;
    }

    synset const ss( SYNSETS[ synset_id ] );

    if ( level_ >= at_least_ ) {
#     if DEBUG_FT_THESAURUS
      cout << "+ level (" << level_ << ") >= at_least (" << at_least_ << ')'
           << endl;
#     endif
      if ( !query_ptr_type_ || level_ ) {
        //
        // The first batch of synsets in the queue are from the inital look-up
        // of the query phrase.  All the lemmas in all these synsets are
        // synonyms of the query phrase (they all have the same "sense").
        //
        // If there was no relationship specified in the query (query_ptr_type_
        // is "unknown"), then it's OK to return those lemmas as synonyms.
        //
        // However, if a relationship was specified, then it's not OK to return
        // those lemmas since the user isn't looking for synonyms, but is
        // instead looking for lemmas that have the given relationship.
        //
        // But once we're past the first (zeroth) level (the synsets of the
        // inital look-up), it's always OK to return the lemmas.
        //
        FOR_EACH( synset::lemma_id_list, lemma_id, ss.lemma_ids() ) {
#         if DEBUG_FT_THESAURUS
          cout << "? " << LEMMAS[ *lemma_id ] << " -> ";
#         endif
          if ( synonyms_seen_.insert( *lemma_id ).second ) {
            synonym_queue_.push_back( *lemma_id );
#           if DEBUG_FT_THESAURUS
            cout << "pushed";
#           endif
          }
#         if DEBUG_FT_THESAURUS
          else {
            cout << "skipped";
          }
          cout << endl;
#         endif
        }
#       if DEBUG_FT_THESAURUS
        cout << "  synonym_queue is now: ";
        oseparator comma;
        FOR_EACH( synonym_queue, lemma_id, synonym_queue_ ) {
          cout << comma << LEMMAS[ *lemma_id ];
        }
        cout << endl;
#       endif
      }
    }

    //
    // Examine, and possibly follow, this synset's pointers.
    //
    FOR_EACH( synset::ptr_list, ptr, ss.ptrs() ) {
      if ( query_ptr_type_ ) {
        //
        // A pointer type (relationship) was given for the thesaurus option: if
        // this ptr's type doesn't match, skip it.
        //
        if ( ptr->type_ != query_ptr_type_ )
          continue;
      } else {
        if ( !follow_ptr( ptr->type_ ) )
          continue;
      }

      //
      // Ensure the "direction" of the pointer that got us to this synset and
      // that of the current pointer are "congruous", i.e., they do not point
      // in opposite directions.  Once we start to follow a pointer that has a
      // direction, we want to keep going in the same direction (or at least
      // laterally) in the semantic tree.
      //
      // For example, given the word "poor" (as in "poor people"), if we
      // followed a hypernym pointer to the broader word "people", we would
      // then not want to follow a hyponym pointers to more specific words like
      // "blind" ("blind people"), "brave" ("brave people"), "clientele",
      // "rich" ("rich people"), i.e., all the different kinds of people, since
      // none of those are synonyms of "poor".
      //
      ptr_direction_t const synset_ptr_direction = synset_entry.second;
      ptr_direction_t const current_ptr_direction = ptr_direction( ptr->type_ );
      if ( !congruous( synset_ptr_direction, current_ptr_direction ) )
        continue;

#     if DEBUG_FT_THESAURUS
      cout << "+ pushing \"" << pointer::string_of[ ptr->type_ ]
           << "\" synset (ID=" << ptr->synset_id_ << ')' << endl;
      synset const ptr_ss( SYNSETS[ ptr->synset_id_ ] );
      oseparator comma;
      cout << "  lemmas: ";
      FOR_EACH( synset::lemma_id_list, lemma_id, ptr_ss.lemma_ids() ) {
        cout << comma << LEMMAS[ *lemma_id ];
      }
      cout << endl;
#     endif

      synset_queue_.push_back(
        make_pair(
          ptr->synset_id_,
          add_ptr_directions( synset_ptr_direction, current_ptr_direction )
        )
      );

#if 0
      if ( ptr->source_ ) {
        lemma_id_t const source_lemma_id = ss.lemma_ids()[ ptr->source_ - 1 ];

        synset const tt( SYNSETS[ ptr->synset_id_ ] );
        lemma_id_t const target_lemma_id = tt.lemma_ids()[ ptr->target_ - 1 ];
      }
#endif
    } // FOR_EACH
  } // while

  *synonym = LEMMAS[ pop_front( synonym_queue_ ) ];
# if DEBUG_FT_THESAURUS
  cout << "--> synonym=" << *synonym << endl;
# endif
  return true;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace wordnet
} // namespace zorba
/* vim:set et sw=2 ts=2: */
