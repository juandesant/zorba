let $x := <msg>Beef: it's what's for dinner!</msg>
return $x contains text "meal"
  using thesaurus at "http://wordnet.princeton.edu"
  relationship "NT"

(: vim:set syntax=xquery et sw=2 ts=2: :)
